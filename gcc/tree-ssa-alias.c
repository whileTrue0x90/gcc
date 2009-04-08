/* Alias analysis for trees.
   Copyright (C) 2004, 2005, 2006, 2007, 2008, 2009
   Free Software Foundation, Inc.
   Contributed by Diego Novillo <dnovillo@redhat.com>

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
#include "rtl.h"
#include "tm_p.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "timevar.h"
#include "expr.h"
#include "ggc.h"
#include "langhooks.h"
#include "flags.h"
#include "function.h"
#include "diagnostic.h"
#include "tree-dump.h"
#include "gimple.h"
#include "tree-flow.h"
#include "tree-inline.h"
#include "tree-pass.h"
#include "convert.h"
#include "params.h"
#include "ipa-type-escape.h"
#include "vec.h"
#include "bitmap.h"
#include "vecprim.h"
#include "pointer-set.h"
#include "alloc-pool.h"
#include "tree-ssa-alias.h"

/* Broad overview of how alias analysis on gimple works:

   Statements clobbering or using memory are linked through the
   virtual operand factored use-def chain.  The virtual operand
   is unique per function, its symbol is accessible via gimple_vop (cfun).
   Virtual operands are used for efficiently walking memory statements
   in the gimple IL and are useful for things like value-numbering as
   a generation count for memory references.

   SSA_NAME pointers may have associated points-to information
   accessible via the SSA_NAME_PTR_INFO macro.  Flow-insensitive
   points-to information is (re-)computed by the TODO_rebuild_alias
   pass manager todo.  Points-to information is also used for more
   precise tracking of call-clobbered and call-used variables and
   related disambiguations.

   This file contains functions for disambiguating memory references,
   the so called alias-oracle and tools for walking of the gimple IL.

   The main alias-oracle entry-points are

   bool stmt_may_clobber_ref_p (gimple, tree)

     This function queries if a statement may invalidate (parts of)
     the memory designated by the reference tree argument.

   bool ref_maybe_used_by_stmt_p (gimple, tree)

     This function queries if a statement may need (parts of) the
     memory designated by the reference tree argument.

   There are variants of these functions that only handle the call
   part of a statement, call_may_clobber_ref_p and ref_maybe_used_by_call_p.
   Note that these do not disambiguate against a possible call lhs.

   bool refs_may_alias_p (tree, tree)

     This function tries to disambiguate two reference trees.

   bool ptr_deref_may_alias_global_p (tree)

     This function queries if dereferencing a pointer variable may
     alias global memory.

   More low-level disambiguators are available and documented in
   this file.  Low-level disambiguators dealing with points-to
   information are in tree-ssa-structalias.c.  */


/* Query statistics for the different low-level disambiguators.
   A high-level query may trigger multiple of them.  */

static struct {
  unsigned HOST_WIDE_INT refs_may_alias_p_may_alias;
  unsigned HOST_WIDE_INT refs_may_alias_p_no_alias;
  unsigned HOST_WIDE_INT ref_maybe_used_by_call_p_may_alias;
  unsigned HOST_WIDE_INT ref_maybe_used_by_call_p_no_alias;
  unsigned HOST_WIDE_INT call_may_clobber_ref_p_may_alias;
  unsigned HOST_WIDE_INT call_may_clobber_ref_p_no_alias;
} alias_stats;

void
dump_alias_stats (FILE *s)
{
  fprintf (s, "\nAlias oracle query stats:\n");
  fprintf (s, "  refs_may_alias_p: "
	   HOST_WIDE_INT_PRINT_DEC" disambiguations, "
	   HOST_WIDE_INT_PRINT_DEC" queries\n",
	   alias_stats.refs_may_alias_p_no_alias,
	   alias_stats.refs_may_alias_p_no_alias
	   + alias_stats.refs_may_alias_p_may_alias);
  fprintf (s, "  ref_maybe_used_by_call_p: "
	   HOST_WIDE_INT_PRINT_DEC" disambiguations, "
	   HOST_WIDE_INT_PRINT_DEC" queries\n",
	   alias_stats.ref_maybe_used_by_call_p_no_alias,
	   alias_stats.refs_may_alias_p_no_alias
	   + alias_stats.ref_maybe_used_by_call_p_may_alias);
  fprintf (s, "  call_may_clobber_ref_p: "
	   HOST_WIDE_INT_PRINT_DEC" disambiguations, "
	   HOST_WIDE_INT_PRINT_DEC" queries\n",
	   alias_stats.call_may_clobber_ref_p_no_alias,
	   alias_stats.call_may_clobber_ref_p_no_alias
	   + alias_stats.call_may_clobber_ref_p_may_alias);
}


/* Return true, if dereferencing PTR may alias with a global variable.  */

