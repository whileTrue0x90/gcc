/* Rtl-level loop invariant motion.
   Copyright (C) 2004, 2005 Free Software Foundation, Inc.
   
This file is part of GCC.
   
GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.
   
GCC is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.
   
You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/* This implements the loop invariant motion pass.  It is very simple
   (no calls, libcalls, etc.).  This should be sufficient to cleanup things like
   address arithmetics -- other more complicated invariants should be
   eliminated on tree level either in tree-ssa-loop-im.c or in tree-ssa-pre.c.
   
   We proceed loop by loop -- it is simpler than trying to handle things
   globally and should not lose much.  First we inspect all sets inside loop
   and create a dependency graph on insns (saying "to move this insn, you must
   also move the following insns").

   We then need to determine what to move.  We estimate the number of registers
   used and move as many invariants as possible while we still have enough free
   registers.  We prefer the expensive invariants.
   
   Then we move the selected invariants out of the loop, creating a new
   temporaries for them if necessary.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "hard-reg-set.h"
#include "obstack.h"
#include "basic-block.h"
#include "cfgloop.h"
#include "expr.h"
#include "output.h"
#include "function.h"
#include "flags.h"
#include "df.h"
#include "hashtab.h"

/* The data stored for the loop.  */

struct loop_data
{
  struct loop *outermost_exit;	/* The outermost exit of the loop.  */
  bool has_call;		/* True if the loop contains a call.  */
};

#define LOOP_DATA(LOOP) ((struct loop_data *) (LOOP)->aux)

/* The description of an use.  */

struct use
{
  rtx *pos;			/* Position of the use.  */
  rtx insn;			/* The insn in that the use occurs.  */

  struct use *next;		/* Next use in the list.  */
};

/* The description of a def.  */

struct def
{
  struct use *uses;		/* The list of uses that are uniquely reached
				   by it.  */
  unsigned n_uses;		/* Number of such uses.  */
  unsigned invno;		/* The corresponding invariant.  */
};

/* The data stored for each invariant.  */

struct invariant
{
  /* The number of the invariant.  */
  unsigned invno;

  /* The number of the invariant with the same value.  */
  unsigned eqto;

  /* If we moved the invariant out of the loop, the register that contains its
     value.  */
  rtx reg;

  /* The definition of the invariant.  */
  struct def *def;

  /* The insn in that it is defined.  */
  rtx insn;

  /* Whether it is always executed.  */
  bool always_executed;

  /* Whether to move the invariant.  */
  bool move;

  /* Cost of the invariant.  */
  unsigned cost;

  /* The invariants it depends on.  */
  bitmap depends_on;

  /* Used for detecting already visited invariants during determining
     costs of movements.  */
  unsigned stamp;
};

/* Entry for hash table of invariant expressions.  */

struct invariant_expr_entry
{
  /* The invariant.  */
  struct invariant *inv;

  /* Its value.  */
  rtx expr;

  /* Its mode.  */
  enum machine_mode mode;

  /* Its hash.  */
  hashval_t hash;
};

/* The actual stamp for marking already visited invariants during determining
   costs of movements.  */

static unsigned actual_stamp;

typedef struct invariant *invariant_p;

DEF_VEC_P(invariant_p);
DEF_VEC_ALLOC_P(invariant_p, heap);

/* The invariants.  */

static VEC(invariant_p,heap) *invariants;

/* The dataflow object.  */

static struct df *df;

/* Test for possibility of invariantness of X.  */

static bool
check_maybe_invariant (rtx x)
{
  enum rtx_code code = GET_CODE (x);
  int i, j;
  const char *fmt;

  switch (code)
    {
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
    case CONST:
    case LABEL_REF:
      return true;

    case PC:
    case CC0:
    case UNSPEC_VOLATILE:
    case CALL:
      return false;

    case REG:
      return true;

    case MEM:
      /* Load/store motion is done elsewhere.  ??? Perhaps also add it here?
	 It should not be hard, and might be faster than "elsewhere".  */

      /* Just handle the most trivial case where we load from an unchanging
	 location (most importantly, pic tables).  */
      if (MEM_READONLY_P (x))
	break;

      return false;

    case ASM_OPERANDS:
      /* Don't mess with insns declared volatile.  */
      if (MEM_VOLATILE_P (x))
	return false;
      break;

    default:
      break;
    }

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	{
	  if (!check_maybe_invariant (XEXP (x, i)))
	    return false;
	}
      else if (fmt[i] == 'E')
	{
	  for (j = 0; j < XVECLEN (x, i); j++)
	    if (!check_maybe_invariant (XVECEXP (x, i, j)))
	      return false;
	}
    }

  return true;
}

