/* Straight-line strength reduction.
   Copyright (C) 2012  Free Software Foundation, Inc.
   Contributed by Bill Schmidt, IBM <wschmidt@linux.ibm.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

/* There are many algorithms for performing strength reduction on
   loops.  This is not one of them.  IVOPTS handles strength reduction
   of induction variables just fine.  This pass is intended to pick
   up the crumbs it leaves behind, by considering opportunities for
   strength reduction along dominator paths.

   Strength reduction will be implemented in four stages, gradually
   adding more complex candidates:

   1) Explicit multiplies, known constant multipliers, no
      conditional increments. (complete)
   2) Explicit multiplies, unknown constant multipliers,
      no conditional increments. (data gathering complete,
      replacements pending)
   3) Implicit multiplies in addressing expressions. (complete)
   4) Explicit multiplies, conditional increments. (pending)

   It would also be possible to apply strength reduction to divisions
   and modulos, but such opportunities are relatively uncommon.

   Strength reduction is also currently restricted to integer operations.
   If desired, it could be extended to floating-point operations under
   control of something like -funsafe-math-optimizations.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "gimple.h"
#include "basic-block.h"
#include "tree-pass.h"
#include "cfgloop.h"
#include "gimple-pretty-print.h"
#include "tree-flow.h"
#include "domwalk.h"
#include "pointer-set.h"
#include "expmed.h"

/* Information about a strength reduction candidate.  Each statement
   in the candidate table represents an expression of one of the
   following forms (the special case of CAND_REF will be described
   later):

   (CAND_MULT)  S1:  X = (B + i) * S
   (CAND_ADD)   S1:  X = B + (i * S)

   Here X and B are SSA names, i is an integer constant, and S is
   either an SSA name or a constant.  We call B the "base," i the
   "index", and S the "stride."

   Any statement S0 that dominates S1 and is of the form:

   (CAND_MULT)  S0:  Y = (B + i') * S
   (CAND_ADD)   S0:  Y = B + (i' * S)

   is called a "basis" for S1.  In both cases, S1 may be replaced by
   
                S1':  X = Y + (i - i') * S,

   where (i - i') * S is folded to the extent possible.

   All gimple statements are visited in dominator order, and each
   statement that may contribute to one of the forms of S1 above is
   given at least one entry in the candidate table.  Such statements
   include addition, pointer addition, subtraction, multiplication,
   negation, copies, and nontrivial type casts.  If a statement may
   represent more than one expression of the forms of S1 above, 
   multiple "interpretations" are stored in the table and chained
   together.  Examples:

   * An add of two SSA names may treat either operand as the base.
   * A multiply of two SSA names, likewise.
   * A copy or cast may be thought of as either a CAND_MULT with
     i = 0 and S = 1, or as a CAND_ADD with i = 0 or S = 0.

   Candidate records are allocated from an obstack.  They are addressed
   both from a hash table keyed on S1, and from a vector of candidate
   pointers arranged in predominator order.

   Opportunity note
   ----------------
   Currently we don't recognize:

     S0: Y = (S * i') - B
     S1: X = (S * i) - B

   as a strength reduction opportunity, even though this S1 would
   also be replaceable by the S1' above.  This can be added if it
   comes up in practice.

   Strength reduction in addressing
   --------------------------------
   There is another kind of candidate known as CAND_REF.  A CAND_REF
   describes a statement containing a memory reference having 
   complex addressing that might benefit from strength reduction.
   Specifically, we are interested in references for which 
   get_inner_reference returns a base address, offset, and bitpos as
   follows:

     base:    MEM_REF (T1, C1)
     offset:  MULT_EXPR (PLUS_EXPR (T2, C2), C3)
     bitpos:  C4 * BITS_PER_UNIT

   Here T1 and T2 are arbitrary trees, and C1, C2, C3, C4 are 
   arbitrary integer constants.  Note that C2 may be zero, in which
   case the offset will be MULT_EXPR (T2, C3).

   When this pattern is recognized, the original memory reference
   can be replaced with:

     MEM_REF (POINTER_PLUS_EXPR (T1, MULT_EXPR (T2, C3)),
              C1 + (C2 * C3) + C4)

   which distributes the multiply to allow constant folding.  When
   two or more addressing expressions can be represented by MEM_REFs
   of this form, differing only in the constants C1, C2, and C4,
   making this substitution produces more efficient addressing during
   the RTL phases.  When there are not at least two expressions with
   the same values of T1, T2, and C3, there is nothing to be gained
   by the replacement.

   Strength reduction of CAND_REFs uses the same infrastructure as
   that used by CAND_MULTs and CAND_ADDs.  We record T1 in the base (B)
   field, MULT_EXPR (T2, C3) in the stride (S) field, and 
   C1 + (C2 * C3) + C4 in the index (i) field.  A basis for a CAND_REF
   is thus another CAND_REF with the same B and S values.  When at 
   least two CAND_REFs are chained together using the basis relation,
   each of them is replaced as above, resulting in improved code
   generation for addressing.  */


/* Index into the candidate vector, offset by 1.  VECs are zero-based,
   while cand_idx's are one-based, with zero indicating null.  */
typedef unsigned cand_idx;

/* The kind of candidate.  */
enum cand_kind
{
  CAND_MULT,
  CAND_ADD,
  CAND_REF
};

struct slsr_cand_d
{
  /* The candidate statement S1.  */
  gimple cand_stmt;

  /* The base expression B:  often an SSA name, but not always.  */
  tree base_expr;

  /* The stride S.  */
  tree stride;

  /* The index constant i.  */
  double_int index;

  /* The type of the candidate.  This is normally the type of base_expr,
     but casts may have occurred when combining feeding instructions.
     A candidate can only be a basis for candidates of the same final type.
     (For CAND_REFs, this is the type to be used for operand 1 of the
     replacement MEM_REF.)  */
  tree cand_type;

  /* The kind of candidate (CAND_MULT, etc.).  */
  enum cand_kind kind;

  /* Index of this candidate in the candidate vector.  */
  cand_idx cand_num;

  /* Index of the next candidate record for the same statement.
     A statement may be useful in more than one way (e.g., due to
     commutativity).  So we can have multiple "interpretations"
     of a statement.  */
  cand_idx next_interp;

  /* Index of the basis statement S0, if any, in the candidate vector.  */
  cand_idx basis;

  /* First candidate for which this candidate is a basis, if one exists.  */
  cand_idx dependent;

  /* Next candidate having the same basis as this one.  */
  cand_idx sibling;

  /* If this is a conditional candidate, the defining PHI statement
     for the base SSA name B.  For future use; always NULL for now.  */
  gimple def_phi;

  /* Savings that can be expected from eliminating dead code if this
     candidate is replaced.  */
  int dead_savings;
};

typedef struct slsr_cand_d slsr_cand, *slsr_cand_t;
typedef const struct slsr_cand_d *const_slsr_cand_t;

/* Pointers to candidates are chained together as part of a mapping
   from base expressions to the candidates that use them.  */

struct cand_chain_d
{
  /* Base expression for the chain of candidates:  often, but not
     always, an SSA name.  */
  tree base_expr;

  /* Pointer to a candidate.  */
  slsr_cand_t cand;

  /* Chain pointer.  */
  struct cand_chain_d *next;

};

typedef struct cand_chain_d cand_chain, *cand_chain_t;
typedef const struct cand_chain_d *const_cand_chain_t;

