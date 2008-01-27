/* SSA operands management for trees.
   Copyright (C) 2003, 2004, 2005, 2006, 2007 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "flags.h"
#include "function.h"
#include "diagnostic.h"
#include "tree-flow.h"
#include "tree-inline.h"
#include "tree-pass.h"
#include "ggc.h"
#include "timevar.h"
#include "toplev.h"
#include "langhooks.h"
#include "ipa-reference.h"

/* This file contains the code required to manage the operands cache of the 
   SSA optimizer.  For every stmt, we maintain an operand cache in the stmt 
   annotation.  This cache contains operands that will be of interest to 
   optimizers and other passes wishing to manipulate the IL. 

   The operand type are broken up into REAL and VIRTUAL operands.  The real 
   operands are represented as pointers into the stmt's operand tree.  Thus 
   any manipulation of the real operands will be reflected in the actual tree.
   Virtual operands are represented solely in the cache, although the base 
   variable for the SSA_NAME may, or may not occur in the stmt's tree.  
   Manipulation of the virtual operands will not be reflected in the stmt tree.

   The routines in this file are concerned with creating this operand cache 
   from a stmt tree.

   The operand tree is the parsed by the various get_* routines which look 
   through the stmt tree for the occurrence of operands which may be of 
   interest, and calls are made to the append_* routines whenever one is 
   found.  There are 4 of these routines, each representing one of the 
   4 types of operands. Defs, Uses, Virtual Uses, and Virtual May Defs.

   The append_* routines check for duplication, and simply keep a list of 
   unique objects for each operand type in the build_* extendable vectors.

   Once the stmt tree is completely parsed, the finalize_ssa_operands() 
   routine is called, which proceeds to perform the finalization routine 
   on each of the 4 operand vectors which have been built up.

   If the stmt had a previous operand cache, the finalization routines 
   attempt to match up the new operands with the old ones.  If it's a perfect 
   match, the old vector is simply reused.  If it isn't a perfect match, then 
   a new vector is created and the new operands are placed there.  For 
   virtual operands, if the previous cache had SSA_NAME version of a 
   variable, and that same variable occurs in the same operands cache, then 
   the new cache vector will also get the same SSA_NAME.

   i.e., if a stmt had a VUSE of 'a_5', and 'a' occurs in the new
   operand vector for VUSE, then the new vector will also be modified
   such that it contains 'a_5' rather than 'a'.  */


/* Structure storing statistics on how many call clobbers we have, and
   how many where avoided.  */

static struct 
{
  /* Number of call-clobbered ops we attempt to add to calls in
     add_call_clobbered_mem_symbols.  */
  unsigned int clobbered_vars;

  /* Number of write-clobbers (VDEFs) avoided by using
     not_written information.  */
  unsigned int static_write_clobbers_avoided;

  /* Number of reads (VUSEs) avoided by using not_read information.  */
  unsigned int static_read_clobbers_avoided;
  
  /* Number of write-clobbers avoided because the variable can't escape to
     this call.  */
  unsigned int unescapable_clobbers_avoided;

  /* Number of read-only uses we attempt to add to calls in
     add_call_read_mem_symbols.  */
  unsigned int readonly_clobbers;

  /* Number of read-only uses we avoid using not_read information.  */
  unsigned int static_readonly_clobbers_avoided;
} clobber_stats;


/* Flags to describe operand properties in helpers.  */

/* By default, operands are loaded.  */
#define opf_use		0

/* Operand is the target of an assignment expression or a 
   call-clobbered variable.  */
#define opf_def 	(1 << 0)

/* No virtual operands should be created in the expression.  This is used
   when traversing ADDR_EXPR nodes which have different semantics than
   other expressions.  Inside an ADDR_EXPR node, the only operands that we
   need to consider are indices into arrays.  For instance, &a.b[i] should
   generate a USE of 'i' but it should not generate a VUSE for 'a' nor a
   VUSE for 'b'.  */
#define opf_no_vops 	(1 << 1)

/* Operand is an implicit reference.  This is used to distinguish
   explicit assignments in the form of GIMPLE_MODIFY_STMT from
   clobbering sites like function calls or ASM_EXPRs.  */
#define opf_implicit	(1 << 2)

/* Array for building all the def operands.  */
static VEC(tree,heap) *build_defs;

/* Array for building all the use operands.  */
static VEC(tree,heap) *build_uses;

/* Set for building all the VDEF operands.  */
static VEC(tree,heap) *build_vdefs;

/* Set for building all the VUSE operands.  */
static VEC(tree,heap) *build_vuses;

/* Bitmap obstack for our datastructures that needs to survive across	
   compilations of multiple functions.  */
static bitmap_obstack operands_bitmap_obstack;

/* Set for building all the loaded symbols.  */
static bitmap build_loads;

/* Set for building all the stored symbols.  */
static bitmap build_stores;

static void get_expr_operands (tree, tree *, int);

/* Number of functions with initialized ssa_operands.  */
static int n_initialized = 0;

/* Statement change buffer.  Data structure used to record state
   information for statements.  This is used to determine what needs
   to be done in order to update the SSA web after a statement is
   modified by a pass.  If STMT is a statement that has just been
   created, or needs to be folded via fold_stmt, or anything that
   changes its physical structure then the pass should:

   1- Call push_stmt_changes (&stmt) to record the current state of
      STMT before any modifications are made.

   2- Make all appropriate modifications to the statement.

   3- Call pop_stmt_changes (&stmt) to find new symbols that
      need to be put in SSA form, SSA name mappings for names that
      have disappeared, recompute invariantness for address
      expressions, cleanup EH information, etc.

   If it is possible to determine that the statement was not modified,
   instead of calling pop_stmt_changes it is quicker to call
   discard_stmt_changes to avoid the expensive and unnecessary operand
   re-scan and change comparison.  */

struct scb_d
{
  /* Pointer to the statement being modified.  */
  tree *stmt_p;

  /* If the statement references memory these are the sets of symbols
     loaded and stored by the statement.  */
  bitmap loads;
  bitmap stores;
};

typedef struct scb_d *scb_t;
DEF_VEC_P(scb_t);
DEF_VEC_ALLOC_P(scb_t,heap);

/* Stack of statement change buffers (SCB).  Every call to
   push_stmt_changes pushes a new buffer onto the stack.  Calls to
   pop_stmt_changes pop a buffer off of the stack and compute the set
   of changes for the popped statement.  */
static VEC(scb_t,heap) *scb_stack;

/* Return the DECL_UID of the base variable of T.  */

static inline unsigned
get_name_decl (const_tree t)
{
  if (TREE_CODE (t) != SSA_NAME)
    return DECL_UID (t);
  else
    return DECL_UID (SSA_NAME_VAR (t));
}


/* Comparison function for qsort used in operand_build_sort_virtual.  */

int
operand_build_cmp (const void *p, const void *q)
{
  const_tree const e1 = *((const_tree const *)p);
  const_tree const e2 = *((const_tree const *)q);
  const unsigned int u1 = get_name_decl (e1);
  const unsigned int u2 = get_name_decl (e2);

  /* We want to sort in ascending order.  They can never be equal.  */
#ifdef ENABLE_CHECKING
  gcc_assert (u1 != u2);
#endif
  return (u1 > u2 ? 1 : -1);
}


/* Sort the virtual operands in LIST from lowest DECL_UID to highest.  */

static inline void
operand_build_sort_virtual (VEC(tree,heap) *list)
{
  int num = VEC_length (tree, list);

  if (num < 2)
    return;

  if (num == 2)
    {
      if (get_name_decl (VEC_index (tree, list, 0)) 
	  > get_name_decl (VEC_index (tree, list, 1)))
	{  
	  /* Swap elements if in the wrong order.  */
	  tree tmp = VEC_index (tree, list, 0);
	  VEC_replace (tree, list, 0, VEC_index (tree, list, 1));
	  VEC_replace (tree, list, 1, tmp);
	}
      return;
    }

  /* There are 3 or more elements, call qsort.  */
  qsort (VEC_address (tree, list), 
	 VEC_length (tree, list), 
	 sizeof (tree),
	 operand_build_cmp);
}


/*  Return true if the SSA operands cache is active.  */

bool
ssa_operands_active (void)
{
  return cfun->gimple_df && gimple_ssa_operands (cfun)->ops_active;
}


/* VOPs are of variable sized, so the free list maps "free buckets" to the 
   following table:  
    bucket   # operands
    ------   ----------
	0	1
	1	2
	  ...
	15	16
	16	17-24
	17	25-32
	18	31-40
	  ...
	29	121-128
   Any VOPs larger than this are simply added to the largest bucket when they
   are freed.  */


/* Return the number of operands used in bucket BUCKET.  */

static inline int
vop_free_bucket_size (int bucket)
{
#ifdef ENABLE_CHECKING
  gcc_assert (bucket >= 0 && bucket < NUM_VOP_FREE_BUCKETS);
#endif
  if (bucket < 16)
    return bucket + 1;
  return (bucket - 13) * 8;
}


/* For a vop of NUM operands, return the bucket NUM belongs to.  If NUM is 
   beyond the end of the bucket table, return -1.  */

static inline int 
vop_free_bucket_index (int num)
{
  gcc_assert (num > 0 && NUM_VOP_FREE_BUCKETS > 16);

  /* Sizes 1 through 16 use buckets 0-15.  */
  if (num <= 16)
    return num - 1;
  /* Buckets 16 - NUM_VOP_FREE_BUCKETS represent 8 unit chunks.  */
  num = 14 + (num - 1) / 8;
  if (num >= NUM_VOP_FREE_BUCKETS)
    return -1;
  else
    return num;
}


/* Initialize the VOP free buckets.  */

static inline void
init_vop_buckets (void)
{
  int x;

  for (x = 0; x < NUM_VOP_FREE_BUCKETS; x++)
    gimple_ssa_operands (cfun)->vop_free_buckets[x] = NULL;
}


/* Add PTR to the appropriate VOP bucket.  */

static inline void
add_vop_to_freelist (voptype_p ptr)
{
  int bucket = vop_free_bucket_index (VUSE_VECT_NUM_ELEM (ptr->usev));

  /* Too large, use the largest bucket so its not a complete throw away.  */
  if (bucket == -1)
    bucket = NUM_VOP_FREE_BUCKETS - 1;

  ptr->next = gimple_ssa_operands (cfun)->vop_free_buckets[bucket];
  gimple_ssa_operands (cfun)->vop_free_buckets[bucket] = ptr;
}
 

/* These are the sizes of the operand memory  buffer which gets allocated each 
   time more operands space is required.  The final value is the amount that is
   allocated every time after that.  */
  
#define OP_SIZE_INIT	0
#define OP_SIZE_1	30
#define OP_SIZE_2	110
#define OP_SIZE_3	511

/* Initialize the operand cache routines.  */

void
init_ssa_operands (void)
{
  if (!n_initialized++)
    {
      build_defs = VEC_alloc (tree, heap, 5);
      build_uses = VEC_alloc (tree, heap, 10);
      build_vuses = VEC_alloc (tree, heap, 25);
      build_vdefs = VEC_alloc (tree, heap, 25);
      bitmap_obstack_initialize (&operands_bitmap_obstack);
      build_loads = BITMAP_ALLOC (&operands_bitmap_obstack);
      build_stores = BITMAP_ALLOC (&operands_bitmap_obstack);
      scb_stack = VEC_alloc (scb_t, heap, 20);
    }

  gcc_assert (gimple_ssa_operands (cfun)->operand_memory == NULL);
  gcc_assert (gimple_ssa_operands (cfun)->mpt_table == NULL);
  gimple_ssa_operands (cfun)->operand_memory_index
     = gimple_ssa_operands (cfun)->ssa_operand_mem_size;
  gimple_ssa_operands (cfun)->ops_active = true;
  memset (&clobber_stats, 0, sizeof (clobber_stats));
  init_vop_buckets ();
  gimple_ssa_operands (cfun)->ssa_operand_mem_size = OP_SIZE_INIT;
}


/* Dispose of anything required by the operand routines.  */

