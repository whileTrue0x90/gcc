/* Preamble and helpers for the autogenerated gimple-match.c file.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.

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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "gimple.h"
#include "ssa.h"
#include "cgraph.h"
#include "fold-const.h"
#include "fold-const-call.h"
#include "stor-layout.h"
#include "gimple-fold.h"
#include "calls.h"
#include "tree-dfa.h"
#include "builtins.h"
#include "gimple-match.h"
#include "tree-pass.h"
#include "internal-fn.h"
#include "case-cfn-macros.h"
#include "gimplify.h"


/* Forward declarations of the private auto-generated matchers.
   They expect valueized operands in canonical order and do not
   perform simplification of all-constant operands.  */
static bool gimple_simplify (code_helper *, tree *,
			     gimple_seq *, tree (*)(tree),
			     code_helper, tree, tree);
static bool gimple_simplify (code_helper *, tree *,
			     gimple_seq *, tree (*)(tree),
			     code_helper, tree, tree, tree);
static bool gimple_simplify (code_helper *, tree *,
			     gimple_seq *, tree (*)(tree),
			     code_helper, tree, tree, tree, tree);


/* Return whether T is a constant that we'll dispatch to fold to
   evaluate fully constant expressions.  */

static inline bool
constant_for_folding (tree t)
{
  return (CONSTANT_CLASS_P (t)
	  /* The following is only interesting to string builtins.  */
	  || (TREE_CODE (t) == ADDR_EXPR
	      && TREE_CODE (TREE_OPERAND (t, 0)) == STRING_CST));
}


/* Helper that matches and simplifies the toplevel result from
   a gimple_simplify run (where we don't want to build
   a stmt in case it's used in in-place folding).  Replaces
   *RES_CODE and *RES_OPS with a simplified and/or canonicalized
   result and returns whether any change was made.  */

bool
gimple_resimplify1 (gimple_seq *seq,
		    code_helper *res_code, tree type, tree *res_ops,
		    tree (*valueize)(tree))
{
  if (constant_for_folding (res_ops[0]))
    {
      tree tem = NULL_TREE;
      if (res_code->is_tree_code ())
	tem = const_unop (*res_code, type, res_ops[0]);
      else
	tem = fold_const_call (combined_fn (*res_code), type, res_ops[0]);
      if (tem != NULL_TREE
	  && CONSTANT_CLASS_P (tem))
	{
	  res_ops[0] = tem;
	  res_ops[1] = NULL_TREE;
	  res_ops[2] = NULL_TREE;
	  *res_code = TREE_CODE (res_ops[0]);
	  return true;
	}
    }

  code_helper res_code2;
  tree res_ops2[3] = {};
  if (gimple_simplify (&res_code2, res_ops2, seq, valueize,
		       *res_code, type, res_ops[0]))
    {
      *res_code = res_code2;
      res_ops[0] = res_ops2[0];
      res_ops[1] = res_ops2[1];
      res_ops[2] = res_ops2[2];
      return true;
    }

  return false;
}

/* Helper that matches and simplifies the toplevel result from
   a gimple_simplify run (where we don't want to build
   a stmt in case it's used in in-place folding).  Replaces
   *RES_CODE and *RES_OPS with a simplified and/or canonicalized
   result and returns whether any change was made.  */