/* Candidates are maintained in a vector.  If candidate X dominates
   candidate Y, then X appears before Y in the vector; but the
   converse does not necessarily hold.  */
DEF_VEC_P (slsr_cand_t);
DEF_VEC_ALLOC_P (slsr_cand_t, heap);
static VEC (slsr_cand_t, heap) *cand_vec;

enum cost_consts
{
  COST_NEUTRAL = 0,
  COST_INFINITE = 1000
};

/* Pointer map embodying a mapping from statements to candidates.  */
static struct pointer_map_t *stmt_cand_map;

/* Obstack for candidates.  */
static struct obstack cand_obstack;

/* Hash table embodying a mapping from base exprs to chains of candidates.  */
static htab_t base_cand_map;

/* Obstack for candidate chains.  */
static struct obstack chain_obstack;

/* Produce a pointer to the IDX'th candidate in the candidate vector.  */

static slsr_cand_t
lookup_cand (cand_idx idx)
{
  return VEC_index (slsr_cand_t, cand_vec, idx - 1);
}

/* Callback to produce a hash value for a candidate chain header.  */

static hashval_t
base_cand_hash (const void *p)
{
  tree base_expr = ((const_cand_chain_t) p)->base_expr;
  return iterative_hash_expr (base_expr, 0);
}

/* Callback when an element is removed from the hash table.
   We never remove entries until the entire table is released.  */

static void
base_cand_free (void *p ATTRIBUTE_UNUSED)
{
}

/* Callback to return true if two candidate chain headers are equal.  */

static int
base_cand_eq (const void *p1, const void *p2)
{
  const_cand_chain_t const chain1 = (const_cand_chain_t) p1;
  const_cand_chain_t const chain2 = (const_cand_chain_t) p2;
  return operand_equal_p (chain1->base_expr, chain2->base_expr, 0);
}

/* Use the base expr from candidate C to look for possible candidates
   that can serve as a basis for C.  Each potential basis must also
   appear in a block that dominates the candidate statement and have
   the same stride and type.  If more than one possible basis exists,
   the one with highest index in the vector is chosen; this will be
   the most immediately dominating basis.  */

static int
find_basis_for_candidate (slsr_cand_t c)
{
  cand_chain mapping_key;
  cand_chain_t chain;
  slsr_cand_t basis = NULL;

  mapping_key.base_expr = c->base_expr;
  chain = (cand_chain_t) htab_find (base_cand_map, &mapping_key);

  for (; chain; chain = chain->next)
    {
      slsr_cand_t one_basis = chain->cand;

      if (one_basis->kind != c->kind
	  || !operand_equal_p (one_basis->stride, c->stride, 0)
	  || !types_compatible_p (one_basis->cand_type, c->cand_type)
	  || !dominated_by_p (CDI_DOMINATORS,
			      gimple_bb (c->cand_stmt),
			      gimple_bb (one_basis->cand_stmt)))
	continue;

      if (!basis || basis->cand_num < one_basis->cand_num)
	basis = one_basis;
    }

  if (basis)
    {
      c->sibling = basis->dependent;
      basis->dependent = c->cand_num;
      return basis->cand_num;
    }

  return 0;
}

/* Record a mapping from the base expression of C to C itself, indicating that
   C may potentially serve as a basis using that base expression.  */

static void
record_potential_basis (slsr_cand_t c)
{
  cand_chain_t node;
  void **slot;

  node = (cand_chain_t) obstack_alloc (&chain_obstack, sizeof (cand_chain));
  node->base_expr = c->base_expr;
  node->cand = c;
  node->next = NULL;
  slot = htab_find_slot (base_cand_map, node, INSERT);

  if (*slot)
    {
      cand_chain_t head = (cand_chain_t) (*slot);
      node->next = head->next;
      head->next = node;
    }
  else
    *slot = node;
}

/* Allocate storage for a new candidate and initialize its fields.
   Attempt to find a basis for the candidate.  */

static slsr_cand_t
alloc_cand_and_find_basis (enum cand_kind kind, gimple gs, tree base, 
			   double_int index, tree stride, tree ctype,
			   unsigned savings)
{
  slsr_cand_t c = (slsr_cand_t) obstack_alloc (&cand_obstack,
					       sizeof (slsr_cand));
  c->cand_stmt = gs;
  c->base_expr = base;
  c->stride = stride;
  c->index = index;
  c->cand_type = ctype;
  c->kind = kind;
  c->cand_num = VEC_length (slsr_cand_t, cand_vec) + 1;
  c->next_interp = 0;
  c->dependent = 0;
  c->sibling = 0;
  c->def_phi = NULL;
  c->dead_savings = savings;

  VEC_safe_push (slsr_cand_t, heap, cand_vec, c);
  c->basis = find_basis_for_candidate (c);
  record_potential_basis (c);

  return c;
}

/* Determine the target cost of statement GS when compiling according
   to SPEED.  */

static int
stmt_cost (gimple gs, bool speed)
{
  tree lhs, rhs1, rhs2;
  enum machine_mode lhs_mode;

  gcc_assert (is_gimple_assign (gs));
  lhs = gimple_assign_lhs (gs);
  rhs1 = gimple_assign_rhs1 (gs);
  lhs_mode = TYPE_MODE (TREE_TYPE (lhs));
  
  switch (gimple_assign_rhs_code (gs))
    {
    case MULT_EXPR:
      rhs2 = gimple_assign_rhs2 (gs);

      if (host_integerp (rhs2, 0))
	return mult_by_coeff_cost (TREE_INT_CST_LOW (rhs2), lhs_mode, speed);

      gcc_assert (TREE_CODE (rhs1) != INTEGER_CST);
      return mul_cost (speed, lhs_mode);

    case PLUS_EXPR:
    case POINTER_PLUS_EXPR:
    case MINUS_EXPR:
      rhs2 = gimple_assign_rhs2 (gs);
      return add_cost (speed, lhs_mode);

    case NEGATE_EXPR:
      return neg_cost (speed, lhs_mode);

    case NOP_EXPR:
      return convert_cost (lhs_mode, TYPE_MODE (TREE_TYPE (rhs1)), speed);

    /* Note that we don't assign costs to copies that in most cases
       will go away.  */
    default:
      ;
    }
  
  gcc_unreachable ();
  return 0;
}

/* Look up the defining statement for BASE_IN and return a pointer
   to its candidate in the candidate table, if any; otherwise NULL.
   Only CAND_ADD and CAND_MULT candidates are returned.  */

static slsr_cand_t
base_cand_from_table (tree base_in)
{
  slsr_cand_t *result;

  gimple def = SSA_NAME_DEF_STMT (base_in);
  if (!def)
    return (slsr_cand_t) NULL;

  result = (slsr_cand_t *) pointer_map_contains (stmt_cand_map, def);
  
  if (result && (*result)->kind != CAND_REF)
    return *result;

  return (slsr_cand_t) NULL;
}

/* Add an entry to the statement-to-candidate mapping.  */

static void
add_cand_for_stmt (gimple gs, slsr_cand_t c)
{
  void **slot = pointer_map_insert (stmt_cand_map, gs);
  gcc_assert (!*slot);
  *slot = c;
}