/* Returns the invariant definition for USE, or NULL if USE is not
   invariant.  */

static struct invariant *
invariant_for_use (struct ref *use)
{
  struct df_link *defs;
  struct ref *def;
  basic_block bb = BLOCK_FOR_INSN (use->insn), def_bb;

  defs = DF_REF_CHAIN (use);
  if (!defs || defs->next)
    return NULL;
  def = defs->ref;
  if (!DF_REF_DATA (def))
    return NULL;

  def_bb = DF_REF_BB (def);
  if (!dominated_by_p (CDI_DOMINATORS, bb, def_bb))
    return NULL;
  return DF_REF_DATA (def);
}

/* Computes hash value for invariant expression X in INSN.  */

static hashval_t
hash_invariant_expr_1 (rtx insn, rtx x)
{
  enum rtx_code code = GET_CODE (x);
  int i, j;
  const char *fmt;
  hashval_t val = code;
  int do_not_record_p;
  struct ref *use;
  struct invariant *inv;

  switch (code)
    {
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
    case CONST:
    case LABEL_REF:
      return hash_rtx (x, GET_MODE (x), &do_not_record_p, NULL, false);

    case REG:
      use = df_find_use (df, insn, x);
      if (!use)
	return hash_rtx (x, GET_MODE (x), &do_not_record_p, NULL, false);
      inv = invariant_for_use (use);
      if (!inv)
	return hash_rtx (x, GET_MODE (x), &do_not_record_p, NULL, false);

      gcc_assert (inv->eqto != ~0u);
      return inv->eqto;

    default:
      break;
    }

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	val ^= hash_invariant_expr_1 (insn, XEXP (x, i));
      else if (fmt[i] == 'E')
	{
	  for (j = 0; j < XVECLEN (x, i); j++)
	    val ^= hash_invariant_expr_1 (insn, XVECEXP (x, i, j));
	}
    }

  return val;
}

/* Compares invariant expressions E1 and E2 in modes MODE1 and MODE2 used in
   insns INSN1 and INSN2.  */

static bool
invariant_expr_equal_p (rtx insn1, rtx e1, enum machine_mode mode1,
			rtx insn2, rtx e2, enum machine_mode mode2)
{
  enum rtx_code code = GET_CODE (e1);
  int i, j;
  const char *fmt;
  struct ref *use1, *use2;
  struct invariant *inv1 = NULL, *inv2 = NULL;
  rtx sub1, sub2;
  enum machine_mode submode1, submode2;

  gcc_assert (GET_MODE (e1) == VOIDmode || GET_MODE (e1) == mode1);
  gcc_assert (GET_MODE (e2) == VOIDmode || GET_MODE (e2) == mode2);

  if (code != GET_CODE (e2) || mode1 != mode2)
    return false;

  switch (code)
    {
    case CONST_INT:
    case CONST_DOUBLE:
    case SYMBOL_REF:
    case CONST:
    case LABEL_REF:
      return rtx_equal_p (e1, e2);

    case REG:
      use1 = df_find_use (df, insn1, e1);
      use2 = df_find_use (df, insn2, e2);
      if (use1)
	inv1 = invariant_for_use (use1);
      if (use2)
	inv2 = invariant_for_use (use2);

      if (!inv1 && !inv2)
	return rtx_equal_p (e1, e2);

      if (!inv1 || !inv2)
	return false;
      
      gcc_assert (inv1->eqto != ~0u);
      gcc_assert (inv2->eqto != ~0u);
      return inv1->eqto == inv2->eqto;

    default:
      break;
    }

  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	{
	  sub1 = XEXP (e1, i);
	  sub2 = XEXP (e2, i);
	  submode1 = GET_MODE (sub1);
	  if (submode1 == VOIDmode)
	    submode1 = mode1;
	  submode2 = GET_MODE (sub2);
	  if (submode2 == VOIDmode)
	    submode2 = mode2;

	  if (!invariant_expr_equal_p (insn1, sub1, submode1,
				       insn2, sub2, submode2))
	    return false;
	}

      else if (fmt[i] == 'E')
	{
	  if (XVECLEN (e1, i) != XVECLEN (e2, i))
	    return false;

	  for (j = 0; j < XVECLEN (e1, i); j++)
	    {
	      sub1 = XVECEXP (e1, i, j);
	      sub2 = XVECEXP (e2, i, j);
	      submode1 = GET_MODE (sub1);
	      if (submode1 == VOIDmode)
		submode1 = mode1;
	      submode2 = GET_MODE (sub2);
	      if (submode2 == VOIDmode)
		submode2 = mode2;

	      if (!invariant_expr_equal_p (insn1, sub1, submode1,
					   insn2, sub2, submode2))
		return false;
	    }
	}
    }

  return true;
}