bool
gimple_resimplify2 (gimple_seq *seq,
		    code_helper *res_code, tree type, tree *res_ops,
		    tree (*valueize)(tree))
{
  if (constant_for_folding (res_ops[0]) && constant_for_folding (res_ops[1]))
    {
      tree tem = NULL_TREE;
      if (res_code->is_tree_code ())
	tem = const_binop (*res_code, type, res_ops[0], res_ops[1]);
      else
	tem = fold_const_call (combined_fn (*res_code), type,
			       res_ops[0], res_ops[1]);
      if (tem != NULL_TREE
	  && CONSTANT_CLASS_P (tem))
	{
	  res_ops[0] = tem;
	  res_ops[1] = NULL_TREE;
	  res_ops[2] = NULL_TREE;
	  *res_code = TREE_CODE (res_ops[0]);
	  return true;
	}
    }

  /* Canonicalize operand order.  */
  bool canonicalized = false;
  if (res_code->is_tree_code ()
      && (TREE_CODE_CLASS ((enum tree_code) *res_code) == tcc_comparison
	  || commutative_tree_code (*res_code))
      && tree_swap_operands_p (res_ops[0], res_ops[1], false))
    {
      std::swap (res_ops[0], res_ops[1]);
      if (TREE_CODE_CLASS ((enum tree_code) *res_code) == tcc_comparison)
	*res_code = swap_tree_comparison (*res_code);
      canonicalized = true;
    }

  code_helper res_code2;
  tree res_ops2[3] = {};
  if (gimple_simplify (&res_code2, res_ops2, seq, valueize,
		       *res_code, type, res_ops[0], res_ops[1]))
    {
      *res_code = res_code2;
      res_ops[0] = res_ops2[0];
      res_ops[1] = res_ops2[1];
      res_ops[2] = res_ops2[2];
      return true;
    }

  return canonicalized;
}

/* Helper that matches and simplifies the toplevel result from
   a gimple_simplify run (where we don't want to build
   a stmt in case it's used in in-place folding).  Replaces
   *RES_CODE and *RES_OPS with a simplified and/or canonicalized
   result and returns whether any change was made.  */

bool
gimple_resimplify3 (gimple_seq *seq,
		    code_helper *res_code, tree type, tree *res_ops,
		    tree (*valueize)(tree))
{
  if (constant_for_folding (res_ops[0]) && constant_for_folding (res_ops[1])
      && constant_for_folding (res_ops[2]))
    {
      tree tem = NULL_TREE;
      if (res_code->is_tree_code ())
	tem = fold_ternary/*_to_constant*/ (*res_code, type, res_ops[0],
					    res_ops[1], res_ops[2]);
      else
	tem = fold_const_call (combined_fn (*res_code), type,
			       res_ops[0], res_ops[1], res_ops[2]);
      if (tem != NULL_TREE
	  && CONSTANT_CLASS_P (tem))
	{
	  res_ops[0] = tem;
	  res_ops[1] = NULL_TREE;
	  res_ops[2] = NULL_TREE;
	  *res_code = TREE_CODE (res_ops[0]);
	  return true;
	}
    }

  /* Canonicalize operand order.  */
  bool canonicalized = false;
  if (res_code->is_tree_code ()
      && commutative_ternary_tree_code (*res_code)
      && tree_swap_operands_p (res_ops[0], res_ops[1], false))
    {
      std::swap (res_ops[0], res_ops[1]);
      canonicalized = true;
    }

  code_helper res_code2;
  tree res_ops2[3] = {};
  if (gimple_simplify (&res_code2, res_ops2, seq, valueize,
		       *res_code, type,
		       res_ops[0], res_ops[1], res_ops[2]))
    {
      *res_code = res_code2;
      res_ops[0] = res_ops2[0];
      res_ops[1] = res_ops2[1];
      res_ops[2] = res_ops2[2];
      return true;
    }

  return canonicalized;
}


/* If in GIMPLE expressions with CODE go as single-rhs build
   a GENERIC tree for that expression into *OP0.  */

void
maybe_build_generic_op (enum tree_code code, tree type, tree *ops)
{
  switch (code)
    {
    case REALPART_EXPR:
    case IMAGPART_EXPR:
    case VIEW_CONVERT_EXPR:
      ops[0] = build1 (code, type, ops[0]);
      break;
    case BIT_FIELD_REF:
      ops[0] = build3 (code, type, ops[0], ops[1], ops[2]);
      ops[1] = ops[2] = NULL_TREE;
      break;
    default:;
    }
}

tree (*mprts_hook) (code_helper, tree, tree *);