/* Look for the following pattern:

    *PBASE:    MEM_REF (T1, C1)

    *POFFSET:  MULT_EXPR (T2, C3)        [C2 is zero]
                     or
               MULT_EXPR (PLUS_EXPR (T2, C2), C3)
                     or
               MULT_EXPR (MINUS_EXPR (T2, -C2), C3)

    *PINDEX:   C4 * BITS_PER_UNIT

   If not present, leave the input values unchanged and return FALSE.
   Otherwise, modify the input values as follows and return TRUE:

    *PBASE:    T1
    *POFFSET:  MULT_EXPR (T2, C3)
    *PINDEX:   C1 + (C2 * C3) + C4  */

static bool
restructure_reference (tree *pbase, tree *poffset, double_int *pindex,
		       tree *ptype)
{
  tree base = *pbase, offset = *poffset;
  double_int index = *pindex;
  double_int bpu = uhwi_to_double_int (BITS_PER_UNIT);
  tree mult_op0, mult_op1, t1, t2, type;
  double_int c1, c2, c3, c4;

  if (!base
      || !offset
      || TREE_CODE (base) != MEM_REF
      || TREE_CODE (offset) != MULT_EXPR
      || TREE_CODE (TREE_OPERAND (offset, 1)) != INTEGER_CST
      || !double_int_zero_p (double_int_umod (index, bpu, FLOOR_MOD_EXPR)))
    return false;

  t1 = TREE_OPERAND (base, 0);
  c1 = mem_ref_offset (base);
  type = TREE_TYPE (TREE_OPERAND (base, 1));

  mult_op0 = TREE_OPERAND (offset, 0);
  mult_op1 = TREE_OPERAND (offset, 1);

  c3 = tree_to_double_int (mult_op1);

  if (TREE_CODE (mult_op0) == PLUS_EXPR)

    if (TREE_CODE (TREE_OPERAND (mult_op0, 1)) == INTEGER_CST)
      {
	t2 = TREE_OPERAND (mult_op0, 0);
	c2 = tree_to_double_int (TREE_OPERAND (mult_op0, 1));
      }
    else
      return false;

  else if (TREE_CODE (mult_op0) == MINUS_EXPR)

    if (TREE_CODE (TREE_OPERAND (mult_op0, 1)) == INTEGER_CST)
      {
	t2 = TREE_OPERAND (mult_op0, 0);
	c2 = double_int_neg (tree_to_double_int (TREE_OPERAND (mult_op0, 1)));
      }
    else
      return false;

  else
    {
      t2 = mult_op0;
      c2 = double_int_zero;
    }

  c4 = double_int_udiv (index, bpu, FLOOR_DIV_EXPR);

  *pbase = t1;
  *poffset = fold_build2 (MULT_EXPR, sizetype, t2,
			  double_int_to_tree (sizetype, c3));
  *pindex = double_int_add (double_int_add (c1, double_int_mul (c2, c3)), c4);
  *ptype = type;

  return true;
}

/* Given GS which contains a data reference, create a CAND_REF entry in
   the candidate table and attempt to find a basis.  */

static void
slsr_process_ref (gimple gs)
{
  tree ref_expr, base, offset, type;
  HOST_WIDE_INT bitsize, bitpos;
  enum machine_mode mode;
  int unsignedp, volatilep;
  double_int index;
  slsr_cand_t c;

  if (gimple_vdef (gs))
    ref_expr = gimple_assign_lhs (gs);
  else
    ref_expr = gimple_assign_rhs1 (gs);

  if (!handled_component_p (ref_expr)
      || TREE_CODE (ref_expr) == BIT_FIELD_REF
      || (TREE_CODE (ref_expr) == COMPONENT_REF
	  && DECL_BIT_FIELD (TREE_OPERAND (ref_expr, 1))))
    return;

  base = get_inner_reference (ref_expr, &bitsize, &bitpos, &offset, &mode,
			      &unsignedp, &volatilep, false);
  index = uhwi_to_double_int (bitpos);

  if (!restructure_reference (&base, &offset, &index, &type))
    return;

  c = alloc_cand_and_find_basis (CAND_REF, gs, base, index, offset,
				 type, 0);

  /* Add the candidate to the statement-candidate mapping.  */
  add_cand_for_stmt (gs, c);
}

/* Create a candidate entry for a statement GS, where GS multiplies
   two SSA names BASE_IN and STRIDE_IN.  Propagate any known information
   about the two SSA names into the new candidate.  Return the new
   candidate.  */

static slsr_cand_t
create_mul_ssa_cand (gimple gs, tree base_in, tree stride_in, bool speed)
{
  tree base = NULL_TREE, stride = NULL_TREE, ctype = NULL_TREE;
  double_int index;
  unsigned savings = 0;
  slsr_cand_t c;
  slsr_cand_t base_cand = base_cand_from_table (base_in);

  /* Look at all interpretations of the base candidate, if necessary,
     to find information to propagate into this candidate.  */
  while (base_cand && !base)
    {

      if (base_cand->kind == CAND_MULT
	  && operand_equal_p (base_cand->stride, integer_one_node, 0))
	{
	  /* Y = (B + i') * 1
	     X = Y * Z
	     ================
	     X = (B + i') * Z  */
	  base = base_cand->base_expr;
	  index = base_cand->index;
	  stride = stride_in;
	  ctype = base_cand->cand_type;
	  if (has_single_use (base_in))
	    savings = (base_cand->dead_savings 
		       + stmt_cost (base_cand->cand_stmt, speed));
	}
      else if (base_cand->kind == CAND_ADD
	       && TREE_CODE (base_cand->stride) == INTEGER_CST)
	{
	  /* Y = B + (i' * S), S constant
	     X = Y * Z
	     ============================
	     X = B + ((i' * S) * Z)  */
	  base = base_cand->base_expr;
	  index = double_int_mul (base_cand->index,
				  tree_to_double_int (base_cand->stride));
	  stride = stride_in;
	  ctype = base_cand->cand_type;
	  if (has_single_use (base_in))
	    savings = (base_cand->dead_savings
		       + stmt_cost (base_cand->cand_stmt, speed));
	}

      if (base_cand->next_interp)
	base_cand = lookup_cand (base_cand->next_interp);
      else
	base_cand = NULL;
    }

  if (!base)
    {
      /* No interpretations had anything useful to propagate, so
	 produce X = (Y + 0) * Z.  */
      base = base_in;
      index = double_int_zero;
      stride = stride_in;
      ctype = TREE_TYPE (base_in);
    }

  c = alloc_cand_and_find_basis (CAND_MULT, gs, base, index, stride,
				 ctype, savings);
  return c;
}

/* Create a candidate entry for a statement GS, where GS multiplies
   SSA name BASE_IN by constant STRIDE_IN.  Propagate any known
   information about BASE_IN into the new candidate.  Return the new
   candidate.  */