void
fini_ssa_operands (void)
{
  struct ssa_operand_memory_d *ptr;
  unsigned ix;
  tree mpt;

  if (!--n_initialized)
    {
      VEC_free (tree, heap, build_defs);
      VEC_free (tree, heap, build_uses);
      VEC_free (tree, heap, build_vdefs);
      VEC_free (tree, heap, build_vuses);
      BITMAP_FREE (build_loads);
      BITMAP_FREE (build_stores);

      /* The change buffer stack had better be empty.  */
      gcc_assert (VEC_length (scb_t, scb_stack) == 0);
      VEC_free (scb_t, heap, scb_stack);
      scb_stack = NULL;
    }

  gimple_ssa_operands (cfun)->free_defs = NULL;
  gimple_ssa_operands (cfun)->free_uses = NULL;

  while ((ptr = gimple_ssa_operands (cfun)->operand_memory) != NULL)
    {
      gimple_ssa_operands (cfun)->operand_memory
	= gimple_ssa_operands (cfun)->operand_memory->next;
      ggc_free (ptr);
    }

  for (ix = 0;
       VEC_iterate (tree, gimple_ssa_operands (cfun)->mpt_table, ix, mpt);
       ix++)
    {
      if (mpt)
	BITMAP_FREE (MPT_SYMBOLS (mpt));
    }

  VEC_free (tree, heap, gimple_ssa_operands (cfun)->mpt_table);

  gimple_ssa_operands (cfun)->ops_active = false;

  if (!n_initialized)
    bitmap_obstack_release (&operands_bitmap_obstack);
  if (dump_file && (dump_flags & TDF_STATS))
    {
      fprintf (dump_file, "Original clobbered vars:           %d\n",
	       clobber_stats.clobbered_vars);
      fprintf (dump_file, "Static write clobbers avoided:     %d\n",
	       clobber_stats.static_write_clobbers_avoided);
      fprintf (dump_file, "Static read clobbers avoided:      %d\n",
	       clobber_stats.static_read_clobbers_avoided);
      fprintf (dump_file, "Unescapable clobbers avoided:      %d\n",
	       clobber_stats.unescapable_clobbers_avoided);
      fprintf (dump_file, "Original read-only clobbers:       %d\n",
	       clobber_stats.readonly_clobbers);
      fprintf (dump_file, "Static read-only clobbers avoided: %d\n",
	       clobber_stats.static_readonly_clobbers_avoided);
    }
}


/* Return memory for operands of SIZE chunks.  */
                                                                              
static inline void *
ssa_operand_alloc (unsigned size)
{
  char *ptr;

  if (gimple_ssa_operands (cfun)->operand_memory_index + size
      >= gimple_ssa_operands (cfun)->ssa_operand_mem_size)
    {
      struct ssa_operand_memory_d *ptr;

      if (gimple_ssa_operands (cfun)->ssa_operand_mem_size == OP_SIZE_INIT)
	gimple_ssa_operands (cfun)->ssa_operand_mem_size
	   = OP_SIZE_1 * sizeof (struct voptype_d);
      else
	if (gimple_ssa_operands (cfun)->ssa_operand_mem_size
	    == OP_SIZE_1 * sizeof (struct voptype_d))
	  gimple_ssa_operands (cfun)->ssa_operand_mem_size
	     = OP_SIZE_2 * sizeof (struct voptype_d);
	else
	  gimple_ssa_operands (cfun)->ssa_operand_mem_size
	     = OP_SIZE_3 * sizeof (struct voptype_d);

      /* Go right to the maximum size if the request is too large.  */
      if (size > gimple_ssa_operands (cfun)->ssa_operand_mem_size)
        gimple_ssa_operands (cfun)->ssa_operand_mem_size
	  = OP_SIZE_3 * sizeof (struct voptype_d);

      /* We can reliably trigger the case that we need arbitrary many
	 operands (see PR34093), so allocate a buffer just for this request.  */
      if (size > gimple_ssa_operands (cfun)->ssa_operand_mem_size)
	gimple_ssa_operands (cfun)->ssa_operand_mem_size = size;

      ptr = (struct ssa_operand_memory_d *) 
	      ggc_alloc (sizeof (struct ssa_operand_memory_d) 
			 + gimple_ssa_operands (cfun)->ssa_operand_mem_size - 1);
      ptr->next = gimple_ssa_operands (cfun)->operand_memory;
      gimple_ssa_operands (cfun)->operand_memory = ptr;
      gimple_ssa_operands (cfun)->operand_memory_index = 0;
    }
  ptr = &(gimple_ssa_operands (cfun)->operand_memory
	  ->mem[gimple_ssa_operands (cfun)->operand_memory_index]);
  gimple_ssa_operands (cfun)->operand_memory_index += size;
  return ptr;
}


/* Allocate a DEF operand.  */

static inline struct def_optype_d *
alloc_def (void)
{
  struct def_optype_d *ret;
  if (gimple_ssa_operands (cfun)->free_defs)
    {
      ret = gimple_ssa_operands (cfun)->free_defs;
      gimple_ssa_operands (cfun)->free_defs
	= gimple_ssa_operands (cfun)->free_defs->next;
    }
  else
    ret = (struct def_optype_d *)
	  ssa_operand_alloc (sizeof (struct def_optype_d));
  return ret;
}


/* Allocate a USE operand.  */

static inline struct use_optype_d *
alloc_use (void)
{
  struct use_optype_d *ret;
  if (gimple_ssa_operands (cfun)->free_uses)
    {
      ret = gimple_ssa_operands (cfun)->free_uses;
      gimple_ssa_operands (cfun)->free_uses
	= gimple_ssa_operands (cfun)->free_uses->next;
    }
  else
    ret = (struct use_optype_d *)
          ssa_operand_alloc (sizeof (struct use_optype_d));
  return ret;
}


/* Allocate a vop with NUM elements.  */

static inline struct voptype_d *
alloc_vop (int num)
{
  struct voptype_d *ret = NULL;
  int alloc_size = 0;

  int bucket = vop_free_bucket_index (num);
  if (bucket != -1)
    {
      /* If there is a free operand, use it.  */
      if (gimple_ssa_operands (cfun)->vop_free_buckets[bucket] != NULL)
	{
	  ret = gimple_ssa_operands (cfun)->vop_free_buckets[bucket];
	  gimple_ssa_operands (cfun)->vop_free_buckets[bucket] = 
		  gimple_ssa_operands (cfun)->vop_free_buckets[bucket]->next;
	}
      else
        alloc_size = vop_free_bucket_size(bucket);
    }
  else
    alloc_size = num;

  if (alloc_size > 0)
    ret = (struct voptype_d *)ssa_operand_alloc (
	sizeof (struct voptype_d) + (alloc_size - 1) * sizeof (vuse_element_t));

  VUSE_VECT_NUM_ELEM (ret->usev) = num;
  return ret;
}


/* This routine makes sure that PTR is in an immediate use list, and makes
   sure the stmt pointer is set to the current stmt.  */

static inline void
set_virtual_use_link (use_operand_p ptr, tree stmt)
{
  /*  fold_stmt may have changed the stmt pointers.  */
  if (ptr->stmt != stmt)
    ptr->stmt = stmt;

  /* If this use isn't in a list, add it to the correct list.  */
  if (!ptr->prev)
    link_imm_use (ptr, *(ptr->use));
}


/* Adds OP to the list of defs after LAST.  */

static inline def_optype_p 
add_def_op (tree *op, def_optype_p last)
{
  def_optype_p new_def;

  new_def = alloc_def ();
  DEF_OP_PTR (new_def) = op;
  last->next = new_def;
  new_def->next = NULL;
  return new_def;
}


/* Adds OP to the list of uses of statement STMT after LAST.  */

static inline use_optype_p
add_use_op (tree stmt, tree *op, use_optype_p last)
{
  use_optype_p new_use;

  new_use = alloc_use ();
  USE_OP_PTR (new_use)->use = op;
  link_imm_use_stmt (USE_OP_PTR (new_use), *op, stmt);
  last->next = new_use;
  new_use->next = NULL;
  return new_use;
}


/* Return a virtual op pointer with NUM elements which are all
   initialized to OP and are linked into the immediate uses for STMT.
   The new vop is appended after PREV.  */

static inline voptype_p
add_vop (tree stmt, tree op, int num, voptype_p prev)
{
  voptype_p new_vop;
  int x;

  new_vop = alloc_vop (num);
  for (x = 0; x < num; x++)
    {
      VUSE_OP_PTR (new_vop, x)->prev = NULL;
      SET_VUSE_OP (new_vop, x, op);
      VUSE_OP_PTR (new_vop, x)->use = &new_vop->usev.uses[x].use_var;
      link_imm_use_stmt (VUSE_OP_PTR (new_vop, x),
			 new_vop->usev.uses[x].use_var, stmt);
    }

  if (prev)
    prev->next = new_vop;
  new_vop->next = NULL;
  return new_vop;
}


/* Adds OP to the list of vuses of statement STMT after LAST, and moves
   LAST to the new element.  */

static inline voptype_p
add_vuse_op (tree stmt, tree op, int num, voptype_p last)
{
  voptype_p new_vop = add_vop (stmt, op, num, last);
  VDEF_RESULT (new_vop) = NULL_TREE;
  return new_vop;
}


/* Adds OP to the list of vdefs of statement STMT after LAST, and moves
   LAST to the new element.  */

static inline voptype_p
add_vdef_op (tree stmt, tree op, int num, voptype_p last)
{
  voptype_p new_vop = add_vop (stmt, op, num, last);
  VDEF_RESULT (new_vop) = op;
  return new_vop;
}
  

/* Takes elements from build_defs and turns them into def operands of STMT.
   TODO -- Make build_defs VEC of tree *.  */

static inline void
finalize_ssa_defs (tree stmt)
{
  unsigned new_i;
  struct def_optype_d new_list;
  def_optype_p old_ops, last;
  unsigned int num = VEC_length (tree, build_defs);

  /* There should only be a single real definition per assignment.  */
  gcc_assert ((stmt && TREE_CODE (stmt) != GIMPLE_MODIFY_STMT) || num <= 1);

  new_list.next = NULL;
  last = &new_list;

  old_ops = DEF_OPS (stmt);

  new_i = 0;

  /* Check for the common case of 1 def that hasn't changed.  */
  if (old_ops && old_ops->next == NULL && num == 1
      && (tree *) VEC_index (tree, build_defs, 0) == DEF_OP_PTR (old_ops))
    return;

  /* If there is anything in the old list, free it.  */
  if (old_ops)
    {
      old_ops->next = gimple_ssa_operands (cfun)->free_defs;
      gimple_ssa_operands (cfun)->free_defs = old_ops;
    }

  /* If there is anything remaining in the build_defs list, simply emit it.  */
  for ( ; new_i < num; new_i++)
    last = add_def_op ((tree *) VEC_index (tree, build_defs, new_i), last);

  /* Now set the stmt's operands.  */
  DEF_OPS (stmt) = new_list.next;

#ifdef ENABLE_CHECKING
  {
    def_optype_p ptr;
    unsigned x = 0;
    for (ptr = DEF_OPS (stmt); ptr; ptr = ptr->next)
      x++;

    gcc_assert (x == num);
  }
#endif
}


/* Takes elements from build_uses and turns them into use operands of STMT.
   TODO -- Make build_uses VEC of tree *.  */

static inline void
finalize_ssa_uses (tree stmt)
{
  unsigned new_i;
  struct use_optype_d new_list;
  use_optype_p old_ops, ptr, last;

#ifdef ENABLE_CHECKING
  {
    unsigned x;
    unsigned num = VEC_length (tree, build_uses);

    /* If the pointer to the operand is the statement itself, something is
       wrong.  It means that we are pointing to a local variable (the 
       initial call to update_stmt_operands does not pass a pointer to a 
       statement).  */
    for (x = 0; x < num; x++)
      gcc_assert (*((tree *)VEC_index (tree, build_uses, x)) != stmt);
  }
#endif

  new_list.next = NULL;
  last = &new_list;

  old_ops = USE_OPS (stmt);

  /* If there is anything in the old list, free it.  */
  if (old_ops)
    {
      for (ptr = old_ops; ptr; ptr = ptr->next)
	delink_imm_use (USE_OP_PTR (ptr));
      old_ops->next = gimple_ssa_operands (cfun)->free_uses;
      gimple_ssa_operands (cfun)->free_uses = old_ops;
    }

  /* Now create nodes for all the new nodes.  */
  for (new_i = 0; new_i < VEC_length (tree, build_uses); new_i++)
    last = add_use_op (stmt, 
		       (tree *) VEC_index (tree, build_uses, new_i), 
		       last);

  /* Now set the stmt's operands.  */
  USE_OPS (stmt) = new_list.next;

#ifdef ENABLE_CHECKING
  {
    unsigned x = 0;
    for (ptr = USE_OPS (stmt); ptr; ptr = ptr->next)
      x++;

    gcc_assert (x == VEC_length (tree, build_uses));
  }
#endif
}


/* Takes elements from BUILD_VDEFS and turns them into vdef operands of
   STMT.  FIXME, for now VDEF operators should have a single operand
   in their RHS.  */