bool
ptr_deref_may_alias_global_p (tree ptr)
{
  struct ptr_info_def *pi;

  /* If we end up with a pointer constant here that may point
     to global memory.  */
  if (TREE_CODE (ptr) != SSA_NAME)
    return true;

  pi = SSA_NAME_PTR_INFO (ptr);

  /* If we do not have points-to information for this variable,
     we have to punt.  */
  if (!pi)
    return true;

  return pt_solution_includes_global (&pi->pt);
}

/* Return true if dereferencing PTR may alias DECL.  */

static bool
ptr_deref_may_alias_decl_p (tree ptr, tree decl)
{
  struct ptr_info_def *pi;

  /* ???  During SCCVN/PRE we can end up with *&x during valueizing
     operands.  Likewise we can end up with dereferencing constant
     pointers.  Just bail out in these cases for now.  */
  if (TREE_CODE (ptr) == ADDR_EXPR
      || TREE_CODE (ptr) == INTEGER_CST)
    return true;

  gcc_assert (TREE_CODE (ptr) == SSA_NAME
	      && (TREE_CODE (decl) == VAR_DECL
		  || TREE_CODE (decl) == PARM_DECL
		  || TREE_CODE (decl) == RESULT_DECL));

  /* Non-aliased variables can not be pointed to.  */
  if (!may_be_aliased (decl))
    return false;

  /* If we do not have useful points-to information for this pointer
     we cannot disambiguate anything else.  */
  pi = SSA_NAME_PTR_INFO (ptr);
  if (!pi)
    return true;

  return pt_solution_includes (&pi->pt, decl);
}

/* Return true if dereferenced PTR1 and PTR2 may alias.  */

static bool
ptr_derefs_may_alias_p (tree ptr1, tree ptr2)
{
  struct ptr_info_def *pi1, *pi2;

  /* ???  During SCCVN/PRE we can end up with *&x during valueizing
     operands.  Likewise we can end up with dereferencing constant
     pointers.  Just bail out in these cases for now.  */
  if (TREE_CODE (ptr1) == ADDR_EXPR
      || TREE_CODE (ptr1) == INTEGER_CST
      || TREE_CODE (ptr2) == ADDR_EXPR
      || TREE_CODE (ptr2) == INTEGER_CST)
    return true;

  gcc_assert (TREE_CODE (ptr1) == SSA_NAME
	      && TREE_CODE (ptr2) == SSA_NAME);

  /* We may end up with two empty points-to solutions for two same pointers.
     In this case we still want to say both pointers alias, so shortcut
     that here.  */
  if (ptr1 == ptr2)
    return true;

  /* If we do not have useful points-to information for either pointer
     we cannot disambiguate anything else.  */
  pi1 = SSA_NAME_PTR_INFO (ptr1);
  pi2 = SSA_NAME_PTR_INFO (ptr2);
  if (!pi1 || !pi2)
    return true;

  return pt_solutions_intersect (&pi1->pt, &pi2->pt);
}

/* Return true if STMT is an "escape" site from the current function.  Escape
   sites those statements which might expose the address of a variable
   outside the current function.  STMT is an escape site iff:

   	1- STMT is a function call, or
	2- STMT is an __asm__ expression, or
	3- STMT is an assignment to a non-local variable, or
	4- STMT is a return statement.

   Return the type of escape site found, if we found one, or NO_ESCAPE
   if none.  */

enum escape_type
is_escape_site (gimple stmt)
{
  if (is_gimple_call (stmt))
    {
      if (gimple_call_flags (stmt) & (ECF_PURE | ECF_CONST))
	return ESCAPE_TO_PURE_CONST;

      return ESCAPE_TO_CALL;
    }
  else if (gimple_code (stmt) == GIMPLE_ASM)
    return ESCAPE_TO_ASM;
  else if (is_gimple_assign (stmt))
    {
      tree lhs = gimple_assign_lhs (stmt);

      /* Get to the base of _REF nodes.  */
      if (TREE_CODE (lhs) != SSA_NAME)
	lhs = get_base_address (lhs);

      /* If we couldn't recognize the LHS of the assignment, assume that it
	 is a non-local store.  */
      if (lhs == NULL_TREE)
	return ESCAPE_UNKNOWN;

      if (gimple_assign_cast_p (stmt))
	{
	  tree from = TREE_TYPE (gimple_assign_rhs1 (stmt));
	  tree to = TREE_TYPE (lhs);

	  /* If the RHS is a conversion between a pointer and an integer, the
	     pointer escapes since we can't track the integer.  */
	  if (POINTER_TYPE_P (from) && !POINTER_TYPE_P (to))
	    return ESCAPE_BAD_CAST;
	}

      /* If the LHS is an SSA name, it can't possibly represent a non-local
	 memory store.  */
      if (TREE_CODE (lhs) == SSA_NAME)
	return NO_ESCAPE;

      /* If the LHS is a non-global decl, it isn't a non-local memory store.
	 If the LHS escapes, the RHS escape is dealt with in the PTA solver.  */
      if (DECL_P (lhs)
	  && !is_global_var (lhs))
	return NO_ESCAPE;

      /* FIXME: LHS is not an SSA_NAME.  Even if it's an assignment to a
	 local variables we cannot be sure if it will escape, because we
	 don't have information about objects not in SSA form.  Need to
	 implement something along the lines of

	 J.-D. Choi, M. Gupta, M. J. Serrano, V. C. Sreedhar, and S. P.
	 Midkiff, ``Escape analysis for java,'' in Proceedings of the
	 Conference on Object-Oriented Programming Systems, Languages, and
	 Applications (OOPSLA), pp. 1-19, 1999.  */
      return ESCAPE_STORED_IN_GLOBAL;
    }
  else if (gimple_code (stmt) == GIMPLE_RETURN)
    return ESCAPE_TO_RETURN;

  return NO_ESCAPE;
}