static slsr_cand_t
create_mul_imm_cand (gimple gs, tree base_in, tree stride_in, bool speed)
{
  tree base = NULL_TREE, stride = NULL_TREE, ctype = NULL_TREE;
  double_int index, temp;
  unsigned savings = 0;
  slsr_cand_t c;
  slsr_cand_t base_cand = base_cand_from_table (base_in);

  /* Look at all interpretations of the base candidate, if necessary,
     to find information to propagate into this candidate.  */
  while (base_cand && !base)
    {
      if (base_cand->kind == CAND_MULT
	  && TREE_CODE (base_cand->stride) == INTEGER_CST)
	{
	  /* Y = (B + i') * S, S constant
	     X = Y * c
	     ============================
	     X = (B + i') * (S * c)  */
	  base = base_cand->base_expr;
	  index = base_cand->index;
	  temp = double_int_mul (tree_to_double_int (base_cand->stride),
				 tree_to_double_int (stride_in));
	  stride = double_int_to_tree (TREE_TYPE (stride_in), temp);
	  ctype = base_cand->cand_type;
	  if (has_single_use (base_in))
	    savings = (base_cand->dead_savings 
		       + stmt_cost (base_cand->cand_stmt, speed));
	}
      else if (base_cand->kind == CAND_ADD
	       && operand_equal_p (base_cand->stride, integer_one_node, 0))
	{
	  /* Y = B + (i' * 1)
	     X = Y * c
	     ===========================
	     X = (B + i') * c  */
	  base = base_cand->base_expr;
	  index = base_cand->index;
	  stride = stride_in;
	  ctype = base_cand->cand_type;
	  if (has_single_use (base_in))
	    savings = (base_cand->dead_savings
		       + stmt_cost (base_cand->cand_stmt, speed));
	}
      else if (base_cand->kind == CAND_ADD
	       && double_int_one_p (base_cand->index)
	       && TREE_CODE (base_cand->stride) == INTEGER_CST)
	{
	  /* Y = B + (1 * S), S constant
	     X = Y * c
	     ===========================
	     X = (B + S) * c  */
	  base = base_cand->base_expr;
	  index = tree_to_double_int (base_cand->stride);
	  stride = stride_in;
	  ctype = base_cand->cand_type;
	  if (has_single_use (base_in))
	    savings = (base_cand->dead_savings
		       + stmt_cost (base_cand->cand_stmt, speed));
	}

      if (base_cand->next_interp)
	base_cand = lookup_cand (base_cand->next_interp);
      else
	base_cand = NULL;
    }

  if (!base)
    {
      /* No interpretations had anything useful to propagate, so
	 produce X = (Y + 0) * c.  */
      base = base_in;
      index = double_int_zero;
      stride = stride_in;
      ctype = TREE_TYPE (base_in);
    }

  c = alloc_cand_and_find_basis (CAND_MULT, gs, base, index, stride,
				 ctype, savings);
  return c;
}

/* Given GS which is a multiply of scalar integers, make an appropriate
   entry in the candidate table.  If this is a multiply of two SSA names,
   create two CAND_MULT interpretations and attempt to find a basis for
   each of them.  Otherwise, create a single CAND_MULT and attempt to
   find a basis.  */

static void
slsr_process_mul (gimple gs, tree rhs1, tree rhs2, bool speed)
{
  slsr_cand_t c, c2;

  /* If this is a multiply of an SSA name with itself, it is highly
     unlikely that we will get a strength reduction opportunity, so
     don't record it as a candidate.  This simplifies the logic for
     finding a basis, so if this is removed that must be considered.  */
  if (rhs1 == rhs2)
    return;

  if (TREE_CODE (rhs2) == SSA_NAME)
    {
      /* Record an interpretation of this statement in the candidate table
	 assuming RHS1 is the base expression and RHS2 is the stride.  */
      c = create_mul_ssa_cand (gs, rhs1, rhs2, speed);

      /* Add the first interpretation to the statement-candidate mapping.  */
      add_cand_for_stmt (gs, c);

      /* Record another interpretation of this statement assuming RHS1
	 is the stride and RHS2 is the base expression.  */
      c2 = create_mul_ssa_cand (gs, rhs2, rhs1, speed);
      c->next_interp = c2->cand_num;
    }
  else
    {
      /* Record an interpretation for the multiply-immediate.  */
      c = create_mul_imm_cand (gs, rhs1, rhs2, speed);

      /* Add the interpretation to the statement-candidate mapping.  */
      add_cand_for_stmt (gs, c);
    }
}

/* Create a candidate entry for a statement GS, where GS adds two
   SSA names BASE_IN and ADDEND_IN if SUBTRACT_P is false, and
   subtracts ADDEND_IN from BASE_IN otherwise.  Propagate any known
   information about the two SSA names into the new candidate.
   Return the new candidate.  */

static slsr_cand_t
create_add_ssa_cand (gimple gs, tree base_in, tree addend_in,
		     bool subtract_p, bool speed)
{
  tree base = NULL_TREE, stride = NULL_TREE, ctype = NULL;
  double_int index;
  unsigned savings = 0;
  slsr_cand_t c;
  slsr_cand_t base_cand = base_cand_from_table (base_in);
  slsr_cand_t addend_cand = base_cand_from_table (addend_in);

  /* The most useful transformation is a multiply-immediate feeding
     an add or subtract.  Look for that first.  */
  while (addend_cand && !base)
    {
      if (addend_cand->kind == CAND_MULT
	  && double_int_zero_p (addend_cand->index)
	  && TREE_CODE (addend_cand->stride) == INTEGER_CST)
	{
	  /* Z = (B + 0) * S, S constant
	     X = Y +/- Z
	     ===========================
	     X = Y + ((+/-1 * S) * B)  */
	  base = base_in;
	  index = tree_to_double_int (addend_cand->stride);
	  if (subtract_p)
	    index = double_int_neg (index);
	  stride = addend_cand->base_expr;
	  ctype = TREE_TYPE (base_in);
	  if (has_single_use (addend_in))
	    savings = (addend_cand->dead_savings
		       + stmt_cost (addend_cand->cand_stmt, speed));
	}

      if (addend_cand->next_interp)
	addend_cand = lookup_cand (addend_cand->next_interp);
      else
	addend_cand = NULL;
    }

  while (base_cand && !base)
    {
      if (base_cand->kind == CAND_ADD
	  && (double_int_zero_p (base_cand->index)
	      || operand_equal_p (base_cand->stride,
				  integer_zero_node, 0)))
	{
	  /* Y = B + (i' * S), i' * S = 0
	     X = Y +/- Z
	     ============================
	     X = B + (+/-1 * Z)  */
	  base = base_cand->base_expr;
	  index = subtract_p ? double_int_minus_one : double_int_one;
	  stride = addend_in;
	  ctype = base_cand->cand_type;
	  if (has_single_use (base_in))
	    savings = (base_cand->dead_savings
		       + stmt_cost (base_cand->cand_stmt, speed));
	}
      else if (subtract_p)
	{
	  slsr_cand_t subtrahend_cand = base_cand_from_table (addend_in);

	  while (subtrahend_cand && !base)
	    {
	      if (subtrahend_cand->kind == CAND_MULT
		  && double_int_zero_p (subtrahend_cand->index)
		  && TREE_CODE (subtrahend_cand->stride) == INTEGER_CST)
		{
		  /* Z = (B + 0) * S, S constant
		     X = Y - Z
		     ===========================
		     Value:  X = Y + ((-1 * S) * B)  */
		  base = base_in;
		  index = tree_to_double_int (subtrahend_cand->stride);
		  index = double_int_neg (index);
		  stride = subtrahend_cand->base_expr;
		  ctype = TREE_TYPE (base_in);
		  if (has_single_use (addend_in))
		    savings = (subtrahend_cand->dead_savings 
			       + stmt_cost (subtrahend_cand->cand_stmt, speed));
		}
	      
	      if (subtrahend_cand->next_interp)
		subtrahend_cand = lookup_cand (subtrahend_cand->next_interp);
	      else
		subtrahend_cand = NULL;
	    }
	}
      
      if (base_cand->next_interp)
	base_cand = lookup_cand (base_cand->next_interp);
      else
	base_cand = NULL;
    }

  if (!base)
    {
      /* No interpretations had anything useful to propagate, so
	 produce X = Y + (1 * Z).  */
      base = base_in;
      index = subtract_p ? double_int_minus_one : double_int_one;
      stride = addend_in;
      ctype = TREE_TYPE (base_in);
    }

  c = alloc_cand_and_find_basis (CAND_ADD, gs, base, index, stride,
				 ctype, savings);
  return c;
}

