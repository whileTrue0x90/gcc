/* Gimple IR support functions.

   Copyright 2007, 2008 Free Software Foundation, Inc.
   Contributed by Aldy Hernandez <aldyh@redhat.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "ggc.h"
#include "errors.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "tree-gimple.h"
#include "gimple.h"
#include "diagnostic.h"
#include "tree-flow.h"
#include "value-prof.h"
#include "flags.h"

#define DEFGSCODE(SYM, NAME)	NAME,
const char *const gimple_code_name[] = {
#include "gimple.def"
};
#undef DEFGSCODE

#ifdef GATHER_STATISTICS
/* Gimple stats.  */

int gimple_alloc_counts[(int) gimple_alloc_kind_all];
int gimple_alloc_sizes[(int) gimple_alloc_kind_all];

/* Keep in sync with gimple.h:enum gimple_alloc_kind.  */
static const char * const gimple_alloc_kind_names[] = {
    "assignments",
    "phi nodes",
    "conditionals",
    "sequences",
    "everything else"
};

#endif /* GATHER_STATISTICS */

/* A cache of gimple_seq objects.  Sequences are created and destroyed
   fairly often during gimplification.  */
static GTY ((deletable)) struct gimple_seq_d *gimple_seq_cache;

/* Gimple tuple constructors.
   Note: Any constructor taking a ``gimple_seq'' as a parameter, can
   be passed a NULL to start with an empty sequence.  */

/* Set the code for statement G to CODE.  */

static inline void
gimple_set_code (gimple g, enum gimple_code code)
{
  g->gsbase.code = code;
}


/* Return the GSS_* identifier for the given GIMPLE statement CODE.  */

static enum gimple_statement_structure_enum
gss_for_code (enum gimple_code code)
{
  switch (code)
    {
    case GIMPLE_ASSIGN:
    case GIMPLE_CALL:
    case GIMPLE_RETURN:			return GSS_WITH_MEM_OPS;
    case GIMPLE_COND:
    case GIMPLE_GOTO:
    case GIMPLE_LABEL:
    case GIMPLE_SWITCH:			return GSS_WITH_OPS;
    case GIMPLE_ASM:			return GSS_ASM;
    case GIMPLE_BIND:			return GSS_BIND;
    case GIMPLE_CATCH:			return GSS_CATCH;
    case GIMPLE_EH_FILTER:		return GSS_EH_FILTER;
    case GIMPLE_NOP:			return GSS_BASE;
    case GIMPLE_PHI:			return GSS_PHI;
    case GIMPLE_RESX:			return GSS_RESX;
    case GIMPLE_TRY:			return GSS_TRY;
    case GIMPLE_WITH_CLEANUP_EXPR:	return GSS_WCE;
    case GIMPLE_OMP_CRITICAL:		return GSS_OMP_CRITICAL;
    case GIMPLE_OMP_FOR:		return GSS_OMP_FOR;
    case GIMPLE_OMP_MASTER:		
    case GIMPLE_OMP_ORDERED:
    case GIMPLE_OMP_SECTION:		return GSS_OMP;
    case GIMPLE_OMP_RETURN:
    case GIMPLE_OMP_SECTIONS_SWITCH:    return GSS_BASE;
    case GIMPLE_OMP_CONTINUE:		return GSS_OMP_CONTINUE;
    case GIMPLE_OMP_PARALLEL:		return GSS_OMP_PARALLEL;
    case GIMPLE_OMP_TASK:		return GSS_OMP_TASK;
    case GIMPLE_OMP_SECTIONS:		return GSS_OMP_SECTIONS;
    case GIMPLE_OMP_SINGLE:		return GSS_OMP_SINGLE;
    case GIMPLE_CHANGE_DYNAMIC_TYPE:	return GSS_CHANGE_DYNAMIC_TYPE;
    case GIMPLE_OMP_ATOMIC_LOAD:	return GSS_OMP_ATOMIC_LOAD;
    case GIMPLE_OMP_ATOMIC_STORE:	return GSS_OMP_ATOMIC_STORE;
    default:				gcc_unreachable ();
    }
}


/* Return the number of bytes needed to hold a GIMPLE statement with
   code CODE.  */

static size_t
gimple_size (enum gimple_code code)
{
  enum gimple_statement_structure_enum gss = gss_for_code (code);

  if (gss == GSS_WITH_OPS)
    return sizeof (struct gimple_statement_with_ops);
  else if (gss == GSS_WITH_MEM_OPS)
    return sizeof (struct gimple_statement_with_memory_ops);

  switch (code)
    {
    case GIMPLE_ASM:
      return sizeof (struct gimple_statement_asm);
    case GIMPLE_NOP:
      return sizeof (struct gimple_statement_base);
    case GIMPLE_BIND:
      return sizeof (struct gimple_statement_bind);
    case GIMPLE_CATCH:
      return sizeof (struct gimple_statement_catch);
    case GIMPLE_EH_FILTER:
      return sizeof (struct gimple_statement_eh_filter);
    case GIMPLE_TRY:
      return sizeof (struct gimple_statement_try);
    case GIMPLE_RESX:
      return sizeof (struct gimple_statement_resx);
    case GIMPLE_OMP_CRITICAL:
      return sizeof (struct gimple_statement_omp_critical);
    case GIMPLE_OMP_FOR:
      return sizeof (struct gimple_statement_omp_for);
    case GIMPLE_OMP_PARALLEL:
      return sizeof (struct gimple_statement_omp_parallel);
    case GIMPLE_OMP_TASK:
      return sizeof (struct gimple_statement_omp_task);
    case GIMPLE_OMP_SECTION:
    case GIMPLE_OMP_MASTER:
    case GIMPLE_OMP_ORDERED:
      return sizeof (struct gimple_statement_omp);
    case GIMPLE_OMP_RETURN:
      return sizeof (struct gimple_statement_base);
    case GIMPLE_OMP_CONTINUE:
      return sizeof (struct gimple_statement_omp_continue);
    case GIMPLE_OMP_SECTIONS:
      return sizeof (struct gimple_statement_omp_sections);
    case GIMPLE_OMP_SECTIONS_SWITCH:
      return sizeof (struct gimple_statement_base);
    case GIMPLE_OMP_SINGLE:
      return sizeof (struct gimple_statement_omp_single);
    case GIMPLE_OMP_ATOMIC_LOAD:
      return sizeof (struct gimple_statement_omp_atomic_load);
    case GIMPLE_OMP_ATOMIC_STORE:
      return sizeof (struct gimple_statement_omp_atomic_store);
    case GIMPLE_WITH_CLEANUP_EXPR:
      return sizeof (struct gimple_statement_wce);
    case GIMPLE_CHANGE_DYNAMIC_TYPE:
      return sizeof (struct gimple_statement_change_dynamic_type);
    default:
      break;
    }

  gcc_unreachable ();
}


/* Allocate memory for a GIMPLE statement with code CODE.  */

static gimple
gimple_alloc (enum gimple_code code)
{
  size_t size = gimple_size (code);
  gimple stmt;

#ifdef GATHER_STATISTICS
  enum gimple_alloc_kind kind = gimple_alloc_kind (code);

  gimple_alloc_counts[(int) kind]++;
  /* Statements with operands take more space.  Add that later.  */
  gimple_alloc_sizes[(int) kind] += size;
#endif

  stmt = ggc_alloc_cleared (size);
  gimple_set_code (stmt, code);
  return stmt;
}


/* Allocate memory for NUM operands for statement STMT.  The allocated
   memory is not returned.  After calling this function, the new
   operand vector will be available via gimple_op.  */

static void
gimple_alloc_ops (gimple stmt, size_t num_ops)
{
  unsigned int size = sizeof (tree) * num_ops;

  stmt->with_ops.op = ggc_alloc_cleared (size);
  stmt->with_ops.num_ops = num_ops;
  stmt->with_ops.modified = 1;
#ifdef GATHER_STATISTICS
  gimple_alloc_sizes[(int) gimple_alloc_kind (gimple_code (stmt))] += size;
#endif
}


/* Build a tuple with operands.  CODE is the statement to build (which
   must be one of the GIMPLE_WITH_OPS tuples).  SUBCODE is the sub-code
   for the new tuple.  NUM_OPS is the number of operands to allocate.  */ 

static gimple
gimple_build_with_ops (enum gimple_code code, enum tree_code subcode,
		       size_t num_ops)
{
  gimple s = gimple_alloc (code);
  gimple_set_subcode (s, subcode);
  if (num_ops > 0)
    gimple_alloc_ops (s, num_ops);

  return s;
}


/* Build a GIMPLE_RETURN statement returning RETVAL.  */

gimple
gimple_build_return (tree retval)
{
  gimple s = gimple_build_with_ops (GIMPLE_RETURN, 0, 1);
  if (retval)
    gimple_return_set_retval (s, retval);
  return s;
}

/* Helper for gimple_build_call, gimple_build_call_vec and
   gimple_build_call_from_tree.  Build the basic components of a
   GIMPLE_CALL statement to function FN with NARGS arguments.  */

static inline gimple
gimple_build_call_1 (tree fn, size_t nargs)
{
  gimple s = gimple_build_with_ops (GIMPLE_CALL, 0, nargs + 3);
  s->with_ops.op[1] = fn;
  return s;
}


/* Build a GIMPLE_CALL statement to function FN with the arguments
   specified in vector ARGS.  */

gimple
gimple_build_call_vec (tree fn, VEC(tree, heap) *args)
{
  size_t i;
  size_t nargs = VEC_length (tree, args);
  gimple call = gimple_build_call_1 (fn, nargs);

  for (i = 0; i < nargs; i++)
    gimple_call_set_arg (call, i, VEC_index (tree, args, i));

  return call;
}


