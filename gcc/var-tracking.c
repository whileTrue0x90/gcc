/* Variable tracking routines for the GNU compiler.
   Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/* This file contains the variable tracking pass.  It computes where
   variables are located (which registers or where in memory) at each position
   in instruction stream and emits notes describing the locations.
   Debug information (DWARF2 location lists) is finally generated from
   these notes.
   With this debug information, it is possible to show variables
   even when debugging optimized code.

   How does the variable tracking pass work?

   First, it scans RTL code for uses, stores and clobbers (register/memory
   references in instructions), for call insns and for stack adjustments
   separately for each basic block and saves them to an array of micro
   operations.
   The micro operations of one instruction are ordered so that
   pre-modifying stack adjustment < use < use with no var < call insn <
     < set < clobber < post-modifying stack adjustment

   Then, a forward dataflow analysis is performed to find out how locations
   of variables change through code and to propagate the variable locations
   along control flow graph.
   The IN set for basic block BB is computed as a union of OUT sets of BB's
   predecessors, the OUT set for BB is copied from the IN set for BB and
   is changed according to micro operations in BB.

   The IN and OUT sets for basic blocks consist of a current stack adjustment
   (used for adjusting offset of variables addressed using stack pointer),
   the table of structures describing the locations of parts of a variable
   and for each physical register a linked list for each physical register.
   The linked list is a list of variable parts stored in the register,
   i.e. it is a list of triplets (reg, decl, offset) where decl is
   REG_EXPR (reg) and offset is REG_OFFSET (reg).  The linked list is used for
   effective deleting appropriate variable parts when we set or clobber the
   register.

   There may be more than one variable part in a register.  The linked lists
   should be pretty short so it is a good data structure here.
   For example in the following code, register allocator may assign same
   register to variables A and B, and both of them are stored in the same
   register in CODE:

     if (cond)
       set A;
     else
       set B;
     CODE;
     if (cond)
       use A;
     else
       use B;

   Finally, the NOTE_INSN_VAR_LOCATION notes describing the variable locations
   are emitted to appropriate positions in RTL code.  Each such a note describes
   the location of one variable at the point in instruction stream where the
   note is.  There is no need to emit a note for each variable before each
   instruction, we only emit these notes where the location of variable changes
   (this means that we also emit notes for changes between the OUT set of the
   previous block and the IN set of the current block).

   The notes consist of two parts:
   1. the declaration (from REG_EXPR or MEM_EXPR)
   2. the location of a variable - it is either a simple register/memory
      reference (for simple variables, for example int),
      or a parallel of register/memory references (for a large variables
      which consist of several parts, for example long long).

*/

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "tree.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "flags.h"
#include "output.h"
#include "insn-config.h"
#include "reload.h"
#include "sbitmap.h"
#include "alloc-pool.h"
#include "fibheap.h"
#include "hashtab.h"

/* Type of micro operation.  */
enum micro_operation_type
{
  MO_USE,	/* Use location (REG or MEM).  */
  MO_USE_NO_VAR,/* Use location which is not associated with a variable
		   or the variable is not trackable.  */
  MO_SET,	/* Set location.  */
  MO_CLOBBER,	/* Clobber location.  */
  MO_CALL,	/* Call insn.  */
  MO_ADJUST	/* Adjust stack pointer.  */
};

/* Where shall the note be emitted?  BEFORE or AFTER the instruction.  */
enum emit_note_where
{
  EMIT_NOTE_BEFORE_INSN,
  EMIT_NOTE_AFTER_INSN
};

/* Structure holding information about micro operation.  */
typedef struct micro_operation_def
{
  /* Type of micro operation.  */
  enum micro_operation_type type;

  union {
    /* Location.  */
    rtx loc;

    /* Stack adjustment.  */
    HOST_WIDE_INT adjust;
  } u;

  /* The instruction which the micro operation is in.  */
  rtx insn;
} micro_operation;

/* Structure for passing some other parameters to function
   emit_note_insn_var_location.  */
typedef struct emit_note_data_def
{
  /* The instruction which the note will be emitted before/after.  */
  rtx insn;

  /* Where the note will be emitted (before/after insn)?  */
  enum emit_note_where where;
} emit_note_data;

/* Description of location of a part of a variable.  The content of a physical
   register is described by a chain of these structures.
   The chains are pretty short (usually 1 or 2 elements) and thus
   chain is the best data structure.  */
typedef struct attrs_def
{
  /* Pointer to next member of the list.  */
  struct attrs_def *next;

  /* The rtx of register.  */
  rtx loc;

  /* The declaration corresponding to LOC.  */
  tree decl;

  /* Offset from start of DECL.  */
  HOST_WIDE_INT offset;
} *attrs;

/* Structure holding the IN or OUT set for a basic block.  */
typedef struct dataflow_set_def
{
  /* Adjustment of stack offset.  */
  HOST_WIDE_INT stack_adjust;

  /* Attributes for registers (lists of attrs).  */
  attrs regs[FIRST_PSEUDO_REGISTER];

  /* Variable locations.  */
  htab_t vars;
} dataflow_set;

/* The structure (one for each basic block) containing the information
   needed for variable tracking.  */
typedef struct variable_tracking_info_def
{
  /* Number of micro operations stored in the MOS array.  */
  int n_mos;

  /* The array of micro operations.  */
  micro_operation *mos;

  /* The IN and OUT set for dataflow analysis.  */
  dataflow_set in;
  dataflow_set out;

  /* Has the block been visited in DFS?  */
  bool visited;
} *variable_tracking_info;

/* Structure for chaining the locations.  */
typedef struct location_chain_def
{
  /* Next element in the chain.  */
  struct location_chain_def *next;

  /* The location (REG or MEM).  */
  rtx loc;
} *location_chain;

/* Structure describing one part of variable.  */
typedef struct variable_part_def
{
  /* Chain of locations of the part.  */
  location_chain loc_chain;

  /* Location which was last emitted to location list.  */
  rtx cur_loc;

  /* The offset in the variable.  */
  HOST_WIDE_INT offset;
} variable_part;

/* Maximum number of location parts.  */
#define MAX_VAR_PARTS 16

/* Structure describing where the variable is located.  */
typedef struct variable_def
{
  /* The declaration of the variable.  */
  tree decl;

  /* Reference count.  */
  int refcount;

  /* Number of variable parts.  */
  int n_var_parts;

  /* The variable parts.  */
  variable_part var_part[MAX_VAR_PARTS];
} *variable;

/* Hash function for DECL for VARIABLE_HTAB.  */
#define VARIABLE_HASH_VAL(decl) ((size_t) (decl))

/* Pointer to the BB's information specific to variable tracking pass.  */
#define VTI(BB) ((variable_tracking_info) (BB)->aux)

/* Alloc pool for struct attrs_def.  */
static alloc_pool attrs_pool;

/* Alloc pool for struct variable_def.  */
static alloc_pool var_pool;

/* Alloc pool for struct location_chain_def.  */
static alloc_pool loc_chain_pool;

/* Changed variables, notes will be emitted for them.  */
static htab_t changed_variables;

/* Shall notes be emitted?  */
static bool emit_notes;

/* Fake variable for stack pointer.  */
tree frame_base_decl;

/* Stack adjust caused by function prologue.  */
static HOST_WIDE_INT frame_stack_adjust;

/* Local function prototypes.  */
static void stack_adjust_offset_pre_post (rtx, HOST_WIDE_INT *,
					  HOST_WIDE_INT *);
static void insn_stack_adjust_offset_pre_post (rtx, HOST_WIDE_INT *,
					       HOST_WIDE_INT *);
static void bb_stack_adjust_offset (basic_block);
static HOST_WIDE_INT prologue_stack_adjust (void);
static bool vt_stack_adjustments (void);
static rtx adjust_stack_reference (rtx, HOST_WIDE_INT);
static hashval_t variable_htab_hash (const void *);
static int variable_htab_eq (const void *, const void *);
static void variable_htab_free (void *);

static void init_attrs_list_set (attrs *);
static void attrs_list_clear (attrs *);
static attrs attrs_list_member (attrs, tree, HOST_WIDE_INT);
static void attrs_list_insert (attrs *, tree, HOST_WIDE_INT, rtx);
static void attrs_list_copy (attrs *, attrs);
static void attrs_list_union (attrs *, attrs);

static void vars_clear (htab_t);
static variable unshare_variable (dataflow_set *set, variable var);
static int vars_copy_1 (void **, void *);
static void vars_copy (htab_t, htab_t);
static void var_reg_delete_and_set (dataflow_set *, rtx);
static void var_reg_delete (dataflow_set *, rtx);
static void var_regno_delete (dataflow_set *, int);
static void var_mem_delete_and_set (dataflow_set *, rtx);
static void var_mem_delete (dataflow_set *, rtx);

static void dataflow_set_init (dataflow_set *, int);
static void dataflow_set_clear (dataflow_set *);
static void dataflow_set_copy (dataflow_set *, dataflow_set *);
static int variable_union_info_cmp_pos (const void *, const void *);
static int variable_union (void **, void *);
static void dataflow_set_union (dataflow_set *, dataflow_set *);
static bool variable_part_different_p (variable_part *, variable_part *);
static bool variable_different_p (variable, variable, bool);
static int dataflow_set_different_1 (void **, void *);
static int dataflow_set_different_2 (void **, void *);
static bool dataflow_set_different (dataflow_set *, dataflow_set *);
static void dataflow_set_destroy (dataflow_set *);

static bool contains_symbol_ref (rtx);
static bool track_expr_p (tree);
static int count_uses (rtx *, void *);
static void count_uses_1 (rtx *, void *);
static void count_stores (rtx, rtx, void *);
static int add_uses (rtx *, void *);
static void add_uses_1 (rtx *, void *);
static void add_stores (rtx, rtx, void *);
static bool compute_bb_dataflow (basic_block);
static void vt_find_locations (void);

static void dump_attrs_list (attrs);
static int dump_variable (void **, void *);
static void dump_vars (htab_t);
static void dump_dataflow_set (dataflow_set *);
static void dump_dataflow_sets (void);