/* Try to build a call to FN with return type TYPE and the NARGS
   arguments given in OPS.  Return null if the target doesn't support
   the function.  */

static gcall *
build_call_internal (internal_fn fn, tree type, unsigned int nargs, tree *ops)
{
  if (direct_internal_fn_p (fn))
    {
      tree_pair types = direct_internal_fn_types (fn, type, ops);
      if (!direct_internal_fn_supported_p (fn, types, OPTIMIZE_FOR_BOTH))
	return NULL;
    }
  return gimple_build_call_internal (fn, nargs, ops[0], ops[1], ops[2]);
}

/* Push the exploded expression described by RCODE, TYPE and OPS
   as a statement to SEQ if necessary and return a gimple value
   denoting the value of the expression.  If RES is not NULL
   then the result will be always RES and even gimple values are
   pushed to SEQ.  */

tree
maybe_push_res_to_seq (code_helper rcode, tree type, tree *ops,
		       gimple_seq *seq, tree res)
{
  if (rcode.is_tree_code ())
    {
      if (!res
	  && gimple_simplified_result_is_gimple_val (rcode, ops))
	return ops[0];
      if (mprts_hook)
	{
	  tree tem = mprts_hook (rcode, type, ops);
	  if (tem)
	    return tem;
	}
      if (!seq)
	return NULL_TREE;
      /* Play safe and do not allow abnormals to be mentioned in
         newly created statements.  */
      if ((TREE_CODE (ops[0]) == SSA_NAME
	   && SSA_NAME_OCCURS_IN_ABNORMAL_PHI (ops[0]))
	  || (ops[1]
	      && TREE_CODE (ops[1]) == SSA_NAME
	      && SSA_NAME_OCCURS_IN_ABNORMAL_PHI (ops[1]))
	  || (ops[2]
	      && TREE_CODE (ops[2]) == SSA_NAME
	      && SSA_NAME_OCCURS_IN_ABNORMAL_PHI (ops[2]))
	  || (COMPARISON_CLASS_P (ops[0])
	      && ((TREE_CODE (TREE_OPERAND (ops[0], 0)) == SSA_NAME
		   && SSA_NAME_OCCURS_IN_ABNORMAL_PHI (TREE_OPERAND (ops[0],
								     0)))
		  || (TREE_CODE (TREE_OPERAND (ops[0], 1)) == SSA_NAME
		      && SSA_NAME_OCCURS_IN_ABNORMAL_PHI (TREE_OPERAND (ops[0],
									1))))))
	return NULL_TREE;
      if (!res)
	{
	  if (gimple_in_ssa_p (cfun))
	    res = make_ssa_name (type);
	  else
	    res = create_tmp_reg (type);
	}
      maybe_build_generic_op (rcode, type, ops);
      gimple *new_stmt = gimple_build_assign (res, rcode,
					     ops[0], ops[1], ops[2]);
      gimple_seq_add_stmt_without_update (seq, new_stmt);
      return res;
    }
  else
    {
      if (!seq)
	return NULL_TREE;
      combined_fn fn = rcode;
      /* Play safe and do not allow abnormals to be mentioned in
         newly created statements.  */
      unsigned nargs;
      for (nargs = 0; nargs < 3; ++nargs)
	{
	  if (!ops[nargs])
	    break;
	  if (TREE_CODE (ops[nargs]) == SSA_NAME
	      && SSA_NAME_OCCURS_IN_ABNORMAL_PHI (ops[nargs]))
	    return NULL_TREE;
	}
      gcc_assert (nargs != 0);
      gcall *new_stmt = NULL;
      if (internal_fn_p (fn))
	{
	  /* Generate the given function if we can.  */
	  internal_fn ifn = as_internal_fn (fn);
	  new_stmt = build_call_internal (ifn, type, nargs, ops);
	  if (!new_stmt)
	    return NULL_TREE;
	}
      else
	{
	  /* Find the function we want to call.  */
	  tree decl = builtin_decl_implicit (as_builtin_fn (fn));
	  if (!decl)
	    return NULL;

	  /* We can't and should not emit calls to non-const functions.  */
	  if (!(flags_from_decl_or_type (decl) & ECF_CONST))
	    return NULL;

	  new_stmt = gimple_build_call (decl, nargs, ops[0], ops[1], ops[2]);
	}
      if (!res)
	{
	  if (gimple_in_ssa_p (cfun))
	    res = make_ssa_name (type);
	  else
	    res = create_tmp_reg (type);
	}
      gimple_call_set_lhs (new_stmt, res);
      gimple_seq_add_stmt_without_update (seq, new_stmt);
      return res;
    }
}