/* Build a GIMPLE_CALL statement to function FN.  NARGS is the number of
   arguments.  The ... are the arguments.  */

gimple
gimple_build_call (tree fn, size_t nargs, ...)
{
  va_list ap;
  gimple call;
  size_t i;

  gcc_assert (TREE_CODE (fn) == FUNCTION_DECL || is_gimple_call_addr (fn));

  call = gimple_build_call_1 (fn, nargs);

  va_start (ap, nargs);
  for (i = 0; i < nargs; i++)
    gimple_call_set_arg (call, i, va_arg (ap, tree));
  va_end (ap);

  return call;
}


/* Build a GIMPLE_CALL statement from CALL_EXPR T.  Note that T is
   assumed to be in GIMPLE form already.  Minimal checking is done of
   this fact.  */

gimple
gimple_build_call_from_tree (tree t)
{
  size_t i, nargs;
  gimple call;
  tree fndecl = get_callee_fndecl (t);

  gcc_assert (TREE_CODE (t) == CALL_EXPR);

  nargs = call_expr_nargs (t);
  call = gimple_build_call_1 (fndecl ? fndecl : CALL_EXPR_FN (t), nargs);

  for (i = 0; i < nargs; i++)
    gimple_call_set_arg (call, i, CALL_EXPR_ARG (t, i));

  gimple_set_block (call, TREE_BLOCK (t));

  /* Carry all the CALL_EXPR flags to the new GIMPLE_CALL.  */
  gimple_call_set_chain (call, CALL_EXPR_STATIC_CHAIN (t));
  gimple_call_set_tail (call, CALL_EXPR_TAILCALL (t));
  gimple_call_set_cannot_inline (call, CALL_CANNOT_INLINE_P (t));
  gimple_call_set_return_slot_opt (call, CALL_EXPR_RETURN_SLOT_OPT (t));
  gimple_call_set_from_thunk (call, CALL_FROM_THUNK_P (t));
  gimple_call_set_va_arg_pack (call, CALL_EXPR_VA_ARG_PACK (t));

  return call;
}


/* Extract the operands and code for expression EXPR into *SUBCODE_P,
   *OP1_P and *OP2_P respectively.  */

void
extract_ops_from_tree (tree expr, enum tree_code *subcode_p, tree *op1_p,
		       tree *op2_p)
{
  enum gimple_rhs_class class;

  /* Make sure the EXPR is a valid GIMPLE RHS.  */
  gcc_assert (is_gimple_formal_tmp_rhs (expr));

  *subcode_p = TREE_CODE (expr);
  class = get_gimple_rhs_class (*subcode_p);

  if (class == GIMPLE_BINARY_RHS)
    {
      *op1_p = TREE_OPERAND (expr, 0);
      *op2_p = TREE_OPERAND (expr, 1);
    }
  else if (class == GIMPLE_UNARY_RHS)
    {
      *op1_p = TREE_OPERAND (expr, 0);
      *op2_p = NULL_TREE;
    }
  else if (class == GIMPLE_SINGLE_RHS)
    {
      *op1_p = expr;
      *op2_p = NULL_TREE;
    }
  else
    gcc_unreachable ();
}


/* Build a GIMPLE_ASSIGN statement.

   LHS of the assignment.
   RHS of the assignment which can be unary or binary.  */

gimple
gimple_build_assign (tree lhs, tree rhs)
{
  enum tree_code subcode;
  tree op1, op2;

  extract_ops_from_tree (rhs, &subcode, &op1, &op2);
  return gimple_build_assign_with_ops (subcode, lhs, op1, op2);
}


/* Build a GIMPLE_ASSIGN statement with sub-code SUBCODE and operands
   OP1 and OP2.  If OP2 is NULL then SUBCODE must be of class
   GIMPLE_UNARY_RHS or GIMPLE_SINGLE_RHS.  */

gimple
gimple_build_assign_with_ops (enum tree_code subcode, tree lhs, tree op1,
                              tree op2)
{
  size_t num_ops;
  gimple p;

  /* Need 1 operand for LHS and 1 or 2 for the RHS (depending on the
     code).  */
  num_ops = get_gimple_rhs_num_ops (subcode) + 1;
  
  p = gimple_build_with_ops (GIMPLE_ASSIGN, subcode, num_ops);
  gimple_assign_set_lhs (p, lhs);
  gimple_assign_set_rhs1 (p, op1);
  if (op2)
    {
      gcc_assert (num_ops > 2);
      gimple_assign_set_rhs2 (p, op2);
    }

  return p;
}


/* Build a GIMPLE_COND statement.

   PRED is the condition used to compare LHS and the RHS.
   T_LABEL is the label to jump to if the condition is true.
   F_LABEL is the label to jump to otherwise.  */

gimple
gimple_build_cond (enum tree_code pred_code, tree lhs, tree rhs,
		   tree t_label, tree f_label)
{
  gimple p;

  gcc_assert (TREE_CODE_CLASS (pred_code) == tcc_comparison);
  p = gimple_build_with_ops (GIMPLE_COND, pred_code, 4);
  gimple_cond_set_lhs (p, lhs);
  gimple_cond_set_rhs (p, rhs);
  gimple_cond_set_true_label (p, t_label);
  gimple_cond_set_false_label (p, f_label);
  return p;
}


/* Extract operands for a GIMPLE_COND statement out of COND_EXPR tree COND.  */

void
gimple_cond_get_ops_from_tree (tree cond, enum tree_code *code_p,
                               tree *lhs_p, tree *rhs_p)
{
  gcc_assert (TREE_CODE_CLASS (TREE_CODE (cond)) == tcc_comparison
	      || is_gimple_min_invariant (cond)
	      || SSA_VAR_P (cond));

  extract_ops_from_tree (cond, code_p, lhs_p, rhs_p);

  /* Canonicalize conditionals of the form 'if (VAL)'  */
  if (TREE_CODE_CLASS (*code_p) != tcc_comparison)
    {
      *code_p = NE_EXPR;
      gcc_assert (*lhs_p && *rhs_p == NULL_TREE);
      *rhs_p = fold_convert (TREE_TYPE (*lhs_p), integer_zero_node);
    }
}


/* Build a GIMPLE_COND statement from the conditional expression tree
   COND.  T_LABEL and F_LABEL are as in gimple_build_cond.  */

gimple
gimple_build_cond_from_tree (tree cond, tree t_label, tree f_label)
{
  enum tree_code code;
  tree lhs, rhs;

  gimple_cond_get_ops_from_tree (cond, &code, &lhs, &rhs);
  return gimple_build_cond (code, lhs, rhs, t_label, f_label);
}

/* Set code, lhs, and rhs of a GIMPLE_COND from a suitable
   boolean expression tree COND.  */

void
gimple_cond_set_condition_from_tree (gimple stmt, tree cond)
{
  enum tree_code code;
  tree lhs, rhs;

  gimple_cond_get_ops_from_tree (cond, &code, &lhs, &rhs);
  gimple_cond_set_code (stmt, code);
  gimple_cond_set_lhs (stmt, lhs);
  gimple_cond_set_rhs (stmt, rhs);
}

/* Build a GIMPLE_LABEL statement for LABEL.  */

gimple
gimple_build_label (tree label)
{
  gimple p = gimple_build_with_ops (GIMPLE_LABEL, 0, 1);
  gimple_label_set_label (p, label);
  return p;
}

/* Build a GIMPLE_GOTO statement to label DEST.  */

gimple
gimple_build_goto (tree dest)
{
  gimple p = gimple_build_with_ops (GIMPLE_GOTO, 0, 1);
  gimple_goto_set_dest (p, dest);
  return p;
}


/* Build a GIMPLE_NOP statement.  */

gimple 
gimple_build_nop (void)
{
  return gimple_alloc (GIMPLE_NOP);
}


/* Build a GIMPLE_BIND statement.
   VARS are the variables in BODY.
   BLOCK is the containing block.  */

gimple
gimple_build_bind (tree vars, gimple_seq body, tree block)
{
  gimple p = gimple_alloc (GIMPLE_BIND);
  gimple_bind_set_vars (p, vars);
  if (body)
    gimple_bind_set_body (p, body);
  gimple_set_block (p, block);
  return p;
}

/* Helper function to set the simple fields of a asm stmt.

   STRING is a pointer to a string that is the asm blocks assembly code.
   NINPUT is the number of register inputs.
   NOUTPUT is the number of register outputs.
   NCLOBBERS is the number of clobbered registers.
   */

static inline gimple
gimple_build_asm_1 (const char *string, size_t ninputs, size_t noutputs, 
                    size_t nclobbers)
{
  gimple p;
  int size = strlen (string);

  p = gimple_build_with_ops (GIMPLE_ASM, 0, ninputs + noutputs + nclobbers);

  p->gimple_asm.ni = ninputs;
  p->gimple_asm.no = noutputs;
  p->gimple_asm.nc = nclobbers;
  p->gimple_asm.string = ggc_alloc_string (string, size);
#ifdef GATHER_STATISTICS
  gimple_alloc_sizes[(int) gimple_alloc_kind (GIMPLE_ASM)] += size;
#endif
  
  return p;
}

/* Build a GIMPLE_ASM statement.

   STRING is the assembly code.
   NINPUT is the number of register inputs.
   NOUTPUT is the number of register outputs.
   NCLOBBERS is the number of clobbered registers.
   INPUTS is a vector of the input register parameters.
   OUTPUTS is a vector of the output register parameters.
   CLOBBERS is a vector of the clobbered register parameters.  */