static void variable_was_changed (variable, htab_t);
static void set_frame_base_location (dataflow_set *, rtx);
static void set_variable_part (dataflow_set *, rtx, tree, HOST_WIDE_INT);
static void delete_variable_part (dataflow_set *, rtx, tree, HOST_WIDE_INT);
static int emit_note_insn_var_location (void **, void *);
static void emit_notes_for_changes (rtx, enum emit_note_where);
static int emit_notes_for_differences_1 (void **, void *);
static int emit_notes_for_differences_2 (void **, void *);
static void emit_notes_for_differences (rtx, dataflow_set *, dataflow_set *);
static void emit_notes_in_bb (basic_block);
static void vt_emit_notes (void);

static bool vt_get_decl_and_offset (rtx, tree *, HOST_WIDE_INT *);
static void vt_add_function_parameters (void);
static void vt_initialize (void);
static void vt_finalize (void);

/* Given a SET, calculate the amount of stack adjustment it contains
   PRE- and POST-modifying stack pointer.
   This function is similar to stack_adjust_offset.  */

static void
stack_adjust_offset_pre_post (rtx pattern, HOST_WIDE_INT *pre,
			      HOST_WIDE_INT *post)
{
  rtx src = SET_SRC (pattern);
  rtx dest = SET_DEST (pattern);
  enum rtx_code code;

  if (dest == stack_pointer_rtx)
    {
      /* (set (reg sp) (plus (reg sp) (const_int))) */
      code = GET_CODE (src);
      if (! (code == PLUS || code == MINUS)
	  || XEXP (src, 0) != stack_pointer_rtx
	  || GET_CODE (XEXP (src, 1)) != CONST_INT)
	return;

      if (code == MINUS)
	*post += INTVAL (XEXP (src, 1));
      else
	*post -= INTVAL (XEXP (src, 1));
    }
  else if (MEM_P (dest))
    {
      /* (set (mem (pre_dec (reg sp))) (foo)) */
      src = XEXP (dest, 0);
      code = GET_CODE (src);

      switch (code)
	{
	case PRE_MODIFY:
	case POST_MODIFY:
	  if (XEXP (src, 0) == stack_pointer_rtx)
	    {
	      rtx val = XEXP (XEXP (src, 1), 1);
	      /* We handle only adjustments by constant amount.  */
	      if (GET_CODE (XEXP (src, 1)) != PLUS ||
		  GET_CODE (val) != CONST_INT)
		abort ();
	      if (code == PRE_MODIFY)
		*pre -= INTVAL (val);
	      else
		*post -= INTVAL (val);
	      break;
	    }
	  return;

	case PRE_DEC:
	  if (XEXP (src, 0) == stack_pointer_rtx)
	    {
	      *pre += GET_MODE_SIZE (GET_MODE (dest));
	      break;
	    }
	  return;

	case POST_DEC:
	  if (XEXP (src, 0) == stack_pointer_rtx)
	    {
	      *post += GET_MODE_SIZE (GET_MODE (dest));
	      break;
	    }
	  return;

	case PRE_INC:
	  if (XEXP (src, 0) == stack_pointer_rtx)
	    {
	      *pre -= GET_MODE_SIZE (GET_MODE (dest));
	      break;
	    }
	  return;

	case POST_INC:
	  if (XEXP (src, 0) == stack_pointer_rtx)
	    {
	      *post -= GET_MODE_SIZE (GET_MODE (dest));
	      break;
	    }
	  return;

	default:
	  return;
	}
    }
}

/* Given an INSN, calculate the amount of stack adjustment it contains
   PRE- and POST-modifying stack pointer.  */

static void
insn_stack_adjust_offset_pre_post (rtx insn, HOST_WIDE_INT *pre,
				   HOST_WIDE_INT *post)
{
  *pre = 0;
  *post = 0;

  if (GET_CODE (PATTERN (insn)) == SET)
    stack_adjust_offset_pre_post (PATTERN (insn), pre, post);
  else if (GET_CODE (PATTERN (insn)) == PARALLEL
	   || GET_CODE (PATTERN (insn)) == SEQUENCE)
    {
      int i;

      /* There may be stack adjustments inside compound insns.  Search
	 for them.  */
      for ( i = XVECLEN (PATTERN (insn), 0) - 1; i >= 0; i--)
	if (GET_CODE (XVECEXP (PATTERN (insn), 0, i)) == SET)
	  stack_adjust_offset_pre_post (XVECEXP (PATTERN (insn), 0, i),
					pre, post);
    }
}

/* Compute stack adjustment in basic block BB.  */

static void
bb_stack_adjust_offset (basic_block bb)
{
  HOST_WIDE_INT offset;
  int i;

  offset = VTI (bb)->in.stack_adjust;
  for (i = 0; i < VTI (bb)->n_mos; i++)
    {
      if (VTI (bb)->mos[i].type == MO_ADJUST)
	offset += VTI (bb)->mos[i].u.adjust;
      else if (VTI (bb)->mos[i].type != MO_CALL)
	{
	  if (MEM_P (VTI (bb)->mos[i].u.loc))
	    {
	      VTI (bb)->mos[i].u.loc
		= adjust_stack_reference (VTI (bb)->mos[i].u.loc, -offset);
	    }
	}
    }
  VTI (bb)->out.stack_adjust = offset;
}

/* Compute stack adjustment caused by function prologue.  */

static HOST_WIDE_INT
prologue_stack_adjust (void)
{
  HOST_WIDE_INT offset = 0;
  basic_block bb = ENTRY_BLOCK_PTR->next_bb;
  rtx insn;
  rtx end;

  if (!BB_END (bb))
    return 0;

  end = NEXT_INSN (BB_END (bb));
  for (insn = BB_HEAD (bb); insn != end; insn = NEXT_INSN (insn))
    {
      if (NOTE_P (insn)
	  && NOTE_LINE_NUMBER (insn) == NOTE_INSN_PROLOGUE_END)
	break;

      if (INSN_P (insn))
	{
	  HOST_WIDE_INT tmp;

	  insn_stack_adjust_offset_pre_post (insn, &tmp, &tmp);
	  offset += tmp;
	}
    }

  return offset;
}

/* Compute stack adjustments for all blocks by traversing DFS tree.
   Return true when the adjustments on all incoming edges are consistent.
   Heavily borrowed from flow_depth_first_order_compute.  */

static bool
vt_stack_adjustments (void)
{
  struct edge_stack *stack;
  int sp;
  unsigned ix;

  /* Initialize entry block.  */
  VTI (ENTRY_BLOCK_PTR)->visited = true;
  VTI (ENTRY_BLOCK_PTR)->out.stack_adjust = frame_stack_adjust;

  /* Allocate stack for back-tracking up CFG.  */
  stack = xmalloc ((n_basic_blocks + 1) * sizeof (struct edge_stack));
  sp = 0;

  /* Push the first edge on to the stack.  */
  stack[sp].ev = ENTRY_BLOCK_PTR->succs;
  stack[sp++].ix = 0;

  while (sp)
    {
      VEC(edge) *ev;
      basic_block src;
      basic_block dest;

      /* Look at the edge on the top of the stack.  */
      ev = stack[sp - 1].ev;
      ix = stack[sp - 1].ix;
      src = EDGE_I (ev, ix)->src;
      dest = EDGE_I (ev, ix)->dest;

      /* Check if the edge destination has been visited yet.  */
      if (!VTI (dest)->visited)
	{
	  VTI (dest)->visited = true;
	  VTI (dest)->in.stack_adjust = VTI (src)->out.stack_adjust;
	  bb_stack_adjust_offset (dest);

	  if (EDGE_COUNT (dest->succs) > 0)
	    {
	      /* Since the DEST node has been visited for the first
		 time, check its successors.  */
	      stack[sp].ev = dest->succs;
	      stack[sp++].ix = 0;
	    }
	}
      else
	{
	  /* Check whether the adjustments on the edges are the same.  */
	  if (VTI (dest)->in.stack_adjust != VTI (src)->out.stack_adjust)
	    {
	      free (stack);
	      return false;
	    }

	  if (EDGE_COUNT (ev) > (ix + 1))
	    /* Go to the next edge.  */
	    stack[sp - 1].ix++;
	  else
	    /* Return to previous level if there are no more edges.  */
	    sp--;
	}
    }

  free (stack);
  return true;
}

/* Adjust stack reference MEM by ADJUSTMENT bytes and return the new rtx.  */

static rtx
adjust_stack_reference (rtx mem, HOST_WIDE_INT adjustment)
{
  rtx adjusted_mem;
  rtx tmp;

  if (adjustment == 0)
    return mem;

  adjusted_mem = copy_rtx (mem);
  XEXP (adjusted_mem, 0) = replace_rtx (XEXP (adjusted_mem, 0),
					stack_pointer_rtx,
					gen_rtx_PLUS (Pmode, stack_pointer_rtx,
						      GEN_INT (adjustment)));
  tmp = simplify_rtx (XEXP (adjusted_mem, 0));
  if (tmp)
    XEXP (adjusted_mem, 0) = tmp;

  return adjusted_mem;
}

/* The hash function for variable_htab, computes the hash value
   from the declaration of variable X.  */

static hashval_t
variable_htab_hash (const void *x)
{
  const variable v = (const variable) x;

  return (VARIABLE_HASH_VAL (v->decl));
}

/* Compare the declaration of variable X with declaration Y.  */

static int
variable_htab_eq (const void *x, const void *y)
{
  const variable v = (const variable) x;
  const tree decl = (const tree) y;

  return (VARIABLE_HASH_VAL (v->decl) == VARIABLE_HASH_VAL (decl));
}

/* Free the element of VARIABLE_HTAB (its type is struct variable_def).  */

static void
variable_htab_free (void *elem)
{
  int i;
  variable var = (variable) elem;
  location_chain node, next;

#ifdef ENABLE_CHECKING
  if (var->refcount <= 0)
    abort ();
#endif

  var->refcount--;
  if (var->refcount > 0)
    return;

  for (i = 0; i < var->n_var_parts; i++)
    {
      for (node = var->var_part[i].loc_chain; node; node = next)
	{
	  next = node->next;
	  pool_free (loc_chain_pool, node);
	}
      var->var_part[i].loc_chain = NULL;
    }
  pool_free (var_pool, var);
}