/* Create a candidate entry for a statement GS, where GS adds SSA
   name BASE_IN to constant INDEX_IN.  Propagate any known information
   about BASE_IN into the new candidate.  Return the new candidate.  */

static slsr_cand_t
create_add_imm_cand (gimple gs, tree base_in, double_int index_in, bool speed)
{
  enum cand_kind kind = CAND_ADD;
  tree base = NULL_TREE, stride = NULL_TREE, ctype = NULL_TREE;
  double_int index, multiple;
  unsigned savings = 0;
  slsr_cand_t c;
  slsr_cand_t base_cand = base_cand_from_table (base_in);

  while (base_cand && !base)
    {
      bool unsigned_p = TYPE_UNSIGNED (TREE_TYPE (base_cand->stride));

      if (TREE_CODE (base_cand->stride) == INTEGER_CST
	  && double_int_multiple_of (index_in,
				     tree_to_double_int (base_cand->stride),
				     unsigned_p,
				     &multiple))
	{
	  /* Y = (B + i') * S, S constant, c = kS for some integer k
	     X = Y + c
	     ============================
	     X = (B + (i'+ k)) * S  
	  OR
	     Y = B + (i' * S), S constant, c = kS for some integer k
	     X = Y + c
	     ============================
	     X = (B + (i'+ k)) * S  */
	  kind = base_cand->kind;
	  base = base_cand->base_expr;
	  index = double_int_add (base_cand->index, multiple);
	  stride = base_cand->stride;
	  ctype = base_cand->cand_type;
	  if (has_single_use (base_in))
	    savings = (base_cand->dead_savings 
		       + stmt_cost (base_cand->cand_stmt, speed));
	}

      if (base_cand->next_interp)
	base_cand = lookup_cand (base_cand->next_interp);
      else
	base_cand = NULL;
    }

  if (!base)
    {
      /* No interpretations had anything useful to propagate, so
	 produce X = Y + (c * 1).  */
      kind = CAND_ADD;
      base = base_in;
      index = index_in;
      stride = integer_one_node;
      ctype = TREE_TYPE (base_in);
    }

  c = alloc_cand_and_find_basis (kind, gs, base, index, stride,
				 ctype, savings);
  return c;
}

/* Given GS which is an add or subtract of scalar integers or pointers,
   make at least one appropriate entry in the candidate table.  */

static void
slsr_process_add (gimple gs, tree rhs1, tree rhs2, bool speed)
{
  bool subtract_p = gimple_assign_rhs_code (gs) == MINUS_EXPR;
  slsr_cand_t c = NULL, c2;

  if (TREE_CODE (rhs2) == SSA_NAME)
    {
      /* First record an interpretation assuming RHS1 is the base expression
	 and RHS2 is the stride.  But it doesn't make sense for the
	 stride to be a pointer, so don't record a candidate in that case.  */
      if (!POINTER_TYPE_P (TREE_TYPE (rhs2)))
	{
	  c = create_add_ssa_cand (gs, rhs1, rhs2, subtract_p, speed);

	  /* Add the first interpretation to the statement-candidate
	     mapping.  */
	  add_cand_for_stmt (gs, c);
	}

      /* If the two RHS operands are identical, or this is a subtract,
	 we're done.  */
      if (operand_equal_p (rhs1, rhs2, 0) || subtract_p)
	return;

      /* Otherwise, record another interpretation assuming RHS2 is the
	 base expression and RHS1 is the stride, again provided that the
	 stride is not a pointer.  */
      if (!POINTER_TYPE_P (TREE_TYPE (rhs1)))
	{
	  c2 = create_add_ssa_cand (gs, rhs2, rhs1, false, speed);
	  if (c)
	    c->next_interp = c2->cand_num;
	  else
	    add_cand_for_stmt (gs, c2);
	}
    }
  else
    {
      double_int index;

      /* Record an interpretation for the add-immediate.  */
      index = tree_to_double_int (rhs2);
      if (subtract_p)
	index = double_int_neg (index);

      c = create_add_imm_cand (gs, rhs1, index, speed);

      /* Add the interpretation to the statement-candidate mapping.  */
      add_cand_for_stmt (gs, c);
    }
}

/* Given GS which is a negate of a scalar integer, make an appropriate
   entry in the candidate table.  A negate is equivalent to a multiply
   by -1.  */

static void
slsr_process_neg (gimple gs, tree rhs1, bool speed)
{
  /* Record a CAND_MULT interpretation for the multiply by -1.  */
  slsr_cand_t c = create_mul_imm_cand (gs, rhs1, integer_minus_one_node, speed);

  /* Add the interpretation to the statement-candidate mapping.  */
  add_cand_for_stmt (gs, c);
}

/* Return TRUE if GS is a statement that defines an SSA name from
   a conversion and is legal for us to combine with an add and multiply
   in the candidate table.  For example, suppose we have:

     A = B + i;
     C = (type) A;
     D = C * S;

   Without the type-cast, we would create a CAND_MULT for D with base B,
   index i, and stride S.  We want to record this candidate only if it
   is equivalent to apply the type cast following the multiply:

     A = B + i;
     E = A * S;
     D = (type) E;

   We will record the type with the candidate for D.  This allows us
   to use a similar previous candidate as a basis.  If we have earlier seen

     A' = B + i';
     C' = (type) A';
     D' = C' * S;

   we can replace D with

     D = D' + (i - i') * S;

   But if moving the type-cast would change semantics, we mustn't do this.

   This is legitimate for casts from a non-wrapping integral type to
   any integral type of the same or larger size.  It is not legitimate
   to convert a wrapping type to a non-wrapping type, or to a wrapping
   type of a different size.  I.e., with a wrapping type, we must
   assume that the addition B + i could wrap, in which case performing
   the multiply before or after one of the "illegal" type casts will
   have different semantics.  */

static bool
legal_cast_p (gimple gs, tree rhs)
{
  tree lhs, lhs_type, rhs_type;
  unsigned lhs_size, rhs_size;
  bool lhs_wraps, rhs_wraps;

  if (!is_gimple_assign (gs)
      || !CONVERT_EXPR_CODE_P (gimple_assign_rhs_code (gs)))
    return false;

  lhs = gimple_assign_lhs (gs);
  lhs_type = TREE_TYPE (lhs);
  rhs_type = TREE_TYPE (rhs);
  lhs_size = TYPE_PRECISION (lhs_type);
  rhs_size = TYPE_PRECISION (rhs_type);
  lhs_wraps = TYPE_OVERFLOW_WRAPS (lhs_type);
  rhs_wraps = TYPE_OVERFLOW_WRAPS (rhs_type);

  if (lhs_size < rhs_size
      || (rhs_wraps && !lhs_wraps)
      || (rhs_wraps && lhs_wraps && rhs_size != lhs_size))
    return false;

  return true;
}