static inline void
finalize_ssa_vdefs (tree stmt)
{
  unsigned new_i;
  struct voptype_d new_list;
  voptype_p old_ops, ptr, last;
  stmt_ann_t ann = stmt_ann (stmt);

  /* Set the symbols referenced by STMT.  */
  if (!bitmap_empty_p (build_stores))
    {
      if (ann->operands.stores == NULL)
	ann->operands.stores = BITMAP_ALLOC (&operands_bitmap_obstack);

      bitmap_copy (ann->operands.stores, build_stores);
    }
  else
    BITMAP_FREE (ann->operands.stores);

  /* If aliases have not been computed, do not instantiate a virtual
     operator on STMT.  Initially, we only compute the SSA form on
     GIMPLE registers.  The virtual SSA form is only computed after
     alias analysis, so virtual operators will remain unrenamed and
     the verifier will complain.  However, alias analysis needs to
     access symbol load/store information, so we need to compute
     those.  */
  if (!gimple_aliases_computed_p (cfun))
    return;

  new_list.next = NULL;
  last = &new_list;

  old_ops = VDEF_OPS (stmt);
  new_i = 0;
  while (old_ops && new_i < VEC_length (tree, build_vdefs))
    {
      tree op = VEC_index (tree, build_vdefs, new_i);
      unsigned new_uid = get_name_decl (op);
      unsigned old_uid = get_name_decl (VDEF_RESULT (old_ops));

      /* FIXME, for now each VDEF operator should have at most one
	 operand in their RHS.  */
      gcc_assert (VDEF_NUM (old_ops) == 1);

      if (old_uid == new_uid)
        {
	  /* If the symbols are the same, reuse the existing operand.  */
	  last->next = old_ops;
	  last = old_ops;
	  old_ops = old_ops->next;
	  last->next = NULL;
	  set_virtual_use_link (VDEF_OP_PTR (last, 0), stmt);
	  new_i++;
	}
      else if (old_uid < new_uid)
	{
	  /* If old is less than new, old goes to the free list.  */
	  voptype_p next;
	  delink_imm_use (VDEF_OP_PTR (old_ops, 0));
	  next = old_ops->next;
	  add_vop_to_freelist (old_ops);
	  old_ops = next;
	}
      else
	{
	  /* This is a new operand.  */
	  last = add_vdef_op (stmt, op, 1, last);
	  new_i++;
	}
    }

  /* If there is anything remaining in BUILD_VDEFS, simply emit it.  */
  for ( ; new_i < VEC_length (tree, build_vdefs); new_i++)
    last = add_vdef_op (stmt, VEC_index (tree, build_vdefs, new_i), 1, last);

  /* If there is anything in the old list, free it.  */
  if (old_ops)
    {
      for (ptr = old_ops; ptr; ptr = last)
        {
	  last = ptr->next;
	  delink_imm_use (VDEF_OP_PTR (ptr, 0));
	  add_vop_to_freelist (ptr);
	}
    }

  /* Now set STMT's operands.  */
  VDEF_OPS (stmt) = new_list.next;

#ifdef ENABLE_CHECKING
  {
    unsigned x = 0;
    for (ptr = VDEF_OPS (stmt); ptr; ptr = ptr->next)
      x++;

    gcc_assert (x == VEC_length (tree, build_vdefs));
  }
#endif
}


/* Takes elements from BUILD_VUSES and turns them into VUSE operands of
   STMT.  */

static inline void
finalize_ssa_vuse_ops (tree stmt)
{
  unsigned new_i, old_i;
  voptype_p old_ops, last;
  VEC(tree,heap) *new_ops;
  stmt_ann_t ann;

  /* Set the symbols referenced by STMT.  */
  ann = stmt_ann (stmt);
  if (!bitmap_empty_p (build_loads))
    {
      if (ann->operands.loads == NULL)
	ann->operands.loads = BITMAP_ALLOC (&operands_bitmap_obstack);

      bitmap_copy (ann->operands.loads, build_loads);
    }
  else
    BITMAP_FREE (ann->operands.loads);

  /* If aliases have not been computed, do not instantiate a virtual
     operator on STMT.  Initially, we only compute the SSA form on
     GIMPLE registers.  The virtual SSA form is only computed after
     alias analysis, so virtual operators will remain unrenamed and
     the verifier will complain.  However, alias analysis needs to
     access symbol load/store information, so we need to compute
     those.  */
  if (!gimple_aliases_computed_p (cfun))
    return;

  /* STMT should have at most one VUSE operator.  */
  old_ops = VUSE_OPS (stmt);
  gcc_assert (old_ops == NULL || old_ops->next == NULL);

  new_ops = NULL;
  new_i = old_i = 0;
  while (old_ops
         && old_i < VUSE_NUM (old_ops)
	 && new_i < VEC_length (tree, build_vuses))
    {
      tree new_op = VEC_index (tree, build_vuses, new_i);
      tree old_op = VUSE_OP (old_ops, old_i);
      unsigned new_uid = get_name_decl (new_op);
      unsigned old_uid = get_name_decl (old_op);

      if (old_uid == new_uid)
        {
	  /* If the symbols are the same, reuse the existing operand.  */
	  VEC_safe_push (tree, heap, new_ops, old_op);
	  new_i++;
	  old_i++;
	}
      else if (old_uid < new_uid)
	{
	  /* If OLD_UID is less than NEW_UID, the old operand has
	     disappeared, skip to the next old operand.  */
	  old_i++;
	}
      else
	{
	  /* This is a new operand.  */
	  VEC_safe_push (tree, heap, new_ops, new_op);
	  new_i++;
	}
    }

  /* If there is anything remaining in the build_vuses list, simply emit it.  */
  for ( ; new_i < VEC_length (tree, build_vuses); new_i++)
    VEC_safe_push (tree, heap, new_ops, VEC_index (tree, build_vuses, new_i));

  /* If there is anything in the old list, free it.  */
  if (old_ops)
    {
      for (old_i = 0; old_i < VUSE_NUM (old_ops); old_i++)
	delink_imm_use (VUSE_OP_PTR (old_ops, old_i));
      add_vop_to_freelist (old_ops);
      VUSE_OPS (stmt) = NULL;
    }

  /* If there are any operands, instantiate a VUSE operator for STMT.  */
  if (new_ops)
    {
      tree op;
      unsigned i;

      last = add_vuse_op (stmt, NULL, VEC_length (tree, new_ops), NULL);

      for (i = 0; VEC_iterate (tree, new_ops, i, op); i++)
	SET_USE (VUSE_OP_PTR (last, (int) i), op);

      VUSE_OPS (stmt) = last;
      VEC_free (tree, heap, new_ops);
    }

#ifdef ENABLE_CHECKING
  {
    unsigned x;
    
    if (VUSE_OPS (stmt))
      {
	gcc_assert (VUSE_OPS (stmt)->next == NULL);
	x = VUSE_NUM (VUSE_OPS (stmt));
      }
    else
      x = 0;

    gcc_assert (x == VEC_length (tree, build_vuses));
  }
#endif
}

/* Return a new VUSE operand vector for STMT.  */
                                                                              
static void
finalize_ssa_vuses (tree stmt)
{
  unsigned num, num_vdefs;
  unsigned vuse_index;

  /* Remove superfluous VUSE operands.  If the statement already has a
     VDEF operator for a variable 'a', then a VUSE for 'a' is not
     needed because VDEFs imply a VUSE of the variable.  For instance,
     suppose that variable 'a' is pointed-to by p and q:

	      # VUSE <a_2>
	      # a_3 = VDEF <a_2>
	      *p = *q;

     The VUSE <a_2> is superfluous because it is implied by the
     VDEF operator.  */
  num = VEC_length (tree, build_vuses);
  num_vdefs = VEC_length (tree, build_vdefs);

  if (num > 0 && num_vdefs > 0)
    for (vuse_index = 0; vuse_index < VEC_length (tree, build_vuses); )
      {
	tree vuse;
	vuse = VEC_index (tree, build_vuses, vuse_index);
	if (TREE_CODE (vuse) != SSA_NAME)
	  {
	    var_ann_t ann = var_ann (vuse);
	    ann->in_vuse_list = 0;
	    if (ann->in_vdef_list)
	      {
		VEC_ordered_remove (tree, build_vuses, vuse_index);
		continue;
	      }
	  }
	vuse_index++;
      }

  finalize_ssa_vuse_ops (stmt);
}


/* Clear the in_list bits and empty the build array for VDEFs and
   VUSEs.  */

static inline void
cleanup_build_arrays (void)
{
  unsigned i;
  tree t;

  for (i = 0; VEC_iterate (tree, build_vdefs, i, t); i++)
    if (TREE_CODE (t) != SSA_NAME)
      var_ann (t)->in_vdef_list = false;

  for (i = 0; VEC_iterate (tree, build_vuses, i, t); i++)
    if (TREE_CODE (t) != SSA_NAME)
      var_ann (t)->in_vuse_list = false;

  VEC_truncate (tree, build_vdefs, 0);
  VEC_truncate (tree, build_vuses, 0);
  VEC_truncate (tree, build_defs, 0);
  VEC_truncate (tree, build_uses, 0);
  bitmap_clear (build_loads);
  bitmap_clear (build_stores);
}


/* Finalize all the build vectors, fill the new ones into INFO.  */
                                                                              
static inline void
finalize_ssa_stmt_operands (tree stmt)
{
  finalize_ssa_defs (stmt);
  finalize_ssa_uses (stmt);
  finalize_ssa_vdefs (stmt);
  finalize_ssa_vuses (stmt);
  cleanup_build_arrays ();
}


/* Start the process of building up operands vectors in INFO.  */

static inline void
start_ssa_stmt_operands (void)
{
  gcc_assert (VEC_length (tree, build_defs) == 0);
  gcc_assert (VEC_length (tree, build_uses) == 0);
  gcc_assert (VEC_length (tree, build_vuses) == 0);
  gcc_assert (VEC_length (tree, build_vdefs) == 0);
  gcc_assert (bitmap_empty_p (build_loads));
  gcc_assert (bitmap_empty_p (build_stores));
}


/* Add DEF_P to the list of pointers to operands.  */

static inline void
append_def (tree *def_p)
{
  VEC_safe_push (tree, heap, build_defs, (tree) def_p);
}


/* Add USE_P to the list of pointers to operands.  */

static inline void
append_use (tree *use_p)
{
  VEC_safe_push (tree, heap, build_uses, (tree) use_p);
}


/* Add VAR to the set of variables that require a VDEF operator.  */

static inline void
append_vdef (tree var)
{
  tree sym;

  if (TREE_CODE (var) != SSA_NAME)
    {
      tree mpt;
      var_ann_t ann;

      /* If VAR belongs to a memory partition, use it instead of VAR.  */
      mpt = memory_partition (var);
      if (mpt)
	var = mpt;

      /* Don't allow duplicate entries.  */
      ann = get_var_ann (var);
      if (ann->in_vdef_list)
        return;

      ann->in_vdef_list = true;
      sym = var;
    }
  else
    sym = SSA_NAME_VAR (var);

  VEC_safe_push (tree, heap, build_vdefs, var);
  bitmap_set_bit (build_stores, DECL_UID (sym));
}


/* Add VAR to the set of variables that require a VUSE operator.  */

static inline void
append_vuse (tree var)
{
  tree sym;

  if (TREE_CODE (var) != SSA_NAME)
    {
      tree mpt;
      var_ann_t ann;

      /* If VAR belongs to a memory partition, use it instead of VAR.  */
      mpt = memory_partition (var);
      if (mpt)
	var = mpt;

      /* Don't allow duplicate entries.  */
      ann = get_var_ann (var);
      if (ann->in_vuse_list)
	return;
      else if (ann->in_vdef_list)
       {
         /* We don't want a vuse if we already have a vdef, but we must
            still put this in build_loads.  */
         bitmap_set_bit (build_loads, DECL_UID (var));
         return;
       }

      ann->in_vuse_list = true;
      sym = var;
    }
  else
    sym = SSA_NAME_VAR (var);

  VEC_safe_push (tree, heap, build_vuses, var);
  bitmap_set_bit (build_loads, DECL_UID (sym));
}


/* REF is a tree that contains the entire pointer dereference
   expression, if available, or NULL otherwise.  ALIAS is the variable
   we are asking if REF can access.  OFFSET and SIZE come from the
   memory access expression that generated this virtual operand.

   XXX: We should handle the NO_ALIAS attributes here.  */