/* Dump alias information on FILE.  */

void
dump_alias_info (FILE *file)
{
  size_t i;
  const char *funcname
    = lang_hooks.decl_printable_name (current_function_decl, 2);
  referenced_var_iterator rvi;
  tree var;

  fprintf (file, "\n\nAlias information for %s\n\n", funcname);

  fprintf (file, "Aliased symbols\n\n");
  
  FOR_EACH_REFERENCED_VAR (var, rvi)
    {
      if (may_be_aliased (var))
	dump_variable (file, var);
    }

  fprintf (file, "\n\nFlow-insensitive points-to information for %s\n\n", funcname);

  for (i = 1; i < num_ssa_names; i++)
    {
      tree ptr = ssa_name (i);
      struct ptr_info_def *pi;
      
      if (ptr == NULL_TREE
	  || SSA_NAME_IN_FREE_LIST (ptr))
	continue;

      pi = SSA_NAME_PTR_INFO (ptr);
      if (pi)
	dump_points_to_info_for (file, ptr);
    }

  fprintf (file, "\n");
}


/* Dump alias information on stderr.  */

void
debug_alias_info (void)
{
  dump_alias_info (stderr);
}


/* Return the alias information associated with pointer T.  It creates a
   new instance if none existed.  */

struct ptr_info_def *
get_ptr_info (tree t)
{
  struct ptr_info_def *pi;

  gcc_assert (POINTER_TYPE_P (TREE_TYPE (t)));

  pi = SSA_NAME_PTR_INFO (t);
  if (pi == NULL)
    {
      pi = GGC_CNEW (struct ptr_info_def);
      pt_solution_reset (&pi->pt);
      SSA_NAME_PTR_INFO (t) = pi;
    }

  return pi;
}

/* Dump points-to information for SSA_NAME PTR into FILE.  */

void
dump_points_to_info_for (FILE *file, tree ptr)
{
  struct ptr_info_def *pi = SSA_NAME_PTR_INFO (ptr);

  print_generic_expr (file, ptr, dump_flags);

  if (pi)
    {
      if (pi->pt.anything)
	fprintf (file, ", points-to anything");

      if (pi->pt.nonlocal)
	fprintf (file, ", points-to non-local");

      if (pi->pt.escaped)
	fprintf (file, ", points-to escaped");

      if (pi->pt.null)
	fprintf (file, ", points-to NULL");

      if (pi->pt.vars)
	{
	  fprintf (file, ", points-to vars: ");
	  dump_decl_set (file, pi->pt.vars);
	  if (pi->pt.vars_contains_global)
	    fprintf (file, " (includes global vars)");
	}
    }

  fprintf (file, "\n");
}


/* Dump points-to information for VAR into stderr.  */

void
debug_points_to_info_for (tree var)
{
  dump_points_to_info_for (stderr, var);
}

/* Return 1 if TYPE1 and TYPE2 are to be considered equivalent for the
   purpose of TBAA.  Return 0 if they are distinct and -1 if we cannot
   decide.  */

static inline int
same_type_for_tbaa (tree type1, tree type2)
{
  type1 = TYPE_MAIN_VARIANT (type1);
  type2 = TYPE_MAIN_VARIANT (type2);

  /* If we would have to do structural comparison bail out.  */
  if (TYPE_STRUCTURAL_EQUALITY_P (type1)
      || TYPE_STRUCTURAL_EQUALITY_P (type2))
    return -1;

  /* Compare the canonical types.  */
  if (TYPE_CANONICAL (type1) == TYPE_CANONICAL (type2))
    return 1;

  /* ???  Array types are not properly unified in all cases as we have
     spurious changes in the index types for example.  Removing this
     causes all sorts of problems with the Fortran frontend.  */
  if (TREE_CODE (type1) == ARRAY_TYPE
      && TREE_CODE (type2) == ARRAY_TYPE)
    return -1;

  /* The types are known to be not equal.  */
  return 0;
}

/* Determine if the two component references REF1 and REF2 which are
   based on access types TYPE1 and TYPE2 and of which at least one is based
   on an indirect reference may alias.  */