gimple
gimple_build_asm_vec (const char *string, VEC(tree,gc)* inputs, 
                      VEC(tree,gc)* outputs, VEC(tree,gc)* clobbers)
{
  gimple p;
  size_t i;
  p = gimple_build_asm_1 (string,
                          VEC_length (tree, inputs),
                          VEC_length (tree, outputs), 
                          VEC_length (tree, clobbers));
  
  for (i = 0; i < VEC_length (tree, inputs); i++)
    gimple_asm_set_input_op (p, i, VEC_index (tree, inputs, i));

  for (i = 0; i < VEC_length (tree, outputs); i++)
    gimple_asm_set_output_op (p, i, VEC_index (tree, outputs, i));

  for (i = 0; i < VEC_length (tree, clobbers); i++)
    gimple_asm_set_clobber_op (p, i, VEC_index (tree, clobbers, i));
  
  return p;
}

/* Build a GIMPLE_ASM statement.

   STRING is the assembly code.
   NINPUT is the number of register inputs.
   NOUTPUT is the number of register outputs.
   NCLOBBERS is the number of clobbered registers.
   ... are trees for each input, output and clobbered register.  */

gimple
gimple_build_asm (const char *string, size_t ninputs, size_t noutputs, 
		  size_t nclobbers, ...)
{
  gimple p;
  size_t i;
  va_list ap;
  
  p = gimple_build_asm_1 (string, ninputs, noutputs, nclobbers);
  
  va_start (ap, nclobbers);

  for (i = 0; i < ninputs; i++)
    gimple_asm_set_input_op (p, i, va_arg (ap, tree));

  for (i = 0; i < noutputs; i++)
    gimple_asm_set_output_op (p, i, va_arg (ap, tree));

  for (i = 0; i < nclobbers; i++)
    gimple_asm_set_clobber_op (p, i, va_arg (ap, tree));

  va_end (ap);
  
  return p;
}

/* Build a GIMPLE_CATCH statement.

  TYPES are the catch types.
  HANDLER is the exception handler.  */

gimple
gimple_build_catch (tree types, gimple_seq handler)
{
  gimple p = gimple_alloc (GIMPLE_CATCH);
  gimple_catch_set_types (p, types);
  if (handler)
    gimple_catch_set_handler (p, handler);

  return p;
}

/* Build a GIMPLE_EH_FILTER statement.

   TYPES are the filter's types.
   FAILURE is the filter's failure action.  */

gimple
gimple_build_eh_filter (tree types, gimple_seq failure)
{
  gimple p = gimple_alloc (GIMPLE_EH_FILTER);
  gimple_eh_filter_set_types (p, types);
  if (failure)
    gimple_eh_filter_set_failure (p, failure);

  return p;
}

/* Build a GIMPLE_TRY statement.

   EVAL is the expression to evaluate.
   CLEANUP is the cleanup expression.
   KIND is either GIMPLE_TRY_CATCH or GIMPLE_TRY_FINALLY depending on
   whether this is a try/catch or a try/finally respectively.  */

gimple
gimple_build_try (gimple_seq eval, gimple_seq cleanup,
    		  enum gimple_try_flags kind)
{
  gimple p;

  gcc_assert (kind == GIMPLE_TRY_CATCH || kind == GIMPLE_TRY_FINALLY);
  p = gimple_alloc (GIMPLE_TRY);
  gimple_set_subcode (p, kind);
  if (eval)
    gimple_try_set_eval (p, eval);
  if (cleanup)
    gimple_try_set_cleanup (p, cleanup);

  return p;
}

/* Construct a GIMPLE_WITH_CLEANUP_EXPR statement.

   CLEANUP is the cleanup expression.  */

gimple
gimple_build_wce (gimple_seq cleanup)
{
  gimple p = gimple_alloc (GIMPLE_WITH_CLEANUP_EXPR);
  if (cleanup)
    gimple_wce_set_cleanup (p, cleanup);

  return p;
}


/* Build a GIMPLE_RESX statement.

   REGION is the region number from which this resx causes control flow to 
   leave.  */

gimple
gimple_build_resx (int region)
{
  gimple p = gimple_alloc (GIMPLE_RESX);
  gimple_resx_set_region (p, region);
  return p;
}


/* The helper for constructing a gimple switch statement.
   INDEX is the switch's index.
   NLABELS is the number of labels in the switch excluding the default.
   DEFAULT_LABEL is the default label for the switch statement.  */

static inline gimple 
gimple_build_switch_1 (size_t nlabels, tree index, tree default_label)
{
  /* nlabels + 1 default label + 1 index.  */
  gimple p = gimple_build_with_ops (GIMPLE_SWITCH, 0, nlabels + 1 + 1);
  gimple_switch_set_index (p, index);
  gimple_switch_set_default_label (p, default_label);
  return p;
}


/* Build a GIMPLE_SWITCH statement.

   INDEX is the switch's index.
   NLABELS is the number of labels in the switch excluding the DEFAULT_LABEL. 
   ... are the labels excluding the default.  */

gimple 
gimple_build_switch (size_t nlabels, tree index, tree default_label, ...)
{
  va_list al;
  size_t i;
  gimple p;
  
  p = gimple_build_switch_1 (nlabels, index, default_label);

  /* Store the rest of the labels.  */
  va_start (al, default_label);
  for (i = 1; i <= nlabels; i++)
    gimple_switch_set_label (p, i, va_arg (al, tree));
  va_end (al);

  return p;
}


/* Build a GIMPLE_SWITCH statement.

   INDEX is the switch's index.
   DEFAULT_LABEL is the default label
   ARGS is a vector of labels excluding the default.  */

gimple
gimple_build_switch_vec (tree index, tree default_label, VEC(tree, heap) *args)
{
  size_t i;
  size_t nlabels = VEC_length (tree, args);
  gimple p = gimple_build_switch_1 (nlabels, index, default_label);

  /*  Put labels in labels[1 - (nlabels + 1)].
     Default label is in labels[0].  */
  for (i = 1; i <= nlabels; i++)
    gimple_switch_set_label (p, i, VEC_index (tree, args, i - 1));

  return p;
}


/* Build a GIMPLE_OMP_CRITICAL statement.

   BODY is the sequence of statements for which only one thread can execute.
   NAME is optional identifier for this critical block.  */

gimple 
gimple_build_omp_critical (gimple_seq body, tree name)
{
  gimple p = gimple_alloc (GIMPLE_OMP_CRITICAL);
  gimple_omp_critical_set_name (p, name);
  if (body)
    gimple_omp_set_body (p, body);

  return p;
}

/* Build a GIMPLE_OMP_FOR statement.

   BODY is sequence of statements inside the for loop.
   CLAUSES, are any of the OMP loop construct's clauses: private, firstprivate, 
   lastprivate, reductions, ordered, schedule, and nowait.
   COLLAPSE is the collapse count.
   PRE_BODY is the sequence of statements that are loop invariant.  */

gimple
gimple_build_omp_for (gimple_seq body, tree clauses, size_t collapse,
		      gimple_seq pre_body)
{
  gimple p = gimple_alloc (GIMPLE_OMP_FOR);
  if (body)
    gimple_omp_set_body (p, body);
  gimple_omp_for_set_clauses (p, clauses);
  p->gimple_omp_for.collapse = collapse;
  p->gimple_omp_for.iter
    = ggc_alloc_cleared (collapse * sizeof (*p->gimple_omp_for.iter));
  if (pre_body)
    gimple_omp_for_set_pre_body (p, pre_body);

  return p;
}


/* Build a GIMPLE_OMP_PARALLEL statement.

   BODY is sequence of statements which are executed in parallel.
   CLAUSES, are the OMP parallel construct's clauses.
   CHILD_FN is the function created for the parallel threads to execute.
   DATA_ARG are the shared data argument(s).  */

gimple 
gimple_build_omp_parallel (gimple_seq body, tree clauses, tree child_fn, 
			   tree data_arg)
{
  gimple p = gimple_alloc (GIMPLE_OMP_PARALLEL);
  if (body)
    gimple_omp_set_body (p, body);
  gimple_omp_parallel_set_clauses (p, clauses);
  gimple_omp_parallel_set_child_fn (p, child_fn);
  gimple_omp_parallel_set_data_arg (p, data_arg);

  return p;
}


/* Build a GIMPLE_OMP_TASK statement.

   BODY is sequence of statements which are executed by the explicit task.
   CLAUSES, are the OMP parallel construct's clauses.
   CHILD_FN is the function created for the parallel threads to execute.
   DATA_ARG are the shared data argument(s).
   COPY_FN is the optional function for firstprivate initialization.
   ARG_SIZE and ARG_ALIGN are size and alignment of the data block.  */

gimple 
gimple_build_omp_task (gimple_seq body, tree clauses, tree child_fn,
		       tree data_arg, tree copy_fn, tree arg_size,
		       tree arg_align)
{
  gimple p = gimple_alloc (GIMPLE_OMP_TASK);
  if (body)
    gimple_omp_set_body (p, body);
  gimple_omp_task_set_clauses (p, clauses);
  gimple_omp_task_set_child_fn (p, child_fn);
  gimple_omp_task_set_data_arg (p, data_arg);
  gimple_omp_task_set_copy_fn (p, copy_fn);
  gimple_omp_task_set_arg_size (p, arg_size);
  gimple_omp_task_set_arg_align (p, arg_align);

  return p;
}


/* Build a GIMPLE_OMP_SECTION statement for a sections statement.

   BODY is the sequence of statements in the section.  */

gimple
gimple_build_omp_section (gimple_seq body)
{
  gimple p = gimple_alloc (GIMPLE_OMP_SECTION);
  if (body)
    gimple_omp_set_body (p, body);

  return p;
}


/* Build a GIMPLE_OMP_MASTER statement.

   BODY is the sequence of statements to be executed by just the master.  */

gimple 
gimple_build_omp_master (gimple_seq body)
{
  gimple p = gimple_alloc (GIMPLE_OMP_MASTER);
  if (body)
    gimple_omp_set_body (p, body);

  return p;
}