/* Initialize the set (array) SET of attrs to empty lists.  */

static void
init_attrs_list_set (attrs *set)
{
  int i;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    set[i] = NULL;
}

/* Make the list *LISTP empty.  */

static void
attrs_list_clear (attrs *listp)
{
  attrs list, next;

  for (list = *listp; list; list = next)
    {
      next = list->next;
      pool_free (attrs_pool, list);
    }
  *listp = NULL;
}

/* Return true if the pair of DECL and OFFSET is the member of the LIST.  */

static attrs
attrs_list_member (attrs list, tree decl, HOST_WIDE_INT offset)
{
  for (; list; list = list->next)
    if (list->decl == decl && list->offset == offset)
      return list;
  return NULL;
}

/* Insert the triplet DECL, OFFSET, LOC to the list *LISTP.  */

static void
attrs_list_insert (attrs *listp, tree decl, HOST_WIDE_INT offset, rtx loc)
{
  attrs list;

  list = pool_alloc (attrs_pool);
  list->loc = loc;
  list->decl = decl;
  list->offset = offset;
  list->next = *listp;
  *listp = list;
}

/* Copy all nodes from SRC and create a list *DSTP of the copies.  */

static void
attrs_list_copy (attrs *dstp, attrs src)
{
  attrs n;

  attrs_list_clear (dstp);
  for (; src; src = src->next)
    {
      n = pool_alloc (attrs_pool);
      n->loc = src->loc;
      n->decl = src->decl;
      n->offset = src->offset;
      n->next = *dstp;
      *dstp = n;
    }
}

/* Add all nodes from SRC which are not in *DSTP to *DSTP.  */

static void
attrs_list_union (attrs *dstp, attrs src)
{
  for (; src; src = src->next)
    {
      if (!attrs_list_member (*dstp, src->decl, src->offset))
	attrs_list_insert (dstp, src->decl, src->offset, src->loc);
    }
}

/* Delete all variables from hash table VARS.  */

static void
vars_clear (htab_t vars)
{
  htab_empty (vars);
}

/* Return a copy of a variable VAR and insert it to dataflow set SET.  */

static variable
unshare_variable (dataflow_set *set, variable var)
{
  void **slot;
  variable new_var;
  int i;

  new_var = pool_alloc (var_pool);
  new_var->decl = var->decl;
  new_var->refcount = 1;
  var->refcount--;
  new_var->n_var_parts = var->n_var_parts;

  for (i = 0; i < var->n_var_parts; i++)
    {
      location_chain node;
      location_chain *nextp;

      new_var->var_part[i].offset = var->var_part[i].offset;
      nextp = &new_var->var_part[i].loc_chain;
      for (node = var->var_part[i].loc_chain; node; node = node->next)
	{
	  location_chain new_lc;

	  new_lc = pool_alloc (loc_chain_pool);
	  new_lc->next = NULL;
	  new_lc->loc = node->loc;

	  *nextp = new_lc;
	  nextp = &new_lc->next;
	}

      /* We are at the basic block boundary when copying variable description
	 so set the CUR_LOC to be the first element of the chain.  */
      if (new_var->var_part[i].loc_chain)
	new_var->var_part[i].cur_loc = new_var->var_part[i].loc_chain->loc;
      else
	new_var->var_part[i].cur_loc = NULL;
    }

  slot = htab_find_slot_with_hash (set->vars, new_var->decl,
				   VARIABLE_HASH_VAL (new_var->decl),
				   INSERT);
  *slot = new_var;
  return new_var;
}

/* Add a variable from *SLOT to hash table DATA and increase its reference
   count.  */

static int
vars_copy_1 (void **slot, void *data)
{
  htab_t dst = (htab_t) data;
  variable src, *dstp;

  src = *(variable *) slot;
  src->refcount++;

  dstp = (variable *) htab_find_slot_with_hash (dst, src->decl,
						VARIABLE_HASH_VAL (src->decl),
						INSERT);
  *dstp = src;

  /* Continue traversing the hash table.  */
  return 1;
}

/* Copy all variables from hash table SRC to hash table DST.  */

static void
vars_copy (htab_t dst, htab_t src)
{
  vars_clear (dst);
  htab_traverse (src, vars_copy_1, dst);
}

/* Delete current content of register LOC in dataflow set SET
   and set the register to contain REG_EXPR (LOC), REG_OFFSET (LOC).  */

static void
var_reg_delete_and_set (dataflow_set *set, rtx loc)
{
  tree decl = REG_EXPR (loc);
  HOST_WIDE_INT offset = REG_OFFSET (loc);
  attrs node, next;
  attrs *nextp;

  nextp = &set->regs[REGNO (loc)];
  for (node = *nextp; node; node = next)
    {
      next = node->next;
      if (node->decl != decl || node->offset != offset)
	{
	  delete_variable_part (set, node->loc, node->decl, node->offset);
	  pool_free (attrs_pool, node);
	  *nextp = next;
	}
      else
	{
	  node->loc = loc;
	  nextp = &node->next;
	}
    }
  if (set->regs[REGNO (loc)] == NULL)
    attrs_list_insert (&set->regs[REGNO (loc)], decl, offset, loc);
  set_variable_part (set, loc, decl, offset);
}

/* Delete current content of register LOC in dataflow set SET.  */

static void
var_reg_delete (dataflow_set *set, rtx loc)
{
  attrs *reg = &set->regs[REGNO (loc)];
  attrs node, next;

  for (node = *reg; node; node = next)
    {
      next = node->next;
      delete_variable_part (set, node->loc, node->decl, node->offset);
      pool_free (attrs_pool, node);
    }
  *reg = NULL;
}

/* Delete content of register with number REGNO in dataflow set SET.  */

static void
var_regno_delete (dataflow_set *set, int regno)
{
  attrs *reg = &set->regs[regno];
  attrs node, next;

  for (node = *reg; node; node = next)
    {
      next = node->next;
      delete_variable_part (set, node->loc, node->decl, node->offset);
      pool_free (attrs_pool, node);
    }
  *reg = NULL;
}

/* Delete and set the location part of variable MEM_EXPR (LOC)
   in dataflow set SET to LOC.
   Adjust the address first if it is stack pointer based.  */

static void
var_mem_delete_and_set (dataflow_set *set, rtx loc)
{
  tree decl = MEM_EXPR (loc);
  HOST_WIDE_INT offset = MEM_OFFSET (loc) ? INTVAL (MEM_OFFSET (loc)) : 0;

  set_variable_part (set, loc, decl, offset);
}

/* Delete the location part LOC from dataflow set SET.
   Adjust the address first if it is stack pointer based.  */

static void
var_mem_delete (dataflow_set *set, rtx loc)
{
  tree decl = MEM_EXPR (loc);
  HOST_WIDE_INT offset = MEM_OFFSET (loc) ? INTVAL (MEM_OFFSET (loc)) : 0;

  delete_variable_part (set, loc, decl, offset);
}

/* Initialize dataflow set SET to be empty. 
   VARS_SIZE is the initial size of hash table VARS.  */

static void
dataflow_set_init (dataflow_set *set, int vars_size)
{
  init_attrs_list_set (set->regs);
  set->vars = htab_create (vars_size, variable_htab_hash, variable_htab_eq,
			   variable_htab_free);
  set->stack_adjust = 0;
}

/* Delete the contents of dataflow set SET.  */

static void
dataflow_set_clear (dataflow_set *set)
{
  int i;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    attrs_list_clear (&set->regs[i]);

  vars_clear (set->vars);
}

/* Copy the contents of dataflow set SRC to DST.  */

static void
dataflow_set_copy (dataflow_set *dst, dataflow_set *src)
{
  int i;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    attrs_list_copy (&dst->regs[i], src->regs[i]);

  vars_copy (dst->vars, src->vars);
  dst->stack_adjust = src->stack_adjust;
}

/* Information for merging lists of locations for a given offset of variable.
 */
struct variable_union_info
{
  /* Node of the location chain.  */
  location_chain lc;

  /* The sum of positions in the input chains.  */
  int pos;

  /* The position in the chains of SRC and DST dataflow sets.  */
  int pos_src;
  int pos_dst;
};

/* Compare function for qsort, order the structures by POS element.  */

static int
variable_union_info_cmp_pos (const void *n1, const void *n2)
{
  const struct variable_union_info *i1 = n1;
  const struct variable_union_info *i2 = n2;

  if (i1->pos != i2->pos)
    return i1->pos - i2->pos;
  
  return (i1->pos_dst - i2->pos_dst);
}

/* Compute union of location parts of variable *SLOT and the same variable
   from hash table DATA.  Compute "sorted" union of the location chains
   for common offsets, i.e. the locations of a variable part are sorted by
   a priority where the priority is the sum of the positions in the 2 chains
   (if a location is only in one list the position in the second list is
   defined to be larger than the length of the chains).
   When we are updating the location parts the newest location is in the
   beginning of the chain, so when we do the described "sorted" union
   we keep the newest locations in the beginning.  */