/* Public API overloads follow for operation being tree_code or
   built_in_function and for one to three operands or arguments.
   They return NULL_TREE if nothing could be simplified or
   the resulting simplified value with parts pushed to SEQ.
   If SEQ is NULL then if the simplification needs to create
   new stmts it will fail.  If VALUEIZE is non-NULL then all
   SSA names will be valueized using that hook prior to
   applying simplifications.  */

/* Unary ops.  */

tree
gimple_simplify (enum tree_code code, tree type,
		 tree op0,
		 gimple_seq *seq, tree (*valueize)(tree))
{
  if (constant_for_folding (op0))
    {
      tree res = const_unop (code, type, op0);
      if (res != NULL_TREE
	  && CONSTANT_CLASS_P (res))
	return res;
    }

  code_helper rcode;
  tree ops[3] = {};
  if (!gimple_simplify (&rcode, ops, seq, valueize,
			code, type, op0))
    return NULL_TREE;
  return maybe_push_res_to_seq (rcode, type, ops, seq);
}

/* Binary ops.  */

tree
gimple_simplify (enum tree_code code, tree type,
		 tree op0, tree op1,
		 gimple_seq *seq, tree (*valueize)(tree))
{
  if (constant_for_folding (op0) && constant_for_folding (op1))
    {
      tree res = const_binop (code, type, op0, op1);
      if (res != NULL_TREE
	  && CONSTANT_CLASS_P (res))
	return res;
    }

  /* Canonicalize operand order both for matching and fallback stmt
     generation.  */
  if ((commutative_tree_code (code)
       || TREE_CODE_CLASS (code) == tcc_comparison)
      && tree_swap_operands_p (op0, op1, false))
    {
      std::swap (op0, op1);
      if (TREE_CODE_CLASS (code) == tcc_comparison)
	code = swap_tree_comparison (code);
    }

  code_helper rcode;
  tree ops[3] = {};
  if (!gimple_simplify (&rcode, ops, seq, valueize,
			code, type, op0, op1))
    return NULL_TREE;
  return maybe_push_res_to_seq (rcode, type, ops, seq);
}

/* Ternary ops.  */

tree
gimple_simplify (enum tree_code code, tree type,
		 tree op0, tree op1, tree op2,
		 gimple_seq *seq, tree (*valueize)(tree))
{
  if (constant_for_folding (op0) && constant_for_folding (op1)
      && constant_for_folding (op2))
    {
      tree res = fold_ternary/*_to_constant */ (code, type, op0, op1, op2);
      if (res != NULL_TREE
	  && CONSTANT_CLASS_P (res))
	return res;
    }

  /* Canonicalize operand order both for matching and fallback stmt
     generation.  */
  if (commutative_ternary_tree_code (code)
      && tree_swap_operands_p (op0, op1, false))
    std::swap (op0, op1);

  code_helper rcode;
  tree ops[3] = {};
  if (!gimple_simplify (&rcode, ops, seq, valueize,
			code, type, op0, op1, op2))
    return NULL_TREE;
  return maybe_push_res_to_seq (rcode, type, ops, seq);
}

/* Builtin function with one argument.  */