static bool
access_can_touch_variable (tree ref, tree alias, HOST_WIDE_INT offset,
			   HOST_WIDE_INT size)
{
  bool offsetgtz = offset > 0;
  unsigned HOST_WIDE_INT uoffset = (unsigned HOST_WIDE_INT) offset;
  tree base = ref ? get_base_address (ref) : NULL;

  /* If ALIAS is .GLOBAL_VAR then the memory reference REF must be
     using a call-clobbered memory tag.  By definition, call-clobbered
     memory tags can always touch .GLOBAL_VAR.  */
  if (alias == gimple_global_var (cfun))
    return true;

  /* If ref is a TARGET_MEM_REF, just return true, as we can't really
     disambiguate them right now.  */
  if (ref && TREE_CODE (ref) == TARGET_MEM_REF)
    return true;
  
  /* If ALIAS is an SFT, it can't be touched if the offset     
     and size of the access is not overlapping with the SFT offset and
     size.  This is only true if we are accessing through a pointer
     to a type that is the same as SFT_PARENT_VAR.  Otherwise, we may
     be accessing through a pointer to some substruct of the
     structure, and if we try to prune there, we will have the wrong
     offset, and get the wrong answer.
     i.e., we can't prune without more work if we have something like

     struct gcc_target
     {
       struct asm_out
       {
         const char *byte_op;
	 struct asm_int_op
	 {    
	   const char *hi;
	 } aligned_op;
       } asm_out;
     } targetm;
     
     foo = &targetm.asm_out.aligned_op;
     return foo->hi;

     SFT.1, which represents hi, will have SFT_OFFSET=32 because in
     terms of SFT_PARENT_VAR, that is where it is.
     However, the access through the foo pointer will be at offset 0.  */
  if (size != -1
      && TREE_CODE (alias) == STRUCT_FIELD_TAG
      && base
      && TREE_TYPE (base) == TREE_TYPE (SFT_PARENT_VAR (alias))
      && !overlap_subvar (offset, size, alias, NULL))
    {
#ifdef ACCESS_DEBUGGING
      fprintf (stderr, "Access to ");
      print_generic_expr (stderr, ref, 0);
      fprintf (stderr, " may not touch ");
      print_generic_expr (stderr, alias, 0);
      fprintf (stderr, " in function %s\n", get_name (current_function_decl));
#endif
      return false;
    }

  /* Without strict aliasing, it is impossible for a component access
     through a pointer to touch a random variable, unless that
     variable *is* a structure or a pointer.

     That is, given p->c, and some random global variable b,
     there is no legal way that p->c could be an access to b.
     
     Without strict aliasing on, we consider it legal to do something
     like:

     struct foos { int l; };
     int foo;
     static struct foos *getfoo(void);
     int main (void)
     {
       struct foos *f = getfoo();
       f->l = 1;
       foo = 2;
       if (f->l == 1)
         abort();
       exit(0);
     }
     static struct foos *getfoo(void)     
     { return (struct foos *)&foo; }
     
     (taken from 20000623-1.c)

     The docs also say/imply that access through union pointers
     is legal (but *not* if you take the address of the union member,
     i.e. the inverse), such that you can do

     typedef union {
       int d;
     } U;

     int rv;
     void breakme()
     {
       U *rv0;
       U *pretmp = (U*)&rv;
       rv0 = pretmp;
       rv0->d = 42;    
     }
     To implement this, we just punt on accesses through union
     pointers entirely.

     Another case we have to allow is accessing a variable
     through an array access at offset zero.  This happens from
     code generated by the fortran frontend like

     char[1:1] & my_char_ref;
     char my_char;
     my_char_ref_1 = (char[1:1] &) &my_char;
     D.874_2 = (*my_char_ref_1)[1]{lb: 1 sz: 1};
  */
  else if (ref 
	   && flag_strict_aliasing
	   && TREE_CODE (ref) != INDIRECT_REF
	   && !MTAG_P (alias)
	   && base
	   && (TREE_CODE (base) != INDIRECT_REF
	       || TREE_CODE (TREE_TYPE (base)) != UNION_TYPE)
	   && (TREE_CODE (base) != INDIRECT_REF
	       || TREE_CODE (ref) != ARRAY_REF
	       || offset != 0
	       || (DECL_SIZE (alias)
		   && TREE_CODE (DECL_SIZE (alias)) == INTEGER_CST
		   && size != -1
		   && (unsigned HOST_WIDE_INT)size
		      != TREE_INT_CST_LOW (DECL_SIZE (alias))))
	   && !AGGREGATE_TYPE_P (TREE_TYPE (alias))
	   && TREE_CODE (TREE_TYPE (alias)) != COMPLEX_TYPE
	   && !var_ann (alias)->is_heapvar
	   /* When the struct has may_alias attached to it, we need not to
	      return true.  */
	   && get_alias_set (base))
    {
#ifdef ACCESS_DEBUGGING
      fprintf (stderr, "Access to ");
      print_generic_expr (stderr, ref, 0);
      fprintf (stderr, " may not touch ");
      print_generic_expr (stderr, alias, 0);
      fprintf (stderr, " in function %s\n", get_name (current_function_decl));
#endif
      return false;
    }

  /* If the offset of the access is greater than the size of one of
     the possible aliases, it can't be touching that alias, because it
     would be past the end of the structure.  */
  else if (ref
	   && flag_strict_aliasing
	   && TREE_CODE (ref) != INDIRECT_REF
	   && !MTAG_P (alias)
	   && !POINTER_TYPE_P (TREE_TYPE (alias))
	   && offsetgtz
	   && DECL_SIZE (alias)
	   && TREE_CODE (DECL_SIZE (alias)) == INTEGER_CST
	   && uoffset > TREE_INT_CST_LOW (DECL_SIZE (alias)))
    {
#ifdef ACCESS_DEBUGGING
      fprintf (stderr, "Access to ");
      print_generic_expr (stderr, ref, 0);
      fprintf (stderr, " may not touch ");
      print_generic_expr (stderr, alias, 0);
      fprintf (stderr, " in function %s\n", get_name (current_function_decl));
#endif
      return false;
    }	   

  return true;
}

/* Add the actual variables accessed, given a member of a points-to set
   that is the SFT VAR, where the access is of SIZE at OFFSET from VAR.
   IS_CALL_SITE is true if this is a call, and IS_DEF is true if this is
   supposed to be a vdef, and false if this should be a VUSE.

   The real purpose of this function is to take a points-to set for a
   pointer to a structure, say

   struct s {
     int a;
     int b;
   } foo, *foop = &foo;

   and discover which variables an access, such as foop->b, can alias.
   
   This is necessary because foop only actually points to foo's first
   member, so that is all the points-to set contains.  However, an access
   to foop->a may be touching some single SFT if we have created some
   SFT's for a structure.  */

static bool
add_vars_for_offset (tree var, unsigned HOST_WIDE_INT offset,
		     unsigned HOST_WIDE_INT size, bool is_def)
{
  bool added = false;
  tree subvar;
  subvar_t sv;
  unsigned int i;

  /* Adjust offset by the pointed-to location.  */
  offset += SFT_OFFSET (var);

  /* Add all subvars of var that overlap with the access.
     Binary search for the first relevant SFT.  */
  sv = get_subvars_for_var (SFT_PARENT_VAR (var));
  if (!get_first_overlapping_subvar (sv, offset, size, &i))
    return false;

  for (; VEC_iterate (tree, sv, i, subvar); ++i)
    {
      if (SFT_OFFSET (subvar) > offset
	  && size <= SFT_OFFSET (subvar) - offset)
	break;

      if (is_def)
	append_vdef (subvar);
      else
	append_vuse (subvar);
      added = true;
    }

  return added;
}


/* Add VAR to the virtual operands array.  FLAGS is as in
   get_expr_operands.  FULL_REF is a tree that contains the entire
   pointer dereference expression, if available, or NULL otherwise.
   OFFSET and SIZE come from the memory access expression that
   generated this virtual operand.  IS_CALL_SITE is true if the
   affected statement is a call site.  */

static void
add_virtual_operand (tree var, stmt_ann_t s_ann, int flags,
		     tree full_ref, HOST_WIDE_INT offset,
		     HOST_WIDE_INT size, bool is_call_site)
{
  bitmap aliases = NULL;
  tree sym;
  var_ann_t v_ann;
  
  sym = (TREE_CODE (var) == SSA_NAME ? SSA_NAME_VAR (var) : var);
  v_ann = var_ann (sym);
  
  /* Mark the statement as having memory operands.  */
  s_ann->references_memory = true;

  /* If the variable cannot be modified and this is a VDEF change
     it into a VUSE.  This happens when read-only variables are marked
     call-clobbered and/or aliased to writable variables.  So we only
     check that this only happens on non-specific stores.

     Note that if this is a specific store, i.e. associated with a
     GIMPLE_MODIFY_STMT, then we can't suppress the VDEF, lest we run
     into validation problems.

     This can happen when programs cast away const, leaving us with a
     store to read-only memory.  If the statement is actually executed
     at runtime, then the program is ill formed.  If the statement is
     not executed then all is well.  At the very least, we cannot ICE.  */
  if ((flags & opf_implicit) && unmodifiable_var_p (var))
    flags &= ~opf_def;
  
  /* The variable is not a GIMPLE register.  Add it (or its aliases) to
     virtual operands, unless the caller has specifically requested
     not to add virtual operands (used when adding operands inside an
     ADDR_EXPR expression).  */
  if (flags & opf_no_vops)
    return;
  
  if (MTAG_P (var))
    aliases = MTAG_ALIASES (var);

  if (aliases == NULL)
    {
      if (!gimple_aliases_computed_p (cfun)
	  && (flags & opf_def))
        s_ann->has_volatile_ops = true;

      /* The variable is not aliased or it is an alias tag.  */
      if (flags & opf_def)
	append_vdef (var);
      else
	append_vuse (var);
    }
  else
    {
      bitmap_iterator bi;
      unsigned int i;
      bool none_added = true;
      
      /* The variable is aliased.  Add its aliases to the virtual
	 operands.  */
      gcc_assert (!bitmap_empty_p (aliases));

      EXECUTE_IF_SET_IN_BITMAP (aliases, 0, i, bi)
	{
	  tree al = referenced_var (i);

	  /* For SFTs we have to consider all subvariables of the parent var
	     if it is a potential points-to location.  */
	  if (TREE_CODE (al) == STRUCT_FIELD_TAG
	      && TREE_CODE (var) == NAME_MEMORY_TAG)
	    {
	      if (SFT_BASE_FOR_COMPONENTS_P (al))
		{
		  /* If AL is the first SFT of a component, it can be used
		     to find other SFTs at [offset, size] adjacent to it.  */
		  none_added &= !add_vars_for_offset (al, offset, size,
						      flags & opf_def);
		}
	      else if ((unsigned HOST_WIDE_INT)offset < SFT_SIZE (al))
		{
		  /* Otherwise, we only need to consider it if
		     [offset, size] overlaps with AL.  */
		  if (flags & opf_def)
		    append_vdef (al);
		  else
		    append_vuse (al);
		  none_added = false;
		}
	    }
	  else
	    {
	      /* Call-clobbered tags may have non-call-clobbered
		 symbols in their alias sets.  Ignore them if we are
		 adding VOPs for a call site.  */
	      if (is_call_site && !is_call_clobbered (al))
		 continue;

	      /* If we do not know the full reference tree or if the access is
		 unspecified [0, -1], we cannot prune it.  Otherwise try doing
		 so using access_can_touch_variable.  */
	      if (full_ref
		  && !access_can_touch_variable (full_ref, al, offset, size))
		continue;

	      if (flags & opf_def)
		append_vdef (al);
	      else
		append_vuse (al);
	      none_added = false;
	    }
	}

      if (flags & opf_def)
	{
	  /* If the variable is also an alias tag, add a virtual
	     operand for it, otherwise we will miss representing
	     references to the members of the variable's alias set.	     
	     This fixes the bug in gcc.c-torture/execute/20020503-1.c.
	     
	     It is also necessary to add bare defs on clobbers for
	     SMT's, so that bare SMT uses caused by pruning all the
	     aliases will link up properly with calls.   In order to
	     keep the number of these bare defs we add down to the
	     minimum necessary, we keep track of which SMT's were used
	     alone in statement vdefs or VUSEs.  */
	  if (none_added
	      || (TREE_CODE (var) == SYMBOL_MEMORY_TAG
		  && is_call_site))
	    append_vdef (var);
	}
      else
	{
	  /* Even if no aliases have been added, we still need to
	     establish def-use and use-def chains, lest
	     transformations think that this is not a memory
	     reference.  For an example of this scenario, see
	     testsuite/g++.dg/opt/cleanup1.C.  */
	  if (none_added)
	    append_vuse (var);
	}
    }
}


/* Add *VAR_P to the appropriate operand array for S_ANN.  FLAGS is as in
   get_expr_operands.  If *VAR_P is a GIMPLE register, it will be added to
   the statement's real operands, otherwise it is added to virtual
   operands.  */

static void
add_stmt_operand (tree *var_p, stmt_ann_t s_ann, int flags)
{
  tree var, sym;
  var_ann_t v_ann;

  gcc_assert (SSA_VAR_P (*var_p) && s_ann);

  var = *var_p;
  sym = (TREE_CODE (var) == SSA_NAME ? SSA_NAME_VAR (var) : var);
  v_ann = var_ann (sym);

  /* Mark statements with volatile operands.  */
  if (TREE_THIS_VOLATILE (sym))
    s_ann->has_volatile_ops = true;

  if (is_gimple_reg (sym))
    {
      /* The variable is a GIMPLE register.  Add it to real operands.  */
      if (flags & opf_def)
	append_def (var_p);
      else
	append_use (var_p);
    }
  else
    add_virtual_operand (var, s_ann, flags, NULL_TREE, 0, -1, false);
}