/* Given GS which is a cast to a scalar integer type, determine whether
   the cast is legal for strength reduction.  If so, make at least one
   appropriate entry in the candidate table.  */

static void
slsr_process_cast (gimple gs, tree rhs1, bool speed)
{
  tree lhs, ctype;
  slsr_cand_t base_cand, c, c2;
  unsigned savings = 0;

  if (!legal_cast_p (gs, rhs1))
    return;

  lhs = gimple_assign_lhs (gs);
  base_cand = base_cand_from_table (rhs1);
  ctype = TREE_TYPE (lhs);

  if (base_cand)
    {
      while (base_cand)
	{
	  /* Propagate all data from the base candidate except the type,
	     which comes from the cast, and the base candidate's cast,
	     which is no longer applicable.  */
	  if (has_single_use (rhs1))
	    savings = (base_cand->dead_savings 
		       + stmt_cost (base_cand->cand_stmt, speed));

	  c = alloc_cand_and_find_basis (base_cand->kind, gs,
					 base_cand->base_expr,
					 base_cand->index, base_cand->stride,
					 ctype, savings);
	  if (base_cand->next_interp)
	    base_cand = lookup_cand (base_cand->next_interp);
	  else
	    base_cand = NULL;
	}
    }
  else 
    {
      /* If nothing is known about the RHS, create fresh CAND_ADD and
	 CAND_MULT interpretations:

	 X = Y + (0 * 1)
	 X = (Y + 0) * 1

	 The first of these is somewhat arbitrary, but the choice of
	 1 for the stride simplifies the logic for propagating casts
	 into their uses.  */
      c = alloc_cand_and_find_basis (CAND_ADD, gs, rhs1, double_int_zero,
				     integer_one_node, ctype, 0);
      c2 = alloc_cand_and_find_basis (CAND_MULT, gs, rhs1, double_int_zero,
				      integer_one_node, ctype, 0);
      c->next_interp = c2->cand_num;
    }

  /* Add the first (or only) interpretation to the statement-candidate
     mapping.  */
  add_cand_for_stmt (gs, c);
}

/* Given GS which is a copy of a scalar integer type, make at least one
   appropriate entry in the candidate table.

   This interface is included for completeness, but is unnecessary
   if this pass immediately follows a pass that performs copy 
   propagation, such as DOM.  */

static void
slsr_process_copy (gimple gs, tree rhs1, bool speed)
{
  slsr_cand_t base_cand, c, c2;
  unsigned savings = 0;

  base_cand = base_cand_from_table (rhs1);

  if (base_cand)
    {
      while (base_cand)
	{
	  /* Propagate all data from the base candidate.  */
	  if (has_single_use (rhs1))
	    savings = (base_cand->dead_savings 
		       + stmt_cost (base_cand->cand_stmt, speed));

	  c = alloc_cand_and_find_basis (base_cand->kind, gs,
					 base_cand->base_expr,
					 base_cand->index, base_cand->stride,
					 base_cand->cand_type, savings);
	  if (base_cand->next_interp)
	    base_cand = lookup_cand (base_cand->next_interp);
	  else
	    base_cand = NULL;
	}
    }
  else 
    {
      /* If nothing is known about the RHS, create fresh CAND_ADD and
	 CAND_MULT interpretations:

	 X = Y + (0 * 1)
	 X = (Y + 0) * 1

	 The first of these is somewhat arbitrary, but the choice of
	 1 for the stride simplifies the logic for propagating casts
	 into their uses.  */
      c = alloc_cand_and_find_basis (CAND_ADD, gs, rhs1, double_int_zero,
				     integer_one_node, TREE_TYPE (rhs1), 0);
      c2 = alloc_cand_and_find_basis (CAND_MULT, gs, rhs1, double_int_zero,
				      integer_one_node, TREE_TYPE (rhs1), 0);
      c->next_interp = c2->cand_num;
    }

  /* Add the first (or only) interpretation to the statement-candidate
     mapping.  */
  add_cand_for_stmt (gs, c);
}

/* Find strength-reduction candidates in block BB.  */

static void
find_candidates_in_block (struct dom_walk_data *walk_data ATTRIBUTE_UNUSED,
			  basic_block bb)
{
  bool speed = optimize_bb_for_speed_p (bb);
  gimple_stmt_iterator gsi;

  for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi))
    {
      gimple gs = gsi_stmt (gsi);

      if (gimple_vuse (gs) && gimple_assign_single_p (gs))
	slsr_process_ref (gs);

      else if (is_gimple_assign (gs)
	       && SCALAR_INT_MODE_P
	            (TYPE_MODE (TREE_TYPE (gimple_assign_lhs (gs)))))
	{
	  tree rhs1 = NULL_TREE, rhs2 = NULL_TREE;

	  switch (gimple_assign_rhs_code (gs))
	    {
	    case MULT_EXPR:
	    case PLUS_EXPR:
	      rhs1 = gimple_assign_rhs1 (gs);
	      rhs2 = gimple_assign_rhs2 (gs);
	      /* Should never happen, but currently some buggy situations
		 in earlier phases put constants in rhs1.  */
	      if (TREE_CODE (rhs1) != SSA_NAME)
		continue;
	      break;

	    /* Possible future opportunity: rhs1 of a ptr+ can be
	       an ADDR_EXPR.  */
	    case POINTER_PLUS_EXPR:
	    case MINUS_EXPR:
	      rhs2 = gimple_assign_rhs2 (gs);
	      /* Fall-through.  */

	    case NOP_EXPR:
	    case MODIFY_EXPR:
	    case NEGATE_EXPR:
	      rhs1 = gimple_assign_rhs1 (gs);
	      if (TREE_CODE (rhs1) != SSA_NAME)
		continue;
	      break;

	    default:
	      ;
	    }

	  switch (gimple_assign_rhs_code (gs))
	    {
	    case MULT_EXPR:
	      slsr_process_mul (gs, rhs1, rhs2, speed);
	      break;

	    case PLUS_EXPR:
	    case POINTER_PLUS_EXPR:
	    case MINUS_EXPR:
	      slsr_process_add (gs, rhs1, rhs2, speed);
	      break;

	    case NEGATE_EXPR:
	      slsr_process_neg (gs, rhs1, speed);
	      break;

	    case NOP_EXPR:
	      slsr_process_cast (gs, rhs1, speed);
	      break;

	    case MODIFY_EXPR:
	      slsr_process_copy (gs, rhs1, speed);
	      break;

	    default:
	      ;
	    }
	}
    }
}

/* Dump a candidate for debug.  */