tree
gimple_simplify (enum built_in_function fn, tree type,
		 tree arg0,
		 gimple_seq *seq, tree (*valueize)(tree))
{
  if (constant_for_folding (arg0))
    {
      tree res = fold_const_call (as_combined_fn (fn), type, arg0);
      if (res && CONSTANT_CLASS_P (res))
	return res;
    }

  code_helper rcode;
  tree ops[3] = {};
  if (!gimple_simplify (&rcode, ops, seq, valueize,
			as_combined_fn (fn), type, arg0))
    return NULL_TREE;
  return maybe_push_res_to_seq (rcode, type, ops, seq);
}

/* Builtin function with two arguments.  */

tree
gimple_simplify (enum built_in_function fn, tree type,
		 tree arg0, tree arg1,
		 gimple_seq *seq, tree (*valueize)(tree))
{
  if (constant_for_folding (arg0)
      && constant_for_folding (arg1))
    {
      tree res = fold_const_call (as_combined_fn (fn), type, arg0, arg1);
      if (res && CONSTANT_CLASS_P (res))
	return res;
    }

  code_helper rcode;
  tree ops[3] = {};
  if (!gimple_simplify (&rcode, ops, seq, valueize,
			as_combined_fn (fn), type, arg0, arg1))
    return NULL_TREE;
  return maybe_push_res_to_seq (rcode, type, ops, seq);
}

/* Builtin function with three arguments.  */

tree
gimple_simplify (enum built_in_function fn, tree type,
		 tree arg0, tree arg1, tree arg2,
		 gimple_seq *seq, tree (*valueize)(tree))
{
  if (constant_for_folding (arg0)
      && constant_for_folding (arg1)
      && constant_for_folding (arg2))
    {
      tree res = fold_const_call (as_combined_fn (fn), type, arg0, arg1, arg2);
      if (res && CONSTANT_CLASS_P (res))
	return res;
    }

  code_helper rcode;
  tree ops[3] = {};
  if (!gimple_simplify (&rcode, ops, seq, valueize,
			as_combined_fn (fn), type, arg0, arg1, arg2))
    return NULL_TREE;
  return maybe_push_res_to_seq (rcode, type, ops, seq);
}

/* Helper for gimple_simplify valueizing OP using VALUEIZE and setting
   VALUEIZED to true if valueization changed OP.  */

static inline tree
do_valueize (tree op, tree (*valueize)(tree), bool &valueized)
{
  if (valueize && TREE_CODE (op) == SSA_NAME)
    {
      tree tem = valueize (op);
      if (tem && tem != op)
	{
	  op = tem;
	  valueized = true;
	}
    }
  return op;
}

/* The main STMT based simplification entry.  It is used by the fold_stmt
   and the fold_stmt_to_constant APIs.  */