/* Returns hash value for invariant expression entry E.  */

static hashval_t
hash_invariant_expr (const void *e)
{
  const struct invariant_expr_entry *entry = e;

  return entry->hash;
}

/* Compares invariant expression entries E1 and E2.  */

static int
eq_invariant_expr (const void *e1, const void *e2)
{
  const struct invariant_expr_entry *entry1 = e1;
  const struct invariant_expr_entry *entry2 = e2;

  return invariant_expr_equal_p (entry1->inv->insn, entry1->expr, entry1->mode,
				 entry2->inv->insn, entry2->expr, entry2->mode);
}

/* Checks whether invariant with value EXPR in machine mode MODE is
   recorded in EQ.  If this is the case, return the invariant.  Otherwise
   insert INV to the table for this expression and return INV.  */

static struct invariant *
find_or_insert_inv (htab_t eq, rtx expr, enum machine_mode mode,
		    struct invariant *inv)
{
  hashval_t hash = hash_invariant_expr_1 (inv->insn, expr);
  struct invariant_expr_entry *entry;
  struct invariant_expr_entry pentry;
  PTR *slot;
  
  pentry.expr = expr;
  pentry.inv = inv;
  pentry.mode = mode;
  slot = htab_find_slot_with_hash (eq, &pentry, hash, INSERT);
  entry = *slot;

  if (entry)
    return entry->inv;

  entry = xmalloc (sizeof (struct invariant_expr_entry));
  entry->inv = inv;
  entry->expr = expr;
  entry->mode = mode;
  entry->hash = hash;
  *slot = entry;

  return inv;
}

/* Finds invariants identical to INV and records the equivalence.  EQ is the
   hash table of the invariants.  */

static void
find_identical_invariants (htab_t eq, struct invariant *inv)
{
  unsigned depno;
  bitmap_iterator bi;
  struct invariant *dep;
  rtx expr, set;
  enum machine_mode mode;

  if (inv->eqto != ~0u)
    return;

  EXECUTE_IF_SET_IN_BITMAP (inv->depends_on, 0, depno, bi)
    {
      dep = VEC_index (invariant_p, invariants, depno);
      find_identical_invariants (eq, dep);
    }

  set = single_set (inv->insn);
  expr = SET_SRC (set);
  mode = GET_MODE (expr);
  if (mode == VOIDmode)
    mode = GET_MODE (SET_DEST (set));
  inv->eqto = find_or_insert_inv (eq, expr, mode, inv)->invno;

  if (dump_file && inv->eqto != inv->invno)
    fprintf (dump_file,
	     "Invariant %d is equivalent to invariant %d.\n ",
	     inv->invno, inv->eqto);
}

/* Find invariants with the same value and record the equivalences.  */

static void
merge_identical_invariants (void)
{
  unsigned i;
  struct invariant *inv;
  htab_t eq = htab_create (VEC_length (invariant_p, invariants),
			   hash_invariant_expr, eq_invariant_expr, free);

  for (i = 0; VEC_iterate (invariant_p, invariants, i, inv); i++)
    find_identical_invariants (eq, inv);

  htab_delete (eq);
}

/* Determines the basic blocks inside LOOP that are always executed and
   stores their bitmap to ALWAYS_REACHED.  MAY_EXIT is a bitmap of
   basic blocks that may either exit the loop, or contain the call that
   does not have to return.  BODY is body of the loop obtained by
   get_loop_body_in_dom_order.  */