/* Build a GIMPLE_OMP_CONTINUE statement.

   CONTROL_DEF is the definition of the control variable.
   CONTROL_USE is the use of the control variable.  */

gimple 
gimple_build_omp_continue (tree control_def, tree control_use)
{
  gimple p = gimple_alloc (GIMPLE_OMP_CONTINUE);
  gimple_omp_continue_set_control_def (p, control_def);
  gimple_omp_continue_set_control_use (p, control_use);
  return p;
}

/* Build a GIMPLE_OMP_ORDERED statement.

   BODY is the sequence of statements inside a loop that will executed in
   sequence.  */

gimple 
gimple_build_omp_ordered (gimple_seq body)
{
  gimple p = gimple_alloc (GIMPLE_OMP_ORDERED);
  if (body)
    gimple_omp_set_body (p, body);

  return p;
}


/* Build a GIMPLE_OMP_RETURN statement.
   WAIT_P is true if this is a non-waiting return.  */

gimple 
gimple_build_omp_return (bool wait_p)
{
  gimple p = gimple_alloc (GIMPLE_OMP_RETURN);
  if (wait_p)
    gimple_omp_return_set_nowait (p);

  return p;
}


/* Build a GIMPLE_OMP_SECTIONS statement.

   BODY is a sequence of section statements.
   CLAUSES are any of the OMP sections contsruct's clauses: private,
   firstprivate, lastprivate, reduction, and nowait.  */

gimple 
gimple_build_omp_sections (gimple_seq body, tree clauses)
{
  gimple p = gimple_alloc (GIMPLE_OMP_SECTIONS);
  if (body)
    gimple_omp_set_body (p, body);
  gimple_omp_sections_set_clauses (p, clauses);

  return p;
}


/* Build a GIMPLE_OMP_SECTIONS_SWITCH.  */

gimple
gimple_build_omp_sections_switch (void)
{
  return gimple_alloc (GIMPLE_OMP_SECTIONS_SWITCH);
}


/* Build a GIMPLE_OMP_SINGLE statement.

   BODY is the sequence of statements that will be executed once.
   CLAUSES are any of the OMP single construct's clauses: private, firstprivate,
   copyprivate, nowait.  */

gimple 
gimple_build_omp_single (gimple_seq body, tree clauses)
{
  gimple p = gimple_alloc (GIMPLE_OMP_SINGLE);
  if (body)
    gimple_omp_set_body (p, body);
  gimple_omp_single_set_clauses (p, clauses);

  return p;
}


/* Build a GIMPLE_CHANGE_DYNAMIC_TYPE statement.  TYPE is the new type
   for the location PTR.  */

gimple
gimple_build_cdt (tree type, tree ptr)
{
  gimple p = gimple_build_with_ops (GIMPLE_CHANGE_DYNAMIC_TYPE, 0, 1);
  gimple_cdt_set_new_type (p, type);
  gimple_cdt_set_location (p, ptr);

  return p;
}


/* Build a GIMPLE_OMP_ATOMIC_LOAD statement.  */

gimple
gimple_build_omp_atomic_load (tree lhs, tree rhs)
{
  gimple p = gimple_alloc (GIMPLE_OMP_ATOMIC_LOAD);
  gimple_omp_atomic_load_set_lhs (p, lhs);
  gimple_omp_atomic_load_set_rhs (p, rhs);
  return p;
}

/* Build a GIMPLE_OMP_ATOMIC_STORE statement.

   VAL is the value we are storing.  */

gimple
gimple_build_omp_atomic_store (tree val)
{
  gimple p = gimple_alloc (GIMPLE_OMP_ATOMIC_STORE);
  gimple_omp_atomic_store_set_val (p, val);
  return p;
}

/* Return which gimple structure is used by T.  The enums here are defined
   in gsstruct.def.  */

enum gimple_statement_structure_enum
gimple_statement_structure (gimple gs)
{
  return gss_for_code (gimple_code (gs));
}

#if defined ENABLE_GIMPLE_CHECKING && (GCC_VERSION >= 2007)
/* Complain of a gimple type mismatch and die.  */

void
gimple_check_failed (const_gimple gs, const char *file, int line,
		     const char *function, enum gimple_code code,
		     enum tree_code subcode)
{
  internal_error ("gimple check: expected %s(%s), have %s(%s) in %s, at %s:%d",
      		  gimple_code_name[code],
		  tree_code_name[subcode],
		  gimple_code_name[gimple_code (gs)],
		  gimple_subcode (gs) > 0
		    ? tree_code_name[gimple_subcode (gs)]
		    : "",
		  function, trim_filename (file), line);
}


/* Similar to gimple_check_failed, except that instead of specifying a
   dozen codes, use the knowledge that they're all sequential.  */

void
gimple_range_check_failed (const_gimple gs, const char *file, int line,
		           const char *function, enum gimple_code c1,
		           enum gimple_code c2)
{
  char *buffer;
  size_t length = 0;
  enum gimple_code c;

  for (c = c1; c <= c2; ++c)
    length += 4 + strlen (gimple_code_name[c]);

  length += strlen ("expected ");
  buffer = alloca (length);
  length = 0;

  for (c = c1; c <= c2; ++c)
    {
      const char *prefix = length ? " or " : "expected ";

      strcpy (buffer + length, prefix);
      length += strlen (prefix);
      strcpy (buffer + length, gimple_code_name[c]);
      length += strlen (gimple_code_name[c]);
    }

  internal_error ("gimple check: %s, have %s in %s, at %s:%d",
		  buffer, gimple_code_name[gimple_code (gs)],
		  function, trim_filename (file), line);
}
#endif /* ENABLE_GIMPLE_CHECKING */


/* Allocate a new GIMPLE sequence in GC memory and return it.  If
   there are free sequences in GIMPLE_SEQ_CACHE return one of those
   instead.  */

gimple_seq
gimple_seq_alloc (void)
{
  gimple_seq seq = gimple_seq_cache;
  if (seq)
    {
      gimple_seq_cache = gimple_seq_cache->next_free;
      gcc_assert (gimple_seq_cache != seq);
      memset (seq, 0, sizeof (*seq));
    }
  else
    {
      seq = (gimple_seq) ggc_alloc_cleared (sizeof (*seq));
#ifdef GATHER_STATISTICS
      gimple_alloc_counts[(int) gimple_alloc_kind_seq]++;
      gimple_alloc_sizes[(int) gimple_alloc_kind_seq] += sizeof (*seq);
#endif
    }

  return seq;
}

/* Return SEQ to the free pool of GIMPLE sequences.  */

void
gimple_seq_free (gimple_seq seq)
{
  if (seq == NULL)
    return;

  gcc_assert (gimple_seq_first (seq) == NULL);
  gcc_assert (gimple_seq_last (seq) == NULL);

  /* If this triggers, it's a sign that the same list is being freed
     twice.  */
  gcc_assert (seq != gimple_seq_cache || gimple_seq_cache == NULL);
  
  /* Add SEQ to the pool of free sequences.  */
  seq->next_free = gimple_seq_cache;
  gimple_seq_cache = seq;
}


/* Link gimple statement GS to the end of the sequence *SEQ_P.  If
   *SEQ_P is NULL, a new sequence is allocated.  */

void
gimple_seq_add_stmt (gimple_seq *seq_p, gimple gs)
{
  gimple_stmt_iterator si;

  if (gs == NULL)
    return;

  if (*seq_p == NULL)
    *seq_p = gimple_seq_alloc ();

  si = gsi_last (*seq_p);
  gsi_insert_after (&si, gs, GSI_NEW_STMT);
}


/* Append sequence SRC to the end of sequence *DST_P.  If *DST_P is
   NULL, a new sequence is allocated.  */

void
gimple_seq_add_seq (gimple_seq *dst_p, gimple_seq src)
{
  gimple_stmt_iterator si;

  if (src == NULL)
    return;

  if (*dst_p == NULL)
    *dst_p = gimple_seq_alloc ();

  si = gsi_last (*dst_p);
  gsi_insert_seq_after (&si, src, GSI_NEW_STMT);
}


/* Helper function of empty_body_p.  Return true if STMT is an empty
   statement.  */

static bool
empty_stmt_p (gimple stmt)
{
  if (gimple_code (stmt) == GIMPLE_NOP)
    return true;
  if (gimple_code (stmt) == GIMPLE_BIND)
    return empty_body_p (gimple_bind_body (stmt));
  return false;
}


/* Return true if BODY contains nothing but empty statements.  */

bool
empty_body_p (gimple_seq body)
{
  gimple_stmt_iterator i;


  if (gimple_seq_empty_p (body))
    return true;
  for (i = gsi_start (body); !gsi_end_p (i); gsi_next (&i))
    if (!empty_stmt_p (gsi_stmt (i)))
      return false;

  return true;
}


/* Perform a deep copy of sequence SRC and return the result.  */

gimple_seq
gimple_seq_copy (gimple_seq src)
{
  gimple_stmt_iterator gsi;
  gimple_seq new = gimple_seq_alloc ();
  gimple stmt;

  for (gsi = gsi_start (src); !gsi_end_p (gsi); gsi_next (&gsi))
    {
      stmt = gimple_copy (gsi_stmt (gsi));
      gimple_seq_add_stmt (&new, stmt);
    }

  return new;
}


/* Walk all the statements in the sequence SEQ calling walk_gimple_stmt
   on each one.  WI is as in walk_gimple_stmt.
   
   If walk_gimple_stmt returns non-NULL, the walk is stopped, the
   value is stored in WI->CALLBACK_RESULT and the statement that
   produced the value is returned.

   Otherwise, all the statements are walked and NULL returned.  */