static bool
nonaliasing_component_refs_p (tree ref1, tree type1,
			      HOST_WIDE_INT offset1, HOST_WIDE_INT max_size1,
			      tree ref2, tree type2,
			      HOST_WIDE_INT offset2, HOST_WIDE_INT max_size2)
{
  /* If one reference is a component references through pointers try to find a
     common base and apply offset based disambiguation.  This handles
     for example
       struct A { int i; int j; } *q;
       struct B { struct A a; int k; } *p;
     disambiguating q->i and p->a.j.  */
  tree *refp;
  int same_p;

  /* Now search for the type1 in the access path of ref2.  This
     would be a common base for doing offset based disambiguation on.  */
  /* Skip the outermost ref - we would have caught that earlier.  */
  refp = &TREE_OPERAND (ref2, 0);
  while (handled_component_p (*refp)
	 && same_type_for_tbaa (TREE_TYPE (*refp), type1) == 0)
    refp = &TREE_OPERAND (*refp, 0);
  same_p = same_type_for_tbaa (TREE_TYPE (*refp), type1);
  /* If we couldn't compare types we have to bail out.  */
  if (same_p == -1)
    return true;
  else if (same_p == 1)
    {
      HOST_WIDE_INT offadj, sztmp, msztmp;
      get_ref_base_and_extent (*refp, &offadj, &sztmp, &msztmp);
      offset2 -= offadj;
      return ranges_overlap_p (offset1, max_size1, offset2, max_size2);
    }
  /* If we didn't find a common base, try the other way around.  */
  refp = &TREE_OPERAND (ref1, 0);
  while (handled_component_p (*refp)
	 && same_type_for_tbaa (TREE_TYPE (*refp), type2) == 0)
    refp = &TREE_OPERAND (*refp, 0);
  same_p = same_type_for_tbaa (TREE_TYPE (*refp), type2);
  /* If we couldn't compare types we have to bail out.  */
  if (same_p == -1)
    return true;
  else if (same_p == 1)
    {
      HOST_WIDE_INT offadj, sztmp, msztmp;
      get_ref_base_and_extent (*refp, &offadj, &sztmp, &msztmp);
      offset1 -= offadj;
      return ranges_overlap_p (offset1, max_size1, offset2, max_size2);
    }
  /* If we have two type access paths B1.path1 and B2.path2 they may
     only alias if either B1 is in B2.path2 or B2 is in B1.path1.  */
  return false;
}

/* Return true if two memory references based on the variables BASE1
   and BASE2 constrained to [OFFSET1, OFFSET1 + MAX_SIZE1[ and
   [OFFSET2, OFFSET2 + MAX_SIZE2[ may alias.  */

static bool
decl_refs_may_alias_p (tree base1,
		       HOST_WIDE_INT offset1, HOST_WIDE_INT max_size1,
		       tree base2,
		       HOST_WIDE_INT offset2, HOST_WIDE_INT max_size2)
{
  gcc_assert (SSA_VAR_P (base1) && SSA_VAR_P (base2));

  /* If both references are based on different variables, they cannot alias.  */
  if (!operand_equal_p (base1, base2, 0))
    return false;

  /* If both references are based on the same variable, they cannot alias if
     the accesses do not overlap.  */
  return ranges_overlap_p (offset1, max_size1, offset2, max_size2);
}

/* Return true if an indirect reference based on *PTR1 constrained
   to [OFFSET1, OFFSET1 + MAX_SIZE1[ may alias a variable based on BASE2
   constrained to [OFFSET2, OFFSET2 + MAX_SIZE2[.  *PTR1 and BASE2 have
   the alias sets BASE1_ALIAS_SET and BASE2_ALIAS_SET which can be -1
   in which case they are computed on-demand.  REF1 and REF2
   if non-NULL are the complete memory reference trees.  */

static bool
indirect_ref_may_alias_decl_p (tree ref1, tree ptr1,
			       HOST_WIDE_INT offset1, HOST_WIDE_INT max_size1,
			       alias_set_type base1_alias_set,
			       tree ref2, tree base2,
			       HOST_WIDE_INT offset2, HOST_WIDE_INT max_size2,
			       alias_set_type base2_alias_set)
{
  /* If only one reference is based on a variable, they cannot alias if
     the pointer access is beyond the extent of the variable access.
     (the pointer base cannot validly point to an offset less than zero
     of the variable).
     They also cannot alias if the pointer may not point to the decl.  */
  if (max_size2 != -1
      && !ranges_overlap_p (offset1, max_size1, 0, offset2 + max_size2))
    return false;
  if (!ptr_deref_may_alias_decl_p (ptr1, base2))
    return false;

  /* Disambiguations that rely on strict aliasing rules follow.  */
  if (!flag_strict_aliasing)
    return true;

  /* If the alias set for a pointer access is zero all bets are off.  */
  if (base1_alias_set == -1)
    base1_alias_set = get_deref_alias_set (ptr1);
  if (base1_alias_set == 0)
    return true;
  if (base2_alias_set == -1)
    base2_alias_set = get_alias_set (base2);

  /* If both references are through the same type, they do not alias
     if the accesses do not overlap.  This does extra disambiguation
     for mixed/pointer accesses but requires strict aliasing.  */
  if (same_type_for_tbaa (TREE_TYPE (TREE_TYPE (ptr1)),
			  TREE_TYPE (base2)) == 1)
    return ranges_overlap_p (offset1, max_size1, offset2, max_size2);

  /* The only way to access a variable is through a pointer dereference
     of the same alias set or a subset of it.  */
  if (base1_alias_set != base2_alias_set
      && !alias_set_subset_of (base1_alias_set, base2_alias_set))
    return false;

  /* Do access-path based disambiguation.  */
  if (ref1 && ref2
      && handled_component_p (ref1)
      && handled_component_p (ref2))
    return nonaliasing_component_refs_p (ref1, TREE_TYPE (TREE_TYPE (ptr1)),
					 offset1, max_size1,
					 ref2, TREE_TYPE (base2),
					 offset2, max_size2);

  return true;
}