bool
gimple_simplify (gimple *stmt,
		 code_helper *rcode, tree *ops,
		 gimple_seq *seq,
		 tree (*valueize)(tree), tree (*top_valueize)(tree))
{
  switch (gimple_code (stmt))
    {
    case GIMPLE_ASSIGN:
      {
	enum tree_code code = gimple_assign_rhs_code (stmt);
	tree type = TREE_TYPE (gimple_assign_lhs (stmt));
	switch (gimple_assign_rhs_class (stmt))
	  {
	  case GIMPLE_SINGLE_RHS:
	    if (code == REALPART_EXPR
		|| code == IMAGPART_EXPR
		|| code == VIEW_CONVERT_EXPR)
	      {
		tree op0 = TREE_OPERAND (gimple_assign_rhs1 (stmt), 0);
		bool valueized = false;
		op0 = do_valueize (op0, top_valueize, valueized);
		*rcode = code;
		ops[0] = op0;
		return (gimple_resimplify1 (seq, rcode, type, ops, valueize)
			|| valueized);
	      }
	    else if (code == BIT_FIELD_REF)
	      {
		tree rhs1 = gimple_assign_rhs1 (stmt);
		tree op0 = TREE_OPERAND (rhs1, 0);
		bool valueized = false;
		op0 = do_valueize (op0, top_valueize, valueized);
		*rcode = code;
		ops[0] = op0;
		ops[1] = TREE_OPERAND (rhs1, 1);
		ops[2] = TREE_OPERAND (rhs1, 2);
		return (gimple_resimplify3 (seq, rcode, type, ops, valueize)
			|| valueized);
	      }
	    else if (code == SSA_NAME
		     && top_valueize)
	      {
		tree op0 = gimple_assign_rhs1 (stmt);
		tree valueized = top_valueize (op0);
		if (!valueized || op0 == valueized)
		  return false;
		ops[0] = valueized;
		*rcode = TREE_CODE (op0);
		return true;
	      }
	    break;
	  case GIMPLE_UNARY_RHS:
	    {
	      tree rhs1 = gimple_assign_rhs1 (stmt);
	      bool valueized = false;
	      rhs1 = do_valueize (rhs1, top_valueize, valueized);
	      *rcode = code;
	      ops[0] = rhs1;
	      return (gimple_resimplify1 (seq, rcode, type, ops, valueize)
		      || valueized);
	    }
	  case GIMPLE_BINARY_RHS:
	    {
	      tree rhs1 = gimple_assign_rhs1 (stmt);
	      tree rhs2 = gimple_assign_rhs2 (stmt);
	      bool valueized = false;
	      rhs1 = do_valueize (rhs1, top_valueize, valueized);
	      rhs2 = do_valueize (rhs2, top_valueize, valueized);
	      *rcode = code;
	      ops[0] = rhs1;
	      ops[1] = rhs2;
	      return (gimple_resimplify2 (seq, rcode, type, ops, valueize)
		      || valueized);
	    }
	  case GIMPLE_TERNARY_RHS:
	    {
	      bool valueized = false;
	      tree rhs1 = gimple_assign_rhs1 (stmt);
	      /* If this is a [VEC_]COND_EXPR first try to simplify an
		 embedded GENERIC condition.  */
	      if (code == COND_EXPR
		  || code == VEC_COND_EXPR)
		{
		  if (COMPARISON_CLASS_P (rhs1))
		    {
		      tree lhs = TREE_OPERAND (rhs1, 0);
		      tree rhs = TREE_OPERAND (rhs1, 1);
		      lhs = do_valueize (lhs, top_valueize, valueized);
		      rhs = do_valueize (rhs, top_valueize, valueized);
		      code_helper rcode2 = TREE_CODE (rhs1);
		      tree ops2[3] = {};
		      ops2[0] = lhs;
		      ops2[1] = rhs;
		      if ((gimple_resimplify2 (seq, &rcode2, TREE_TYPE (rhs1),
					       ops2, valueize)
			   || valueized)
			  && rcode2.is_tree_code ())
			{
			  valueized = true;
			  if (TREE_CODE_CLASS ((enum tree_code)rcode2)
			      == tcc_comparison)
			    rhs1 = build2 (rcode2, TREE_TYPE (rhs1),
					   ops2[0], ops2[1]);
			  else if (rcode2 == SSA_NAME
				   || rcode2 == INTEGER_CST
				   || rcode2 == VECTOR_CST)
			    rhs1 = ops2[0];
			  else
			    valueized = false;
			}
		    }
		}
	      tree rhs2 = gimple_assign_rhs2 (stmt);
	      tree rhs3 = gimple_assign_rhs3 (stmt);
	      rhs1 = do_valueize (rhs1, top_valueize, valueized);
	      rhs2 = do_valueize (rhs2, top_valueize, valueized);
	      rhs3 = do_valueize (rhs3, top_valueize, valueized);
	      *rcode = code;
	      ops[0] = rhs1;
	      ops[1] = rhs2;
	      ops[2] = rhs3;
	      return (gimple_resimplify3 (seq, rcode, type, ops, valueize)
		      || valueized);
	    }
	  default:
	    gcc_unreachable ();
	  }
	break;
      }

    case GIMPLE_CALL:
      /* ???  This way we can't simplify calls with side-effects.  */
      if (gimple_call_lhs (stmt) != NULL_TREE
	  && gimple_call_num_args (stmt) >= 1
	  && gimple_call_num_args (stmt) <= 3)
	{
	  bool valueized = false;
	  if (gimple_call_internal_p (stmt))
	    *rcode = as_combined_fn (gimple_call_internal_fn (stmt));
	  else
	    {
	      tree fn = gimple_call_fn (stmt);
	      if (!fn)
		return false;

	      fn = do_valueize (fn, top_valueize, valueized);
	      if (TREE_CODE (fn) != ADDR_EXPR
		  || TREE_CODE (TREE_OPERAND (fn, 0)) != FUNCTION_DECL)
		return false;

	      tree decl = TREE_OPERAND (fn, 0);
	      if (DECL_BUILT_IN_CLASS (decl) != BUILT_IN_NORMAL
		  || !gimple_builtin_call_types_compatible_p (stmt, decl))
		return false;

	      *rcode = as_combined_fn (DECL_FUNCTION_CODE (decl));
	    }

	  tree type = TREE_TYPE (gimple_call_lhs (stmt));
	  for (unsigned i = 0; i < gimple_call_num_args (stmt); ++i)
	    {
	      tree arg = gimple_call_arg (stmt, i);
	      ops[i] = do_valueize (arg, top_valueize, valueized);
	    }
	  switch (gimple_call_num_args (stmt))
	    {
	    case 1:
	      return (gimple_resimplify1 (seq, rcode, type, ops, valueize)
		      || valueized);
	    case 2:
	      return (gimple_resimplify2 (seq, rcode, type, ops, valueize)
		      || valueized);
	    case 3:
	      return (gimple_resimplify3 (seq, rcode, type, ops, valueize)
		      || valueized);
	    default:
	     gcc_unreachable ();
	    }
	}
      break;

    case GIMPLE_COND:
      {
	tree lhs = gimple_cond_lhs (stmt);
	tree rhs = gimple_cond_rhs (stmt);
	bool valueized = false;
	lhs = do_valueize (lhs, top_valueize, valueized);
	rhs = do_valueize (rhs, top_valueize, valueized);
	*rcode = gimple_cond_code (stmt);
	ops[0] = lhs;
	ops[1] = rhs;
        return (gimple_resimplify2 (seq, rcode,
				    boolean_type_node, ops, valueize)
		|| valueized);
      }

    default:
      break;
    }

  return false;
}


/* Helper for the autogenerated code, valueize OP.  */

inline tree
do_valueize (tree (*valueize)(tree), tree op)
{
  if (valueize && TREE_CODE (op) == SSA_NAME)
    return valueize (op);
  return op;
}

/* Routine to determine if the types T1 and T2 are effectively
   the same for GIMPLE.  If T1 or T2 is not a type, the test
   applies to their TREE_TYPE.  */

static inline bool
types_match (tree t1, tree t2)
{
  if (!TYPE_P (t1))
    t1 = TREE_TYPE (t1);
  if (!TYPE_P (t2))
    t2 = TREE_TYPE (t2);

  return types_compatible_p (t1, t2);
}

/* Return if T has a single use.  For GIMPLE, we also allow any
   non-SSA_NAME (ie constants) and zero uses to cope with uses
   that aren't linked up yet.  */

static inline bool
single_use (tree t)
{
  return TREE_CODE (t) != SSA_NAME || has_zero_uses (t) || has_single_use (t);
}

/* Return true if math operations should be canonicalized,
   e.g. sqrt(sqrt(x)) -> pow(x, 0.25).  */

static inline bool
canonicalize_math_p ()
{
  return !cfun || (cfun->curr_properties & PROP_gimple_opt_math) == 0;
}