gimple
walk_gimple_seq (gimple_seq seq, walk_stmt_fn callback_stmt,
		 walk_tree_fn callback_op, struct walk_stmt_info *wi)
{
  gimple_stmt_iterator gsi;

  for (gsi = gsi_start (seq); !gsi_end_p (gsi); gsi_next (&gsi))
    {
      tree ret = walk_gimple_stmt (&gsi, callback_stmt, callback_op, wi);
      if (ret)
	{
	  /* If CALLBACK_STMT or CALLBACK_OP return a value, WI must exist
	     to hold it.  */
	  gcc_assert (wi);
	  wi->callback_result = ret;
	  return gsi_stmt (gsi);
	}
    }

  if (wi)
    wi->callback_result = NULL_TREE;

  return NULL;
}


/* Helper function for walk_gimple_stmt.  Walk operands of a GIMPLE_ASM.  */

static tree
walk_gimple_asm (gimple stmt, walk_tree_fn callback_op,
		 struct walk_stmt_info *wi)
{
  tree ret;
  size_t noutputs;
  const char **oconstraints;
  size_t i;
  const char *constraint;
  bool allows_mem, allows_reg, is_inout;

  noutputs = gimple_asm_noutputs (stmt);
  oconstraints = (const char **) alloca ((noutputs) * sizeof (const char *));

  if (wi)
    wi->is_lhs = true;

  for (i = 0; i < noutputs; i++)
    {
      tree op = gimple_asm_output_op (stmt, i);
      constraint = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (op)));
      oconstraints[i] = constraint;
      parse_output_constraint (&constraint, i, 0, 0, &allows_mem, &allows_reg,
	                       &is_inout);
      if (wi)
	wi->val_only = (allows_reg || !allows_mem);
      ret = walk_tree (&TREE_VALUE (op), callback_op, wi, NULL);
      if (ret)
	return ret;
    }

  for (i = 0; i < gimple_asm_ninputs (stmt); i++)
    {
      tree op = gimple_asm_input_op (stmt, i);
      constraint = TREE_STRING_POINTER (TREE_VALUE (TREE_PURPOSE (op)));
      parse_input_constraint (&constraint, 0, 0, noutputs, 0,
			      oconstraints, &allows_mem, &allows_reg);
      if (wi)
	wi->val_only = (allows_reg || !allows_mem);

      /* Although input "m" is not really a LHS, we need a lvalue.  */
      if (wi)
	wi->is_lhs = !wi->val_only;
      ret = walk_tree (&TREE_VALUE (op), callback_op, wi, NULL);
      if (ret)
	return ret;
    }

  if (wi)
    {
      wi->is_lhs = false;
      wi->val_only = true;
    }

  return NULL_TREE;
}


/* Helper function of WALK_GIMPLE_STMT.  Walk every tree operand in
   STMT.  CALLBACK_OP and WI are as in WALK_GIMPLE_STMT.

   CALLBACK_OP is called on each operand of STMT via walk_tree.
   Additional parameters to walk_tree must be stored in WI.  For each operand
   OP, walk_tree is called as:

	walk_tree (&OP, CALLBACK_OP, WI, WI->PSET)

   If CALLBACK_OP returns non-NULL for an operand, the remaining
   operands are not scanned.

   The return value is that returned by the last call to walk_tree, or
   NULL_TREE if no CALLBACK_OP is specified.  */

inline tree
walk_gimple_op (gimple stmt, walk_tree_fn callback_op,
		struct walk_stmt_info *wi)
{
  struct pointer_set_t *pset = (wi) ? wi->pset : NULL;
  size_t i;
  tree ret = NULL_TREE;