static void
compute_always_reached (struct loop *loop, basic_block *body,
			bitmap may_exit, bitmap always_reached)
{
  unsigned i;

  for (i = 0; i < loop->num_nodes; i++)
    {
      if (dominated_by_p (CDI_DOMINATORS, loop->latch, body[i]))
	bitmap_set_bit (always_reached, i);

      if (bitmap_bit_p (may_exit, i))
	return;
    }
}

/* Finds exits out of the LOOP with body BODY.  Marks blocks in that we may
   exit the loop by cfg edge to HAS_EXIT and MAY_EXIT.  In MAY_EXIT
   additionally mark blocks that may exit due to a call.  */

static void
find_exits (struct loop *loop, basic_block *body,
	    bitmap may_exit, bitmap has_exit)
{
  unsigned i;
  edge_iterator ei;
  edge e;
  struct loop *outermost_exit = loop, *aexit;
  bool has_call = false;
  rtx insn;

  for (i = 0; i < loop->num_nodes; i++)
    {
      if (body[i]->loop_father == loop)
	{
	  FOR_BB_INSNS (body[i], insn)
	    {
	      if (CALL_P (insn)
		  && !CONST_OR_PURE_CALL_P (insn))
		{
		  has_call = true;
		  bitmap_set_bit (may_exit, i);
		  break;
		}
	    }

	  FOR_EACH_EDGE (e, ei, body[i]->succs)
	    {
	      if (flow_bb_inside_loop_p (loop, e->dest))
		continue;

	      bitmap_set_bit (may_exit, i);
	      bitmap_set_bit (has_exit, i);
	      outermost_exit = find_common_loop (outermost_exit,
						 e->dest->loop_father);
	    }
	  continue;
	}
     
      /* Use the data stored for the subloop to decide whether we may exit
	 through it.  It is sufficient to do this for header of the loop,
	 as other basic blocks inside it must be dominated by it.  */
      if (body[i]->loop_father->header != body[i])
	continue;

      if (LOOP_DATA (body[i]->loop_father)->has_call)
	{
	  has_call = true;
	  bitmap_set_bit (may_exit, i);
	}
      aexit = LOOP_DATA (body[i]->loop_father)->outermost_exit;
      if (aexit != loop)
	{
	  bitmap_set_bit (may_exit, i);
	  bitmap_set_bit (has_exit, i);

	  if (flow_loop_nested_p (aexit, outermost_exit))
	    outermost_exit = aexit;
	}
    }

  loop->aux = xcalloc (1, sizeof (struct loop_data));
  LOOP_DATA (loop)->outermost_exit = outermost_exit;
  LOOP_DATA (loop)->has_call = has_call;
}

/* Check whether we may assign a value to X from a register.  */

static bool
may_assign_reg_p (rtx x)
{
  return can_copy_p (GET_MODE (x));
}

/* Finds definitions that may correspond to invariants in LOOP with body
   BODY.  */

static void
find_defs (struct loop *loop, basic_block *body)
{
  unsigned i;
  bitmap blocks = BITMAP_ALLOC (NULL);

  for (i = 0; i < loop->num_nodes; i++)
    bitmap_set_bit (blocks, body[i]->index);

  df_analyze_subcfg (df, blocks, DF_UD_CHAIN | DF_HARD_REGS | DF_EQUIV_NOTES);
  BITMAP_FREE (blocks);
}

/* Creates a new invariant for definition DEF in INSN, depending on invariants
   in DEPENDS_ON.  ALWAYS_EXECUTED is true if the insn is always executed,
   unless the program ends due to a function call.  The newly created invariant
   is returned.  */

static struct invariant *
create_new_invariant (struct def *def, rtx insn, bitmap depends_on,
		      bool always_executed)
{
  struct invariant *inv = xmalloc (sizeof (struct invariant));
  rtx set = single_set (insn);

  inv->def = def;
  inv->always_executed = always_executed;
  inv->depends_on = depends_on;

  /* If the set is simple, usually by moving it we move the whole store out of
     the loop.  Otherwise we save only cost of the computation.  */
  if (def)
    inv->cost = rtx_cost (set, SET);
  else
    inv->cost = rtx_cost (SET_SRC (set), SET);

  inv->move = false;
  inv->reg = NULL_RTX;
  inv->stamp = 0;
  inv->insn = insn;

  inv->invno = VEC_length (invariant_p, invariants);
  inv->eqto = ~0u;
  if (def)
    def->invno = inv->invno;
  VEC_safe_push (invariant_p, heap, invariants, inv);

  if (dump_file)
    {
      fprintf (dump_file,
	       "Set in insn %d is invariant (%d), cost %d, depends on ",
	       INSN_UID (insn), inv->invno, inv->cost);
      dump_bitmap (dump_file, inv->depends_on);
    }

  return inv;
}

