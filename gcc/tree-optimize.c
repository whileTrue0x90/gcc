/* Control and data flow functions for trees.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
   Contributed by Diego Novillo <dnovillo@redhat.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "rtl.h"
#include "tm_p.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "output.h"
#include "expr.h"
#include "diagnostic.h"
#include "basic-block.h"
#include "flags.h"
#include "tree-flow.h"
#include "tree-dump.h"
#include "timevar.h"
#include "function.h"
#include "langhooks.h"
#include "toplev.h"
#include "flags.h"
#include "cgraph.h"
#include "tree-inline.h"
#include "tree-mudflap.h"
#include "ggc.h"

/* Rewrite a function tree to the SSA form and perform the SSA-based
   optimizations on it.  */

/* Main entry point to the tree SSA transformation routines.  FNDECL is the
   FUNCTION_DECL node for the function to optimize.  */

void
optimize_function_tree (tree fndecl)
{
  tree fnbody;

  /* Don't bother doing anything if the program has errors.  */
  if (errorcount || sorrycount)
    return;

  fnbody = DECL_SAVED_TREE (fndecl);

  /* Build the flowgraph.  */
  init_flow ();

  /* Run a pass over the statements deleting any obviously useless
     statements before we build the CFG.  */
  remove_useless_stmts_and_vars (&DECL_SAVED_TREE (fndecl), 0);

  build_tree_cfg (fnbody);

  /* Begin analysis and optimization passes.  */
  if (n_basic_blocks > 0 && ! (errorcount || sorrycount))
    {
      /* Initialize common SSA structures.  */
      init_tree_ssa ();

      /* Find all the variables referenced in the function.  */
      find_referenced_vars (fndecl);

      /* Compute aliasing information for all the variables referenced in
	 the function.  */
      compute_may_aliases (fndecl);

      /* Rewrite the function into SSA form.  */
      rewrite_into_ssa (fndecl, NULL);

      if (flag_tree_dce)
	{
	  /* If the dominator optimizations propagated ADDR_EXPRs into
	     INDIRECT_REF expressions, we may be able to promote may-alias
	     into must-alias relations.  If DCE eliminated all the pointer
	     assignments that were taking the address of a local variable X,
	     we can now rename X as a non-aliased local.  */
	  tree_ssa_dce (fndecl);
	  if (flag_tree_dom && flag_tree_must_alias)
	    tree_compute_must_alias (fndecl);
	}

      if (flag_tree_pre)
	tree_perform_ssapre (fndecl);

      if (flag_tree_ccp)
	tree_ssa_ccp (fndecl);

      if (flag_tree_copyprop)
	tree_ssa_copyprop (fndecl);

      if (flag_tree_dce)
	tree_ssa_dce (fndecl);

      /* Rewrite the function out of SSA form.  */
      rewrite_out_of_ssa (fndecl);
    }

  /* Debugging dump after optimization.  */
  dump_function (TDI_optimized, fndecl);
}


/* Called to move the SAVE_EXPRs for parameter declarations in a
   nested function into the nested function.  DATA is really the
   nested FUNCTION_DECL.  */

static tree
set_save_expr_context (tree *tp,
		       int *walk_subtrees,
		       void *data)
{
  if (TREE_CODE (*tp) == SAVE_EXPR && !SAVE_EXPR_CONTEXT (*tp))
    SAVE_EXPR_CONTEXT (*tp) = (tree) data;
  /* Do not walk back into the SAVE_EXPR_CONTEXT; that will cause
     circularity.  */
  else if (DECL_P (*tp))
    *walk_subtrees = 0;

  return NULL;
}

/* Clear out the DECL_RTL for the non-static local variables in BLOCK and
   its sub-blocks.  DATA is the decl of the function being processed.  */

static tree
clear_decl_rtl (tree *tp, int *walk_subtrees ATTRIBUTE_UNUSED, void *data)
{
  bool nonstatic_p, local_p;
  tree t = *tp;

  switch (TREE_CODE (t))
    {
    case VAR_DECL:
      nonstatic_p = !TREE_STATIC (t) && !DECL_EXTERNAL (t);
      local_p = DECL_CONTEXT (t) == data;
      break;

    case PARM_DECL:
    case LABEL_DECL:
      nonstatic_p = true;
      local_p = DECL_CONTEXT (t) == data;
      break;

    case RESULT_DECL:
      nonstatic_p = local_p = true;
      break;

    default:
      nonstatic_p = local_p = false;
      break;
    }

  if (nonstatic_p && local_p)
    SET_DECL_RTL (t, NULL);

  return NULL;
}

/* For functions-as-trees languages, this performs all optimization and
   compilation for FNDECL.  */