/* Subroutine of get_indirect_ref_operands.  ADDR is the address
   that is dereferenced, the meaning of the rest of the arguments
   is the same as in get_indirect_ref_operands.  */

static void
get_addr_dereference_operands (tree stmt, tree *addr, int flags, tree full_ref,
			       HOST_WIDE_INT offset, HOST_WIDE_INT size,
			       bool recurse_on_base)
{
  tree ptr = *addr;
  stmt_ann_t s_ann = stmt_ann (stmt);

  s_ann->references_memory = true;

  if (SSA_VAR_P (ptr))
    {
      struct ptr_info_def *pi = NULL;

      /* If PTR has flow-sensitive points-to information, use it.  */
      if (TREE_CODE (ptr) == SSA_NAME
	  && (pi = SSA_NAME_PTR_INFO (ptr)) != NULL
	  && pi->name_mem_tag)
	{
	  /* PTR has its own memory tag.  Use it.  */
	  add_virtual_operand (pi->name_mem_tag, s_ann, flags,
			       full_ref, offset, size, false);
	}
      else
	{
	  /* If PTR is not an SSA_NAME or it doesn't have a name
	     tag, use its symbol memory tag.  */
	  var_ann_t v_ann;

	  /* If we are emitting debugging dumps, display a warning if
	     PTR is an SSA_NAME with no flow-sensitive alias
	     information.  That means that we may need to compute
	     aliasing again or that a propagation pass forgot to
	     update the alias information on the pointers.  */
	  if (dump_file
	      && TREE_CODE (ptr) == SSA_NAME
	      && (pi == NULL
		  || pi->name_mem_tag == NULL_TREE))
	    {
	      fprintf (dump_file,
		  "NOTE: no flow-sensitive alias info for ");
	      print_generic_expr (dump_file, ptr, dump_flags);
	      fprintf (dump_file, " in ");
	      print_generic_stmt (dump_file, stmt, 0);
	    }

	  if (TREE_CODE (ptr) == SSA_NAME)
	    ptr = SSA_NAME_VAR (ptr);
	  v_ann = var_ann (ptr);

	  /* If we don't know what this pointer points to then we have
	     to make sure to not prune virtual operands based on offset
	     and size.  */
	  if (v_ann->symbol_mem_tag)
	    {
	      add_virtual_operand (v_ann->symbol_mem_tag, s_ann, flags,
				   full_ref, 0, -1, false);
	      /* Make sure we add the SMT itself.  */
	      if (!(flags & opf_no_vops))
		{
		  if (flags & opf_def)
		    append_vdef (v_ann->symbol_mem_tag);
		  else
		    append_vuse (v_ann->symbol_mem_tag);
		}
	    }

	  /* Aliasing information is missing; mark statement as
	     volatile so we won't optimize it out too actively.  */
          else if (!gimple_aliases_computed_p (cfun)
                   && (flags & opf_def))
            s_ann->has_volatile_ops = true;
	}
    }
  else if (TREE_CODE (ptr) == INTEGER_CST)
    {
      /* If a constant is used as a pointer, we can't generate a real
	 operand for it but we mark the statement volatile to prevent
	 optimizations from messing things up.  */
      s_ann->has_volatile_ops = true;
      return;
    }
  else
    {
      /* Ok, this isn't even is_gimple_min_invariant.  Something's broke.  */
      gcc_unreachable ();
    }

  /* If requested, add a USE operand for the base pointer.  */
  if (recurse_on_base)
    get_expr_operands (stmt, addr, opf_use);
}


/* A subroutine of get_expr_operands to handle INDIRECT_REF,
   ALIGN_INDIRECT_REF and MISALIGNED_INDIRECT_REF.  

   STMT is the statement being processed, EXPR is the INDIRECT_REF
      that got us here.
   
   FLAGS is as in get_expr_operands.

   FULL_REF contains the full pointer dereference expression, if we
      have it, or NULL otherwise.

   OFFSET and SIZE are the location of the access inside the
      dereferenced pointer, if known.

   RECURSE_ON_BASE should be set to true if we want to continue
      calling get_expr_operands on the base pointer, and false if
      something else will do it for us.  */

static void
get_indirect_ref_operands (tree stmt, tree expr, int flags, tree full_ref,
			   HOST_WIDE_INT offset, HOST_WIDE_INT size,
			   bool recurse_on_base)
{
  tree *pptr = &TREE_OPERAND (expr, 0);
  stmt_ann_t s_ann = stmt_ann (stmt);

  if (TREE_THIS_VOLATILE (expr))
    s_ann->has_volatile_ops = true; 

  get_addr_dereference_operands (stmt, pptr, flags, full_ref, offset, size,
				 recurse_on_base);
}


/* A subroutine of get_expr_operands to handle TARGET_MEM_REF.  */

static void
get_tmr_operands (tree stmt, tree expr, int flags)
{
  tree tag;
  stmt_ann_t s_ann = stmt_ann (stmt);

  /* This statement references memory.  */
  s_ann->references_memory = 1;

  /* First record the real operands.  */
  get_expr_operands (stmt, &TMR_BASE (expr), opf_use);
  get_expr_operands (stmt, &TMR_INDEX (expr), opf_use);

  if (TMR_SYMBOL (expr))
    add_to_addressable_set (TMR_SYMBOL (expr), &s_ann->addresses_taken);

  tag = TMR_TAG (expr);
  if (!tag)
    {
      /* Something weird, so ensure that we will be careful.  */
      s_ann->has_volatile_ops = true;
      return;
    }
  if (!MTAG_P (tag))
    {
      get_expr_operands (stmt, &tag, flags);
      return;
    }

  add_virtual_operand (tag, s_ann, flags, expr, 0, -1, false);
}


/* Add clobbering definitions for .GLOBAL_VAR or for each of the call
   clobbered variables in the function.  */

static void
add_call_clobber_ops (tree stmt, tree callee)
{
  unsigned u;
  bitmap_iterator bi;
  stmt_ann_t s_ann = stmt_ann (stmt);
  bitmap not_read_b, not_written_b;
  
  /* If we created .GLOBAL_VAR earlier, just use it.  */
  if (gimple_global_var (cfun))
    {
      tree var = gimple_global_var (cfun);
      add_virtual_operand (var, s_ann, opf_def, NULL, 0, -1, true);
      return;
    }

  /* Get info for local and module level statics.  There is a bit
     set for each static if the call being processed does not read
     or write that variable.  */
  not_read_b = callee ? ipa_reference_get_not_read_global (callee) : NULL; 
  not_written_b = callee ? ipa_reference_get_not_written_global (callee) : NULL; 

  /* Add a VDEF operand for every call clobbered variable.  */
  EXECUTE_IF_SET_IN_BITMAP (gimple_call_clobbered_vars (cfun), 0, u, bi)
    {
      tree var = referenced_var_lookup (u);
      unsigned int escape_mask = var_ann (var)->escape_mask;
      tree real_var = var;
      bool not_read;
      bool not_written;
      
      /* Not read and not written are computed on regular vars, not
	 subvars, so look at the parent var if this is an SFT. */
      if (TREE_CODE (var) == STRUCT_FIELD_TAG)
	real_var = SFT_PARENT_VAR (var);

      not_read = not_read_b
	         ? bitmap_bit_p (not_read_b, DECL_UID (real_var))
	         : false;

      not_written = not_written_b
	            ? bitmap_bit_p (not_written_b, DECL_UID (real_var))
		    : false;
      gcc_assert (!unmodifiable_var_p (var));
      
      clobber_stats.clobbered_vars++;

      /* See if this variable is really clobbered by this function.  */

      /* Trivial case: Things escaping only to pure/const are not
	 clobbered by non-pure-const, and only read by pure/const. */
      if ((escape_mask & ~(ESCAPE_TO_PURE_CONST)) == 0)
	{
	  tree call = get_call_expr_in (stmt);
	  if (call_expr_flags (call) & (ECF_CONST | ECF_PURE))
	    {
	      add_virtual_operand (var, s_ann, opf_use, NULL, 0, -1, true);
	      clobber_stats.unescapable_clobbers_avoided++;
	      continue;
	    }
	  else
	    {
	      clobber_stats.unescapable_clobbers_avoided++;
	      continue;
	    }
	}
            
      if (not_written)
	{
	  clobber_stats.static_write_clobbers_avoided++;
	  if (!not_read)
	    add_virtual_operand (var, s_ann, opf_use, NULL, 0, -1, true);
	  else
	    clobber_stats.static_read_clobbers_avoided++;
	}
      else
	add_virtual_operand (var, s_ann, opf_def, NULL, 0, -1, true);
    }
}


/* Add VUSE operands for .GLOBAL_VAR or all call clobbered variables in the
   function.  */

static void
add_call_read_ops (tree stmt, tree callee)
{
  unsigned u;
  bitmap_iterator bi;
  stmt_ann_t s_ann = stmt_ann (stmt);
  bitmap not_read_b;

  /* if the function is not pure, it may reference memory.  Add
     a VUSE for .GLOBAL_VAR if it has been created.  See add_referenced_var
     for the heuristic used to decide whether to create .GLOBAL_VAR.  */
  if (gimple_global_var (cfun))
    {
      tree var = gimple_global_var (cfun);
      add_virtual_operand (var, s_ann, opf_use, NULL, 0, -1, true);
      return;
    }
  
  not_read_b = callee ? ipa_reference_get_not_read_global (callee) : NULL; 

  /* Add a VUSE for each call-clobbered variable.  */
  EXECUTE_IF_SET_IN_BITMAP (gimple_call_clobbered_vars (cfun), 0, u, bi)
    {
      tree var = referenced_var (u);
      tree real_var = var;
      bool not_read;
      
      clobber_stats.readonly_clobbers++;

      /* Not read and not written are computed on regular vars, not
	 subvars, so look at the parent var if this is an SFT. */

      if (TREE_CODE (var) == STRUCT_FIELD_TAG)
	real_var = SFT_PARENT_VAR (var);

      not_read = not_read_b ? bitmap_bit_p (not_read_b, DECL_UID (real_var))
	                    : false;
      
      if (not_read)
	{
	  clobber_stats.static_readonly_clobbers_avoided++;
	  continue;
	}
            
      add_virtual_operand (var, s_ann, opf_use, NULL, 0, -1, true);
    }
}


/* A subroutine of get_expr_operands to handle CALL_EXPR.  */

static void
get_call_expr_operands (tree stmt, tree expr)
{
  int call_flags = call_expr_flags (expr);
  int i, nargs;
  stmt_ann_t ann = stmt_ann (stmt);

  ann->references_memory = true;

  /* If aliases have been computed already, add VDEF or VUSE
     operands for all the symbols that have been found to be
     call-clobbered.  */
  if (gimple_aliases_computed_p (cfun)
      && !(call_flags & ECF_NOVOPS))
    {
      /* A 'pure' or a 'const' function never call-clobbers anything. 
	 A 'noreturn' function might, but since we don't return anyway 
	 there is no point in recording that.  */ 
      if (TREE_SIDE_EFFECTS (expr)
	  && !(call_flags & (ECF_PURE | ECF_CONST | ECF_NORETURN)))
	add_call_clobber_ops (stmt, get_callee_fndecl (expr));
      else if (!(call_flags & ECF_CONST))
	add_call_read_ops (stmt, get_callee_fndecl (expr));
    }

  /* Find uses in the called function.  */
  get_expr_operands (stmt, &CALL_EXPR_FN (expr), opf_use);
  nargs = call_expr_nargs (expr);
  for (i = 0; i < nargs; i++)
    get_expr_operands (stmt, &CALL_EXPR_ARG (expr, i), opf_use);

  get_expr_operands (stmt, &CALL_EXPR_STATIC_CHAIN (expr), opf_use);
}


/* Scan operands in the ASM_EXPR stmt referred to in INFO.  */