/* Record USE at DEF.  */

static void
record_use (struct def *def, rtx *use, rtx insn)
{
  struct use *u = xmalloc (sizeof (struct use));

  if (GET_CODE (*use) == SUBREG)
    use = &SUBREG_REG (*use);
  gcc_assert (REG_P (*use));

  u->pos = use;
  u->insn = insn;
  u->next = def->uses;
  def->uses = u;
  def->n_uses++;
}

/* Finds the invariants INSN depends on and store them to the DEPENDS_ON
   bitmap.  */

static bool
check_dependencies (rtx insn, bitmap depends_on)
{
  struct df_link *uses, *defs;
  struct ref *use, *def;
  basic_block bb = BLOCK_FOR_INSN (insn), def_bb;
  struct def *def_data;
  struct invariant *inv;
  
  for (uses = DF_INSN_USES (df, insn); uses; uses = uses->next)
    {
      use = uses->ref;

      defs = DF_REF_CHAIN (use);
      if (!defs)
	continue;

      if (defs->next)
	return false;

      def = defs->ref;
      inv = DF_REF_DATA (def);
      if (!inv)
	return false;

      def_data = inv->def;
      gcc_assert (def_data != NULL);

      def_bb = DF_REF_BB (def);
      /* Note that in case bb == def_bb, we know that the definition dominates
	 insn, because def has DF_REF_DATA defined and we process the insns
	 in the basic block bb sequentially.  */
      if (!dominated_by_p (CDI_DOMINATORS, bb, def_bb))
	return false;

      bitmap_set_bit (depends_on, def_data->invno);
    }

  return true;
}

/* Finds invariant in INSN.  ALWAYS_REACHED is true if the insn is always
   executed.  ALWAYS_EXECUTED is true if the insn is always executed,
   unless the program ends due to a function call.  */

static void
find_invariant_insn (rtx insn, bool always_reached, bool always_executed)
{
  struct ref *ref;
  struct def *def;
  bitmap depends_on;
  rtx set, dest;
  bool simple = true;
  struct invariant *inv;

  /* Until we get rid of LIBCALLS.  */
  if (find_reg_note (insn, REG_RETVAL, NULL_RTX)
      || find_reg_note (insn, REG_LIBCALL, NULL_RTX)
      || find_reg_note (insn, REG_NO_CONFLICT, NULL_RTX))
    return;
      
  set = single_set (insn);
  if (!set)
    return;
  dest = SET_DEST (set);

  if (!REG_P (dest)
      || HARD_REGISTER_P (dest))
    simple = false;

  if (!check_maybe_invariant (SET_SRC (set))
      || !may_assign_reg_p (SET_DEST (set)))
    return;

  if (may_trap_p (PATTERN (insn)))
    {
      if (!always_reached)
	return;

      /* Unless the exceptions are handled, the behavior is undefined
 	 if the trap occurs.  */
      if (flag_non_call_exceptions)
	return;
    }

  depends_on = BITMAP_ALLOC (NULL);
  if (!check_dependencies (insn, depends_on))
    {
      BITMAP_FREE (depends_on);
      return;
    }

  if (simple)
    def = xcalloc (1, sizeof (struct def));
  else
    def = NULL;

  inv = create_new_invariant (def, insn, depends_on, always_executed);

  if (simple)
    {
      ref = df_find_def (df, insn, dest);
      DF_REF_DATA (ref) = inv;
    }
}

/* Record registers used in INSN that have an unique invariant definition.  */

static void
record_uses (rtx insn)
{
  struct df_link *uses;
  struct ref *use;
  struct invariant *inv;
  
  for (uses = DF_INSN_USES (df, insn); uses; uses = uses->next)
    {
      use = uses->ref;
      inv = invariant_for_use (use);
      if (inv)
	record_use (inv->def, DF_REF_LOC (use), DF_REF_INSN (use));
    }
}