static int
variable_union (void **slot, void *data)
{
  variable src, dst, *dstp;
  dataflow_set *set = (dataflow_set *) data;
  int i, j, k;

  src = *(variable *) slot;
  dstp = (variable *) htab_find_slot_with_hash (set->vars, src->decl,
						VARIABLE_HASH_VAL (src->decl),
						INSERT);
  if (!*dstp)
    {
      src->refcount++;

      /* If CUR_LOC of some variable part is not the first element of
	 the location chain we are going to change it so we have to make
	 a copy of the variable.  */
      for (k = 0; k < src->n_var_parts; k++)
	{
	  if (src->var_part[k].loc_chain)
	    {
#ifdef ENABLE_CHECKING
	      if (src->var_part[k].cur_loc == NULL)
		abort ();
#endif
	      if (src->var_part[k].cur_loc != src->var_part[k].loc_chain->loc)
		break;
	    }
#ifdef ENABLE_CHECKING
	  else
	    {
	      if (src->var_part[k].cur_loc != NULL)
		abort ();
	    }
#endif
	}
      if (k < src->n_var_parts)
	unshare_variable (set, src);
      else
	*dstp = src;

      /* Continue traversing the hash table.  */
      return 1;
    }
  else
    dst = *dstp;

#ifdef ENABLE_CHECKING
  if (src->n_var_parts == 0)
    abort ();
#endif

  /* Count the number of location parts, result is K.  */
  for (i = 0, j = 0, k = 0;
       i < src->n_var_parts && j < dst->n_var_parts; k++)
    {
      if (src->var_part[i].offset == dst->var_part[j].offset)
	{
	  i++;
	  j++;
	}
      else if (src->var_part[i].offset < dst->var_part[j].offset)
	i++;
      else
	j++;
    }
  k += src->n_var_parts - i;
  k += dst->n_var_parts - j;
#ifdef ENABLE_CHECKING
  /* We track only variables whose size is <= MAX_VAR_PARTS bytes
     thus there are at most MAX_VAR_PARTS different offsets.  */
  if (k > MAX_VAR_PARTS)
    abort ();
#endif

  if (dst->refcount > 1 && dst->n_var_parts != k)
    dst = unshare_variable (set, dst);

  i = src->n_var_parts - 1;
  j = dst->n_var_parts - 1;
  dst->n_var_parts = k;

  for (k--; k >= 0; k--)
    {
      location_chain node, node2;

      if (i >= 0 && j >= 0
	  && src->var_part[i].offset == dst->var_part[j].offset)
	{
	  /* Compute the "sorted" union of the chains, i.e. the locations which
	     are in both chains go first, they are sorted by the sum of
	     positions in the chains.  */
	  int dst_l, src_l;
	  int ii, jj, n;
	  struct variable_union_info *vui;

	  /* If DST is shared compare the location chains.
	     If they are different we will modify the chain in DST with
	     high probability so make a copy of DST.  */
	  if (dst->refcount > 1)
	    {
	      for (node = src->var_part[i].loc_chain,
		   node2 = dst->var_part[j].loc_chain; node && node2;
		   node = node->next, node2 = node2->next)
		{
		  if (!((REG_P (node2->loc)
			 && REG_P (node->loc)
			 && REGNO (node2->loc) == REGNO (node->loc))
			|| rtx_equal_p (node2->loc, node->loc)))
		    break;
		}
	      if (node || node2)
		dst = unshare_variable (set, dst);
	    }

	  src_l = 0;
	  for (node = src->var_part[i].loc_chain; node; node = node->next)
	    src_l++;
	  dst_l = 0;
	  for (node = dst->var_part[j].loc_chain; node; node = node->next)
	    dst_l++;
	  vui = xcalloc (src_l + dst_l, sizeof (struct variable_union_info));

	  /* Fill in the locations from DST.  */
	  for (node = dst->var_part[j].loc_chain, jj = 0; node;
	       node = node->next, jj++)
	    {
	      vui[jj].lc = node;
	      vui[jj].pos_dst = jj;

	      /* Value larger than a sum of 2 valid positions.  */
	      vui[jj].pos_src = src_l + dst_l;
	    }

	  /* Fill in the locations from SRC.  */
	  n = dst_l;
	  for (node = src->var_part[i].loc_chain, ii = 0; node;
	       node = node->next, ii++)
	    {
	      /* Find location from NODE.  */
	      for (jj = 0; jj < dst_l; jj++)
		{
		  if ((REG_P (vui[jj].lc->loc)
		       && REG_P (node->loc)
		       && REGNO (vui[jj].lc->loc) == REGNO (node->loc))
		      || rtx_equal_p (vui[jj].lc->loc, node->loc))
		    {
		      vui[jj].pos_src = ii;
		      break;
		    }
		}
	      if (jj >= dst_l)	/* The location has not been found.  */
		{
		  location_chain new_node;

		  /* Copy the location from SRC.  */
		  new_node = pool_alloc (loc_chain_pool);
		  new_node->loc = node->loc;
		  vui[n].lc = new_node;
		  vui[n].pos_src = ii;
		  vui[n].pos_dst = src_l + dst_l;
		  n++;
		}
	    }

	  for (ii = 0; ii < src_l + dst_l; ii++)
	    vui[ii].pos = vui[ii].pos_src + vui[ii].pos_dst;

	  qsort (vui, n, sizeof (struct variable_union_info),
		 variable_union_info_cmp_pos);

	  /* Reconnect the nodes in sorted order.  */
	  for (ii = 1; ii < n; ii++)
	    vui[ii - 1].lc->next = vui[ii].lc;
	  vui[n - 1].lc->next = NULL;

	  dst->var_part[k].loc_chain = vui[0].lc;
	  dst->var_part[k].offset = dst->var_part[j].offset;

	  free (vui);
	  i--;
	  j--;
	}
      else if ((i >= 0 && j >= 0
		&& src->var_part[i].offset < dst->var_part[j].offset)
	       || i < 0)
	{
	  dst->var_part[k] = dst->var_part[j];
	  j--;
	}
      else if ((i >= 0 && j >= 0
		&& src->var_part[i].offset > dst->var_part[j].offset)
	       || j < 0)
	{
	  location_chain *nextp;

	  /* Copy the chain from SRC.  */
	  nextp = &dst->var_part[k].loc_chain;
	  for (node = src->var_part[i].loc_chain; node; node = node->next)
	    {
	      location_chain new_lc;

	      new_lc = pool_alloc (loc_chain_pool);
	      new_lc->next = NULL;
	      new_lc->loc = node->loc;

	      *nextp = new_lc;
	      nextp = &new_lc->next;
	    }

	  dst->var_part[k].offset = src->var_part[i].offset;
	  i--;
	}

      /* We are at the basic block boundary when computing union
	 so set the CUR_LOC to be the first element of the chain.  */
      if (dst->var_part[k].loc_chain)
	dst->var_part[k].cur_loc = dst->var_part[k].loc_chain->loc;
      else
	dst->var_part[k].cur_loc = NULL;
    }

  /* Continue traversing the hash table.  */
  return 1;
}

/* Compute union of dataflow sets SRC and DST and store it to DST.  */

static void
dataflow_set_union (dataflow_set *dst, dataflow_set *src)
{
  int i;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    attrs_list_union (&dst->regs[i], src->regs[i]);

  htab_traverse (src->vars, variable_union, dst);
}

/* Flag whether two dataflow sets being compared contain different data.  */
static bool
dataflow_set_different_value;

static bool
variable_part_different_p (variable_part *vp1, variable_part *vp2)
{
  location_chain lc1, lc2;

  for (lc1 = vp1->loc_chain; lc1; lc1 = lc1->next)
    {
      for (lc2 = vp2->loc_chain; lc2; lc2 = lc2->next)
	{
	  if (REG_P (lc1->loc) && REG_P (lc2->loc))
	    {
	      if (REGNO (lc1->loc) == REGNO (lc2->loc))
		break;
	    }
	  if (rtx_equal_p (lc1->loc, lc2->loc))
	    break;
	}
      if (!lc2)
	return true;
    }
  return false;
}

/* Return true if variables VAR1 and VAR2 are different.
   If COMPARE_CURRENT_LOCATION is true compare also the cur_loc of each
   variable part.  */

static bool
variable_different_p (variable var1, variable var2,
		      bool compare_current_location)
{
  int i;

  if (var1 == var2)
    return false;

  if (var1->n_var_parts != var2->n_var_parts)
    return true;

  for (i = 0; i < var1->n_var_parts; i++)
    {
      if (var1->var_part[i].offset != var2->var_part[i].offset)
	return true;
      if (compare_current_location)
	{
	  if (!((REG_P (var1->var_part[i].cur_loc)
		 && REG_P (var2->var_part[i].cur_loc)
		 && (REGNO (var1->var_part[i].cur_loc)
		     == REGNO (var2->var_part[i].cur_loc)))
		|| rtx_equal_p (var1->var_part[i].cur_loc,
				var2->var_part[i].cur_loc)))
	    return true;
	}
      if (variable_part_different_p (&var1->var_part[i], &var2->var_part[i]))
	return true;
      if (variable_part_different_p (&var2->var_part[i], &var1->var_part[i]))
	return true;
    }
  return false;
}

/* Compare variable *SLOT with the same variable in hash table DATA
   and set DATAFLOW_SET_DIFFERENT_VALUE if they are different.  */

static int
dataflow_set_different_1 (void **slot, void *data)
{
  htab_t htab = (htab_t) data;
  variable var1, var2;

  var1 = *(variable *) slot;
  var2 = htab_find_with_hash (htab, var1->decl,
			      VARIABLE_HASH_VAL (var1->decl));
  if (!var2)
    {
      dataflow_set_different_value = true;

      /* Stop traversing the hash table.  */
      return 0;
    }

  if (variable_different_p (var1, var2, false))
    {
      dataflow_set_different_value = true;

      /* Stop traversing the hash table.  */
      return 0;
    }

  /* Continue traversing the hash table.  */
  return 1;
}

/* Compare variable *SLOT with the same variable in hash table DATA
   and set DATAFLOW_SET_DIFFERENT_VALUE if they are different.  */

static int
dataflow_set_different_2 (void **slot, void *data)
{
  htab_t htab = (htab_t) data;
  variable var1, var2;

  var1 = *(variable *) slot;
  var2 = htab_find_with_hash (htab, var1->decl,
			      VARIABLE_HASH_VAL (var1->decl));
  if (!var2)
    {
      dataflow_set_different_value = true;

      /* Stop traversing the hash table.  */
      return 0;
    }

#ifdef ENABLE_CHECKING
  /* If both variables are defined they have been already checked for
     equivalence.  */
  if (variable_different_p (var1, var2, false))
    abort ();
#endif

  /* Continue traversing the hash table.  */
  return 1;
}

/* Return true if dataflow sets OLD_SET and NEW_SET differ.  */

static bool
dataflow_set_different (dataflow_set *old_set, dataflow_set *new_set)
{
  dataflow_set_different_value = false;

  htab_traverse (old_set->vars, dataflow_set_different_1, new_set->vars);
  if (!dataflow_set_different_value)
    {
      /* We have compared the variables which are in both hash tables
	 so now only check whether there are some variables in NEW_SET->VARS
	 which are not in OLD_SET->VARS.  */
      htab_traverse (new_set->vars, dataflow_set_different_2, old_set->vars);
    }
  return dataflow_set_different_value;
}