static void
get_asm_expr_operands (tree stmt)
{
  stmt_ann_t s_ann;
  int i, noutputs;
  const char **oconstraints;
  const char *constraint;
  bool allows_mem, allows_reg, is_inout;
  tree link;

  s_ann = stmt_ann (stmt);
  noutputs = list_length (ASM_OUTPUTS (stmt));
  oconstraints = (const char **) alloca ((noutputs) * sizeof (const char *));

  /* Gather all output operands.  */
  for (i = 0, link = ASM_OUTPUTS (stmt); link; i++, link = TREE_CHAIN (link))
    {
      constraint = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (link)));
      oconstraints[i] = constraint;
      parse_output_constraint (&constraint, i, 0, 0, &allows_mem,
	                       &allows_reg, &is_inout);

      /* This should have been split in gimplify_asm_expr.  */
      gcc_assert (!allows_reg || !is_inout);

      /* Memory operands are addressable.  Note that STMT needs the
	 address of this operand.  */
      if (!allows_reg && allows_mem)
	{
	  tree t = get_base_address (TREE_VALUE (link));
	  if (t && DECL_P (t) && s_ann)
	    add_to_addressable_set (t, &s_ann->addresses_taken);
	}

      get_expr_operands (stmt, &TREE_VALUE (link), opf_def);
    }

  /* Gather all input operands.  */
  for (link = ASM_INPUTS (stmt); link; link = TREE_CHAIN (link))
    {
      constraint = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (link)));
      parse_input_constraint (&constraint, 0, 0, noutputs, 0, oconstraints,
	                      &allows_mem, &allows_reg);

      /* Memory operands are addressable.  Note that STMT needs the
	 address of this operand.  */
      if (!allows_reg && allows_mem)
	{
	  tree t = get_base_address (TREE_VALUE (link));
	  if (t && DECL_P (t) && s_ann)
	    add_to_addressable_set (t, &s_ann->addresses_taken);
	}

      get_expr_operands (stmt, &TREE_VALUE (link), 0);
    }

  /* Clobber all memory and addressable symbols for asm ("" : : : "memory");  */
  for (link = ASM_CLOBBERS (stmt); link; link = TREE_CHAIN (link))
    if (strcmp (TREE_STRING_POINTER (TREE_VALUE (link)), "memory") == 0)
      {
	unsigned i;
	bitmap_iterator bi;

	s_ann->references_memory = true;

	EXECUTE_IF_SET_IN_BITMAP (gimple_call_clobbered_vars (cfun), 0, i, bi)
	  {
	    tree var = referenced_var (i);
	    add_stmt_operand (&var, s_ann, opf_def | opf_implicit);
	  }

	EXECUTE_IF_SET_IN_BITMAP (gimple_addressable_vars (cfun), 0, i, bi)
	  {
	    tree var = referenced_var (i);

	    /* Subvars are explicitly represented in this list, so we
	       don't need the original to be added to the clobber ops,
	       but the original *will* be in this list because we keep
	       the addressability of the original variable up-to-date
	       to avoid confusing the back-end.  */
	    if (var_can_have_subvars (var)
		&& get_subvars_for_var (var) != NULL)
	      continue;		

	    add_stmt_operand (&var, s_ann, opf_def | opf_implicit);
	  }
	break;
      }
}


/* Scan operands for the assignment expression EXPR in statement STMT.  */

static void
get_modify_stmt_operands (tree stmt, tree expr)
{
  /* First get operands from the RHS.  */
  get_expr_operands (stmt, &GIMPLE_STMT_OPERAND (expr, 1), opf_use);

  /* For the LHS, use a regular definition (opf_def) for GIMPLE
     registers.  If the LHS is a store to memory, we will need
     a preserving definition (VDEF).

     Preserving definitions are those that modify a part of an
     aggregate object for which no subvars have been computed (or the
     reference does not correspond exactly to one of them). Stores
     through a pointer are also represented with VDEF operators.

     We used to distinguish between preserving and killing definitions.
     We always emit preserving definitions now.  */
  get_expr_operands (stmt, &GIMPLE_STMT_OPERAND (expr, 0), opf_def);
}


/* Recursively scan the expression pointed to by EXPR_P in statement
   STMT.  FLAGS is one of the OPF_* constants modifying how to
   interpret the operands found.  */

static void
get_expr_operands (tree stmt, tree *expr_p, int flags)
{
  enum tree_code code;
  enum tree_code_class codeclass;
  tree expr = *expr_p;
  stmt_ann_t s_ann = stmt_ann (stmt);

  if (expr == NULL)
    return;

  code = TREE_CODE (expr);
  codeclass = TREE_CODE_CLASS (code);

  switch (code)
    {
    case ADDR_EXPR:
      /* Taking the address of a variable does not represent a
	 reference to it, but the fact that the statement takes its
	 address will be of interest to some passes (e.g. alias
	 resolution).  */
      add_to_addressable_set (TREE_OPERAND (expr, 0), &s_ann->addresses_taken);

      /* If the address is invariant, there may be no interesting
	 variable references inside.  */
      if (is_gimple_min_invariant (expr))
	return;

      /* Otherwise, there may be variables referenced inside but there
	 should be no VUSEs created, since the referenced objects are
	 not really accessed.  The only operands that we should find
	 here are ARRAY_REF indices which will always be real operands
	 (GIMPLE does not allow non-registers as array indices).  */
      flags |= opf_no_vops;
      get_expr_operands (stmt, &TREE_OPERAND (expr, 0), flags);
      return;

    case SSA_NAME:
    case STRUCT_FIELD_TAG:
    case SYMBOL_MEMORY_TAG:
    case NAME_MEMORY_TAG:
     add_stmt_operand (expr_p, s_ann, flags);
     return;

    case VAR_DECL:
    case PARM_DECL:
    case RESULT_DECL:
      {
	subvar_t svars;
	
	/* Add the subvars for a variable, if it has subvars, to DEFS
	   or USES.  Otherwise, add the variable itself.  Whether it
	   goes to USES or DEFS depends on the operand flags.  */
	if (var_can_have_subvars (expr)
	    && (svars = get_subvars_for_var (expr)))
	  {
	    unsigned int i;
	    tree subvar;
	    for (i = 0; VEC_iterate (tree, svars, i, subvar); ++i)
	      add_stmt_operand (&subvar, s_ann, flags);
	  }
	else
	  add_stmt_operand (expr_p, s_ann, flags);

	return;
      }

    case MISALIGNED_INDIRECT_REF:
      get_expr_operands (stmt, &TREE_OPERAND (expr, 1), flags);
      /* fall through */

    case ALIGN_INDIRECT_REF:
    case INDIRECT_REF:
      get_indirect_ref_operands (stmt, expr, flags, expr, 0, -1, true);
      return;

    case TARGET_MEM_REF:
      get_tmr_operands (stmt, expr, flags);
      return;

    case ARRAY_REF:
    case ARRAY_RANGE_REF:
    case COMPONENT_REF:
    case REALPART_EXPR:
    case IMAGPART_EXPR:
      {
	tree ref;
	HOST_WIDE_INT offset, size, maxsize;
	bool none = true;

	if (TREE_THIS_VOLATILE (expr))
	  s_ann->has_volatile_ops = true;

	/* This component reference becomes an access to all of the
	   subvariables it can touch, if we can determine that, but
	   *NOT* the real one.  If we can't determine which fields we
	   could touch, the recursion will eventually get to a
	   variable and add *all* of its subvars, or whatever is the
	   minimum correct subset.  */
	ref = get_ref_base_and_extent (expr, &offset, &size, &maxsize);
	if (SSA_VAR_P (ref) && get_subvars_for_var (ref))
	  {
	    subvar_t svars = get_subvars_for_var (ref);
	    unsigned int i;
	    tree subvar;

	    for (i = 0; VEC_iterate (tree, svars, i, subvar); ++i)
	      {
		bool exact;		

		if (overlap_subvar (offset, maxsize, subvar, &exact))
		  {
	            int subvar_flags = flags;
		    none = false;
		    add_stmt_operand (&subvar, s_ann, subvar_flags);
		  }
	      }

	    if (!none)
	      flags |= opf_no_vops;

	    if ((DECL_P (ref) && TREE_THIS_VOLATILE (ref))
		|| (TREE_CODE (ref) == SSA_NAME
		    && TREE_THIS_VOLATILE (SSA_NAME_VAR (ref))))
	      s_ann->has_volatile_ops = true;
	  }
	else if (TREE_CODE (ref) == INDIRECT_REF)
	  {
	    get_indirect_ref_operands (stmt, ref, flags, expr, offset,
		                       maxsize, false);
	    flags |= opf_no_vops;
	  }

	/* Even if we found subvars above we need to ensure to see
	   immediate uses for d in s.a[d].  In case of s.a having
	   a subvar or we would miss it otherwise.  */
	get_expr_operands (stmt, &TREE_OPERAND (expr, 0), flags);
	
	if (code == COMPONENT_REF)
	  {
	    if (TREE_THIS_VOLATILE (TREE_OPERAND (expr, 1)))
	      s_ann->has_volatile_ops = true; 
	    get_expr_operands (stmt, &TREE_OPERAND (expr, 2), opf_use);
	  }
	else if (code == ARRAY_REF || code == ARRAY_RANGE_REF)
	  {
            get_expr_operands (stmt, &TREE_OPERAND (expr, 1), opf_use);
            get_expr_operands (stmt, &TREE_OPERAND (expr, 2), opf_use);
            get_expr_operands (stmt, &TREE_OPERAND (expr, 3), opf_use);
	  }

	return;
      }

    case WITH_SIZE_EXPR:
      /* WITH_SIZE_EXPR is a pass-through reference to its first argument,
	 and an rvalue reference to its second argument.  */
      get_expr_operands (stmt, &TREE_OPERAND (expr, 1), opf_use);
      get_expr_operands (stmt, &TREE_OPERAND (expr, 0), flags);
      return;

    case CALL_EXPR:
      get_call_expr_operands (stmt, expr);
      return;

    case COND_EXPR:
    case VEC_COND_EXPR:
      get_expr_operands (stmt, &TREE_OPERAND (expr, 0), opf_use);
      get_expr_operands (stmt, &TREE_OPERAND (expr, 1), opf_use);
      get_expr_operands (stmt, &TREE_OPERAND (expr, 2), opf_use);
      return;

    case GIMPLE_MODIFY_STMT:
      get_modify_stmt_operands (stmt, expr);
      return;

    case CONSTRUCTOR:
      {
	/* General aggregate CONSTRUCTORs have been decomposed, but they
	   are still in use as the COMPLEX_EXPR equivalent for vectors.  */
	constructor_elt *ce;
	unsigned HOST_WIDE_INT idx;

	for (idx = 0;
	     VEC_iterate (constructor_elt, CONSTRUCTOR_ELTS (expr), idx, ce);
	     idx++)
	  get_expr_operands (stmt, &ce->value, opf_use);

	return;
      }

    case BIT_FIELD_REF:
    case TRUTH_NOT_EXPR:
    case VIEW_CONVERT_EXPR:
    do_unary:
      get_expr_operands (stmt, &TREE_OPERAND (expr, 0), flags);
      return;

    case TRUTH_AND_EXPR:
    case TRUTH_OR_EXPR:
    case TRUTH_XOR_EXPR:
    case COMPOUND_EXPR:
    case OBJ_TYPE_REF:
    case ASSERT_EXPR:
    do_binary:
      {
	get_expr_operands (stmt, &TREE_OPERAND (expr, 0), flags);
	get_expr_operands (stmt, &TREE_OPERAND (expr, 1), flags);
	return;
      }

    case DOT_PROD_EXPR:
    case REALIGN_LOAD_EXPR:
      {
	get_expr_operands (stmt, &TREE_OPERAND (expr, 0), flags);
        get_expr_operands (stmt, &TREE_OPERAND (expr, 1), flags);
        get_expr_operands (stmt, &TREE_OPERAND (expr, 2), flags);
        return;
      }

    case CHANGE_DYNAMIC_TYPE_EXPR:
      get_expr_operands (stmt, &CHANGE_DYNAMIC_TYPE_LOCATION (expr), opf_use);
      return;

    case OMP_FOR:
      {
	tree init = OMP_FOR_INIT (expr);
	tree cond = OMP_FOR_COND (expr);
	tree incr = OMP_FOR_INCR (expr);
	tree c, clauses = OMP_FOR_CLAUSES (stmt);

	get_expr_operands (stmt, &GIMPLE_STMT_OPERAND (init, 0), opf_def);
	get_expr_operands (stmt, &GIMPLE_STMT_OPERAND (init, 1), opf_use);
	get_expr_operands (stmt, &TREE_OPERAND (cond, 1), opf_use);
	get_expr_operands (stmt,
	                   &TREE_OPERAND (GIMPLE_STMT_OPERAND (incr, 1), 1),
			   opf_use);

	c = find_omp_clause (clauses, OMP_CLAUSE_SCHEDULE);
	if (c)
	  get_expr_operands (stmt, &OMP_CLAUSE_SCHEDULE_CHUNK_EXPR (c),
			     opf_use);
	return;
      }

    case OMP_CONTINUE:
      {
	get_expr_operands (stmt, &TREE_OPERAND (expr, 0), opf_def);
	get_expr_operands (stmt, &TREE_OPERAND (expr, 1), opf_use);
	return;
      }

    case OMP_PARALLEL:
      {
	tree c, clauses = OMP_PARALLEL_CLAUSES (stmt);

	if (OMP_PARALLEL_DATA_ARG (stmt))
	  {
	    get_expr_operands (stmt, &OMP_PARALLEL_DATA_ARG (stmt), opf_use);
	    add_to_addressable_set (OMP_PARALLEL_DATA_ARG (stmt),
				    &s_ann->addresses_taken);
	  }

	c = find_omp_clause (clauses, OMP_CLAUSE_IF);
	if (c)
	  get_expr_operands (stmt, &OMP_CLAUSE_IF_EXPR (c), opf_use);
	c = find_omp_clause (clauses, OMP_CLAUSE_NUM_THREADS);
	if (c)
	  get_expr_operands (stmt, &OMP_CLAUSE_NUM_THREADS_EXPR (c), opf_use);
	return;
      }

    case OMP_SECTIONS:
      {
	get_expr_operands (stmt, &OMP_SECTIONS_CONTROL (expr), opf_def);
	return;
      }

    case OMP_ATOMIC_LOAD:
      {
	tree *addr = &TREE_OPERAND (expr, 1);
	get_expr_operands (stmt, &TREE_OPERAND (expr, 0), opf_def);

	if (TREE_CODE (*addr) == ADDR_EXPR)
	  get_expr_operands (stmt, &TREE_OPERAND (*addr, 0), opf_def);
	else
	  get_addr_dereference_operands (stmt, addr, opf_def,
					 NULL_TREE, 0, -1, true);
	return;
      }

    case OMP_ATOMIC_STORE:
      {
	get_expr_operands (stmt, &TREE_OPERAND (expr, 0), opf_use);
	return;
      }

    case BLOCK:
    case FUNCTION_DECL:
    case EXC_PTR_EXPR:
    case FILTER_EXPR:
    case LABEL_DECL:
    case CONST_DECL:
    case OMP_SINGLE:
    case OMP_MASTER:
    case OMP_ORDERED:
    case OMP_CRITICAL:
    case OMP_RETURN:
    case OMP_SECTION:
    case OMP_SECTIONS_SWITCH:
      /* Expressions that make no memory references.  */
      return;

    default:
      if (codeclass == tcc_unary)
	goto do_unary;
      if (codeclass == tcc_binary || codeclass == tcc_comparison)
	goto do_binary;
      if (codeclass == tcc_constant || codeclass == tcc_type)
	return;
    }

  /* If we get here, something has gone wrong.  */
#ifdef ENABLE_CHECKING
  fprintf (stderr, "unhandled expression in get_expr_operands():\n");
  debug_tree (expr);
  fputs ("\n", stderr);
#endif
  gcc_unreachable ();
}