/* Return true if two indirect references based on *PTR1
   and *PTR2 constrained to [OFFSET1, OFFSET1 + MAX_SIZE1[ and
   [OFFSET2, OFFSET2 + MAX_SIZE2[ may alias.  *PTR1 and *PTR2 have
   the alias sets BASE1_ALIAS_SET and BASE2_ALIAS_SET which can be -1
   in which case they are computed on-demand.  REF1 and REF2
   if non-NULL are the complete memory reference trees. */

static bool
indirect_refs_may_alias_p (tree ref1, tree ptr1,
			   HOST_WIDE_INT offset1, HOST_WIDE_INT max_size1,
			   alias_set_type base1_alias_set,
			   tree ref2, tree ptr2,
			   HOST_WIDE_INT offset2, HOST_WIDE_INT max_size2,
			   alias_set_type base2_alias_set)
{
  /* If both bases are based on pointers they cannot alias if they may not
     point to the same memory object or if they point to the same object
     and the accesses do not overlap.  */
  if (operand_equal_p (ptr1, ptr2, 0))
    return ranges_overlap_p (offset1, max_size1, offset2, max_size2);
  if (!ptr_derefs_may_alias_p (ptr1, ptr2))
    return false;

  /* Disambiguations that rely on strict aliasing rules follow.  */
  if (!flag_strict_aliasing)
    return true;

  /* If the alias set for a pointer access is zero all bets are off.  */
  if (base1_alias_set == -1)
    base1_alias_set = get_deref_alias_set (ptr1);
  if (base1_alias_set == 0)
    return true;
  if (base2_alias_set == -1)
    base2_alias_set = get_deref_alias_set (ptr2);
  if (base2_alias_set == 0)
    return true;

  /* If both references are through the same type, they do not alias
     if the accesses do not overlap.  This does extra disambiguation
     for mixed/pointer accesses but requires strict aliasing.  */
  if (same_type_for_tbaa (TREE_TYPE (TREE_TYPE (ptr1)),
			  TREE_TYPE (TREE_TYPE (ptr2))) == 1)
    return ranges_overlap_p (offset1, max_size1, offset2, max_size2);

  /* Do type-based disambiguation.  */
  if (base1_alias_set != base2_alias_set
      && !alias_sets_conflict_p (base1_alias_set, base2_alias_set))
    return false;

  /* Do access-path based disambiguation.  */
  if (ref1 && ref2
      && handled_component_p (ref1)
      && handled_component_p (ref2))
    return nonaliasing_component_refs_p (ref1, TREE_TYPE (TREE_TYPE (ptr1)),
					 offset1, max_size1,
					 ref2, TREE_TYPE (TREE_TYPE (ptr2)),
					 offset2, max_size2);

  return true;
}

/* Return true, if the two memory references REF1 and REF2 may alias.  */