/* Finds invariants in INSN.  ALWAYS_REACHED is true if the insn is always
   executed.  ALWAYS_EXECUTED is true if the insn is always executed,
   unless the program ends due to a function call.  */

static void
find_invariants_insn (rtx insn, bool always_reached, bool always_executed)
{
  find_invariant_insn (insn, always_reached, always_executed);
  record_uses (insn);
}

/* Finds invariants in basic block BB.  ALWAYS_REACHED is true if the
   basic block is always executed.  ALWAYS_EXECUTED is true if the basic
   block is always executed, unless the program ends due to a function
   call.  */

static void
find_invariants_bb (basic_block bb, bool always_reached, bool always_executed)
{
  rtx insn;

  FOR_BB_INSNS (bb, insn)
    {
      if (!INSN_P (insn))
	continue;

      find_invariants_insn (insn, always_reached, always_executed);

      if (always_reached
	  && CALL_P (insn)
	  && !CONST_OR_PURE_CALL_P (insn))
	always_reached = false;
    }
}

/* Finds invariants in LOOP with body BODY.  ALWAYS_REACHED is the bitmap of
   basic blocks in BODY that are always executed.  ALWAYS_EXECUTED is the
   bitmap of basic blocks in BODY that are always executed unless the program
   ends due to a function call.  */

static void
find_invariants_body (struct loop *loop, basic_block *body,
		      bitmap always_reached, bitmap always_executed)
{
  unsigned i;

  for (i = 0; i < loop->num_nodes; i++)
    find_invariants_bb (body[i],
			bitmap_bit_p (always_reached, i),
			bitmap_bit_p (always_executed, i));
}

/* Finds invariants in LOOP.  */

static void
find_invariants (struct loop *loop)
{
  bitmap may_exit = BITMAP_ALLOC (NULL);
  bitmap always_reached = BITMAP_ALLOC (NULL);
  bitmap has_exit = BITMAP_ALLOC (NULL);
  bitmap always_executed = BITMAP_ALLOC (NULL);
  basic_block *body = get_loop_body_in_dom_order (loop);

  find_exits (loop, body, may_exit, has_exit);
  compute_always_reached (loop, body, may_exit, always_reached);
  compute_always_reached (loop, body, has_exit, always_executed);

  find_defs (loop, body);
  find_invariants_body (loop, body, always_reached, always_executed);
  merge_identical_invariants ();

  BITMAP_FREE (always_reached);
  BITMAP_FREE (always_executed);
  BITMAP_FREE (may_exit);
  BITMAP_FREE (has_exit);
  free (body);
}

/* Frees a list of uses USE.  */

static void
free_use_list (struct use *use)
{
  struct use *next;

  for (; use; use = next)
    {
      next = use->next;
      free (use);
    }
}

/* Calculates cost and number of registers needed for moving invariant INV
   out of the loop and stores them to *COST and *REGS_NEEDED.  */

static void
get_inv_cost (struct invariant *inv, int *comp_cost, unsigned *regs_needed)
{
  int acomp_cost;
  unsigned aregs_needed;
  unsigned depno;
  struct invariant *dep;
  bitmap_iterator bi;

  /* Find the representative of the class of the equivalent invariants.  */
  inv = VEC_index (invariant_p, invariants, inv->eqto);

  *comp_cost = 0;
  *regs_needed = 0;
  if (inv->move
      || inv->stamp == actual_stamp)
    return;
  inv->stamp = actual_stamp;

  (*regs_needed)++;
  (*comp_cost) += inv->cost;

  EXECUTE_IF_SET_IN_BITMAP (inv->depends_on, 0, depno, bi)
    {
      dep = VEC_index (invariant_p, invariants, depno);

      get_inv_cost (dep, &acomp_cost, &aregs_needed);

      if (aregs_needed
	  /* We need to check always_executed, since if the original value of
	     the invariant may be preserved, we may need to keep it in a
	     separate register.  TODO check whether the register has an
	     use outside of the loop.  */
	  && dep->always_executed
	  && !dep->def->uses->next)
	{
	  /* If this is a single use, after moving the dependency we will not
	     need a new register.  */
	  aregs_needed--;
	}

      (*regs_needed) += aregs_needed;
      (*comp_cost) += acomp_cost;
    }
}