/* Free the contents of dataflow set SET.  */

static void
dataflow_set_destroy (dataflow_set *set)
{
  int i;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    attrs_list_clear (&set->regs[i]);

  htab_delete (set->vars);
  set->vars = NULL;
}

/* Return true if RTL X contains a SYMBOL_REF.  */

static bool
contains_symbol_ref (rtx x)
{
  const char *fmt;
  RTX_CODE code;
  int i;

  if (!x)
    return false;

  code = GET_CODE (x);
  if (code == SYMBOL_REF)
    return true;

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	{
	  if (contains_symbol_ref (XEXP (x, i)))
	    return true;
	}
      else if (fmt[i] == 'E')
	{
	  int j;
	  for (j = 0; j < XVECLEN (x, i); j++)
	    if (contains_symbol_ref (XVECEXP (x, i, j)))
	      return true;
	}
    }

  return false;
}

/* Shall EXPR be tracked?  */

static bool
track_expr_p (tree expr)
{
  rtx decl_rtl;

  /* If EXPR is not a parameter or a variable do not track it.  */
  if (TREE_CODE (expr) != VAR_DECL && TREE_CODE (expr) != PARM_DECL)
    return 0;

  /* It also must have a name...  */
  if (!DECL_NAME (expr))
    return 0;

  /* ... and a RTL assigned to it.  */
  decl_rtl = DECL_RTL_IF_SET (expr);
  if (!decl_rtl)
    return 0;

  /* Do not track EXPR if it should be ignored for debugging purposes.  */
  if (DECL_IGNORED_P (expr))
    return 0;

  /* Do not track global variables until we are able to emit correct location
     list for them.  */
  if (TREE_STATIC (expr))
    return 0;

  /* When the EXPR is a DECL for alias of some variable (see example)
     the TREE_STATIC flag is not used.  Disable tracking all DECLs whose
     DECL_RTL contains SYMBOL_REF.

     Example:
     extern char **_dl_argv_internal __attribute__ ((alias ("_dl_argv")));
     char **_dl_argv;
  */
  if (MEM_P (decl_rtl)
      && contains_symbol_ref (XEXP (decl_rtl, 0)))
    return 0;

  /* If RTX is a memory it should not be very large (because it would be
     an array or struct).  */
  if (MEM_P (decl_rtl))
    {
      /* Do not track structures and arrays.  */
      if (GET_MODE (decl_rtl) == BLKmode)
	return 0;
      if (MEM_SIZE (decl_rtl)
	  && INTVAL (MEM_SIZE (decl_rtl)) > MAX_VAR_PARTS)
	return 0;
    }

  return 1;
}

/* Count uses (register and memory references) LOC which will be tracked.
   INSN is instruction which the LOC is part of.  */

static int
count_uses (rtx *loc, void *insn)
{
  basic_block bb = BLOCK_FOR_INSN ((rtx) insn);

  if (REG_P (*loc))
    {
#ifdef ENABLE_CHECKING
	if (REGNO (*loc) >= FIRST_PSEUDO_REGISTER)
	  abort ();
#endif
	VTI (bb)->n_mos++;
    }
  else if (MEM_P (*loc)
	   && MEM_EXPR (*loc)
	   && track_expr_p (MEM_EXPR (*loc)))
    {
	  VTI (bb)->n_mos++;
    }

  return 0;
}

/* Helper function for finding all uses of REG/MEM in X in insn INSN.  */

static void
count_uses_1 (rtx *x, void *insn)
{
  for_each_rtx (x, count_uses, insn);
}

/* Count stores (register and memory references) LOC which will be tracked.
   INSN is instruction which the LOC is part of.  */

static void
count_stores (rtx loc, rtx expr ATTRIBUTE_UNUSED, void *insn)
{
  count_uses (&loc, insn);
}

/* Add uses (register and memory references) LOC which will be tracked
   to VTI (bb)->mos.  INSN is instruction which the LOC is part of.  */

static int
add_uses (rtx *loc, void *insn)
{
  if (REG_P (*loc))
    {
      basic_block bb = BLOCK_FOR_INSN ((rtx) insn);
      micro_operation *mo = VTI (bb)->mos + VTI (bb)->n_mos++;

      mo->type = ((REG_EXPR (*loc) && track_expr_p (REG_EXPR (*loc)))
		  ? MO_USE : MO_USE_NO_VAR);
      mo->u.loc = *loc;
      mo->insn = (rtx) insn;
    }
  else if (MEM_P (*loc)
	   && MEM_EXPR (*loc)
	   && track_expr_p (MEM_EXPR (*loc)))
    {
      basic_block bb = BLOCK_FOR_INSN ((rtx) insn);
      micro_operation *mo = VTI (bb)->mos + VTI (bb)->n_mos++;

      mo->type = MO_USE;
      mo->u.loc = *loc;
      mo->insn = (rtx) insn;
    }

  return 0;
}

/* Helper function for finding all uses of REG/MEM in X in insn INSN.  */

static void
add_uses_1 (rtx *x, void *insn)
{
  for_each_rtx (x, add_uses, insn);
}

/* Add stores (register and memory references) LOC which will be tracked
   to VTI (bb)->mos. EXPR is the RTL expression containing the store.
   INSN is instruction which the LOC is part of.  */

static void
add_stores (rtx loc, rtx expr, void *insn)
{
  if (REG_P (loc))
    {
      basic_block bb = BLOCK_FOR_INSN ((rtx) insn);
      micro_operation *mo = VTI (bb)->mos + VTI (bb)->n_mos++;

      mo->type = ((GET_CODE (expr) != CLOBBER && REG_EXPR (loc)
		   && track_expr_p (REG_EXPR (loc)))
		  ? MO_SET : MO_CLOBBER);
      mo->u.loc = loc;
      mo->insn = (rtx) insn;
    }
  else if (MEM_P (loc)
	   && MEM_EXPR (loc)
	   && track_expr_p (MEM_EXPR (loc)))
    {
      basic_block bb = BLOCK_FOR_INSN ((rtx) insn);
      micro_operation *mo = VTI (bb)->mos + VTI (bb)->n_mos++;

      mo->type = GET_CODE (expr) == CLOBBER ? MO_CLOBBER : MO_SET;
      mo->u.loc = loc;
      mo->insn = (rtx) insn;
    }
}

/* Compute the changes of variable locations in the basic block BB.  */

static bool
compute_bb_dataflow (basic_block bb)
{
  int i, n, r;
  bool changed;
  dataflow_set old_out;
  dataflow_set *in = &VTI (bb)->in;
  dataflow_set *out = &VTI (bb)->out;

  dataflow_set_init (&old_out, htab_elements (VTI (bb)->out.vars) + 3);
  dataflow_set_copy (&old_out, out);
  dataflow_set_copy (out, in);

  n = VTI (bb)->n_mos;
  for (i = 0; i < n; i++)
    {
      switch (VTI (bb)->mos[i].type)
	{
	  case MO_CALL:
	    for (r = 0; r < FIRST_PSEUDO_REGISTER; r++)
	      if (TEST_HARD_REG_BIT (call_used_reg_set, r))
		var_regno_delete (out, r);
	    break;

	  case MO_USE:
	  case MO_SET:
	    {
	      rtx loc = VTI (bb)->mos[i].u.loc;

	      if (REG_P (loc))
		var_reg_delete_and_set (out, loc);
	      else if (MEM_P (loc))
		var_mem_delete_and_set (out, loc);
	    }
	    break;

	  case MO_USE_NO_VAR:
	  case MO_CLOBBER:
	    {
	      rtx loc = VTI (bb)->mos[i].u.loc;

	      if (REG_P (loc))
		var_reg_delete (out, loc);
	      else if (MEM_P (loc))
		var_mem_delete (out, loc);
	    }
	    break;

	  case MO_ADJUST:
	    {
	      rtx base;

	      out->stack_adjust += VTI (bb)->mos[i].u.adjust;
	      base = gen_rtx_MEM (Pmode, plus_constant (stack_pointer_rtx,
							out->stack_adjust));
	      set_frame_base_location (out, base);
	    }
	    break;
	}
    }

  changed = dataflow_set_different (&old_out, out);
  dataflow_set_destroy (&old_out);
  return changed;
}

/* Find the locations of variables in the whole function.  */

static void
vt_find_locations (void)
{
  fibheap_t worklist, pending, fibheap_swap;
  sbitmap visited, in_worklist, in_pending, sbitmap_swap;
  basic_block bb;
  edge e;
  int *bb_order;
  int *rc_order;
  int i;
  unsigned ix;

  /* Compute reverse completion order of depth first search of the CFG
     so that the data-flow runs faster.  */
  rc_order = xmalloc (n_basic_blocks * sizeof (int));
  bb_order = xmalloc (last_basic_block * sizeof (int));
  flow_depth_first_order_compute (NULL, rc_order);
  for (i = 0; i < n_basic_blocks; i++)
    bb_order[rc_order[i]] = i;
  free (rc_order);

  worklist = fibheap_new ();
  pending = fibheap_new ();
  visited = sbitmap_alloc (last_basic_block);
  in_worklist = sbitmap_alloc (last_basic_block);
  in_pending = sbitmap_alloc (last_basic_block);
  sbitmap_zero (in_worklist);

  FOR_EACH_BB (bb)
    fibheap_insert (pending, bb_order[bb->index], bb);
  sbitmap_ones (in_pending);

  while (!fibheap_empty (pending))
    {
      fibheap_swap = pending;
      pending = worklist;
      worklist = fibheap_swap;
      sbitmap_swap = in_pending;
      in_pending = in_worklist;
      in_worklist = sbitmap_swap;

      sbitmap_zero (visited);

      while (!fibheap_empty (worklist))
	{
	  bb = fibheap_extract_min (worklist);
	  RESET_BIT (in_worklist, bb->index);
	  if (!TEST_BIT (visited, bb->index))
	    {
	      bool changed;

	      SET_BIT (visited, bb->index);

	      /* Calculate the IN set as union of predecessor OUT sets.  */
	      dataflow_set_clear (&VTI (bb)->in);
	      FOR_EACH_PRED_EDGE (e, bb, ix)
		{
		  dataflow_set_union (&VTI (bb)->in, &VTI (e->src)->out);
		}

	      changed = compute_bb_dataflow (bb);
	      if (changed)
		{
		  FOR_EACH_SUCC_EDGE (e, bb, ix)
		    {
		      if (e->dest == EXIT_BLOCK_PTR)
			continue;

		      if (e->dest == bb)
			continue;

		      if (TEST_BIT (visited, e->dest->index))
			{
			  if (!TEST_BIT (in_pending, e->dest->index))
			    {
			      /* Send E->DEST to next round.  */
			      SET_BIT (in_pending, e->dest->index);
			      fibheap_insert (pending,
					      bb_order[e->dest->index],
					      e->dest);
			    }
			}
		      else if (!TEST_BIT (in_worklist, e->dest->index))
			{
			  /* Add E->DEST to current round.  */
			  SET_BIT (in_worklist, e->dest->index);
			  fibheap_insert (worklist, bb_order[e->dest->index],
					  e->dest);
			}
		    }
		}
	    }
	}
    }

  free (bb_order);
  fibheap_delete (worklist);
  fibheap_delete (pending);
  sbitmap_free (visited);
  sbitmap_free (in_worklist);
  sbitmap_free (in_pending);
}