static bool
refs_may_alias_p_1 (tree ref1, tree ref2)
{
  tree base1, base2;
  HOST_WIDE_INT offset1 = 0, offset2 = 0;
  HOST_WIDE_INT size1 = -1, size2 = -1;
  HOST_WIDE_INT max_size1 = -1, max_size2 = -1;
  bool var1_p, var2_p, ind1_p, ind2_p;

  gcc_assert ((SSA_VAR_P (ref1)
	       || handled_component_p (ref1)
	       || INDIRECT_REF_P (ref1)
	       || TREE_CODE (ref1) == TARGET_MEM_REF)
	      && (SSA_VAR_P (ref2)
		  || handled_component_p (ref2)
		  || INDIRECT_REF_P (ref2)
		  || TREE_CODE (ref2) == TARGET_MEM_REF));

  /* Defer to TBAA if possible.  */
  if (flag_strict_aliasing
      && !alias_sets_conflict_p (get_alias_set (ref1), get_alias_set (ref2)))
    return false;

  /* If one reference is a TARGET_MEM_REF weird things are allowed.  */
  if (TREE_CODE (ref1) == TARGET_MEM_REF
      || TREE_CODE (ref2) == TARGET_MEM_REF)
    return true;

  /* Decompose the references into their base objects and the access.  */
  base1 = get_ref_base_and_extent (ref1, &offset1, &size1, &max_size1);
  base2 = get_ref_base_and_extent (ref2, &offset2, &size2, &max_size2);

  /* We can end up with registers or constants as bases for example from
     *D.1663_44 = VIEW_CONVERT_EXPR<struct DB_LSN>(__tmp$B0F64_59);
     which is seen as a struct copy.  */
  if (TREE_CODE (base1) == SSA_NAME
      || CONSTANT_CLASS_P (base1)
      || TREE_CODE (base2) == SSA_NAME
      || CONSTANT_CLASS_P (base2))
    return false;

  var1_p = SSA_VAR_P (base1);
  var2_p = SSA_VAR_P (base2);
  ind1_p = INDIRECT_REF_P (base1);
  ind2_p = INDIRECT_REF_P (base2);
  if (var1_p && var2_p)
    return decl_refs_may_alias_p (base1, offset1, max_size1,
				  base2, offset2, max_size2);
  else if (var1_p && ind2_p)
    return indirect_ref_may_alias_decl_p (ref2, TREE_OPERAND (base2, 0),
					  offset2, max_size2, -1,
					  ref1, base1,
					  offset1, max_size1, -1);
  else if (ind1_p && var2_p)
    return indirect_ref_may_alias_decl_p (ref1, TREE_OPERAND (base1, 0),
					  offset1, max_size1, -1,
					  ref2, base2,
					  offset2, max_size2, -1);
  else if (ind1_p && ind2_p)
    return indirect_refs_may_alias_p (ref1, TREE_OPERAND (base1, 0),
				      offset1, max_size1, -1,
				      ref2, TREE_OPERAND (base2, 0),
				      offset2, max_size2, -1);

  gcc_unreachable ();
}

bool
refs_may_alias_p (tree ref1, tree ref2)
{
  bool res = refs_may_alias_p_1 (ref1, ref2);
  if (res)
    ++alias_stats.refs_may_alias_p_may_alias;
  else
    ++alias_stats.refs_may_alias_p_no_alias;
  return res;
}


/* If the call CALL may use the memory reference REF return true,
   otherwise return false.  */

static bool
ref_maybe_used_by_call_p_1 (gimple call, tree ref)
{
  tree base;
  unsigned i;
  int flags = gimple_call_flags (call);

  /* Const functions without a static chain do not implicitly use memory.  */
  if (!gimple_call_chain (call)
      && (flags & (ECF_CONST|ECF_NOVOPS)))
    goto process_args;

  /* If the reference is not based on a decl give up.
     ???  Handle indirect references by intersecting the call-used
     	  solution with that of the pointer.  */
  base = get_base_address (ref);
  if (!base
      || !DECL_P (base))
    return true;

  /* Check if base is a global static variable that is not read
     by the function.  */
  if (TREE_CODE (base) == VAR_DECL
      && TREE_STATIC (base)
      && !TREE_PUBLIC (base))
    {
      tree callee = gimple_call_fndecl (call);
      bitmap not_read;

      if (callee != NULL_TREE
	  && (not_read
	        = ipa_reference_get_not_read_global (cgraph_node (callee)))
	  && bitmap_bit_p (not_read, DECL_UID (base)))
	goto process_args;
    }

  /* If the base variable is call-used or call-clobbered then
     it may be used.  */
  if (flags & (ECF_PURE|ECF_CONST|ECF_LOOPING_CONST_OR_PURE|ECF_NOVOPS))
    {
      if (is_call_used (base))
	return true;
    }
  else
    {
      if (is_call_clobbered (base))
	return true;
    }

  /* Inspect call arguments for passed-by-value aliases.  */
process_args:
  for (i = 0; i < gimple_call_num_args (call); ++i)
    {
      tree op = gimple_call_arg (call, i);

      if (TREE_CODE (op) == EXC_PTR_EXPR
	  || TREE_CODE (op) == FILTER_EXPR)
	continue;

      if (TREE_CODE (op) == WITH_SIZE_EXPR)
	op = TREE_OPERAND (op, 0);

      if (TREE_CODE (op) != SSA_NAME
	  && !is_gimple_min_invariant (op)
	  && refs_may_alias_p (op, ref))
	return true;
    }

  return false;
}

static bool
ref_maybe_used_by_call_p (gimple call, tree ref)
{
  bool res = ref_maybe_used_by_call_p_1 (call, ref);
  if (res)
    ++alias_stats.ref_maybe_used_by_call_p_may_alias;
  else
    ++alias_stats.ref_maybe_used_by_call_p_no_alias;
  return res;
}


/* If the statement STMT may use the memory reference REF return
   true, otherwise return false.  */