static void
dump_candidate (slsr_cand_t c)
{
  fprintf (dump_file, "%3d  [%d] ", c->cand_num,
	   gimple_bb (c->cand_stmt)->index);
  print_gimple_stmt (dump_file, c->cand_stmt, 0, 0);
  switch (c->kind)
    {
    case CAND_MULT:
      fputs ("     MULT : (", dump_file);
      print_generic_expr (dump_file, c->base_expr, 0);
      fputs (" + ", dump_file);
      dump_double_int (dump_file, c->index, false);
      fputs (") * ", dump_file);
      print_generic_expr (dump_file, c->stride, 0);
      fputs (" : ", dump_file);
      break;
    case CAND_ADD:
      fputs ("     ADD  : ", dump_file);
      print_generic_expr (dump_file, c->base_expr, 0);
      fputs (" + (", dump_file);
      dump_double_int (dump_file, c->index, false);
      fputs (" * ", dump_file);
      print_generic_expr (dump_file, c->stride, 0);
      fputs (") : ", dump_file);
      break;
    case CAND_REF:
      fputs ("     REF  : ", dump_file);
      print_generic_expr (dump_file, c->base_expr, 0);
      fputs (" + (", dump_file);
      print_generic_expr (dump_file, c->stride, 0);
      fputs (") + ", dump_file);
      dump_double_int (dump_file, c->index, false);
      fputs (" : ", dump_file);
      break;
    default:
      gcc_unreachable ();
    }
  print_generic_expr (dump_file, c->cand_type, 0);
  fprintf (dump_file, "\n     basis: %d  dependent: %d  sibling: %d\n",
	   c->basis, c->dependent, c->sibling);
  fprintf (dump_file, "     next-interp: %d  dead-savings: %d\n",
	   c->next_interp, c->dead_savings);
  if (c->def_phi)
    {
      fputs ("     phi:  ", dump_file);
      print_gimple_stmt (dump_file, c->def_phi, 0, 0);
    }
  fputs ("\n", dump_file);
}

/* Dump the candidate vector for debug.  */

static void
dump_cand_vec (void)
{
  unsigned i;
  slsr_cand_t c;

  fprintf (dump_file, "\nStrength reduction candidate vector:\n\n");
  
  FOR_EACH_VEC_ELT (slsr_cand_t, cand_vec, i, c)
    dump_candidate (c);
}

/* Callback used to dump the candidate chains hash table.  */

static int
base_cand_dump_callback (void **slot, void *ignored ATTRIBUTE_UNUSED)
{
  const_cand_chain_t chain = *((const_cand_chain_t *) slot);
  cand_chain_t p;

  print_generic_expr (dump_file, chain->base_expr, 0);
  fprintf (dump_file, " -> %d", chain->cand->cand_num);

  for (p = chain->next; p; p = p->next)
    fprintf (dump_file, " -> %d", p->cand->cand_num);

  fputs ("\n", dump_file);
  return 1;
}

/* Dump the candidate chains.  */

static void
dump_cand_chains (void)
{
  fprintf (dump_file, "\nStrength reduction candidate chains:\n\n");
  htab_traverse_noresize (base_cand_map, base_cand_dump_callback, NULL);
  fputs ("\n", dump_file);
}

/* Recursive helper for unconditional_cands_with_known_stride_p.
   Returns TRUE iff C, its siblings, and its dependents are all
   unconditional candidates.  */

static bool
unconditional_cands (slsr_cand_t c)
{
  if (c->def_phi)
    return false;

  if (c->sibling && !unconditional_cands (lookup_cand (c->sibling)))
    return false;

  if (c->dependent && !unconditional_cands (lookup_cand (c->dependent)))
    return false;

  return true;
}

/* Determine whether or not the tree of candidates rooted at
   ROOT consists entirely of unconditional increments with
   an INTEGER_CST stride.  */

static bool
unconditional_cands_with_known_stride_p (slsr_cand_t root)
{
  /* The stride is identical for all related candidates, so
     check it once.  */
  if (TREE_CODE (root->stride) != INTEGER_CST)
    return false;

  return unconditional_cands (lookup_cand (root->dependent));
}

/* Replace *EXPR in candidate C with an equivalent strength-reduced
   data reference.  */

static void
replace_ref (tree *expr, slsr_cand_t c)
{
  tree add_expr = fold_build2 (POINTER_PLUS_EXPR, TREE_TYPE (c->base_expr),
			       c->base_expr, c->stride);
  tree mem_ref = fold_build2 (MEM_REF, TREE_TYPE (*expr), add_expr,
			      double_int_to_tree (c->cand_type, c->index));
  
  /* Gimplify the base addressing expression for the new MEM_REF tree.  */
  gimple_stmt_iterator gsi = gsi_for_stmt (c->cand_stmt);
  TREE_OPERAND (mem_ref, 0)
    = force_gimple_operand_gsi (&gsi, TREE_OPERAND (mem_ref, 0),
				/*simple_p=*/true, NULL,
				/*before=*/true, GSI_SAME_STMT);
  copy_ref_info (mem_ref, *expr);
  *expr = mem_ref;
  update_stmt (c->cand_stmt);
}

/* Replace CAND_REF candidate C, each sibling of candidate C, and each
   dependent of candidate C with an equivalent strength-reduced data
   reference.  */

static void
replace_refs (slsr_cand_t c)
{
  if (gimple_vdef (c->cand_stmt))
    {
      tree *lhs = gimple_assign_lhs_ptr (c->cand_stmt);
      replace_ref (lhs, c);
    }
  else
    {
      tree *rhs = gimple_assign_rhs1_ptr (c->cand_stmt);
      replace_ref (rhs, c);
    }

  if (c->sibling)
    replace_refs (lookup_cand (c->sibling));

  if (c->dependent)
    replace_refs (lookup_cand (c->dependent));
}

/* Calculate the increment required for candidate C relative to 
   its basis.  */

static double_int
cand_increment (slsr_cand_t c)
{
  slsr_cand_t basis;

  /* If the candidate doesn't have a basis, just return its own
     index.  This is useful in record_increments to help us find
     an existing initializer.  */
  if (!c->basis)
    return c->index;

  basis = lookup_cand (c->basis);
  gcc_assert (operand_equal_p (c->base_expr, basis->base_expr, 0));
  return double_int_sub (c->index, basis->index);
}

/* Return TRUE iff candidate C has already been replaced under
   another interpretation.  */

static inline bool
cand_already_replaced (slsr_cand_t c)
{
  return (gimple_bb (c->cand_stmt) == 0);
}

/* Helper routine for replace_dependents, doing the work for a 
   single candidate C.  */