/* Print the content of the LIST to dump file.  */

static void
dump_attrs_list (attrs list)
{
  for (; list; list = list->next)
    {
      print_mem_expr (dump_file, list->decl);
      fprintf (dump_file, "+");
      fprintf (dump_file, HOST_WIDE_INT_PRINT_DEC, list->offset);
    }
  fprintf (dump_file, "\n");
}

/* Print the information about variable *SLOT to dump file.  */

static int
dump_variable (void **slot, void *data ATTRIBUTE_UNUSED)
{
  variable var = *(variable *) slot;
  int i;
  location_chain node;

  fprintf (dump_file, "  name: %s\n",
	   IDENTIFIER_POINTER (DECL_NAME (var->decl)));
  for (i = 0; i < var->n_var_parts; i++)
    {
      fprintf (dump_file, "    offset %ld\n",
	       (long) var->var_part[i].offset);
      for (node = var->var_part[i].loc_chain; node; node = node->next)
	{
	  fprintf (dump_file, "      ");
	  print_rtl_single (dump_file, node->loc);
	}
    }

  /* Continue traversing the hash table.  */
  return 1;
}

/* Print the information about variables from hash table VARS to dump file.  */

static void
dump_vars (htab_t vars)
{
  if (htab_elements (vars) > 0)
    {
      fprintf (dump_file, "Variables:\n");
      htab_traverse (vars, dump_variable, NULL);
    }
}

/* Print the dataflow set SET to dump file.  */

static void
dump_dataflow_set (dataflow_set *set)
{
  int i;

  fprintf (dump_file, "Stack adjustment: ");
  fprintf (dump_file, HOST_WIDE_INT_PRINT_DEC, set->stack_adjust);
  fprintf (dump_file, "\n");
  for (i = 1; i < FIRST_PSEUDO_REGISTER; i++)
    {
      if (set->regs[i])
	{
	  fprintf (dump_file, "Reg %d:", i);
	  dump_attrs_list (set->regs[i]);
	}
    }
  dump_vars (set->vars);
  fprintf (dump_file, "\n");
}

/* Print the IN and OUT sets for each basic block to dump file.  */

static void
dump_dataflow_sets (void)
{
  basic_block bb;

  FOR_EACH_BB (bb)
    {
      fprintf (dump_file, "\nBasic block %d:\n", bb->index);
      fprintf (dump_file, "IN:\n");
      dump_dataflow_set (&VTI (bb)->in);
      fprintf (dump_file, "OUT:\n");
      dump_dataflow_set (&VTI (bb)->out);
    }
}

/* Add variable VAR to the hash table of changed variables and
   if it has no locations delete it from hash table HTAB.  */

static void
variable_was_changed (variable var, htab_t htab)
{
  hashval_t hash = VARIABLE_HASH_VAL (var->decl);

  if (emit_notes)
    {
      variable *slot;

      slot = (variable *) htab_find_slot_with_hash (changed_variables,
						    var->decl, hash, INSERT);

      if (htab && var->n_var_parts == 0)
	{
	  variable empty_var;
	  void **old;

	  empty_var = pool_alloc (var_pool);
	  empty_var->decl = var->decl;
	  empty_var->refcount = 1;
	  empty_var->n_var_parts = 0;
	  *slot = empty_var;

	  old = htab_find_slot_with_hash (htab, var->decl, hash,
					  NO_INSERT);
	  if (old)
	    htab_clear_slot (htab, old);
	}
      else
	{
	  *slot = var;
	}
    }
  else
    {
#ifdef ENABLE_CHECKING
      if (!htab)
	abort ();
#endif
      if (var->n_var_parts == 0)
	{
	  void **slot = htab_find_slot_with_hash (htab, var->decl, hash,
						  NO_INSERT);
	  if (slot)
	    htab_clear_slot (htab, slot);
	}
    }
}

/* Set the location of frame_base_decl to LOC in dataflow set SET.  This
   function expects that frame_base_decl has already one location for offset 0
   in the variable table.  */

static void
set_frame_base_location (dataflow_set *set, rtx loc)
{
  variable var;
  
  var = htab_find_with_hash (set->vars, frame_base_decl,
			     VARIABLE_HASH_VAL (frame_base_decl));
#ifdef ENABLE_CHECKING
  if (!var)
    abort ();
  if (var->n_var_parts != 1)
    abort ();
  if (var->var_part[0].offset != 0)
    abort ();
  if (!var->var_part[0].loc_chain)
    abort ();
#endif

  /* If frame_base_decl is shared unshare it first.  */
  if (var->refcount > 1)
    var = unshare_variable (set, var);

  var->var_part[0].loc_chain->loc = loc;
  var->var_part[0].cur_loc = loc;
  variable_was_changed (var, set->vars);
}

/* Set the part of variable's location in the dataflow set SET.  The variable
   part is specified by variable's declaration DECL and offset OFFSET and the
   part's location by LOC.  */

static void
set_variable_part (dataflow_set *set, rtx loc, tree decl, HOST_WIDE_INT offset)
{
  int pos, low, high;
  location_chain node, next;
  location_chain *nextp;
  variable var;
  void **slot;
  
  slot = htab_find_slot_with_hash (set->vars, decl,
				   VARIABLE_HASH_VAL (decl), INSERT);
  if (!*slot)
    {
      /* Create new variable information.  */
      var = pool_alloc (var_pool);
      var->decl = decl;
      var->refcount = 1;
      var->n_var_parts = 1;
      var->var_part[0].offset = offset;
      var->var_part[0].loc_chain = NULL;
      var->var_part[0].cur_loc = NULL;
      *slot = var;
      pos = 0;
    }
  else
    {
      var = (variable) *slot;

      /* Find the location part.  */
      low = 0;
      high = var->n_var_parts;
      while (low != high)
	{
	  pos = (low + high) / 2;
	  if (var->var_part[pos].offset < offset)
	    low = pos + 1;
	  else
	    high = pos;
	}
      pos = low;

      if (pos < var->n_var_parts && var->var_part[pos].offset == offset)
	{
	  node = var->var_part[pos].loc_chain;

	  if (node
	      && ((REG_P (node->loc) && REG_P (loc)
		   && REGNO (node->loc) == REGNO (loc))
		  || rtx_equal_p (node->loc, loc)))
	    {
	      /* LOC is in the beginning of the chain so we have nothing
		 to do.  */
	      return;
	    }
	  else
	    {
	      /* We have to make a copy of a shared variable.  */
	      if (var->refcount > 1)
		var = unshare_variable (set, var);
	    }
	}
      else
	{
	  /* We have not found the location part, new one will be created.  */

	  /* We have to make a copy of the shared variable.  */
	  if (var->refcount > 1)
	    var = unshare_variable (set, var);

#ifdef ENABLE_CHECKING
	  /* We track only variables whose size is <= MAX_VAR_PARTS bytes
	     thus there are at most MAX_VAR_PARTS different offsets.  */
	  if (var->n_var_parts >= MAX_VAR_PARTS)
	    abort ();
#endif

	  /* We have to move the elements of array starting at index low to the
	     next position.  */
	  for (high = var->n_var_parts; high > low; high--)
	    var->var_part[high] = var->var_part[high - 1];

	  var->n_var_parts++;
	  var->var_part[pos].offset = offset;
	  var->var_part[pos].loc_chain = NULL;
	  var->var_part[pos].cur_loc = NULL;
	}
    }

  /* Delete the location from the list.  */
  nextp = &var->var_part[pos].loc_chain;
  for (node = var->var_part[pos].loc_chain; node; node = next)
    {
      next = node->next;
      if ((REG_P (node->loc) && REG_P (loc)
	   && REGNO (node->loc) == REGNO (loc))
	  || rtx_equal_p (node->loc, loc))
	{
	  pool_free (loc_chain_pool, node);
	  *nextp = next;
	  break;
	}
      else
	nextp = &node->next;
    }

  /* Add the location to the beginning.  */
  node = pool_alloc (loc_chain_pool);
  node->loc = loc;
  node->next = var->var_part[pos].loc_chain;
  var->var_part[pos].loc_chain = node;

  /* If no location was emitted do so.  */
  if (var->var_part[pos].cur_loc == NULL)
    {
      var->var_part[pos].cur_loc = loc;
      variable_was_changed (var, set->vars);
    }
}

/* Delete the part of variable's location from dataflow set SET.  The variable
   part is specified by variable's declaration DECL and offset OFFSET and the
   part's location by LOC.  */