bool
ref_maybe_used_by_stmt_p (gimple stmt, tree ref)
{
  if (is_gimple_assign (stmt))
    {
      tree rhs;

      /* All memory assign statements are single.  */
      if (!gimple_assign_single_p (stmt))
	return false;

      rhs = gimple_assign_rhs1 (stmt);
      if (is_gimple_reg (rhs)
	  || is_gimple_min_invariant (rhs)
	  || gimple_assign_rhs_code (stmt) == CONSTRUCTOR)
	return false;

      return refs_may_alias_p (rhs, ref);
    }
  else if (is_gimple_call (stmt))
    return ref_maybe_used_by_call_p (stmt, ref);

  return true;
}

/* If the call in statement CALL may clobber the memory reference REF
   return true, otherwise return false.  */

static bool
call_may_clobber_ref_p_1 (gimple call, tree ref)
{
  tree base;

  /* If the call is pure or const it cannot clobber anything.  */
  if (gimple_call_flags (call)
      & (ECF_PURE|ECF_CONST|ECF_LOOPING_CONST_OR_PURE|ECF_NOVOPS))
    return false;

  base = get_base_address (ref);
  if (!base)
    return true;

  if (TREE_CODE (base) == SSA_NAME
      || CONSTANT_CLASS_P (base))
    return false;

  /* Check if base is a global static variable that is not written
     by the function.  */
  if (TREE_CODE (base) == VAR_DECL
      && TREE_STATIC (base)
      && !TREE_PUBLIC (base))
    {
      tree callee = gimple_call_fndecl (call);
      bitmap not_written;

      if (callee != NULL_TREE
	  && (not_written
	        = ipa_reference_get_not_written_global (cgraph_node (callee)))
	  && bitmap_bit_p (not_written, DECL_UID (base)))
	return false;
    }

  if (DECL_P (base))
    return is_call_clobbered (base);

  return true;
}

static bool
call_may_clobber_ref_p (gimple call, tree ref)
{
  bool res = call_may_clobber_ref_p_1 (call, ref);
  if (res)
    ++alias_stats.call_may_clobber_ref_p_may_alias;
  else
    ++alias_stats.call_may_clobber_ref_p_no_alias;
  return res;
}


/* If the statement STMT may clobber the memory reference REF return true,
   otherwise return false.  */

bool
stmt_may_clobber_ref_p (gimple stmt, tree ref)
{
  if (is_gimple_call (stmt))
    {
      tree lhs = gimple_call_lhs (stmt);
      if (lhs
	  && !is_gimple_reg (lhs)
	  && refs_may_alias_p (ref, lhs))
	return true;

      return call_may_clobber_ref_p (stmt, ref);
    }
  else if (is_gimple_assign (stmt))
    return refs_may_alias_p (ref, gimple_assign_lhs (stmt));
  else if (gimple_code (stmt) == GIMPLE_ASM)
    return true;

  return false;
}

static tree get_continuation_for_phi (gimple, tree, bitmap *);

/* Walk the virtual use-def chain of VUSE until hitting the virtual operand
   TARGET or a statement clobbering the memory reference REF in which
   case false is returned.  The walk starts with VUSE, one argument of PHI.  */

static bool
maybe_skip_until (gimple phi, tree target, tree ref, tree vuse, bitmap *visited)
{
  if (!*visited)
    *visited = BITMAP_ALLOC (NULL);

  bitmap_set_bit (*visited, SSA_NAME_VERSION (PHI_RESULT (phi)));

  /* Walk until we hit the target.  */
  while (vuse != target)
    {
      gimple def_stmt = SSA_NAME_DEF_STMT (vuse);
      /* Recurse for PHI nodes.  */
      if (gimple_code (def_stmt) == GIMPLE_PHI)
	{
	  /* An already visited PHI node ends the walk successfully.  */
	  if (bitmap_bit_p (*visited, SSA_NAME_VERSION (PHI_RESULT (def_stmt))))
	    return true;
	  vuse = get_continuation_for_phi (def_stmt, ref, visited);
	  if (!vuse)
	    return false;
	  continue;
	}
      /* A clobbering statement or the end of the IL ends it failing.  */
      else if (gimple_nop_p (def_stmt)
	       || stmt_may_clobber_ref_p (def_stmt, ref))
	return false;
      vuse = gimple_vuse (def_stmt);
    }
  return true;
}

/* Starting from a PHI node for the virtual operand of the memory reference
   REF find a continuation virtual operand that allows to continue walking
   statements dominating PHI skipping only statements that cannot possibly
   clobber REF.  Returns NULL_TREE if no suitable virtual operand can
   be found.  */