/* Calculates gain for eliminating invariant INV.  REGS_USED is the number
   of registers used in the loop, N_INV_USES is the number of uses of
   invariants, NEW_REGS is the number of new variables already added due to
   the invariant motion.  The number of registers needed for it is stored in
   *REGS_NEEDED.  */

static int
gain_for_invariant (struct invariant *inv, unsigned *regs_needed,
		    unsigned new_regs, unsigned regs_used, unsigned n_inv_uses)
{
  int comp_cost, size_cost;

  get_inv_cost (inv, &comp_cost, regs_needed);
  actual_stamp++;

  size_cost = (global_cost_for_size (new_regs + *regs_needed,
				     regs_used, n_inv_uses)
	       - global_cost_for_size (new_regs, regs_used, n_inv_uses));

  return comp_cost - size_cost;
}

/* Finds invariant with best gain for moving.  Returns the gain, stores
   the invariant in *BEST and number of registers needed for it to
   *REGS_NEEDED.  REGS_USED is the number of registers used in
   the loop, N_INV_USES is the number of uses of invariants.  NEW_REGS
   is the number of new variables already added due to invariant motion.  */

static int
best_gain_for_invariant (struct invariant **best, unsigned *regs_needed,
			 unsigned new_regs, unsigned regs_used,
			 unsigned n_inv_uses)
{
  struct invariant *inv;
  int gain = 0, again;
  unsigned aregs_needed, invno;

  for (invno = 0; VEC_iterate (invariant_p, invariants, invno, inv); invno++)
    {
      if (inv->move)
	continue;

      /* Only consider the "representatives" of equivalent invariants.  */
      if (inv->eqto != inv->invno)
	continue;

      again = gain_for_invariant (inv, &aregs_needed,
				  new_regs, regs_used, n_inv_uses);
      if (again > gain)
	{
	  gain = again;
	  *best = inv;
	  *regs_needed = aregs_needed;
	}
    }

  return gain;
}

/* Marks invariant INVNO and all its dependencies for moving.  */

static void
set_move_mark (unsigned invno)
{
  struct invariant *inv = VEC_index (invariant_p, invariants, invno);
  bitmap_iterator bi;

  /* Find the representative of the class of the equivalent invariants.  */
  inv = VEC_index (invariant_p, invariants, inv->eqto);

  if (inv->move)
    return;
  inv->move = true;

  if (dump_file)
    fprintf (dump_file, "Decided to move invariant %d\n", invno);

  EXECUTE_IF_SET_IN_BITMAP (inv->depends_on, 0, invno, bi)
    {
      set_move_mark (invno);
    }
}

/* Determines which invariants to move.  */

static void
find_invariants_to_move (void)
{
  unsigned i, regs_used, n_inv_uses, regs_needed = 0, new_regs;
  struct invariant *inv = NULL;

  if (!VEC_length (invariant_p, invariants))
    return;

  /* Now something slightly more involved.  First estimate the number of used
     registers.  */
  n_inv_uses = 0;

  /* We do not really do a good job in this estimation; put some initial bound
     here to stand for induction variables etc. that we do not detect.  */
  regs_used = 2;

  for (i = 0; i < df->n_regs; i++)
    {
      if (!DF_REGNO_FIRST_DEF (df, i) && DF_REGNO_LAST_USE (df, i))
	{
	  /* This is a value that is used but not changed inside loop.  */
	  regs_used++;
	}
    }

  for (i = 0; VEC_iterate (invariant_p, invariants, i, inv); i++)
    {
      if (inv->def)
	n_inv_uses += inv->def->n_uses;
    }

  new_regs = 0;
  while (best_gain_for_invariant (&inv, &regs_needed,
				  new_regs, regs_used, n_inv_uses) > 0)
    {
      set_move_mark (inv->invno);
      new_regs += regs_needed;
    }
}

/* Move invariant INVNO out of the LOOP.  */