static void
delete_variable_part (dataflow_set *set, rtx loc, tree decl,
		      HOST_WIDE_INT offset)
{
  int pos, low, high;
  void **slot;
    
  slot = htab_find_slot_with_hash (set->vars, decl, VARIABLE_HASH_VAL (decl),
				   NO_INSERT);
  if (slot)
    {
      variable var = (variable) *slot;

      /* Find the location part.  */
      low = 0;
      high = var->n_var_parts;
      while (low != high)
	{
	  pos = (low + high) / 2;
	  if (var->var_part[pos].offset < offset)
	    low = pos + 1;
	  else
	    high = pos;
	}
      pos = low;

      if (pos < var->n_var_parts && var->var_part[pos].offset == offset)
	{
	  location_chain node, next;
	  location_chain *nextp;
	  bool changed;

	  if (var->refcount > 1)
	    {
	      /* If the variable contains the location part we have to
		 make a copy of the variable.  */
	      for (node = var->var_part[pos].loc_chain; node;
		   node = node->next)
		{
		  if ((REG_P (node->loc) && REG_P (loc)
		       && REGNO (node->loc) == REGNO (loc))
		      || rtx_equal_p (node->loc, loc))
		    {
		      var = unshare_variable (set, var);
		      break;
		    }
		}
	    }

	  /* Delete the location part.  */
	  nextp = &var->var_part[pos].loc_chain;
	  for (node = *nextp; node; node = next)
	    {
	      next = node->next;
	      if ((REG_P (node->loc) && REG_P (loc)
		   && REGNO (node->loc) == REGNO (loc))
		  || rtx_equal_p (node->loc, loc))
		{
		  pool_free (loc_chain_pool, node);
		  *nextp = next;
		  break;
		}
	      else
		nextp = &node->next;
	    }

	  /* If we have deleted the location which was last emitted
	     we have to emit new location so add the variable to set
	     of changed variables.  */
	  if (var->var_part[pos].cur_loc
	      && ((REG_P (loc)
		   && REG_P (var->var_part[pos].cur_loc)
		   && REGNO (loc) == REGNO (var->var_part[pos].cur_loc))
		  || rtx_equal_p (loc, var->var_part[pos].cur_loc)))
	    {
	      changed = true;
	      if (var->var_part[pos].loc_chain)
		var->var_part[pos].cur_loc = var->var_part[pos].loc_chain->loc;
	    }
	  else
	    changed = false;

	  if (var->var_part[pos].loc_chain == NULL)
	    {
	      var->n_var_parts--;
	      while (pos < var->n_var_parts)
		{
		  var->var_part[pos] = var->var_part[pos + 1];
		  pos++;
		}
	    }
	  if (changed)
	      variable_was_changed (var, set->vars);
	}
    }
}

/* Emit the NOTE_INSN_VAR_LOCATION for variable *VARP.  DATA contains
   additional parameters: WHERE specifies whether the note shall be emitted
   before of after instruction INSN.  */

static int
emit_note_insn_var_location (void **varp, void *data)
{
  variable var = *(variable *) varp;
  rtx insn = ((emit_note_data *)data)->insn;
  enum emit_note_where where = ((emit_note_data *)data)->where;
  rtx note;
  int i;
  bool complete;
  HOST_WIDE_INT last_limit;
  tree type_size_unit;

#ifdef ENABLE_CHECKING
  if (!var->decl)
    abort ();
#endif

  complete = true;
  last_limit = 0;
  for (i = 0; i < var->n_var_parts; i++)
    {
      if (last_limit < var->var_part[i].offset)
	{
	  complete = false;
	  break;
	}
      last_limit
	= (var->var_part[i].offset
	   + GET_MODE_SIZE (GET_MODE (var->var_part[i].loc_chain->loc)));
    }
  type_size_unit = TYPE_SIZE_UNIT (TREE_TYPE (var->decl));
  if ((unsigned HOST_WIDE_INT) last_limit < TREE_INT_CST_LOW (type_size_unit))
    complete = false;

  if (where == EMIT_NOTE_AFTER_INSN)
    note = emit_note_after (NOTE_INSN_VAR_LOCATION, insn);
  else
    note = emit_note_before (NOTE_INSN_VAR_LOCATION, insn);

  if (!complete)
    {
      NOTE_VAR_LOCATION (note) = gen_rtx_VAR_LOCATION (VOIDmode, var->decl,
						       NULL_RTX);
    }
  else if (var->n_var_parts == 1)
    {
      rtx expr_list
	= gen_rtx_EXPR_LIST (VOIDmode,
			     var->var_part[0].loc_chain->loc,
			     GEN_INT (var->var_part[0].offset));

      NOTE_VAR_LOCATION (note) = gen_rtx_VAR_LOCATION (VOIDmode, var->decl,
						       expr_list);
    }
  else if (var->n_var_parts)
    {
      rtx argp[MAX_VAR_PARTS];
      rtx parallel;

      for (i = 0; i < var->n_var_parts; i++)
	argp[i] = gen_rtx_EXPR_LIST (VOIDmode, var->var_part[i].loc_chain->loc,
				     GEN_INT (var->var_part[i].offset));
      parallel = gen_rtx_PARALLEL (VOIDmode,
				   gen_rtvec_v (var->n_var_parts, argp));
      NOTE_VAR_LOCATION (note) = gen_rtx_VAR_LOCATION (VOIDmode, var->decl,
						       parallel);
    }

  htab_clear_slot (changed_variables, varp);

  /* When there are no location parts the variable has been already
     removed from hash table and a new empty variable was created.
     Free the empty variable.  */
  if (var->n_var_parts == 0)
    {
      pool_free (var_pool, var);
    }

  /* Continue traversing the hash table.  */
  return 1;
}

/* Emit NOTE_INSN_VAR_LOCATION note for each variable from a chain
   CHANGED_VARIABLES and delete this chain.  WHERE specifies whether the notes
   shall be emitted before of after instruction INSN.  */

static void
emit_notes_for_changes (rtx insn, enum emit_note_where where)
{
  emit_note_data data;

  data.insn = insn;
  data.where = where;
  htab_traverse (changed_variables, emit_note_insn_var_location, &data);
}

/* Add variable *SLOT to the chain CHANGED_VARIABLES if it differs from the
   same variable in hash table DATA or is not there at all.  */

static int
emit_notes_for_differences_1 (void **slot, void *data)
{
  htab_t new_vars = (htab_t) data;
  variable old_var, new_var;

  old_var = *(variable *) slot;
  new_var = htab_find_with_hash (new_vars, old_var->decl,
				 VARIABLE_HASH_VAL (old_var->decl));

  if (!new_var)
    {
      /* Variable has disappeared.  */
      variable empty_var;

      empty_var = pool_alloc (var_pool);
      empty_var->decl = old_var->decl;
      empty_var->refcount = 1;
      empty_var->n_var_parts = 0;
      variable_was_changed (empty_var, NULL);
    }
  else if (variable_different_p (old_var, new_var, true))
    {
      variable_was_changed (new_var, NULL);
    }

  /* Continue traversing the hash table.  */
  return 1;
}

/* Add variable *SLOT to the chain CHANGED_VARIABLES if it is not in hash
   table DATA.  */

static int
emit_notes_for_differences_2 (void **slot, void *data)
{
  htab_t old_vars = (htab_t) data;
  variable old_var, new_var;

  new_var = *(variable *) slot;
  old_var = htab_find_with_hash (old_vars, new_var->decl,
				 VARIABLE_HASH_VAL (new_var->decl));
  if (!old_var)
    {
      /* Variable has appeared.  */
      variable_was_changed (new_var, NULL);
    }

  /* Continue traversing the hash table.  */
  return 1;
}

/* Emit notes before INSN for differences between dataflow sets OLD_SET and
   NEW_SET.  */

static void
emit_notes_for_differences (rtx insn, dataflow_set *old_set,
			    dataflow_set *new_set)
{
  htab_traverse (old_set->vars, emit_notes_for_differences_1, new_set->vars);
  htab_traverse (new_set->vars, emit_notes_for_differences_2, old_set->vars);
  emit_notes_for_changes (insn, EMIT_NOTE_BEFORE_INSN);
}

/* Emit the notes for changes of location parts in the basic block BB.  */

static void
emit_notes_in_bb (basic_block bb)
{
  int i;
  dataflow_set set;

  dataflow_set_init (&set, htab_elements (VTI (bb)->in.vars) + 3);
  dataflow_set_copy (&set, &VTI (bb)->in);

  for (i = 0; i < VTI (bb)->n_mos; i++)
    {
      rtx insn = VTI (bb)->mos[i].insn;

      switch (VTI (bb)->mos[i].type)
	{
	  case MO_CALL:
	    {
	      int r;

	      for (r = 0; r < FIRST_PSEUDO_REGISTER; r++)
		if (TEST_HARD_REG_BIT (call_used_reg_set, r))
		  {
		    var_regno_delete (&set, r);
		  }
	      emit_notes_for_changes (insn, EMIT_NOTE_AFTER_INSN);
	    }
	    break;

	  case MO_USE:
	  case MO_SET:
	    {
	      rtx loc = VTI (bb)->mos[i].u.loc;

	      if (REG_P (loc))
		var_reg_delete_and_set (&set, loc);
	      else
		var_mem_delete_and_set (&set, loc);

	      if (VTI (bb)->mos[i].type == MO_USE)
		emit_notes_for_changes (insn, EMIT_NOTE_BEFORE_INSN);
	      else
		emit_notes_for_changes (insn, EMIT_NOTE_AFTER_INSN);
	    }
	    break;

	  case MO_USE_NO_VAR:
	  case MO_CLOBBER:
	    {
	      rtx loc = VTI (bb)->mos[i].u.loc;

	      if (REG_P (loc))
		var_reg_delete (&set, loc);
	      else
		var_mem_delete (&set, loc);

	      if (VTI (bb)->mos[i].type == MO_USE_NO_VAR)
		emit_notes_for_changes (insn, EMIT_NOTE_BEFORE_INSN);
	      else
		emit_notes_for_changes (insn, EMIT_NOTE_AFTER_INSN);
	    }
	    break;

	  case MO_ADJUST:
	    {
	      rtx base;

	      set.stack_adjust += VTI (bb)->mos[i].u.adjust;
	      base = gen_rtx_MEM (Pmode, plus_constant (stack_pointer_rtx,
							set.stack_adjust));
	      set_frame_base_location (&set, base);
	      emit_notes_for_changes (insn, EMIT_NOTE_AFTER_INSN);
	    }
	    break;
	}
    }
  dataflow_set_destroy (&set);
}