/* Parse STMT looking for operands.  When finished, the various
   build_* operand vectors will have potential operands in them.  */

static void
parse_ssa_operands (tree stmt)
{
  enum tree_code code;

  code = TREE_CODE (stmt);
  switch (code)
    {
    case GIMPLE_MODIFY_STMT:
      get_modify_stmt_operands (stmt, stmt);
      break;

    case COND_EXPR:
      get_expr_operands (stmt, &COND_EXPR_COND (stmt), opf_use);
      break;

    case SWITCH_EXPR:
      get_expr_operands (stmt, &SWITCH_COND (stmt), opf_use);
      break;

    case ASM_EXPR:
      get_asm_expr_operands (stmt);
      break;

    case RETURN_EXPR:
      get_expr_operands (stmt, &TREE_OPERAND (stmt, 0), opf_use);
      break;

    case GOTO_EXPR:
      get_expr_operands (stmt, &GOTO_DESTINATION (stmt), opf_use);
      break;

    case LABEL_EXPR:
      get_expr_operands (stmt, &LABEL_EXPR_LABEL (stmt), opf_use);
      break;

    case BIND_EXPR:
    case CASE_LABEL_EXPR:
    case TRY_CATCH_EXPR:
    case TRY_FINALLY_EXPR:
    case EH_FILTER_EXPR:
    case CATCH_EXPR:
    case RESX_EXPR:
      /* These nodes contain no variable references.  */
     break;

    default:
      /* Notice that if get_expr_operands tries to use &STMT as the
	 operand pointer (which may only happen for USE operands), we
	 will fail in add_stmt_operand.  This default will handle
	 statements like empty statements, or CALL_EXPRs that may
	 appear on the RHS of a statement or as statements themselves.  */
      get_expr_operands (stmt, &stmt, opf_use);
      break;
    }
}


/* Create an operands cache for STMT.  */

static void
build_ssa_operands (tree stmt)
{
  stmt_ann_t ann = get_stmt_ann (stmt);
  
  /* Initially assume that the statement has no volatile operands and
     makes no memory references.  */
  ann->has_volatile_ops = false;
  ann->references_memory = false;
  /* Just clear the bitmap so we don't end up reallocating it over and over.  */
  if (ann->addresses_taken)
    bitmap_clear (ann->addresses_taken);

  start_ssa_stmt_operands ();
  parse_ssa_operands (stmt);
  operand_build_sort_virtual (build_vuses);
  operand_build_sort_virtual (build_vdefs);
  finalize_ssa_stmt_operands (stmt);

  if (ann->addresses_taken && bitmap_empty_p (ann->addresses_taken))
    ann->addresses_taken = NULL;

  /* For added safety, assume that statements with volatile operands
     also reference memory.  */
  if (ann->has_volatile_ops)
    ann->references_memory = true;
}


/* Releases the operands of STMT back to their freelists, and clears
   the stmt operand lists.  */

void
free_stmt_operands (tree stmt)
{
  def_optype_p defs = DEF_OPS (stmt), last_def;
  use_optype_p uses = USE_OPS (stmt), last_use;
  voptype_p vuses = VUSE_OPS (stmt);
  voptype_p vdefs = VDEF_OPS (stmt), vdef, next_vdef;
  unsigned i;

  if (defs)
    {
      for (last_def = defs; last_def->next; last_def = last_def->next)
	continue;
      last_def->next = gimple_ssa_operands (cfun)->free_defs;
      gimple_ssa_operands (cfun)->free_defs = defs;
      DEF_OPS (stmt) = NULL;
    }

  if (uses)
    {
      for (last_use = uses; last_use->next; last_use = last_use->next)
	delink_imm_use (USE_OP_PTR (last_use));
      delink_imm_use (USE_OP_PTR (last_use));
      last_use->next = gimple_ssa_operands (cfun)->free_uses;
      gimple_ssa_operands (cfun)->free_uses = uses;
      USE_OPS (stmt) = NULL;
    }

  if (vuses)
    {
      for (i = 0; i < VUSE_NUM (vuses); i++)
	delink_imm_use (VUSE_OP_PTR (vuses, i));
      add_vop_to_freelist (vuses);
      VUSE_OPS (stmt) = NULL;
    }

  if (vdefs)
    {
      for (vdef = vdefs; vdef; vdef = next_vdef)
	{
	  next_vdef = vdef->next;
	  delink_imm_use (VDEF_OP_PTR (vdef, 0));
	  add_vop_to_freelist (vdef);
	}
      VDEF_OPS (stmt) = NULL;
    }
}


/* Free any operands vectors in OPS.  */

void 
free_ssa_operands (stmt_operands_p ops)
{
  ops->def_ops = NULL;
  ops->use_ops = NULL;
  ops->vdef_ops = NULL;
  ops->vuse_ops = NULL;
  BITMAP_FREE (ops->loads);
  BITMAP_FREE (ops->stores);
}


/* Get the operands of statement STMT.  */

void
update_stmt_operands (tree stmt)
{
  stmt_ann_t ann = get_stmt_ann (stmt);

  /* If update_stmt_operands is called before SSA is initialized, do
     nothing.  */
  if (!ssa_operands_active ())
    return;

  /* The optimizers cannot handle statements that are nothing but a
     _DECL.  This indicates a bug in the gimplifier.  */
  gcc_assert (!SSA_VAR_P (stmt));

  timevar_push (TV_TREE_OPS);

  gcc_assert (ann->modified);
  build_ssa_operands (stmt);
  ann->modified = 0;

  timevar_pop (TV_TREE_OPS);
}


/* Copies virtual operands from SRC to DST.  */

void
copy_virtual_operands (tree dest, tree src)
{
  unsigned int i, n;
  voptype_p src_vuses, dest_vuses;
  voptype_p src_vdefs, dest_vdefs;
  struct voptype_d vuse;
  struct voptype_d vdef;
  stmt_ann_t dest_ann;

  VDEF_OPS (dest) = NULL;
  VUSE_OPS (dest) = NULL;

  dest_ann = get_stmt_ann (dest);
  BITMAP_FREE (dest_ann->operands.loads);
  BITMAP_FREE (dest_ann->operands.stores);

  if (LOADED_SYMS (src))
    {
      dest_ann->operands.loads = BITMAP_ALLOC (&operands_bitmap_obstack);
      bitmap_copy (dest_ann->operands.loads, LOADED_SYMS (src));
    }

  if (STORED_SYMS (src))
    {
      dest_ann->operands.stores = BITMAP_ALLOC (&operands_bitmap_obstack);
      bitmap_copy (dest_ann->operands.stores, STORED_SYMS (src));
    }

  /* Copy all the VUSE operators and corresponding operands.  */
  dest_vuses = &vuse;
  for (src_vuses = VUSE_OPS (src); src_vuses; src_vuses = src_vuses->next)
    {
      n = VUSE_NUM (src_vuses);
      dest_vuses = add_vuse_op (dest, NULL_TREE, n, dest_vuses);
      for (i = 0; i < n; i++)
	SET_USE (VUSE_OP_PTR (dest_vuses, i), VUSE_OP (src_vuses, i));

      if (VUSE_OPS (dest) == NULL)
	VUSE_OPS (dest) = vuse.next;
    }

  /* Copy all the VDEF operators and corresponding operands.  */
  dest_vdefs = &vdef;
  for (src_vdefs = VDEF_OPS (src); src_vdefs; src_vdefs = src_vdefs->next)
    {
      n = VUSE_NUM (src_vdefs);
      dest_vdefs = add_vdef_op (dest, NULL_TREE, n, dest_vdefs);
      VDEF_RESULT (dest_vdefs) = VDEF_RESULT (src_vdefs);
      for (i = 0; i < n; i++)
	SET_USE (VUSE_OP_PTR (dest_vdefs, i), VUSE_OP (src_vdefs, i));

      if (VDEF_OPS (dest) == NULL)
	VDEF_OPS (dest) = vdef.next;
    }
}


/* Specifically for use in DOM's expression analysis.  Given a store, we
   create an artificial stmt which looks like a load from the store, this can
   be used to eliminate redundant loads.  OLD_OPS are the operands from the 
   store stmt, and NEW_STMT is the new load which represents a load of the
   values stored.  If DELINK_IMM_USES_P is specified, the immediate
   uses of this stmt will be de-linked.  */

void
create_ssa_artificial_load_stmt (tree new_stmt, tree old_stmt,
				 bool delink_imm_uses_p)
{
  tree op;
  ssa_op_iter iter;
  use_operand_p use_p;
  unsigned i;
  stmt_ann_t ann;

  /* Create the stmt annotation but make sure to not mark the stmt
     as modified as we will build operands ourselves.  */
  ann = get_stmt_ann (new_stmt);
  ann->modified = 0;

  /* Process NEW_STMT looking for operands.  */
  start_ssa_stmt_operands ();
  parse_ssa_operands (new_stmt);

  for (i = 0; VEC_iterate (tree, build_vuses, i, op); i++)
    if (TREE_CODE (op) != SSA_NAME)
      var_ann (op)->in_vuse_list = false;
   
  for (i = 0; VEC_iterate (tree, build_vdefs, i, op); i++)
    if (TREE_CODE (op) != SSA_NAME)
      var_ann (op)->in_vdef_list = false;

  /* Remove any virtual operands that were found.  */
  VEC_truncate (tree, build_vdefs, 0);
  VEC_truncate (tree, build_vuses, 0);

  /* Clear the loads and stores bitmaps.  */
  bitmap_clear (build_loads);
  bitmap_clear (build_stores);

  /* For each VDEF on the original statement, we want to create a
     VUSE of the VDEF result operand on the new statement.  */
  FOR_EACH_SSA_TREE_OPERAND (op, old_stmt, iter, SSA_OP_VDEF)
    append_vuse (op);

  finalize_ssa_stmt_operands (new_stmt);

  /* All uses in this fake stmt must not be in the immediate use lists.  */
  if (delink_imm_uses_p)
    FOR_EACH_SSA_USE_OPERAND (use_p, new_stmt, iter, SSA_OP_ALL_USES)
      delink_imm_use (use_p);
}


/* Swap operands EXP0 and EXP1 in statement STMT.  No attempt is done
   to test the validity of the swap operation.  */