  switch (gimple_code (stmt))
    {
    case GIMPLE_ASSIGN:
      /* Walk the RHS operands.  A formal temporary LHS may use a
	 COMPONENT_REF RHS.  */
      if (wi)
	wi->val_only = !is_gimple_formal_tmp_var (gimple_assign_lhs (stmt));

      for (i = 1; i < gimple_num_ops (stmt); i++)
	{
	  ret = walk_tree (gimple_op_ptr (stmt, i), callback_op, wi,
			   pset);
	  if (ret)
	    return ret;
	}

      /* Walk the LHS.  If the RHS is appropriate for a memory, we
	 may use a COMPONENT_REF on the LHS.  */
      if (wi)
	{
          /* If the RHS has more than 1 operand, it is not appropriate
             for the memory.  */
	  wi->val_only = !is_gimple_mem_rhs (gimple_assign_rhs1 (stmt))
                         || !gimple_assign_single_p (stmt);
	  wi->is_lhs = true;
	}

      ret = walk_tree (gimple_op_ptr (stmt, 0), callback_op, wi, pset);
      if (ret)
	return ret;

      if (wi)
	{
	  wi->val_only = true;
	  wi->is_lhs = false;
	}
      break;

    case GIMPLE_CALL:
      if (wi)
	wi->is_lhs = false;

      ret = walk_tree (gimple_call_chain_ptr (stmt), callback_op, wi, pset);
      if (ret)
        return ret;

      ret = walk_tree (gimple_call_fn_ptr (stmt), callback_op, wi, pset);
      if (ret)
        return ret;

      for (i = 0; i < gimple_call_num_args (stmt); i++)
	{
	  ret = walk_tree (gimple_call_arg_ptr (stmt, i), callback_op, wi,
			   pset);
	  if (ret)
	    return ret;
	}

      if (wi)
	wi->is_lhs = true;

      ret = walk_tree (gimple_call_lhs_ptr (stmt), callback_op, wi, pset);
      if (ret)
	return ret;

      if (wi)
	wi->is_lhs = false;
      break;

    case GIMPLE_CATCH:
      ret = walk_tree (gimple_catch_types_ptr (stmt), callback_op, wi,
		       pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_EH_FILTER:
      ret = walk_tree (gimple_eh_filter_types_ptr (stmt), callback_op, wi,
		       pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_CHANGE_DYNAMIC_TYPE:
      ret = walk_tree (gimple_cdt_location_ptr (stmt), callback_op, wi, pset);
      if (ret)
	return ret;

      ret = walk_tree (gimple_cdt_new_type_ptr (stmt), callback_op, wi, pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_ASM:
      ret = walk_gimple_asm (stmt, callback_op, wi);
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_CONTINUE:
      ret = walk_tree (gimple_omp_continue_control_def_ptr (stmt),
	  	       callback_op, wi, pset);
      if (ret)
	return ret;

      ret = walk_tree (gimple_omp_continue_control_use_ptr (stmt),
	  	       callback_op, wi, pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_CRITICAL:
      ret = walk_tree (gimple_omp_critical_name_ptr (stmt), callback_op, wi,
		       pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_FOR:
      ret = walk_tree (gimple_omp_for_clauses_ptr (stmt), callback_op, wi,
		       pset);
      if (ret)
	return ret;
      for (i = 0; i < gimple_omp_for_collapse (stmt); i++)
	{
	  ret = walk_tree (gimple_omp_for_index_ptr (stmt, i), callback_op,
			   wi, pset);
	  if (ret)
	    return ret;
	  ret = walk_tree (gimple_omp_for_initial_ptr (stmt, i), callback_op,
			   wi, pset);
	  if (ret)
	    return ret;
	  ret = walk_tree (gimple_omp_for_final_ptr (stmt, i), callback_op,
			   wi, pset);
	  if (ret)
	    return ret;
	  ret = walk_tree (gimple_omp_for_incr_ptr (stmt, i), callback_op,
			   wi, pset);
	}
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_PARALLEL:
      ret = walk_tree (gimple_omp_parallel_clauses_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      ret = walk_tree (gimple_omp_parallel_child_fn_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      ret = walk_tree (gimple_omp_parallel_data_arg_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_TASK:
      ret = walk_tree (gimple_omp_task_clauses_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      ret = walk_tree (gimple_omp_task_child_fn_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      ret = walk_tree (gimple_omp_task_data_arg_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      ret = walk_tree (gimple_omp_task_copy_fn_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      ret = walk_tree (gimple_omp_task_arg_size_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      ret = walk_tree (gimple_omp_task_arg_align_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_SECTIONS:
      ret = walk_tree (gimple_omp_sections_clauses_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;

      ret = walk_tree (gimple_omp_sections_control_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;

      break;

    case GIMPLE_OMP_SINGLE:
      ret = walk_tree (gimple_omp_single_clauses_ptr (stmt), callback_op, wi,
		       pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_ATOMIC_LOAD:
      ret = walk_tree (gimple_omp_atomic_load_lhs_ptr (stmt), callback_op, wi,
		       pset);
      if (ret)
	return ret;

      ret = walk_tree (gimple_omp_atomic_load_rhs_ptr (stmt), callback_op, wi,
		       pset);
      if (ret)
	return ret;
      break;

    case GIMPLE_OMP_ATOMIC_STORE:
      ret = walk_tree (gimple_omp_atomic_store_val_ptr (stmt), callback_op,
		       wi, pset);
      if (ret)
	return ret;
      break;

      /* Tuples that do not have operands.  */
    case GIMPLE_NOP:
    case GIMPLE_RESX:
    case GIMPLE_OMP_RETURN:
      break;

    default:
      {
	enum gimple_statement_structure_enum gss;
	gss = gimple_statement_structure (stmt);
	if (gss == GSS_WITH_OPS || gss == GSS_WITH_MEM_OPS)
	  for (i = 0; i < gimple_num_ops (stmt); i++)
	    {
	      ret = walk_tree (gimple_op_ptr (stmt, i), callback_op, wi, pset);
	      if (ret)
		return ret;
	    }
      }
      break;
    }

  return NULL_TREE;
}


/* Walk the current statement in GSI (optionally using traversal state
   stored in WI).  If WI is NULL, no state is kept during traversal.
   The callback CALLBACK_STMT is called.  If CALLBACK_STMT indicates
   that it has handled all the operands of the statement, its return
   value is returned.  Otherwise, the return value from CALLBACK_STMT
   is discarded and its operands are scanned.

   If CALLBACK_STMT is NULL or it didn't handle the operands,
   CALLBACK_OP is called on each operand of the statement via
   walk_gimple_op.  If walk_gimple_op returns non-NULL for any
   operand, the remaining operands are not scanned.  In this case, the
   return value from CALLBACK_OP is returned.

   In any other case, NULL_TREE is returned.  */

tree
walk_gimple_stmt (gimple_stmt_iterator *gsi, walk_stmt_fn callback_stmt,
		  walk_tree_fn callback_op, struct walk_stmt_info *wi)
{
  gimple ret;
  tree tree_ret;
  gimple stmt = gsi_stmt (*gsi);

  if (wi)
    wi->gsi = *gsi;

  if (wi && wi->want_locations && gimple_has_location (stmt))
    input_location = gimple_location (stmt);

  ret = NULL;

  /* Invoke the statement callback.  Return if the callback handled
     all of STMT operands by itself.  */
  if (callback_stmt)
    {
      bool handled_ops = false;
      tree_ret = callback_stmt (gsi, &handled_ops, wi);
      if (handled_ops)
	return tree_ret;

      /* If CALLBACK_STMT did not handle operands, it should not have
	 a value to return.  */
      gcc_assert (tree_ret == NULL);

      /* Re-read stmt in case the callback changed it.  */
      stmt = gsi_stmt (*gsi);
    }

  /* If CALLBACK_OP is defined, invoke it on every operand of STMT.  */
  if (callback_op)
    {
      tree_ret = walk_gimple_op (stmt, callback_op, wi);
      if (tree_ret)
	return tree_ret;
    }

  /* If STMT can have statements inside (e.g. GIMPLE_BIND), walk them.  */
  switch (gimple_code (stmt))
    {
    case GIMPLE_BIND:
      ret = walk_gimple_seq (gimple_bind_body (stmt), callback_stmt,
	                     callback_op, wi);
      if (ret)
	return wi->callback_result;
      break;

    case GIMPLE_CATCH:
      ret = walk_gimple_seq (gimple_catch_handler (stmt), callback_stmt,
	                     callback_op, wi);
      if (ret)
	return wi->callback_result;
      break;

    case GIMPLE_EH_FILTER:
      ret = walk_gimple_seq (gimple_eh_filter_failure (stmt), callback_stmt,
		             callback_op, wi);
      if (ret)
	return wi->callback_result;
      break;

    case GIMPLE_TRY:
      ret = walk_gimple_seq (gimple_try_eval (stmt), callback_stmt, callback_op,
	                     wi);
      if (ret)
	return wi->callback_result;

      ret = walk_gimple_seq (gimple_try_cleanup (stmt), callback_stmt,
	                     callback_op, wi);
      if (ret)
	return wi->callback_result;
      break;

    case GIMPLE_OMP_FOR:
      ret = walk_gimple_seq (gimple_omp_for_pre_body (stmt), callback_stmt,
		             callback_op, wi);
      if (ret)
	return wi->callback_result;

      /* FALL THROUGH.  */
    case GIMPLE_OMP_CRITICAL:
    case GIMPLE_OMP_MASTER:
    case GIMPLE_OMP_ORDERED:
    case GIMPLE_OMP_SECTION:
    case GIMPLE_OMP_PARALLEL:
    case GIMPLE_OMP_TASK:
    case GIMPLE_OMP_SECTIONS:
    case GIMPLE_OMP_SINGLE:
      ret = walk_gimple_seq (gimple_omp_body (stmt), callback_stmt, callback_op,
	                     wi);
      if (ret)
	return wi->callback_result;
      break;

    case GIMPLE_WITH_CLEANUP_EXPR:
      ret = walk_gimple_seq (gimple_wce_cleanup (stmt), callback_stmt,
			     callback_op, wi);
      if (ret)
	return wi->callback_result;
      break;

    default:
      gcc_assert (!gimple_has_substatements (stmt));
      break;
    }

  return NULL;
}


/* Set sequence SEQ to be the GIMPLE body for function FN.  */

void
gimple_set_body (tree fndecl, gimple_seq seq)
{
  struct function *fn = DECL_STRUCT_FUNCTION (fndecl);
  if (fn == NULL)
    {
      /* If FNDECL still does not have a function structure associated
	 with it, then it does not make sense for it to receive a
	 GIMPLE body.  */
      gcc_assert (seq == NULL);
    }
  else
    fn->gimple_body = seq;
}


/* Return the body of GIMPLE statements for function FN.  */

gimple_seq
gimple_body (tree fndecl)
{
  struct function *fn = DECL_STRUCT_FUNCTION (fndecl);
  return fn ? fn->gimple_body : NULL;
}


/* Detect flags from a GIMPLE_CALL.  This is just like
   call_expr_flags, but for gimple tuples.  FIXME tuples, remove code
   duplication with call_expr_flags. duplicate code.  */

int
gimple_call_flags (const_gimple stmt)
{
  int flags;
  tree decl = gimple_call_fndecl (stmt);
  tree t;

  if (decl)
    flags = flags_from_decl_or_type (decl);
  else
    {
      t = TREE_TYPE (gimple_call_fn (stmt));
      if (t && TREE_CODE (t) == POINTER_TYPE)
	flags = flags_from_decl_or_type (TREE_TYPE (t));
      else
	flags = 0;
    }

  return flags;
}


/* Return true if GS is a copy assignment.  */

bool
gimple_assign_copy_p (gimple gs)
{
  return gimple_code (gs) == GIMPLE_ASSIGN
         && get_gimple_rhs_class (gimple_subcode (gs)) == GIMPLE_SINGLE_RHS
	 && is_gimple_val (gimple_op (gs, 1));
}

/* Return true if GS is an assignment with a singleton RHS, i.e.,
   there is no operator associated with the assignment itself.
   Unlike gimple_assign_copy_p, this predicate returns true for
   any RHS operand, including those that perform an operation
   and do not have the semantics of a copy, such as COND_EXPR.  */

bool
gimple_assign_single_p (gimple gs)
{
  return (gimple_code (gs) == GIMPLE_ASSIGN
          && get_gimple_rhs_class (gimple_subcode (gs)) == GIMPLE_SINGLE_RHS);
}

/* Return true if GS is an assignment with a unary RHS, but the
   operator has no effect on the assigned value.  The logic is adapted
   from STRIP_NOPS.  This predicate is intended to be used in tuplifying
   instances in which STRIP_NOPS was previously applied to the RHS of
   an assignment.

   NOTE: In the use cases that led to the creation of this function
   and of gimple_assign_single_p, it is typical to test for either
   condition and to proceed in the same manner.  In each case, the
   assigned value is represented by the single RHS operand of the
   assignment.  I suspect there may be cases where gimple_assign_copy_p,
   gimple_assign_single_p, or equivalent logic is used where a similar
   treatment of unary NOPs is appropriate.  */
   
bool
gimple_assign_unary_nop_p (gimple gs)
{
  return (gimple_code (gs) == GIMPLE_ASSIGN
          && (gimple_subcode (gs) == NOP_EXPR
              || gimple_subcode (gs) == CONVERT_EXPR
              || gimple_subcode (gs) == NON_LVALUE_EXPR)
          && gimple_assign_rhs1 (gs) != error_mark_node
          && (TYPE_MODE (TREE_TYPE (gimple_assign_lhs (gs)))
              == TYPE_MODE (GENERIC_TREE_TYPE (gimple_assign_rhs1 (gs)))));
}

/* Set BB to be the basic block holding G.  */

void
gimple_set_bb (gimple stmt, basic_block bb)
{
  stmt->gsbase.bb = bb;

  /* If the statement is a label, add the label to block-to-labels map
     so that we can speed up edge creation for GIMPLE_GOTOs.  */
  if (cfun->cfg && gimple_code (stmt) == GIMPLE_LABEL)
    {
      tree t;
      int uid;

      t = gimple_label_label (stmt);
      uid = LABEL_DECL_UID (t);
      if (uid == -1)
	{
	  size_t old_len = VEC_length (basic_block, label_to_block_map);
	  LABEL_DECL_UID (t) = uid = cfun->cfg->last_label_uid++;
	  if (old_len <= (size_t) uid)
	    {
	      size_t new_len = 3 * uid / 2;

	      VEC_safe_grow_cleared (basic_block, gc, label_to_block_map,
				     new_len);
	    }
	}

      VEC_replace (basic_block, label_to_block_map, uid, bb);
    }
}


/* Fold the expression computed by STMT.  If the expression can be
   folded, return the folded result, otherwise return NULL.  STMT is
   not modified.  */

tree
gimple_fold (const_gimple stmt)
{
  tree t;

  switch (gimple_code (stmt))
    {
    case GIMPLE_COND:
      return fold_binary (gimple_cond_code (stmt),
			  boolean_type_node,
			  gimple_cond_lhs (stmt),
			  gimple_cond_rhs (stmt));
      break;

    case GIMPLE_ASSIGN:
      if (gimple_num_ops (stmt) > 2)
	return fold_binary (gimple_assign_rhs_code (stmt),
			    TREE_TYPE (gimple_assign_lhs (stmt)),
			    gimple_assign_rhs1 (stmt),
			    gimple_assign_rhs2 (stmt));
      else
	return fold_unary (gimple_assign_rhs_code (stmt),
			   TREE_TYPE (gimple_assign_lhs (stmt)),
			   gimple_assign_rhs1 (stmt));
      break;

    case GIMPLE_SWITCH:
      return gimple_switch_index (stmt);
      break;

    case GIMPLE_CALL:
      t = gimple_call_fn (stmt);
      return fold_unary (TREE_CODE (t), TREE_TYPE (t), t);
      break;

    default:
      break;
    }

  gcc_unreachable ();
}


/* Modify the RHS of assignment STMT using the operands in the
   expression tree EXPR.

   This function is useful to convert an existing tree expression into
   the flat representation used for the RHS of a GIMPLE assignment.
   It will reallocate memory as needed to expand or shrink the number
   of operand slots needed to represent EXPR.

   NOTE: If you find yourself building a tree and then calling this
   function, you are most certainly doing it the slow way.  It is much
   better to build a new assignment or to use the function
   gimple_assign_set_rhs_with_ops, which does not require an
   expression tree to be built.  */

void
gimple_assign_set_rhs_from_tree (gimple stmt, tree expr)
{
  enum tree_code subcode;
  tree op1, op2;

  extract_ops_from_tree (expr, &subcode, &op1, &op2);
  gimple_assign_set_rhs_with_ops (stmt, subcode, op1, op2);
}


/* Set the RHS of assignment statement STMT to CODE with operands OP1 and
   OP2.  */

void
gimple_assign_set_rhs_with_ops (gimple stmt, enum tree_code code, tree op1,
			        tree op2)
{
  size_t num_ops = get_gimple_rhs_num_ops (code);

  /* Reallocate the memory operands vector, if necessary.  */
  if (get_gimple_rhs_num_ops (gimple_assign_rhs_code (stmt)) != num_ops)
    {
      tree lhs = gimple_assign_lhs (stmt);
      gimple_alloc_ops (stmt, num_ops + 1);
      gimple_assign_set_lhs (stmt, lhs);
    }

  gimple_set_subcode (stmt, code);
  gimple_assign_set_rhs1 (stmt, op1);
  if (num_ops > 1)
    gimple_assign_set_rhs2 (stmt, op2);
}


/* Return the LHS of a statement that performs an assignment,
   either a GIMPLE_ASSIGN or a GIMPLE_CALL.  Returns NULL_TREE
   for a call to a function that returns no value, or for a
   statement other than an assignment or a call.  */

tree
gimple_get_lhs (gimple stmt)
{
  enum tree_code code = gimple_code (stmt);

  if (code == GIMPLE_ASSIGN)
    return gimple_assign_lhs (stmt);
  else if (code == GIMPLE_CALL)
    return gimple_call_lhs (stmt);
  else
    return NULL_TREE;
}


/* Set the LHS of a statement that performs an assignment,
   either a GIMPLE_ASSIGN or a GIMPLE_CALL.  */

void
gimple_set_lhs (gimple stmt, tree lhs)
{
  enum tree_code code = gimple_code (stmt);

  if (code == GIMPLE_ASSIGN)
    gimple_assign_set_lhs (stmt, lhs);
  else if (code == GIMPLE_CALL)
    gimple_call_set_lhs (stmt, lhs);
  else
    gcc_unreachable();
}


/* Return a deep copy of statement STMT.  All the operands from STMT
   are reallocated and copied using unshare_expr.  The DEF, USE, VDEF
   and VUSE operand arrays are set to empty in the new copy.  */

gimple
gimple_copy (gimple stmt)
{
  enum gimple_code code = gimple_code (stmt);
  size_t num_ops = gimple_num_ops (stmt);
  gimple copy = gimple_alloc (code);
  unsigned i;

  /* Shallow copy all the fields from STMT.  */
  memcpy (copy, stmt, gimple_size (code));

  /* If STMT has sub-statements, deep-copy them as well.  */
  if (gimple_has_substatements (stmt))
    {
      gimple_seq new_seq;
      tree t;

      switch (gimple_code (stmt))
	{
	case GIMPLE_BIND:
	  new_seq = gimple_seq_copy (gimple_bind_body (stmt));
	  gimple_bind_set_body (copy, new_seq);
	  gimple_bind_set_vars (copy, unshare_expr (gimple_bind_vars (stmt)));
	  gimple_bind_set_block (copy, unshare_expr (gimple_bind_block (stmt)));
	  break;

	case GIMPLE_CATCH:
	  new_seq = gimple_seq_copy (gimple_catch_handler (stmt));
	  gimple_catch_set_handler (copy, new_seq);
	  t = unshare_expr (gimple_catch_types (stmt));
	  gimple_catch_set_types (copy, t);
	  break;

	case GIMPLE_EH_FILTER:
	  new_seq = gimple_seq_copy (gimple_eh_filter_failure (stmt));
	  gimple_eh_filter_set_failure (copy, new_seq);
	  t = unshare_expr (gimple_eh_filter_types (stmt));
	  gimple_eh_filter_set_types (copy, t);
	  break;

	case GIMPLE_TRY:
	  new_seq = gimple_seq_copy (gimple_try_eval (stmt));
	  gimple_try_set_eval (copy, new_seq);
	  new_seq = gimple_seq_copy (gimple_try_cleanup (stmt));
	  gimple_try_set_cleanup (copy, new_seq);
	  break;

	case GIMPLE_OMP_FOR:
	  new_seq = gimple_seq_copy (gimple_omp_for_pre_body (stmt));
	  gimple_omp_for_set_pre_body (copy, new_seq);
	  t = unshare_expr (gimple_omp_for_clauses (stmt));
	  gimple_omp_for_set_clauses (copy, t);
	  copy->gimple_omp_for.iter
	    = ggc_alloc (gimple_omp_for_collapse (stmt)
			 * sizeof (*copy->gimple_omp_for.iter));
	  for (i = 0; i < gimple_omp_for_collapse (stmt); i++)
	    {
	      gimple_omp_for_set_cond (copy, i,
				       gimple_omp_for_cond (stmt, i));
	      gimple_omp_for_set_index (copy, i,
					gimple_omp_for_index (stmt, i));
	      t = unshare_expr (gimple_omp_for_initial (stmt, i));
	      gimple_omp_for_set_initial (copy, i, t);
	      t = unshare_expr (gimple_omp_for_final (stmt, i));
	      gimple_omp_for_set_final (copy, i, t);
	      t = unshare_expr (gimple_omp_for_incr (stmt, i));
	      gimple_omp_for_set_incr (copy, i, t);
	    }
	  goto copy_omp_body;

	case GIMPLE_OMP_PARALLEL:
	  t = unshare_expr (gimple_omp_parallel_clauses (stmt));
	  gimple_omp_parallel_set_clauses (copy, t);
	  t = unshare_expr (gimple_omp_parallel_child_fn (stmt));
	  gimple_omp_parallel_set_child_fn (copy, t);
	  t = unshare_expr (gimple_omp_parallel_data_arg (stmt));
	  gimple_omp_parallel_set_data_arg (copy, t);
	  goto copy_omp_body;

	case GIMPLE_OMP_TASK:
	  t = unshare_expr (gimple_omp_task_clauses (stmt));
	  gimple_omp_task_set_clauses (copy, t);
	  t = unshare_expr (gimple_omp_task_child_fn (stmt));
	  gimple_omp_task_set_child_fn (copy, t);
	  t = unshare_expr (gimple_omp_task_data_arg (stmt));
	  gimple_omp_task_set_data_arg (copy, t);
	  t = unshare_expr (gimple_omp_task_copy_fn (stmt));
	  gimple_omp_task_set_copy_fn (copy, t);
	  t = unshare_expr (gimple_omp_task_arg_size (stmt));
	  gimple_omp_task_set_arg_size (copy, t);
	  t = unshare_expr (gimple_omp_task_arg_align (stmt));
	  gimple_omp_task_set_arg_align (copy, t);
	  goto copy_omp_body;

	case GIMPLE_OMP_CRITICAL:
	  t = unshare_expr (gimple_omp_critical_name (stmt));
	  gimple_omp_critical_set_name (copy, t);
	  goto copy_omp_body;

	case GIMPLE_OMP_SECTIONS:
	  t = unshare_expr (gimple_omp_sections_clauses (stmt));
	  gimple_omp_sections_set_clauses (copy, t);
	  t = unshare_expr (gimple_omp_sections_control (stmt));
	  gimple_omp_sections_set_control (copy, t);
	  /* FALLTHRU  */

	case GIMPLE_OMP_SINGLE:
	case GIMPLE_OMP_SECTION:
	case GIMPLE_OMP_MASTER:
	case GIMPLE_OMP_ORDERED:
	copy_omp_body:
	  new_seq = gimple_seq_copy (gimple_omp_body (stmt));
	  gimple_omp_set_body (copy, new_seq);
	  break;

	case GIMPLE_WITH_CLEANUP_EXPR:
	  new_seq = gimple_seq_copy (gimple_wce_cleanup (stmt));
	  gimple_wce_set_cleanup (copy, new_seq);
	  break;

	default:
	  gcc_unreachable ();
	}
    }

  /* Make copy of operands.  */
  if (num_ops > 0)
    {
      gimple_alloc_ops (copy, num_ops);
      for (i = 0; i < num_ops; i++)
	gimple_set_op (copy, i, unshare_expr (gimple_op (stmt, i)));
    }

  /* Clear out SSA operand vectors on COPY.  */
  if (gimple_has_ops (stmt))
    {
      gimple_set_def_ops (copy, NULL);
      gimple_set_use_ops (copy, NULL);
      copy->with_ops.addresses_taken = NULL;
    }

  if (gimple_has_mem_ops (stmt))
    {
      gimple_set_vdef_ops (copy, NULL);
      gimple_set_vuse_ops (copy, NULL);
      stmt->with_mem_ops.stores = NULL;
      stmt->with_mem_ops.loads = NULL;
    }

  return copy;
}


/* Return a copy of statement STMT.  The copy should not retain any use
   information for the variables that appear within it.  */
/* FIXME tuples.  The charter of this function is unclear.  It was
   introduced to replace occurrences of unshare_expr in cases where
   a statement is copied temporarily in order to present a "before
   and after" diagnostic, e.g., showing folding in substitute_and_fold.
   In that case, uses occuring in the saved statement were linked from
   definitions elsewhere, confusing code that expected no such uses to
   exist.  It might be preferable to rewrite such diagnostics to simply
   dump the "before" diagnostic to a string, rather than retaining a
   statement for later processing.  */

gimple
gimple_copy_no_def_use (gimple stmt)
{
  enum gimple_code code = gimple_code (stmt);

  if (code == GIMPLE_PHI)
    {
      unsigned i;
      size_t size = (sizeof (struct gimple_statement_phi)
                     + (sizeof (struct phi_arg_d) * (stmt->gimple_phi.capacity - 1)));
      gimple copy = ggc_alloc_cleared (size);

      memcpy (copy, stmt, size);
      gimple_phi_set_result (copy, unshare_expr (gimple_phi_result (stmt)));
      for (i = 0; i < gimple_phi_num_args(stmt); i++)
      {
        struct phi_arg_d * arg_ptr = gimple_phi_arg (copy, i);
        arg_ptr->def = unshare_expr (gimple_phi_arg_def (stmt, i));
        /*
        arg_ptr->imm_use.prev = &arg_ptr->imm_use;
        arg_ptr->imm_use.next = &arg_ptr->imm_use;
        arg_ptr->imm_use.loc.ssa_name = NULL;
        arg_ptr->imm_use.use = NULL;
        */
      }

      return copy;
    }
  else
    {
      size_t num_ops = gimple_num_ops (stmt);
      gimple copy = gimple_alloc (code);
      unsigned i;

      memcpy (copy, stmt, gimple_size (code));
      if (num_ops > 0)
      {
        gimple_alloc_ops (copy, num_ops);
        for (i = 0; i < num_ops; i++)
          gimple_set_op (copy, i, unshare_expr (gimple_op (stmt, i)));

        gimple_set_def_ops (copy, NULL);
        gimple_set_use_ops (copy, NULL);
      }

      return copy;
    }
}


/* Set the MODIFIED flag to MODIFIEDP, iff the gimple statement G has
   a MODIFIED field.  */

void
gimple_set_modified (gimple s, bool modifiedp)
{
  if (gimple_has_ops (s))
    {
      s->with_ops.modified = (unsigned) modifiedp;

      if (modifiedp
	  && cfun->gimple_df
	  && is_gimple_call (s)
	  && gimple_call_noreturn_p (s))
	VEC_safe_push (gimple, gc, MODIFIED_NORETURN_CALLS (cfun), s);
    }
}


/* Return true if statement S has side-effects.  We consider a
   statement to have side effects if:

   - It is a GIMPLE_CALL not marked with ECF_PURE or ECF_CONST.
   - Any of its operands are marked TREE_THIS_VOLATILE or TREE_SIDE_EFFECTS.  */

bool
gimple_has_side_effects (const_gimple s)
{
  size_t i;

  /* We don't have to scan the arguments to check for
     volatile arguments, though, at present, we still
     do a scan to check for TREE_SIDE_EFFECTS.  */

  if (gimple_has_volatile_ops (s))
    return true;

  if (is_gimple_call (s))
    {
      size_t nargs = gimple_call_num_args (s);

      if (!(gimple_call_flags (s) & (ECF_CONST | ECF_PURE)))
        return true;
      else if (gimple_call_flags (s) & ECF_LOOPING_CONST_OR_PURE)
	/* An infinite loop is considered a side effect.  */
	return true;

      /* FIXME tuples.  Verify that the TREE_SIDE_EFFECTS
         flag is still meaningful on operands.  */

      if (gimple_call_lhs (s)
          && TREE_SIDE_EFFECTS (gimple_call_lhs (s)))
        return true;

      if (TREE_SIDE_EFFECTS (gimple_call_fn (s)))
        return true;

      for (i = 0; i < nargs; i++)
        if (TREE_SIDE_EFFECTS (gimple_call_arg (s, i)))
          return true;

      return false;
    }
  else
    {
      /* FIXME tuples.  Verify that the TREE_SIDE_EFFECTS
         flag is still meaningful on operands.  */

      for (i = 0; i < gimple_num_ops (s); i++)
	if (TREE_SIDE_EFFECTS (gimple_op (s, i)))
	  return true;
    }

  return false;
}

/* Return true if the RHS of statement S has side effects.
   We may use it to determine if it is admissable to replace
   an assignment or call with a copy of a previously-computed
   value.  In such cases, side-effects due the the LHS are
   preserved.  */

bool
gimple_rhs_has_side_effects (const_gimple s)
{
  size_t i;

  if (is_gimple_call (s))
    {
      size_t nargs = gimple_call_num_args (s);

      if (!(gimple_call_flags (s) & (ECF_CONST | ECF_PURE)))
        return true;

      /* We cannot use gimple_has_volatile_ops here,
         because we must ignore a volatile LHS.  */
      /* FIXME tuples.  Verify that the TREE_SIDE_EFFECTS
         flag is still meaningful on operands.  */

      if (TREE_SIDE_EFFECTS (gimple_call_fn (s))
          || TREE_THIS_VOLATILE (gimple_call_fn (s)))
        return true;

      for (i = 0; i < nargs; i++)
        if (TREE_SIDE_EFFECTS (gimple_call_arg (s, i))
            || TREE_THIS_VOLATILE (gimple_call_arg (s, i)))
          return true;

      return false;
    }
  else if (is_gimple_assign (s))
    {
      /* Skip the first operand, the LHS. */
      /* FIXME tuples.  Verify that the TREE_SIDE_EFFECTS
         flag is still meaningful on operands.  */

      for (i = 1; i < gimple_num_ops (s); i++)
	if (TREE_SIDE_EFFECTS (gimple_op (s, i))
            || TREE_THIS_VOLATILE (gimple_op (s, i)))
	  return true;
    }
  else
    {
      /* For statements without an LHS, examine all arguments.  */
      /* FIXME tuples.  Verify that the TREE_SIDE_EFFECTS
         flag is still meaningful on operands.  */

      for (i = 0; i < gimple_num_ops (s); i++)
	if (TREE_SIDE_EFFECTS (gimple_op (s, i))
            || TREE_THIS_VOLATILE (gimple_op (s, i)))
	  return true;
    }

  return false;
}


/* Return true if statement S can trap.  */

bool
gimple_could_trap_p (gimple s)
{
  size_t i;
  tree t, div = NULL_TREE;
  enum tree_code op;

  for (i = 0; i < gimple_num_ops (s); i++)
    if (tree_could_trap_p (gimple_op (s, i)))
      return true;

  switch (gimple_code (s))
    {
    case GIMPLE_ASM:
      return gimple_asm_volatile_p (s);

    case GIMPLE_CALL:
      t = gimple_call_fndecl (s);
      /* Assume that calls to weak functions may trap.  */
      if (!t || !DECL_P (t) || DECL_WEAK (t))
	return true;
      return false;

    case GIMPLE_ASSIGN:
      t = gimple_expr_type (s);
      op = gimple_subcode (s);
      if (get_gimple_rhs_class (op) == GIMPLE_BINARY_RHS)
	div = gimple_assign_rhs2 (s);
      if (operation_could_trap_p (op,
				  FLOAT_TYPE_P (t),
				  (INTEGRAL_TYPE_P (t)
				   && TYPE_OVERFLOW_TRAPS (t)),
				  div))
	return true;
      return false;

    default:
      return false;
    }

}


/* Some transformations like inlining may invalidate the GIMPLE form
   for operands.  This function traverses all the operands in STMT and
   gimplifies anything that is not a valid gimple operand.  Any new
   GIMPLE statements are inserted before *GSI_P.  */

void
gimple_regimplify_operands (gimple stmt, gimple_stmt_iterator *gsi_p)
{
  size_t i, num_ops = gimple_num_ops (stmt);

  for (i = 0; i < num_ops; i++)
    {
      /* NOTE: We start gimplifying operands from last to first to
	 make sure that side-effects on the RHS of calls, assignments
	 and ASMs are executed before the LHS.  The ordering is not
	 important for other statements.  */
      tree op = gimple_op (stmt, num_ops - i - 1);

      /* We probably don't want to touch inline asm operands.  */
      if (gimple_code (stmt) == GIMPLE_ASM)
	continue;

      if (op && !is_gimple_operand (op))
	{
	  op = force_gimple_operand_gsi (gsi_p, op, true, NULL, true,
					 GSI_SAME_STMT);
	  gimple_set_op (stmt, num_ops - i - 1, op);
	}
    }
}


/* Print debugging information for gimple stmts generated.  */

void
dump_gimple_statistics (void)
{
#ifdef GATHER_STATISTICS
  int i, total_tuples = 0, total_bytes = 0;

  fprintf (stderr, "\nGIMPLE statements\n");
  fprintf (stderr, "Kind                   Stmts      Bytes\n");
  fprintf (stderr, "---------------------------------------\n");
  for (i = 0; i < (int) gimple_alloc_kind_all; ++i)
    {
      fprintf (stderr, "%-20s %7d %10d\n", gimple_alloc_kind_names[i],
	  gimple_alloc_counts[i], gimple_alloc_sizes[i]);
      total_tuples += gimple_alloc_counts[i];
      total_bytes += gimple_alloc_sizes[i];
    }
  fprintf (stderr, "---------------------------------------\n");
  fprintf (stderr, "%-20s %7d %10d\n", "Total", total_tuples, total_bytes);
  fprintf (stderr, "---------------------------------------\n");
#else
  fprintf (stderr, "No gimple statistics\n");
#endif
}

#include "gt-gimple.h"