static void
move_invariant_reg (struct loop *loop, unsigned invno)
{
  struct invariant *inv = VEC_index (invariant_p, invariants, invno);
  struct invariant *repr = VEC_index (invariant_p, invariants, inv->eqto);
  unsigned i;
  basic_block preheader = loop_preheader_edge (loop)->src;
  rtx reg, set;
  struct use *use;
  bitmap_iterator bi;

  if (inv->reg
      || !repr->move)
    return;

  /* If this is a representative of the class of equivalent invariants,
     really move the invariant.  Otherwise just replace its use with
     the register used for the representative.  */
  if (inv == repr)
    {
      if (inv->depends_on)
	{
	  EXECUTE_IF_SET_IN_BITMAP (inv->depends_on, 0, i, bi)
	    {
	      move_invariant_reg (loop, i);
	    }
	}
  
      /* Move the set out of the loop.  If the set is always executed (we could
	 omit this condition if we know that the register is unused outside of the
	 loop, but it does not seem worth finding out) and it has no uses that
	 would not be dominated by it, we may just move it (TODO).  Otherwise we
	 need to create a temporary register.  */
      set = single_set (inv->insn);
      reg = gen_reg_rtx (GET_MODE (SET_DEST (set)));
      df_pattern_emit_after (df, gen_move_insn (SET_DEST (set), reg),
			     BLOCK_FOR_INSN (inv->insn), inv->insn);
      SET_DEST (set) = reg;
      reorder_insns (inv->insn, inv->insn, BB_END (preheader));
      df_insn_modify (df, preheader, inv->insn);
    }
  else
    {
      move_invariant_reg (loop, repr->invno);
      reg = repr->reg;
      set = single_set (inv->insn);
      df_pattern_emit_after (df, gen_move_insn (SET_DEST (set), reg),
			     BLOCK_FOR_INSN (inv->insn), inv->insn);
      df_insn_delete (df, BLOCK_FOR_INSN (inv->insn), inv->insn);
    }

  inv->reg = reg;

  /* Replace the uses we know to be dominated.  It saves work for copy
     propagation, and also it is necessary so that dependent invariants
     are computed right.  */
  if (inv->def)
    {
      for (use = inv->def->uses; use; use = use->next)
	{
	  *use->pos = reg;
	  df_insn_modify (df, BLOCK_FOR_INSN (use->insn), use->insn);
	}
    }
}

/* Move selected invariant out of the LOOP.  Newly created regs are marked
   in TEMPORARY_REGS.  */

static void
move_invariants (struct loop *loop)
{
  struct invariant *inv;
  unsigned i;

  for (i = 0; VEC_iterate (invariant_p, invariants, i, inv); i++)
    move_invariant_reg (loop, i);
}

/* Initializes invariant motion data.  */

static void
init_inv_motion_data (void)
{
  actual_stamp = 1;

  invariants = VEC_alloc (invariant_p, heap, 100);
}

/* Frees the data allocated by invariant motion.  */

static void
free_inv_motion_data (void)
{
  unsigned i;
  struct def *def;
  struct invariant *inv;

  for (i = 0; i < df->n_defs; i++)
    {
      if (!df->defs[i])
	continue;

      inv = DF_REF_DATA (df->defs[i]);
      if (!inv)
	continue;
      def = inv->def;
      gcc_assert (def != NULL);

      free_use_list (def->uses);
      free (def);
      DF_REF_DATA (df->defs[i]) = NULL;
    }

  for (i = 0; VEC_iterate (invariant_p, invariants, i, inv); i++)
    {
      BITMAP_FREE (inv->depends_on);
      free (inv);
    }
  VEC_free (invariant_p, heap, invariants);
}

/* Move the invariants out of the LOOP.  */

static void
move_single_loop_invariants (struct loop *loop)
{
  init_inv_motion_data ();

  find_invariants (loop);
  find_invariants_to_move ();
  move_invariants (loop);

  free_inv_motion_data ();
}

/* Releases the auxiliary data for LOOP.  */

static void
free_loop_data (struct loop *loop)
{
  struct loop_data *data = LOOP_DATA (loop);

  free (data);
  loop->aux = NULL;
}

/* Move the invariants out of the LOOPS.  */

void
move_loop_invariants (struct loops *loops)
{
  struct loop *loop;
  unsigned i;

  df = df_init ();

  /* Process the loops, innermost first.  */
  loop = loops->tree_root;
  while (loop->inner)
    loop = loop->inner;

  while (loop != loops->tree_root)
    {
      move_single_loop_invariants (loop);

      if (loop->next)
	{
	  loop = loop->next;
	  while (loop->inner)
	    loop = loop->inner;
	}
      else
	loop = loop->outer;
    }

  for (i = 1; i < loops->num; i++)
    if (loops->parray[i])
      free_loop_data (loops->parray[i]);

  df_finish (df);
}