void
tree_rest_of_compilation (tree fndecl)
{
  static int nesting = -1;
  tree saved_tree;

  /* Don't bother doing anything if there are errors.  */
  if (errorcount || sorrycount)
    return;

  timevar_push (TV_EXPAND);

  ++nesting;

  if (flag_unit_at_a_time && !cgraph_global_info_ready)
    abort ();

  if (nesting > 0)
    /* Squirrel away our current state.  */
    push_function_context ();

  /* Initialize the RTL code for the function.  */
  current_function_decl = fndecl;
  input_filename = TREE_FILENAME (fndecl);
  input_line = TREE_LINENO (fndecl);
  init_function_start (fndecl);

  /* This function is being processed in whole-function mode.  */
  cfun->x_whole_function_mode_p = 1;

  /* Even though we're inside a function body, we still don't want to
     call expand_expr to calculate the size of a variable-sized array.
     We haven't necessarily assigned RTL to all variables yet, so it's
     not safe to try to expand expressions involving them.  */
  immediate_size_expand = 0;
  cfun->x_dont_save_pending_sizes_p = 1;

  /* Gimplify the function.  Don't try to optimize the function if
     gimplification failed.  */
  if (!flag_disable_gimple
      && (keep_function_tree_in_gimple_form (fndecl)
          || gimplify_function_tree (fndecl)))
    {
      /* Debugging dump after gimplification.  */
      dump_function (TDI_gimple, fndecl);

      /* Invoke the SSA tree optimizer.  */
      if (optimize >= 1 && !flag_disable_tree_ssa)
        optimize_function_tree (fndecl);
    }

  /* If the function has a variably modified type, there may be
     SAVE_EXPRs in the parameter types.  Their context must be set to
     refer to this function; they cannot be expanded in the containing
     function.  */
  if (decl_function_context (fndecl)
      && variably_modified_type_p (TREE_TYPE (fndecl)))
    walk_tree (&TREE_TYPE (fndecl), set_save_expr_context, fndecl,
	       NULL);

  /* Set up parameters and prepare for return, for the function.  */
  expand_function_start (fndecl, 0);

  /* Allow language dialects to perform special processing.  */
  (*lang_hooks.rtl_expand.start) ();

  /* If this function is `main', emit a call to `__main'
     to run global initializers, etc.  */
  if (DECL_NAME (fndecl)
      && MAIN_NAME_P (DECL_NAME (fndecl))
      && DECL_FILE_SCOPE_P (fndecl))
    expand_main_function ();

  /* Add mudflap instrumentation to a copy of the function body.  */
  if (flag_mudflap)
    saved_tree = mudflap_c_function (fndecl);
  else
    saved_tree = DECL_SAVED_TREE (fndecl);

  /* Generate the RTL for this function.  */
  (*lang_hooks.rtl_expand.stmt) (saved_tree);

  /* We hard-wired immediate_size_expand to zero above.
     expand_function_end will decrement this variable.  So, we set the
     variable to one here, so that after the decrement it will remain
     zero.  */
  immediate_size_expand = 1;

  /* Allow language dialects to perform special processing.  */
  (*lang_hooks.rtl_expand.end) ();

  /* Generate rtl for function exit.  */
  expand_function_end ();

  /* If this is a nested function, protect the local variables in the stack
     above us from being collected while we're compiling this function.  */
  if (nesting > 0)
    ggc_push_context ();

  /* There's no need to defer outputting this function any more; we
     know we want to output it.  */
  DECL_DEFER_OUTPUT (fndecl) = 0;

  /* Run the optimizers and output the assembler code for this function.  */
  rest_of_compilation (fndecl);

  /* Undo the GC context switch.  */
  if (nesting > 0)
    ggc_pop_context ();

  /* If requested, warn about function definitions where the function will
     return a value (usually of some struct or union type) which itself will
     take up a lot of stack space.  */
  if (warn_larger_than && !DECL_EXTERNAL (fndecl) && TREE_TYPE (fndecl))
    {
      tree ret_type = TREE_TYPE (TREE_TYPE (fndecl));

      if (ret_type && TYPE_SIZE_UNIT (ret_type)
	  && TREE_CODE (TYPE_SIZE_UNIT (ret_type)) == INTEGER_CST
	  && 0 < compare_tree_int (TYPE_SIZE_UNIT (ret_type),
				   larger_than_size))
	{
          const location_t *locus = TREE_LOCUS (fndecl);
	  unsigned int size_as_int
	    = TREE_INT_CST_LOW (TYPE_SIZE_UNIT (ret_type));

	  if (compare_tree_int (TYPE_SIZE_UNIT (ret_type), size_as_int) == 0)
	    warning ("%Hsize of return value of '%D' is %u bytes",
                     locus, fndecl, size_as_int);
	  else
	    warning ("%Hsize of return value of '%D' is larger than %wd bytes",
                     locus, fndecl, larger_than_size);
	}
    }

  /* ??? Looks like some of this could be combined.  */

  /* If possible, obliterate the body of the function so that it can
     be garbage collected.  */
  if (dump_enabled_p (TDI_all))
    /* Keep the body; we're going to dump it.  */
    ;
  else if (DECL_INLINE (fndecl) && flag_inline_trees)
    /* We might need the body of this function so that we can expand
       it inline somewhere else.  */
    ;
  else
    /* We don't need the body; blow it away.  */
    DECL_SAVED_TREE (fndecl) = NULL;

  /* Since we don't need the RTL for this function anymore, stop pointing to
     it.  That's especially important for LABEL_DECLs, since you can reach all
     the instructions in the function from the CODE_LABEL stored in the
     DECL_RTL for the LABEL_DECL.  Walk the BLOCK-tree, clearing DECL_RTL for
     LABEL_DECLs and non-static local variables.  Note that we must check the
     context of the variables, otherwise processing a nested function can kill
     the rtl of a variable from an outer function.  */
  walk_tree_without_duplicates (&DECL_SAVED_TREE (fndecl),
				clear_decl_rtl,
				fndecl);

  if (DECL_SAVED_INSNS (fndecl) == 0 && ! nesting && ! flag_inline_trees)
    {
      /* Stop pointing to the local nodes about to be freed.
	 But DECL_INITIAL must remain nonzero so we know this
	 was an actual function definition.
	 For a nested function, this is done in c_pop_function_context.
	 If rest_of_compilation set this to 0, leave it 0.  */
      if (DECL_INITIAL (fndecl) != 0)
	DECL_INITIAL (fndecl) = error_mark_node;

      DECL_ARGUMENTS (fndecl) = 0;
    }

  if (nesting > 0)
    /* Return to the enclosing function.  */
    pop_function_context ();

  --nesting;

  timevar_pop (TV_EXPAND);
}