/* Emit notes for the whole function.  */

static void
vt_emit_notes (void)
{
  basic_block bb;
  dataflow_set *last_out;
  dataflow_set empty;

#ifdef ENABLE_CHECKING
  if (htab_elements (changed_variables))
    abort ();
#endif

  /* Enable emitting notes by functions (mainly by set_variable_part and
     delete_variable_part).  */
  emit_notes = true;

  dataflow_set_init (&empty, 7);
  last_out = &empty;

  FOR_EACH_BB (bb)
    {
      /* Emit the notes for changes of variable locations between two
	 subsequent basic blocks.  */
      emit_notes_for_differences (BB_HEAD (bb), last_out, &VTI (bb)->in);

      /* Emit the notes for the changes in the basic block itself.  */
      emit_notes_in_bb (bb);

      last_out = &VTI (bb)->out;
    }
  dataflow_set_destroy (&empty);
  emit_notes = false;
}

/* If there is a declaration and offset associated with register/memory RTL
   assign declaration to *DECLP and offset to *OFFSETP, and return true.  */

static bool
vt_get_decl_and_offset (rtx rtl, tree *declp, HOST_WIDE_INT *offsetp)
{
  if (REG_P (rtl))
    {
      if (REG_ATTRS (rtl))
	{
	  *declp = REG_EXPR (rtl);
	  *offsetp = REG_OFFSET (rtl);
	  return true;
	}
    }
  else if (MEM_P (rtl))
    {
      if (MEM_ATTRS (rtl))
	{
	  *declp = MEM_EXPR (rtl);
	  *offsetp = MEM_OFFSET (rtl) ? INTVAL (MEM_OFFSET (rtl)) : 0;
	  return true;
	}
    }
  return false;
}

/* Insert function parameters to IN and OUT sets of ENTRY_BLOCK.  */

static void
vt_add_function_parameters (void)
{
  tree parm;
  
  for (parm = DECL_ARGUMENTS (current_function_decl);
       parm; parm = TREE_CHAIN (parm))
    {
      rtx decl_rtl = DECL_RTL_IF_SET (parm);
      rtx incoming = DECL_INCOMING_RTL (parm);
      tree decl;
      HOST_WIDE_INT offset;
      dataflow_set *out;

      if (TREE_CODE (parm) != PARM_DECL)
	continue;

      if (!DECL_NAME (parm))
	continue;

      if (!decl_rtl || !incoming)
	continue;

      if (GET_MODE (decl_rtl) == BLKmode || GET_MODE (incoming) == BLKmode)
	continue;

      if (!vt_get_decl_and_offset (incoming, &decl, &offset))
	if (!vt_get_decl_and_offset (decl_rtl, &decl, &offset))
	  continue;

      if (!decl)
	continue;

#ifdef ENABLE_CHECKING
      if (parm != decl)
	abort ();
#endif

      incoming = eliminate_regs (incoming, 0, NULL_RTX);
      out = &VTI (ENTRY_BLOCK_PTR)->out;

      if (REG_P (incoming))
	{
#ifdef ENABLE_CHECKING
	  if (REGNO (incoming) >= FIRST_PSEUDO_REGISTER)
	    abort ();
#endif
	  attrs_list_insert (&out->regs[REGNO (incoming)],
			     parm, offset, incoming);
	  set_variable_part (out, incoming, parm, offset);
	}
      else if (MEM_P (incoming))
	{
	  set_variable_part (out, incoming, parm, offset);
	}
    }
}

/* Allocate and initialize the data structures for variable tracking
   and parse the RTL to get the micro operations.  */

static void
vt_initialize (void)
{
  basic_block bb;

  alloc_aux_for_blocks (sizeof (struct variable_tracking_info_def));

  FOR_EACH_BB (bb)
    {
      rtx insn;
      HOST_WIDE_INT pre, post;

      /* Count the number of micro operations.  */
      VTI (bb)->n_mos = 0;
      for (insn = BB_HEAD (bb); insn != NEXT_INSN (BB_END (bb));
	   insn = NEXT_INSN (insn))
	{
	  if (INSN_P (insn))
	    {
	      if (!frame_pointer_needed)
		{
		  insn_stack_adjust_offset_pre_post (insn, &pre, &post);
		  if (pre)
		    VTI (bb)->n_mos++;
		  if (post)
		    VTI (bb)->n_mos++;
		}
	      note_uses (&PATTERN (insn), count_uses_1, insn);
	      note_stores (PATTERN (insn), count_stores, insn);
	      if (CALL_P (insn))
		VTI (bb)->n_mos++;
	    }
	}

      /* Add the micro-operations to the array.  */
      VTI (bb)->mos = xmalloc (VTI (bb)->n_mos
			       * sizeof (struct micro_operation_def));
      VTI (bb)->n_mos = 0;
      for (insn = BB_HEAD (bb); insn != NEXT_INSN (BB_END (bb));
	   insn = NEXT_INSN (insn))
	{
	  if (INSN_P (insn))
	    {
	      int n1, n2;

	      if (!frame_pointer_needed)
		{
		  insn_stack_adjust_offset_pre_post (insn, &pre, &post);
		  if (pre)
		    {
		      micro_operation *mo = VTI (bb)->mos + VTI (bb)->n_mos++;

		      mo->type = MO_ADJUST;
		      mo->u.adjust = pre;
		      mo->insn = insn;
		    }
		}

	      n1 = VTI (bb)->n_mos;
	      note_uses (&PATTERN (insn), add_uses_1, insn);
	      n2 = VTI (bb)->n_mos - 1;

	      /* Order the MO_USEs to be before MO_USE_NO_VARs.  */
	      while (n1 < n2)
		{
		  while (n1 < n2 && VTI (bb)->mos[n1].type == MO_USE)
		    n1++;
		  while (n1 < n2 && VTI (bb)->mos[n2].type == MO_USE_NO_VAR)
		    n2--;
		  if (n1 < n2)
		    {
		      micro_operation sw;

		      sw = VTI (bb)->mos[n1];
		      VTI (bb)->mos[n1] = VTI (bb)->mos[n2];
		      VTI (bb)->mos[n2] = sw;
		    }
		}

	      if (CALL_P (insn))
		{
		  micro_operation *mo = VTI (bb)->mos + VTI (bb)->n_mos++;

		  mo->type = MO_CALL;
		  mo->insn = insn;
		}

	      n1 = VTI (bb)->n_mos;
	      note_stores (PATTERN (insn), add_stores, insn);
	      n2 = VTI (bb)->n_mos - 1;

	      /* Order the MO_SETs to be before MO_CLOBBERs.  */
	      while (n1 < n2)
		{
		  while (n1 < n2 && VTI (bb)->mos[n1].type == MO_SET)
		    n1++;
		  while (n1 < n2 && VTI (bb)->mos[n2].type == MO_CLOBBER)
		    n2--;
		  if (n1 < n2)
		    {
		      micro_operation sw;

		      sw = VTI (bb)->mos[n1];
		      VTI (bb)->mos[n1] = VTI (bb)->mos[n2];
		      VTI (bb)->mos[n2] = sw;
		    }
		}

	      if (!frame_pointer_needed && post)
		{
		  micro_operation *mo = VTI (bb)->mos + VTI (bb)->n_mos++;

		  mo->type = MO_ADJUST;
		  mo->u.adjust = post;
		  mo->insn = insn;
		}
	    }
	}
    }

  /* Init the IN and OUT sets.  */
  FOR_ALL_BB (bb)
    {
      VTI (bb)->visited = false;
      dataflow_set_init (&VTI (bb)->in, 7);
      dataflow_set_init (&VTI (bb)->out, 7);
    }

  attrs_pool = create_alloc_pool ("attrs_def pool",
				  sizeof (struct attrs_def), 1024);
  var_pool = create_alloc_pool ("variable_def pool",
				sizeof (struct variable_def), 64);
  loc_chain_pool = create_alloc_pool ("location_chain_def pool",
				      sizeof (struct location_chain_def),
				      1024);
  changed_variables = htab_create (10, variable_htab_hash, variable_htab_eq,
				   NULL);
  vt_add_function_parameters ();

  if (!frame_pointer_needed)
    {
      rtx base;

      /* Create fake variable for tracking stack pointer changes.  */
      frame_base_decl = make_node (VAR_DECL);
      DECL_NAME (frame_base_decl) = get_identifier ("___frame_base_decl");
      TREE_TYPE (frame_base_decl) = char_type_node;
      DECL_ARTIFICIAL (frame_base_decl) = 1;

      /* Set its initial "location".  */
      frame_stack_adjust = -prologue_stack_adjust ();
      base = gen_rtx_MEM (Pmode, plus_constant (stack_pointer_rtx,
						frame_stack_adjust));
      set_variable_part (&VTI (ENTRY_BLOCK_PTR)->out, base, frame_base_decl, 0);
    }
  else
    {
      frame_base_decl = NULL;
    }
}

/* Free the data structures needed for variable tracking.  */

static void
vt_finalize (void)
{
  basic_block bb;

  FOR_EACH_BB (bb)
    {
      free (VTI (bb)->mos);
    }

  FOR_ALL_BB (bb)
    {
      dataflow_set_destroy (&VTI (bb)->in);
      dataflow_set_destroy (&VTI (bb)->out);
    }
  free_aux_for_blocks ();
  free_alloc_pool (attrs_pool);
  free_alloc_pool (var_pool);
  free_alloc_pool (loc_chain_pool);
  htab_delete (changed_variables);
}

/* The entry point to variable tracking pass.  */

void
variable_tracking_main (void)
{
  if (n_basic_blocks > 500 && n_edges / n_basic_blocks >= 20)
    return;

  mark_dfs_back_edges ();
  vt_initialize ();
  if (!frame_pointer_needed)
    {
      if (!vt_stack_adjustments ())
	{
	  vt_finalize ();
	  return;
	}
    }

  vt_find_locations ();
  vt_emit_notes ();

  if (dump_file)
    {
      dump_dataflow_sets ();
      dump_flow_info (dump_file);
    }

  vt_finalize ();
}