static void
replace_dependent (slsr_cand_t c, enum tree_code cand_code)
{
  double_int stride = tree_to_double_int (c->stride);
  double_int bump = double_int_mul (cand_increment (c), stride);
  gimple stmt_to_print = NULL;
  slsr_cand_t basis;
  tree basis_name, incr_type, bump_tree;
  enum tree_code code;
  
  /* It is highly unlikely, but possible, that the resulting
     bump doesn't fit in a HWI.  Abandon the replacement
     in this case.  Restriction to signed HWI is conservative
     for unsigned types but allows for safe negation without
     twisted logic.  */
  if (!double_int_fits_in_shwi_p (bump))
    return;

  basis = lookup_cand (c->basis);
  basis_name = gimple_assign_lhs (basis->cand_stmt);
  incr_type = TREE_TYPE (gimple_assign_rhs1 (c->cand_stmt));
  code = PLUS_EXPR;

  if (double_int_negative_p (bump))
    {
      code = MINUS_EXPR;
      bump = double_int_neg (bump);
    }

  bump_tree = double_int_to_tree (incr_type, bump);

  if (dump_file && (dump_flags & TDF_DETAILS))
    {
      fputs ("Replacing: ", dump_file);
      print_gimple_stmt (dump_file, c->cand_stmt, 0, 0);
    }

  if (double_int_zero_p (bump))
    {
      tree lhs = gimple_assign_lhs (c->cand_stmt);
      gimple copy_stmt = gimple_build_assign (lhs, basis_name);
      gimple_stmt_iterator gsi = gsi_for_stmt (c->cand_stmt);
      gimple_set_location (copy_stmt, gimple_location (c->cand_stmt));
      gsi_replace (&gsi, copy_stmt, false);
      if (dump_file && (dump_flags & TDF_DETAILS))
	stmt_to_print = copy_stmt;
    }
  else
    {
      tree rhs1 = gimple_assign_rhs1 (c->cand_stmt);
      tree rhs2 = gimple_assign_rhs2 (c->cand_stmt);
      if (cand_code != NEGATE_EXPR
	  && ((operand_equal_p (rhs1, basis_name, 0)
	       && operand_equal_p (rhs2, bump_tree, 0))
	      || (operand_equal_p (rhs1, bump_tree, 0)
		  && operand_equal_p (rhs2, basis_name, 0))))
	{
	  if (dump_file && (dump_flags & TDF_DETAILS))
	    {
	      fputs ("(duplicate, not actually replacing)", dump_file);
	      stmt_to_print = c->cand_stmt;
	    }
	}
      else
	{
	  gimple_stmt_iterator gsi = gsi_for_stmt (c->cand_stmt);
	  gimple_assign_set_rhs_with_ops (&gsi, code, basis_name, bump_tree);
	  update_stmt (gsi_stmt (gsi));
	  if (dump_file && (dump_flags & TDF_DETAILS))
	    stmt_to_print = gsi_stmt (gsi);
	}
    }
  
  if (dump_file && (dump_flags & TDF_DETAILS))
    {
      fputs ("With: ", dump_file);
      print_gimple_stmt (dump_file, stmt_to_print, 0, 0);
      fputs ("\n", dump_file);
    }
}

/* Replace candidate C, each sibling of candidate C, and each
   dependent of candidate C with an add or subtract.  Note that we
   only operate on CAND_MULTs with known strides, so we will never
   generate a POINTER_PLUS_EXPR.  Each candidate X = (B + i) * S is
   replaced by X = Y + ((i - i') * S), as described in the module
   commentary.  The folded value ((i - i') * S) is referred to here
   as the "bump."  */

static void
replace_dependents (slsr_cand_t c)
{
  enum tree_code cand_code = gimple_assign_rhs_code (c->cand_stmt);

  /* It is not useful to replace casts, copies, or adds of an SSA name
     and a constant.  Also skip candidates that have already been
     replaced under another interpretation.  */
  if (cand_code != MODIFY_EXPR
      && cand_code != NOP_EXPR
      && c->kind == CAND_MULT
      && !cand_already_replaced (c))
    replace_dependent (c, cand_code);

  if (c->sibling)
    replace_dependents (lookup_cand (c->sibling));

  if (c->dependent)
    replace_dependents (lookup_cand (c->dependent));
}

/* Analyze costs of related candidates in the candidate vector,
   and make beneficial replacements.  */

static void
analyze_candidates_and_replace (void)
{
  unsigned i;
  slsr_cand_t c;

  /* Each candidate that has a null basis and a non-null
     dependent is the root of a tree of related statements.
     Analyze each tree to determine a subset of those
     statements that can be replaced with maximum benefit.  */
  FOR_EACH_VEC_ELT (slsr_cand_t, cand_vec, i, c)
    {
      slsr_cand_t first_dep;

      if (c->basis != 0 || c->dependent == 0)
	continue;

      if (dump_file && (dump_flags & TDF_DETAILS))
	fprintf (dump_file, "\nProcessing dependency tree rooted at %d.\n",
		 c->cand_num);

      first_dep = lookup_cand (c->dependent);

      /* If this is a chain of CAND_REFs, unconditionally replace
	 each of them with a strength-reduced data reference.  */
      if (c->kind == CAND_REF)
	replace_refs (c);

      /* If the common stride of all related candidates is a
	 known constant, and none of these has a phi-dependence,
	 then all replacements are considered profitable.
	 Each replaces a multiply by a single add, with the
	 possibility that a feeding add also goes dead as a
	 result.  */
      else if (unconditional_cands_with_known_stride_p (c))
	replace_dependents (first_dep);

      /* TODO:  When the stride is an SSA name, it may still be
	 profitable to replace some or all of the dependent
	 candidates, depending on whether the introduced increments
	 can be reused, or are less expensive to calculate than
	 the replaced statements.  */

      /* TODO:  When conditional increments occur so that a 
	 candidate is dependent upon a phi-basis, the cost of
	 introducing a temporary must be accounted for.  */
    }
}

static unsigned
execute_strength_reduction (void)
{
  struct dom_walk_data walk_data;

  /* Create the obstack where candidates will reside.  */
  gcc_obstack_init (&cand_obstack);

  /* Allocate the candidate vector.  */
  cand_vec = VEC_alloc (slsr_cand_t, heap, 128);

  /* Allocate the mapping from statements to candidate indices.  */
  stmt_cand_map = pointer_map_create ();

  /* Create the obstack where candidate chains will reside.  */
  gcc_obstack_init (&chain_obstack);

  /* Allocate the mapping from base expressions to candidate chains.  */
  base_cand_map = htab_create (500, base_cand_hash,
			       base_cand_eq, base_cand_free);

  /* Initialize the loop optimizer.  We need to detect flow across
     back edges, and this gives us dominator information as well.  */
  loop_optimizer_init (AVOID_CFG_MODIFICATIONS);

  /* Set up callbacks for the generic dominator tree walker.  */
  walk_data.dom_direction = CDI_DOMINATORS;
  walk_data.initialize_block_local_data = NULL;
  walk_data.before_dom_children = find_candidates_in_block;
  walk_data.after_dom_children = NULL;
  walk_data.global_data = NULL;
  walk_data.block_local_data_size = 0;
  init_walk_dominator_tree (&walk_data);

  /* Walk the CFG in predominator order looking for strength reduction
     candidates.  */
  walk_dominator_tree (&walk_data, ENTRY_BLOCK_PTR);

  if (dump_file && (dump_flags & TDF_DETAILS))
    {
      dump_cand_vec ();
      dump_cand_chains ();
    }

  /* Analyze costs and make appropriate replacements.  */
  analyze_candidates_and_replace ();

  /* Free resources.  */
  fini_walk_dominator_tree (&walk_data);
  loop_optimizer_finalize ();
  htab_delete (base_cand_map);
  obstack_free (&chain_obstack, NULL);
  pointer_map_destroy (stmt_cand_map);
  VEC_free (slsr_cand_t, heap, cand_vec);
  obstack_free (&cand_obstack, NULL);

  return 0;
}

static bool
gate_strength_reduction (void)
{
  return flag_tree_slsr;
}

struct gimple_opt_pass pass_strength_reduction =
{
 {
  GIMPLE_PASS,
  "slsr",				/* name */
  gate_strength_reduction,		/* gate */
  execute_strength_reduction,		/* execute */
  NULL,					/* sub */
  NULL,					/* next */
  0,					/* static_pass_number */
  TV_GIMPLE_SLSR,			/* tv_id */
  PROP_cfg | PROP_ssa,			/* properties_required */
  0,					/* properties_provided */
  0,					/* properties_destroyed */
  0,					/* todo_flags_start */
  TODO_verify_ssa			/* todo_flags_finish */
 }
};