static tree
get_continuation_for_phi (gimple phi, tree ref, bitmap *visited)
{
  unsigned nargs = gimple_phi_num_args (phi);

  /* Through a single-argument PHI we can simply look through.  */
  if (nargs == 1)
    return PHI_ARG_DEF (phi, 0);

  /* For two arguments try to skip non-aliasing code until we hit
     the phi argument definition that dominates the other one.  */
  if (nargs == 2)
    {
      tree arg0 = PHI_ARG_DEF (phi, 0);
      tree arg1 = PHI_ARG_DEF (phi, 1);
      gimple def0 = SSA_NAME_DEF_STMT (arg0);
      gimple def1 = SSA_NAME_DEF_STMT (arg1);

      if (arg0 == arg1)
	return arg0;
      else if (gimple_nop_p (def0)
	       || (!gimple_nop_p (def1)
		   && dominated_by_p (CDI_DOMINATORS,
				      gimple_bb (def1), gimple_bb (def0))))
	{
	  if (maybe_skip_until (phi, arg0, ref, arg1, visited))
	    return arg0;
	}
      else if (gimple_nop_p (def1)
	       || dominated_by_p (CDI_DOMINATORS,
				  gimple_bb (def0), gimple_bb (def1)))
	{
	  if (maybe_skip_until (phi, arg1, ref, arg0, visited))
	    return arg1;
	}
    }

  return NULL_TREE;
}

/* Based on the memory reference REF and its virtual use VUSE call
   WALKER for each virtual use that is equivalent to VUSE, including VUSE
   itself.  That is, for each virtual use for which its defining statement
   does not clobber REF.

   WALKER is called with REF, the current virtual use and DATA.  If
   WALKER returns non-NULL the walk stops and its result is returned.
   At the end of a non-successful walk NULL is returned.

   TODO: Cache the vector of equivalent vuses per ref, vuse pair.  */

void *
walk_non_aliased_vuses (tree ref, tree vuse,
			void *(*walker)(tree, tree, void *), void *data)
{
  bitmap visited = NULL;
  void *res;

  timevar_push (TV_ALIAS_STMT_WALK);

  do
    {
      gimple def_stmt;

      /* ???  Do we want to account this to TV_ALIAS_STMT_WALK?  */
      res = (*walker) (ref, vuse, data);
      if (res)
	break;

      def_stmt = SSA_NAME_DEF_STMT (vuse);
      if (gimple_nop_p (def_stmt))
	break;
      else if (gimple_code (def_stmt) == GIMPLE_PHI)
	vuse = get_continuation_for_phi (def_stmt, ref, &visited);
      else
	{
	  if (stmt_may_clobber_ref_p (def_stmt, ref))
	    break;
	  vuse = gimple_vuse (def_stmt);
	}
    }
  while (vuse);

  if (visited)
    BITMAP_FREE (visited);

  timevar_pop (TV_ALIAS_STMT_WALK);

  return res;
}


/* Based on the memory reference REF call WALKER for each vdef which
   defining statement may clobber REF, starting with VDEF.  If REF
   is NULL_TREE, each defining statement is visited.

   WALKER is called with REF, the current vdef and DATA.  If WALKER
   returns true the walk is stopped, otherwise it continues.

   At PHI nodes walk_aliased_vdefs forks into one walk for reach
   PHI argument (but only one walk continues on merge points), the
   return value is true if any of the walks was successful.

   The function returns the number of statements walked.  */

static unsigned int
walk_aliased_vdefs_1 (tree ref, tree vdef,
		      bool (*walker)(tree, tree, void *), void *data,
		      bitmap *visited, unsigned int cnt)
{
  do
    {
      gimple def_stmt = SSA_NAME_DEF_STMT (vdef);

      if (*visited
	  && !bitmap_set_bit (*visited, SSA_NAME_VERSION (vdef)))
	return cnt;

      if (gimple_nop_p (def_stmt))
	return cnt;
      else if (gimple_code (def_stmt) == GIMPLE_PHI)
	{
	  unsigned i;
	  if (!*visited)
	    *visited = BITMAP_ALLOC (NULL);
	  for (i = 0; i < gimple_phi_num_args (def_stmt); ++i)
	    cnt += walk_aliased_vdefs_1 (ref, gimple_phi_arg_def (def_stmt, i),
					 walker, data, visited, 0);
	  return cnt;
	}

      /* ???  Do we want to account this to TV_ALIAS_STMT_WALK?  */
      cnt++;
      if ((!ref
	   || stmt_may_clobber_ref_p (def_stmt, ref))
	  && (*walker) (ref, vdef, data))
	return cnt;

      vdef = gimple_vuse (def_stmt);
    }
  while (1);
}

unsigned int
walk_aliased_vdefs (tree ref, tree vdef,
		    bool (*walker)(tree, tree, void *), void *data,
		    bitmap *visited)
{
  bitmap local_visited = NULL;
  unsigned int ret;

  timevar_push (TV_ALIAS_STMT_WALK);

  ret = walk_aliased_vdefs_1 (ref, vdef, walker, data,
			      visited ? visited : &local_visited, 0);
  if (local_visited)
    BITMAP_FREE (local_visited);

  timevar_pop (TV_ALIAS_STMT_WALK);

  return ret;
}