void
swap_tree_operands (tree stmt, tree *exp0, tree *exp1)
{
  tree op0, op1;
  op0 = *exp0;
  op1 = *exp1;

  /* If the operand cache is active, attempt to preserve the relative
     positions of these two operands in their respective immediate use
     lists.  */
  if (ssa_operands_active () && op0 != op1)
    {
      use_optype_p use0, use1, ptr;
      use0 = use1 = NULL;

      /* Find the 2 operands in the cache, if they are there.  */
      for (ptr = USE_OPS (stmt); ptr; ptr = ptr->next)
	if (USE_OP_PTR (ptr)->use == exp0)
	  {
	    use0 = ptr;
	    break;
	  }

      for (ptr = USE_OPS (stmt); ptr; ptr = ptr->next)
	if (USE_OP_PTR (ptr)->use == exp1)
	  {
	    use1 = ptr;
	    break;
	  }

      /* If both uses don't have operand entries, there isn't much we can do
         at this point.  Presumably we don't need to worry about it.  */
      if (use0 && use1)
        {
	  tree *tmp = USE_OP_PTR (use1)->use;
	  USE_OP_PTR (use1)->use = USE_OP_PTR (use0)->use;
	  USE_OP_PTR (use0)->use = tmp;
	}
    }

  /* Now swap the data.  */
  *exp0 = op1;
  *exp1 = op0;
}


/* Add the base address of REF to the set *ADDRESSES_TAKEN.  If
   *ADDRESSES_TAKEN is NULL, a new set is created.  REF may be
   a single variable whose address has been taken or any other valid
   GIMPLE memory reference (structure reference, array, etc).  If the
   base address of REF is a decl that has sub-variables, also add all
   of its sub-variables.  */

void
add_to_addressable_set (tree ref, bitmap *addresses_taken)
{
  tree var;
  subvar_t svars;

  gcc_assert (addresses_taken);

  /* Note that it is *NOT OKAY* to use the target of a COMPONENT_REF
     as the only thing we take the address of.  If VAR is a structure,
     taking the address of a field means that the whole structure may
     be referenced using pointer arithmetic.  See PR 21407 and the
     ensuing mailing list discussion.  */
  var = get_base_address (ref);
  if (var && SSA_VAR_P (var))
    {
      if (*addresses_taken == NULL)
	*addresses_taken = BITMAP_GGC_ALLOC ();      
      
      if (var_can_have_subvars (var)
	  && (svars = get_subvars_for_var (var)))
	{
	  unsigned int i;
	  tree subvar;
	  for (i = 0; VEC_iterate (tree, svars, i, subvar); ++i)
	    {
	      bitmap_set_bit (*addresses_taken, DECL_UID (subvar));
	      TREE_ADDRESSABLE (subvar) = 1;
	    }
	}
      else
	{
	  bitmap_set_bit (*addresses_taken, DECL_UID (var));
	  TREE_ADDRESSABLE (var) = 1;
	}
    }
}


/* Scan the immediate_use list for VAR making sure its linked properly.
   Return TRUE if there is a problem and emit an error message to F.  */

bool
verify_imm_links (FILE *f, tree var)
{
  use_operand_p ptr, prev, list;
  int count;

  gcc_assert (TREE_CODE (var) == SSA_NAME);

  list = &(SSA_NAME_IMM_USE_NODE (var));
  gcc_assert (list->use == NULL);

  if (list->prev == NULL)
    {
      gcc_assert (list->next == NULL);
      return false;
    }

  prev = list;
  count = 0;
  for (ptr = list->next; ptr != list; )
    {
      if (prev != ptr->prev)
	goto error;
      
      if (ptr->use == NULL)
	goto error; /* 2 roots, or SAFE guard node.  */
      else if (*(ptr->use) != var)
	goto error;

      prev = ptr;
      ptr = ptr->next;

      /* Avoid infinite loops.  50,000,000 uses probably indicates a
	 problem.  */
      if (count++ > 50000000)
	goto error;
    }

  /* Verify list in the other direction.  */
  prev = list;
  for (ptr = list->prev; ptr != list; )
    {
      if (prev != ptr->next)
	goto error;
      prev = ptr;
      ptr = ptr->prev;
      if (count-- < 0)
	goto error;
    }

  if (count != 0)
    goto error;

  return false;

 error:
  if (ptr->stmt && stmt_modified_p (ptr->stmt))
    {
      fprintf (f, " STMT MODIFIED. - <%p> ", (void *)ptr->stmt);
      print_generic_stmt (f, ptr->stmt, TDF_SLIM);
    }
  fprintf (f, " IMM ERROR : (use_p : tree - %p:%p)", (void *)ptr, 
	   (void *)ptr->use);
  print_generic_expr (f, USE_FROM_PTR (ptr), TDF_SLIM);
  fprintf(f, "\n");
  return true;
}


/* Dump all the immediate uses to FILE.  */

void
dump_immediate_uses_for (FILE *file, tree var)
{
  imm_use_iterator iter;
  use_operand_p use_p;

  gcc_assert (var && TREE_CODE (var) == SSA_NAME);

  print_generic_expr (file, var, TDF_SLIM);
  fprintf (file, " : -->");
  if (has_zero_uses (var))
    fprintf (file, " no uses.\n");
  else
    if (has_single_use (var))
      fprintf (file, " single use.\n");
    else
      fprintf (file, "%d uses.\n", num_imm_uses (var));

  FOR_EACH_IMM_USE_FAST (use_p, iter, var)
    {
      if (use_p->stmt == NULL && use_p->use == NULL)
        fprintf (file, "***end of stmt iterator marker***\n");
      else
	if (!is_gimple_reg (USE_FROM_PTR (use_p)))
	  print_generic_stmt (file, USE_STMT (use_p), TDF_VOPS|TDF_MEMSYMS);
	else
	  print_generic_stmt (file, USE_STMT (use_p), TDF_SLIM);
    }
  fprintf(file, "\n");
}


/* Dump all the immediate uses to FILE.  */

void
dump_immediate_uses (FILE *file)
{
  tree var;
  unsigned int x;

  fprintf (file, "Immediate_uses: \n\n");
  for (x = 1; x < num_ssa_names; x++)
    {
      var = ssa_name(x);
      if (!var)
        continue;
      dump_immediate_uses_for (file, var);
    }
}


/* Dump def-use edges on stderr.  */

void
debug_immediate_uses (void)
{
  dump_immediate_uses (stderr);
}


/* Dump def-use edges on stderr.  */

void
debug_immediate_uses_for (tree var)
{
  dump_immediate_uses_for (stderr, var);
}


/* Create a new change buffer for the statement pointed by STMT_P and
   push the buffer into SCB_STACK.  Each change buffer
   records state information needed to determine what changed in the
   statement.  Mainly, this keeps track of symbols that may need to be
   put into SSA form, SSA name replacements and other information
   needed to keep the SSA form up to date.  */

void
push_stmt_changes (tree *stmt_p)
{
  tree stmt;
  scb_t buf;
  
  stmt = *stmt_p;

  /* It makes no sense to keep track of PHI nodes.  */
  if (TREE_CODE (stmt) == PHI_NODE)
    return;

  buf = XNEW (struct scb_d);
  memset (buf, 0, sizeof *buf);

  buf->stmt_p = stmt_p;

  if (stmt_references_memory_p (stmt))
    {
      tree op;
      ssa_op_iter i;

      FOR_EACH_SSA_TREE_OPERAND (op, stmt, i, SSA_OP_VUSE)
	{
	  tree sym = TREE_CODE (op) == SSA_NAME ? SSA_NAME_VAR (op) : op;
	  if (buf->loads == NULL)
	    buf->loads = BITMAP_ALLOC (NULL);
	  bitmap_set_bit (buf->loads, DECL_UID (sym));
	}

      FOR_EACH_SSA_TREE_OPERAND (op, stmt, i, SSA_OP_VDEF)
	{
	  tree sym = TREE_CODE (op) == SSA_NAME ? SSA_NAME_VAR (op) : op;
	  if (buf->stores == NULL)
	    buf->stores = BITMAP_ALLOC (NULL);
	  bitmap_set_bit (buf->stores, DECL_UID (sym));
	}
    }

  VEC_safe_push (scb_t, heap, scb_stack, buf);
}


/* Given two sets S1 and S2, mark the symbols that differ in S1 and S2
   for renaming.  The set to mark for renaming is (S1 & ~S2) | (S2 & ~S1).  */

static void
mark_difference_for_renaming (bitmap s1, bitmap s2)
{
  if (s1 == NULL && s2 == NULL)
    return;

  if (s1 && s2 == NULL)
    mark_set_for_renaming (s1);
  else if (s1 == NULL && s2)
    mark_set_for_renaming (s2);
  else if (!bitmap_equal_p (s1, s2))
    {
      bitmap t1 = BITMAP_ALLOC (NULL);
      bitmap t2 = BITMAP_ALLOC (NULL);

      bitmap_and_compl (t1, s1, s2);
      bitmap_and_compl (t2, s2, s1);
      bitmap_ior_into (t1, t2);
      mark_set_for_renaming (t1);

      BITMAP_FREE (t1);
      BITMAP_FREE (t2);
    }
}


/* Pop the top SCB from SCB_STACK and act on the differences between
   what was recorded by push_stmt_changes and the current state of
   the statement.  */

void
pop_stmt_changes (tree *stmt_p)
{
  tree op, stmt;
  ssa_op_iter iter;
  bitmap loads, stores;
  scb_t buf;

  stmt = *stmt_p;

  /* It makes no sense to keep track of PHI nodes.  */
  if (TREE_CODE (stmt) == PHI_NODE)
    return;

  buf = VEC_pop (scb_t, scb_stack);
  gcc_assert (stmt_p == buf->stmt_p);

  /* Force an operand re-scan on the statement and mark any newly
     exposed variables.  */
  update_stmt (stmt);

  /* Determine whether any memory symbols need to be renamed.  If the
     sets of loads and stores are different after the statement is
     modified, then the affected symbols need to be renamed.
     
     Note that it may be possible for the statement to not reference
     memory anymore, but we still need to act on the differences in
     the sets of symbols.  */
  loads = stores = NULL;
  if (stmt_references_memory_p (stmt))
    {
      tree op;
      ssa_op_iter i;

      FOR_EACH_SSA_TREE_OPERAND (op, stmt, i, SSA_OP_VUSE)
	{
	  tree sym = TREE_CODE (op) == SSA_NAME ? SSA_NAME_VAR (op) : op;
	  if (loads == NULL)
	    loads = BITMAP_ALLOC (NULL);
	  bitmap_set_bit (loads, DECL_UID (sym));
	}

      FOR_EACH_SSA_TREE_OPERAND (op, stmt, i, SSA_OP_VDEF)
	{
	  tree sym = TREE_CODE (op) == SSA_NAME ? SSA_NAME_VAR (op) : op;
	  if (stores == NULL)
	    stores = BITMAP_ALLOC (NULL);
	  bitmap_set_bit (stores, DECL_UID (sym));
	}
    }

  /* If LOADS is different from BUF->LOADS, the affected
     symbols need to be marked for renaming.  */
  mark_difference_for_renaming (loads, buf->loads);

  /* Similarly for STORES and BUF->STORES.  */
  mark_difference_for_renaming (stores, buf->stores);

  /* Mark all the naked GIMPLE register operands for renaming.  */
  FOR_EACH_SSA_TREE_OPERAND (op, stmt, iter, SSA_OP_DEF|SSA_OP_USE)
    if (DECL_P (op))
      mark_sym_for_renaming (op);

  /* FIXME, need to add more finalizers here.  Cleanup EH info,
     recompute invariants for address expressions, add
     SSA replacement mappings, etc.  For instance, given
     testsuite/gcc.c-torture/compile/pr16808.c, we fold a statement of
     the form:

	  # SMT.4_20 = VDEF <SMT.4_16>
	  D.1576_11 = 1.0e+0;

     So, the VDEF will disappear, but instead of marking SMT.4 for
     renaming it would be far more efficient to establish a
     replacement mapping that would replace every reference of
     SMT.4_20 with SMT.4_16.  */

  /* Free memory used by the buffer.  */
  BITMAP_FREE (buf->loads);
  BITMAP_FREE (buf->stores);
  BITMAP_FREE (loads);
  BITMAP_FREE (stores);
  buf->stmt_p = NULL;
  free (buf);
}


/* Discard the topmost change buffer from SCB_STACK.  This is useful
   when the caller realized that it did not actually modified the
   statement.  It avoids the expensive operand re-scan.  */

void
discard_stmt_changes (tree *stmt_p)
{
  scb_t buf;
  tree stmt;
  
  /* It makes no sense to keep track of PHI nodes.  */
  stmt = *stmt_p;
  if (TREE_CODE (stmt) == PHI_NODE)
    return;

  buf = VEC_pop (scb_t, scb_stack);
  gcc_assert (stmt_p == buf->stmt_p);

  /* Free memory used by the buffer.  */
  BITMAP_FREE (buf->loads);
  BITMAP_FREE (buf->stores);
  buf->stmt_p = NULL;
  free (buf);
}


/* Returns true if statement STMT may access memory.  */

bool
stmt_references_memory_p (tree stmt)
{
  if (!gimple_ssa_operands (cfun)->ops_active || TREE_CODE (stmt) == PHI_NODE)
    return false;

  return stmt_ann (stmt)->references_memory;
}
