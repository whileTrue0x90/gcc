/* Callgraph based intraprocedural optimizations.
   Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   Contributed by Jan Hubicka

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
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

/* This module implements main driver of compilation process as well as
   few basic intraprocedural optimizers.

   The main scope of this file is to act as an interface in between
   tree based frontends and the backend (and middle end)

   The front-end is supposed to use following functionality:

    - cgraph_finalize_function

      This function is called once front-end has parsed whole body of function
      and it is certain that the function body nor the declaration will change.

      (There is one exception needed for implementing GCC extern inline function.)

    - cgraph_varpool_finalize_variable

      This function has same behavior as the above but is used for static
      variables.

    - cgraph_finalize_compilation_unit

      This function is called once compilation unit is finalized and it will
      no longer change.

      In the unit-at-a-time the call-graph construction and local function
      analysis takes place here.  Bodies of unreachable functions are released
      to conserve memory usage.

      ???  The compilation unit in this point of view should be compilation
      unit as defined by the language - for instance C frontend allows multiple
      compilation units to be parsed at once and it should call function each
      time parsing is done so we save memory.

    - cgraph_optimize

      In this unit-at-a-time compilation the intra procedural analysis takes
      place here.  In particular the static functions whose address is never
      taken are marked as local.  Backend can then use this information to
      modify calling conventions, do better inlining or similar optimizations.

    - cgraph_assemble_pending_functions
    - cgraph_varpool_assemble_pending_variables

      In non-unit-at-a-time mode these functions can be used to force compilation
      of functions or variables that are known to be needed at given stage
      of compilation

    - cgraph_mark_needed_node
    - cgraph_varpool_mark_needed_node

      When function or variable is referenced by some hidden way (for instance
      via assembly code and marked by attribute "used"), the call-graph data structure
      must be updated accordingly by this function.

    - analyze_expr callback

      This function is responsible for lowering tree nodes not understood by
      generic code into understandable ones or alternatively marking
      callgraph and varpool nodes referenced by the as needed.

      ??? On the tree-ssa genericizing should take place here and we will avoid
      need for these hooks (replacing them by genericizing hook)

    - expand_function callback

      This function is used to expand function and pass it into RTL back-end.
      Front-end should not make any assumptions about when this function can be
      called.  In particular cgraph_assemble_pending_functions,
      cgraph_varpool_assemble_pending_variables, cgraph_finalize_function,
      cgraph_varpool_finalize_function, cgraph_optimize can cause arbitrarily
      previously finalized functions to be expanded.

    We implement two compilation modes.

      - unit-at-a-time:  In this mode analyzing of all functions is deferred
	to cgraph_finalize_compilation_unit and expansion into cgraph_optimize.

	In cgraph_finalize_compilation_unit the reachable functions are
	analyzed.  During analysis the call-graph edges from reachable
	functions are constructed and their destinations are marked as
	reachable.  References to functions and variables are discovered too
	and variables found to be needed output to the assembly file.  Via
	mark_referenced call in assemble_variable functions referenced by
	static variables are noticed too.

	The intra-procedural information is produced and it's existence
	indicated by global_info_ready.  Once this flag is set it is impossible
	to change function from !reachable to reachable and thus
	assemble_variable no longer call mark_referenced.

	Finally the call-graph is topologically sorted and all reachable functions
	that has not been completely inlined or are not external are output.

	??? It is possible that reference to function or variable is optimized
	out.  We can not deal with this nicely because topological order is not
	suitable for it.  For tree-ssa we may consider another pass doing
	optimization and re-discovering reachable functions.

	??? Reorganize code so variables are output very last and only if they
	really has been referenced by produced code, so we catch more cases
	where reference has been optimized out.

      - non-unit-at-a-time

	All functions are variables are output as early as possible to conserve
	memory consumption.  This may or may not result in less memory used but
	it is still needed for some legacy code that rely on particular ordering
	of things output from the compiler.

	Varpool data structures are not used and variables are output directly.

	Functions are output early using call of
	cgraph_assemble_pending_function from cgraph_finalize_function.  The
	decision on whether function is needed is made more conservative so
	uninlininable static functions are needed too.  During the call-graph
	construction the edge destinations are not marked as reachable and it
	is completely relied upn assemble_variable to mark them.
	
     Inlining decision heuristics
        ??? Move this to separate file after tree-ssa merge.

	We separate inlining decisions from the inliner itself and store it
	inside callgraph as so called inline plan.  Reffer to cgraph.c
	documentation about particular representation of inline plans in the
	callgraph

	The implementation of particular heuristics is separated from
	the rest of code to make it easier to replace it with more complicated
	implementation in the future.  The rest of inlining code acts as a
	library aimed to modify the callgraph and verify that the parameters
	on code size growth fits.

	To mark given call inline, use cgraph_mark_inline function, the
	verification is performed by cgraph_default_inline_p and
	cgraph_check_inline_limits.

	The heuristics implements simple knapsack style algorithm ordering
	all functions by their "profitability" (estimated by code size growth)
	and inlining them in priority order.

	cgraph_decide_inlining implements heuristics taking whole callgraph
	into account, while cgraph_decide_inlining_incrementally considers
	only one function at a time and is used in non-unit-at-a-time mode.  */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "rtl.h"
#include "tree-inline.h"
#include "langhooks.h"
#include "hashtab.h"
#include "toplev.h"
#include "flags.h"
#include "ggc.h"
#include "debug.h"
#include "target.h"
#include "basic-block.h"
#include "tree-iterator.h"
#include "cgraph.h"
#include "diagnostic.h"
#include "timevar.h"
#include "params.h"
#include "fibheap.h"
#include "c-common.h"
#include "intl.h"
#include "function.h"
#include "tree-flow.h"
#include "value-prof.h"
#include "coverage.h"
#include "except.h"

#define max(A,B)      ((A) > (B) ? (A) : (B))
#define INSNS_PER_CALL 10

static void cgraph_expand_all_functions (void);
static void cgraph_mark_functions_to_output (void);
static void cgraph_expand_function (struct cgraph_node *);
static tree record_call_1 (tree *, int *, void *);
static void cgraph_mark_local_functions (void);
static bool cgraph_default_inline_p (struct cgraph_node *n);
static void cgraph_analyze_function (struct cgraph_node *node);
static void cgraph_decide_inlining_incrementally (struct cgraph_node *);

/* Statistics we collect about inlining algorithm.  */
static int ncalls_inlined;
static int nfunctions_inlined;
static int initial_insns;
static int overall_insns;
static HOST_WIDEST_INT max_insns;

/* Records tree nodes seen in cgraph_create_edges.  Simply using
   walk_tree_without_duplicates doesn't guarantee each node is visited
   once because it gets a new htab upon each recursive call from
   record_calls_1.  */
static htab_t visited_nodes;

/* Determine if function DECL is needed.  That is, visible to something
   either outside this translation unit, something magic in the system
   configury, or (if not doing unit-at-a-time) to something we havn't
   seen yet.  */

static bool
decide_is_function_needed (struct cgraph_node *node, tree decl)
{
  struct cgraph_node *origin;

  /* If we decided it was needed before, but at the time we didn't have
     the body of the function available, then it's still needed.  We have
     to go back and re-check its dependencies now.  */
  if (node->needed)
    return true;

  /* Externally visible functions must be output.  The exception is
     COMDAT functions that must be output only when they are needed.  */
  if (TREE_PUBLIC (decl) && !DECL_COMDAT (decl) && !DECL_EXTERNAL (decl))
    return true;

  /* Constructors and destructors are reachable from the runtime by
     some mechanism.  */
  if (DECL_STATIC_CONSTRUCTOR (decl) || DECL_STATIC_DESTRUCTOR (decl))
    return true;

  /* If the user told us it is used, then it must be so.  */
  if (lookup_attribute ("used", DECL_ATTRIBUTES (decl)))
    return true;

  /* ??? If the assembler name is set by hand, it is possible to assemble
     the name later after finalizing the function and the fact is noticed
     in assemble_name then.  This is arguably a bug.  */
  if (DECL_ASSEMBLER_NAME_SET_P (decl)
      && TREE_SYMBOL_REFERENCED (DECL_ASSEMBLER_NAME (decl)))
    return true;

  if (flag_unit_at_a_time)
    return false;

  /* If not doing unit at a time, then we'll only defer this function
     if its marked for inlining.  Otherwise we want to emit it now.  */

  /* "extern inline" functions are never output locally.  */
  if (DECL_EXTERNAL (decl))
    return false;
  /* Nested functions of extern inline function shall not be emit unless
     we inlined the origin.  */
  for (origin = node->origin; origin; origin = origin->origin)
    if (DECL_EXTERNAL (origin->decl))
      return false;
  /* We want to emit COMDAT functions only when absolutely necessary.  */
  if (DECL_COMDAT (decl))
    return false;
  if (!DECL_INLINE (decl)
      || (!node->local.disregard_inline_limits
	  /* When declared inline, defer even the uninlinable functions.
	     This allows them to be eliminated when unused.  */
	  && !DECL_DECLARED_INLINE_P (decl) 
	  && (!node->local.inlinable || !cgraph_default_inline_p (node))))
    return true;

  return false;
}

/* If we've progressed far enough to have a DECL_STRUCT_FUNCTION allocated,
   but  haven't yet built the CFG for this function, build it now.  */

void 
cgraph_build_cfg (tree decl)
{
  if (DECL_STRUCT_FUNCTION (decl)
      && (!DECL_STRUCT_FUNCTION (decl)->cfg
          || !DECL_STRUCT_FUNCTION (decl)->cfg->x_entry_block_ptr))
    {
      tree saved_current_function_decl = current_function_decl;
      current_function_decl = decl;
      push_cfun (DECL_STRUCT_FUNCTION (decl));
      gimplify_function_tree (decl);
      lower_function_body ();
      lower_eh_constructs ();
      build_tree_cfg (&DECL_SAVED_TREE (decl));
      tree_register_profile_hooks ();
      branch_prob ();
      coverage_end_function ();
      current_function_decl = saved_current_function_decl;
      pop_cfun ();
    }
}

/* When not doing unit-at-a-time, output all functions enqueued.
   Return true when such a functions were found.  */

bool
cgraph_assemble_pending_functions (void)
{
  bool output = false;

  if (flag_unit_at_a_time)
    return false;

  while (cgraph_nodes_queue)
    {
      struct cgraph_node *n = cgraph_nodes_queue;

      cgraph_nodes_queue = cgraph_nodes_queue->next_needed;
      n->next_needed = NULL;
      if (!n->global.inlined_to && !DECL_EXTERNAL (n->decl))
	{
	  cgraph_expand_function (n);
	  output = true;
	}
    }

  return output;
}

/* DECL has been parsed.  Take it, queue it, compile it at the whim of the
   logic in effect.  If NESTED is true, then our caller cannot stand to have
   the garbage collector run at the moment.  We would need to either create
   a new GC context, or just not compile right now.  */

void
cgraph_finalize_function (tree decl, bool nested)
{
  struct cgraph_node *node = cgraph_node (decl);
#if 0
  tree saved_current_function_decl;
#endif
  if (node->local.finalized)
    {
      /* As an GCC extension we allow redefinition of the function.  The
	 semantics when both copies of bodies differ is not well defined.
	 We replace the old body with new body so in unit at a time mode
	 we always use new body, while in normal mode we may end up with
	 old body inlined into some functions and new body expanded and
	 inlined in others.
	 
	 ??? It may make more sense to use one body for inlining and other
	 body for expanding the function but this is difficult to do.  */

      /* If node->output is set, then this is a unit-at-a-time compilation
	 and we have already begun whole-unit analysis.  This is *not*
	 testing for whether we've already emitted the function.  That
	 case can be sort-of legitimately seen with real function 
	 redefinition errors.  I would argue that the front end should
	 never present us with such a case, but don't enforce that for now.  */
      if (node->output)
	abort ();

      /* Reset our data structures so we can analyze the function again.  */
      memset (&node->local, 0, sizeof (node->local));
      memset (&node->global, 0, sizeof (node->global));
      memset (&node->rtl, 0, sizeof (node->rtl));
      node->analyzed = false;
      node->local.redefined_extern_inline = true;
      while (node->callees)
	cgraph_remove_edge (node->callees);

      /* We may need to re-queue the node for assembling in case
         we already proceeded it and ignored as not needed.  */
      if (node->reachable && !flag_unit_at_a_time)
	{
	  struct cgraph_node *n;

	  for (n = cgraph_nodes_queue; n; n = n->next_needed)
	    if (n == node)
	      break;
	  if (!n)
	    node->reachable = 0;
	}
    }

  notice_global_symbol (decl);
  node->decl = decl;
  node->local.finalized = true;

  /* If not unit at a time, then we need to create the call graph
     now, so that called functions can be queued and emitted now.  */
  if (!flag_unit_at_a_time)
    {
      cgraph_analyze_function (node);
      cgraph_decide_inlining_incrementally (node);
    }

  if (decide_is_function_needed (node, decl))
    cgraph_mark_needed_node (node);

  /* If not unit at a time, go ahead and emit everything we've found
     to be reachable at this time.  */
  if (!nested)
    {
      if (!cgraph_assemble_pending_functions ())
	ggc_collect ();
    }

  /* If we've not yet emitted decl, tell the debug info about it.  */
  if (!TREE_ASM_WRITTEN (decl))
    (*debug_hooks->deferred_inline_function) (decl);

  /* Possibly warn about unused parameters.  */
  if (warn_unused_parameter)
    do_warn_unused_parameter (decl);
}

/* Walk tree and record all calls.  Called via walk_tree.  */
static tree
record_call_1 (tree *tp, int *walk_subtrees, void *data)
{
  tree t = *tp;

  switch (TREE_CODE (t))
    {
    case VAR_DECL:
      /* ??? Really, we should mark this decl as *potentially* referenced
	 by this function and re-examine whether the decl is actually used
	 after rtl has been generated.  */
      if (TREE_STATIC (t))
	{
	  cgraph_varpool_mark_needed_node (cgraph_varpool_node (t));
	  if (lang_hooks.callgraph.analyze_expr)
	    return lang_hooks.callgraph.analyze_expr (tp, walk_subtrees, 
						      data);
	}
      break;

    case ADDR_EXPR:
      if (flag_unit_at_a_time)
	{
	  /* Record dereferences to the functions.  This makes the
	     functions reachable unconditionally.  */
	  tree decl = TREE_OPERAND (*tp, 0);
	  if (TREE_CODE (decl) == FUNCTION_DECL)
	    cgraph_mark_needed_node (cgraph_node (decl));
	}
      break;

    case CALL_EXPR:
      {
	tree decl = get_callee_fndecl (*tp);
	if (decl && TREE_CODE (decl) == FUNCTION_DECL)
	  {
	    cgraph_create_edge (data, cgraph_node (decl), *tp);

	    /* When we see a function call, we don't want to look at the
	       function reference in the ADDR_EXPR that is hanging from
	       the CALL_EXPR we're examining here, because we would
	       conclude incorrectly that the function's address could be
	       taken by something that is not a function call.  So only
	       walk the function parameter list, skip the other subtrees.  */

	    walk_tree (&TREE_OPERAND (*tp, 1), record_call_1, data,
		       visited_nodes);
	    *walk_subtrees = 0;
	  }
	break;

      case STATEMENT_LIST:
	{
	  tree_stmt_iterator tsi;
	  /* Track current statement while finding CALL_EXPRs.  */
	  for (tsi = tsi_start (*tp); !tsi_end_p (tsi); tsi_next (&tsi))
	    {
	      walk_tree (tsi_stmt_ptr (tsi), record_call_1, data,
			 visited_nodes);
	    }
	}
	break;
      }

    default:
      /* Save some cycles by not walking types and declaration as we
	 won't find anything useful there anyway.  */
      if (DECL_P (*tp) || TYPE_P (*tp))
	{
	  *walk_subtrees = 0;
	  break;
	}

      if ((unsigned int) TREE_CODE (t) >= LAST_AND_UNUSED_TREE_CODE)
	return lang_hooks.callgraph.analyze_expr (tp, walk_subtrees, data);
      break;
    }

  return NULL;
}

/* Create cgraph edges for function calls inside BODY from NODE.  */

void
cgraph_create_edges (struct cgraph_node *node, tree body)
{
  struct function *this_cfun = DECL_STRUCT_FUNCTION (body);
  basic_block this_block;
  tree step;

  /* The nodes we're interested in are never shared, so walk
     the tree ignoring duplicates.  */
  visited_nodes = htab_create (37, htab_hash_pointer,
				    htab_eq_pointer, NULL);
  /* Reach the trees by walking over the CFG, and note the 
     enclosing basic-blocks in the call edges.  */
  FOR_EACH_BB_FN (this_block, this_cfun)
    {
      node->current_basic_block = this_block;
      walk_tree (&this_block->stmt_list, record_call_1, node, visited_nodes);
    }
  node->current_basic_block = (basic_block)0;
  /* Walk over any private statics that may take addresses of functions.  */
  if (TREE_CODE (DECL_INITIAL (body)) == BLOCK)
    {
      for (step = BLOCK_VARS (DECL_INITIAL (body));
	   step;
	   step = TREE_CHAIN (step))
	if (DECL_INITIAL (step))
	  walk_tree (&DECL_INITIAL (step), record_call_1, node, visited_nodes);
    }
  /* Also look here for private statics.  */
  if (DECL_STRUCT_FUNCTION (body))
    for (step = DECL_STRUCT_FUNCTION (body)->unexpanded_var_list;
	 step;
	 step = TREE_CHAIN (step))
      {
	tree decl = TREE_VALUE (step);
	if (DECL_INITIAL (decl) && TREE_STATIC (decl))
	  walk_tree (&DECL_INITIAL (decl), record_call_1, node, visited_nodes);
      }
  htab_delete (visited_nodes);
  visited_nodes = NULL;
}

static bool error_found;

/* Callbrack of verify_cgraph_node.  Check that all call_exprs have cgraph
   nodes.  */

static tree
verify_cgraph_node_1 (tree *tp, int *walk_subtrees, void *data)
{
  tree t = *tp;
  tree decl;

  if (TREE_CODE (t) == CALL_EXPR && (decl = get_callee_fndecl (t)))
    {
      struct cgraph_edge *e = cgraph_edge (data, t);
      if (e)
	{
	  if (e->aux)
	    {
	      error ("Shared call_expr:");
	      debug_tree (t);
	      error_found = true;
	    }
	  if (e->callee->decl != cgraph_node (decl)->decl)
	    {
	      error ("Edge points to wrong declaration:");
	      debug_tree (e->callee->decl);
	      fprintf (stderr," Instead of:");
	      debug_tree (decl);
	    }
	  e->aux = (void *)1;
	}
      else
	{
	  error ("Missing callgraph edge for call expr:");
	  debug_tree (t);
	  error_found = true;
	}
    }

  /* Save some cycles by not walking types and declaration as we
     won't find anything useful there anyway.  */
  if (DECL_P (*tp) || TYPE_P (*tp))
    *walk_subtrees = 0;

  return NULL_TREE;
}

/* Verify cgraph nodes of given cgraph node.  */
void
verify_cgraph_node (struct cgraph_node *node)
{
  struct cgraph_edge *e;
  struct cgraph_node *main_clone;
  tree decl = node->decl;
  struct function *this_cfun = DECL_STRUCT_FUNCTION (decl);
  basic_block this_block;

  timevar_push (TV_CGRAPH_VERIFY);
  error_found = false;
  for (e = node->callees; e; e = e->next_callee)
    if (e->aux)
      {
	error ("Aux field set for edge %s->%s",
	       cgraph_node_name (e->caller), cgraph_node_name (e->callee));
	error_found = true;
      }
  for (e = node->callers; e; e = e->next_caller)
    {
      if (!e->inline_failed)
	{
	  if (node->global.inlined_to
	      != (e->caller->global.inlined_to
		  ? e->caller->global.inlined_to : e->caller))
	    {
	      error ("Inlined_to pointer is wrong");
	      error_found = true;
	    }
	  if (node->callers->next_caller)
	    {
	      error ("Multiple inline callers");
	      error_found = true;
	    }
	}
      else
	if (node->global.inlined_to)
	  {
	    error ("Inlined_to pointer set for noninline callers");
	    error_found = true;
	  }
    }
  if (!node->callers && node->global.inlined_to)
    {
      error ("Inlined_to pointer is set but no predecesors found");
      error_found = true;
    }
  if (node->global.inlined_to == node)
    {
      error ("Inlined_to pointer reffers to itself");
      error_found = true;
    }

  for (main_clone = cgraph_node (node->decl); main_clone;
       main_clone = main_clone->next_clone)
    if (main_clone == node)
      break;
  if (!node)
    {
      error ("Node not found in DECL_ASSEMBLER_NAME hash");
      error_found = true;
    }
  
  if (node->analyzed
      && DECL_SAVED_TREE (node->decl) && !TREE_ASM_WRITTEN (node->decl)
      && (!DECL_EXTERNAL (node->decl) || node->global.inlined_to))
    {
      if (this_cfun->cfg->x_entry_block_ptr)
	{
	  /* The nodes we're interested in are never shared, so walk
	     the tree ignoring duplicates.  */
	  visited_nodes = htab_create (37, htab_hash_pointer,
				       htab_eq_pointer, NULL);
	  /* Reach the trees by walking over the CFG, and note the
	     enclosing basic-blocks in the call edges.  */
	  FOR_EACH_BB_FN (this_block, this_cfun)
	    {
	      walk_tree (&this_block->stmt_list, verify_cgraph_node_1, node, visited_nodes);
	    }
	  htab_delete (visited_nodes);
	  visited_nodes = NULL;
	}
      else
	{
	  /* No CFG available; walk the trees directly.  */
	  walk_tree_without_duplicates (&DECL_SAVED_TREE (node->decl),
					verify_cgraph_node_1, node);
	}
      for (e = node->callees; e; e = e->next_callee)
	{
	  if (!e->aux)
	    {
	      error ("Edge %s->%s has no corresponding call_expr",
		     cgraph_node_name (e->caller),
		     cgraph_node_name (e->callee));
	      error_found = true;
	    }
	  e->aux = 0;
	}
    }
  if (error_found)
    {
      dump_cgraph_node (stderr, node);
      internal_error ("verify_cgraph_node failed.");
    }
  timevar_pop (TV_CGRAPH_VERIFY);
}

/* Verify whole cgraph structure.  */
void
verify_cgraph (void)
{
  struct cgraph_node *node;

  for (node = cgraph_nodes; node; node = node->next)
    verify_cgraph_node (node);
}

/* Analyze the function scheduled to be output.  */
static void
cgraph_analyze_function (struct cgraph_node *node)
{
  tree decl = node->decl;
  struct cgraph_edge *e;

  current_function_decl = decl;

  cgraph_build_cfg (decl);

  /* First kill forward declaration so reverse inlining works properly.  */
  cgraph_create_edges (node, /* DECL_SAVED_TREE */ (decl));

  node->local.inlinable = tree_inlinable_function_p (decl);
  node->local.self_insns = estimate_num_insns (decl);
  if (node->local.inlinable)
    node->local.disregard_inline_limits
      = lang_hooks.tree_inlining.disregard_inline_limits (decl);
  for (e = node->callers; e; e = e->next_caller)
    {
      if (node->local.redefined_extern_inline)
	e->inline_failed = N_("redefined extern inline functions are not "
			   "considered for inlining");
      else if (!node->local.inlinable)
	e->inline_failed = N_("function not inlinable");
      else
	e->inline_failed = N_("function not considered for inlining");
    }
  if (flag_really_no_inline && !node->local.disregard_inline_limits)
    node->local.inlinable = 0;
  /* Inlining characteristics are maintained by the cgraph_mark_inline.  */
  node->global.insns = node->local.self_insns;

  node->analyzed = true;
  current_function_decl = NULL;
}

/* Analyze the whole compilation unit once it is parsed completely.  */

void
cgraph_finalize_compilation_unit (void)
{
  struct cgraph_node *node;

  if (!flag_unit_at_a_time)
    {
      cgraph_assemble_pending_functions ();
      return;
    }

  cgraph_varpool_assemble_pending_decls ();
  if (!quiet_flag)
    fprintf (stderr, "\nAnalyzing compilation unit\n");

  timevar_push (TV_CGRAPH);
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Initial entry points:");
      for (node = cgraph_nodes; node; node = node->next)
	if (node->needed && DECL_SAVED_TREE (node->decl))
	  fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
      fprintf (cgraph_dump_file, "\n");
    }

  /* Propagate reachability flag and lower representation of all reachable
     functions.  In the future, lowering will introduce new functions and
     new entry points on the way (by template instantiation and virtual
     method table generation for instance).  */
  while (cgraph_nodes_queue)
    {
      struct cgraph_edge *edge;
      tree decl = cgraph_nodes_queue->decl;

      node = cgraph_nodes_queue;
      cgraph_nodes_queue = cgraph_nodes_queue->next_needed;
      node->next_needed = NULL;

      /* ??? It is possible to create extern inline function and later using
	 weak alas attribute to kill its body. See
	 gcc.c-torture/compile/20011119-1.c  */
      if (!DECL_SAVED_TREE (decl))
	continue;

      if (node->analyzed || !node->reachable || !DECL_SAVED_TREE (decl))
	abort ();

      cgraph_analyze_function (node);

      for (edge = node->callees; edge; edge = edge->next_callee)
	if (!edge->callee->reachable)
	  cgraph_mark_reachable_node (edge->callee);

      cgraph_varpool_assemble_pending_decls ();
    }

  /* Collect entry points to the unit.  */

  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Unit entry points:");
      for (node = cgraph_nodes; node; node = node->next)
	if (node->needed && DECL_SAVED_TREE (node->decl))
	  fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
      fprintf (cgraph_dump_file, "\n\nInitial ");
      dump_cgraph (cgraph_dump_file);
    }

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\nReclaiming functions:");

  for (node = cgraph_nodes; node; node = node->next)
    {
      tree decl = node->decl;

      if (!node->reachable && DECL_SAVED_TREE (decl))
	{
	  if (cgraph_dump_file)
	    fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
	  cgraph_remove_node (node);
	}
      else
	node->next_needed = NULL;
    }
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "\n\nReclaimed ");
      dump_cgraph (cgraph_dump_file);
    }
  ggc_collect ();
  timevar_pop (TV_CGRAPH);
}
/* Figure out what functions we want to assemble.  */

static void
cgraph_mark_functions_to_output (void)
{
  struct cgraph_node *node;

  for (node = cgraph_nodes; node; node = node->next)
    {
      tree decl = node->decl;
      struct cgraph_edge *e;
      if (node->output)
	abort ();

      for (e = node->callers; e; e = e->next_caller)
	if (e->inline_failed)
	  break;

      /* We need to output all local functions that are used and not
	 always inlined, as well as those that are reachable from
	 outside the current compilation unit.  */
      if (DECL_SAVED_TREE (decl)
	  && !node->global.inlined_to
	  && (node->needed
	      || (e && node->reachable))
	  && !TREE_ASM_WRITTEN (decl)
	  && !DECL_EXTERNAL (decl))
	node->output = 1;
      /* We should've reclaimed all functions that are not needed.  */
      else if (!node->global.inlined_to && DECL_SAVED_TREE (decl)
	       && !DECL_EXTERNAL (decl))
	{
	  dump_cgraph_node (stderr, node);
	  abort ();
	}
    }
}

/* Expand function specified by NODE.  */

static void
cgraph_expand_function (struct cgraph_node *node)
{
  tree decl = node->decl;

  /* We ought to not compile any inline clones.  */
  if (node->global.inlined_to)
    abort ();

  if (flag_unit_at_a_time)
    announce_function (decl);

  /* Generate RTL for the body of DECL.  */
  lang_hooks.callgraph.expand_function (decl);

  /* Make sure that BE didn't give up on compiling.  */
  /* ??? Can happen with nested function of extern inline.  */
  if (!TREE_ASM_WRITTEN (node->decl))
    abort ();

  current_function_decl = NULL;
  if (DECL_SAVED_TREE (node->decl)
      && !cgraph_preserve_function_body_p (node->decl))
    {
      DECL_SAVED_TREE (node->decl) = NULL;
      DECL_STRUCT_FUNCTION (node->decl) = NULL;
      DECL_INITIAL (node->decl) = error_mark_node;
    }
}

/* Fill array order with all nodes with output flag set in the reverse
   topological order.  */

static int
cgraph_postorder (struct cgraph_node **order)
{
  struct cgraph_node *node, *node2;
  int stack_size = 0;
  int order_pos = 0;
  struct cgraph_edge *edge, last;

  struct cgraph_node **stack =
    xcalloc (cgraph_n_nodes, sizeof (struct cgraph_node *));

  /* We have to deal with cycles nicely, so use a depth first traversal
     output algorithm.  Ignore the fact that some functions won't need
     to be output and put them into order as well, so we get dependencies
     right through intline functions.  */
  for (node = cgraph_nodes; node; node = node->next)
    node->aux = NULL;
  for (node = cgraph_nodes; node; node = node->next)
    if (!node->aux)
      {
	node2 = node;
	if (!node->callers)
	  node->aux = &last;
	else
	  node->aux = node->callers;
	while (node2)
	  {
	    while (node2->aux != &last)
	      {
		edge = node2->aux;
		if (edge->next_caller)
		  node2->aux = edge->next_caller;
		else
		  node2->aux = &last;
		if (!edge->caller->aux)
		  {
		    if (!edge->caller->callers)
		      edge->caller->aux = &last;
		    else
		      edge->caller->aux = edge->caller->callers;
		    stack[stack_size++] = node2;
		    node2 = edge->caller;
		    break;
		  }
	      }
	    if (node2->aux == &last)
	      {
		order[order_pos++] = node2;
		if (stack_size)
		  node2 = stack[--stack_size];
		else
		  node2 = NULL;
	      }
	  }
      }
  free (stack);
  return order_pos;
}

/* Perform reachability analysis and reclaim all unreachable nodes.
   This function also remove unneeded bodies of extern inline functions
   and thus needs to be done only after inlining decisions has been made.  */
static bool
cgraph_remove_unreachable_nodes (void)
{
  struct cgraph_node *first = (void *) 1;
  struct cgraph_node *node;
  bool changed = false;
  int insns = 0;

#ifdef ENABLE_CHECKING
  verify_cgraph ();
#endif
  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\nReclaiming functions:");
#ifdef ENABLE_CHECKING
  for (node = cgraph_nodes; node; node = node->next)
    if (node->aux)
      abort ();
#endif
  for (node = cgraph_nodes; node; node = node->next)
    if (node->needed && !node->global.inlined_to
	&& (!DECL_EXTERNAL (node->decl) || !node->analyzed))
      {
	node->aux = first;
	first = node;
      }
    else if (node->aux)
      abort ();

  /* Perform reachability analysis.  As a special case do not consider
     extern inline functions not inlined as live because we won't output
     them at all.  */
  while (first != (void *) 1)
    {
      struct cgraph_edge *e;
      node = first;
      first = first->aux;

      for (e = node->callees; e; e = e->next_callee)
	if (!e->callee->aux
	    && node->analyzed
	    && (!e->inline_failed || !e->callee->analyzed
		|| !DECL_EXTERNAL (e->callee->decl)))
	  {
	    e->callee->aux = first;
	    first = e->callee;
	  }
    }

  /* Remove unreachable nodes.  Extern inline functions need special care;
     Unreachable extern inline functions shall be removed.
     Reachable extern inline functions we never inlined shall get their bodies
     eliminated.
     Reachable extern inline functions we sometimes inlined will be turned into
     unanalyzed nodes so they look like for true extern functions to the rest
     of code.  Body of such functions is released via remove_node once the
     inline clones are eliminated.  */
  for (node = cgraph_nodes; node; node = node->next)
    {
      if (!node->aux)
	{
	  int local_insns;
	  tree decl = node->decl;

          node->global.inlined_to = NULL;
	  if (DECL_STRUCT_FUNCTION (decl))
	    local_insns = node->local.self_insns;
	  else
	    local_insns = 0;
	  if (cgraph_dump_file)
	    fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
	  if (!node->analyzed || !DECL_EXTERNAL (node->decl))
	    cgraph_remove_node (node);
	  else
	    {
	      struct cgraph_edge *e;

	      for (e = node->callers; e; e = e->next_caller)
		if (e->caller->aux)
		  break;
	      if (e || node->needed)
		{
		  struct cgraph_node *clone;

		  for (clone = node->next_clone; clone;
		       clone = clone->next_clone)
		    if (clone->aux)
		      break;
		  if (!clone)
		    {
		      DECL_SAVED_TREE (node->decl) = NULL;
		      DECL_STRUCT_FUNCTION (node->decl) = NULL;
		      DECL_INITIAL (node->decl) = error_mark_node;
		    }
		  while (node->callees)
		    cgraph_remove_edge (node->callees);
		  node->analyzed = false;
		}
	      else
		cgraph_remove_node (node);
	    }
	  if (!DECL_SAVED_TREE (decl))
	    insns += local_insns;
	  changed = true;
	}
    }
  for (node = cgraph_nodes; node; node = node->next)
    node->aux = NULL;
  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\nReclaimed %i insns", insns);
  return changed;
}

/* Estimate size of the function after inlining WHAT into TO.  */

static int
cgraph_estimate_size_after_inlining (int times, struct cgraph_node *to,
				     struct cgraph_node *what)
{
  /* Avoid negative size estimates when (what->global_insns < INSNS_PER_CALL).  */
  return max (what->global.insns - INSNS_PER_CALL, 0) * times + to->global.insns;
}

/* Estimate the growth caused by inlining NODE into all callees.  */

static int
cgraph_estimate_growth (struct cgraph_node *node)
{
  int growth = 0;
  struct cgraph_edge *e;

  for (e = node->callers; e; e = e->next_caller)
    if (e->inline_failed)
      growth += (cgraph_estimate_size_after_inlining (1, e->caller, node)
		 - e->caller->global.insns);

  /* ??? Wrong for self recursive functions or cases where we decide to not
     inline for different reasons, but it is not big deal as in that case
     we will keep the body around, but we will also avoid some inlining.  */
  if (!node->needed && !DECL_EXTERNAL (node->decl))
    growth -= node->global.insns;

  return growth;
}

static struct cgraph_node *already_cloned;

/* E is expected to be an edge being inlined.  Clone destination node of
   the edge and redirect it to the new clone.
   DUPLICATE is used for bookkeeping on whether we are actually creating new
   clones or re-using node originally representing out-of-line function call.
   */
static void
cgraph_clone_inlined_nodes_1 (struct cgraph_edge *e, bool duplicate)
{
  struct cgraph_node *n = e->callee;
  struct cgraph_edge *step_edge;

  if (e->callee->aux)
    abort();

  /* We may eliminate the need for out-of-line copy to be output.  In that
     case just go ahead and re-use it.  */
  if (!e->callee->callers->next_caller
      && (!e->callee->needed || DECL_EXTERNAL (e->callee->decl))
      && duplicate
      && flag_unit_at_a_time)
    {
      if (e->callee->global.inlined_to)
	abort ();
      if (!DECL_EXTERNAL (e->callee->decl))
        overall_insns -= e->callee->global.insns, nfunctions_inlined++;
      duplicate = 0;
    }
   else if (duplicate)
    {
      e->callee->aux = already_cloned;
      already_cloned = e->callee;
      n = cgraph_clone_node (e->callee);
      cgraph_redirect_edge_callee (e, n);
    }

  if (e->caller->global.inlined_to)
    n->global.inlined_to = e->caller->global.inlined_to;
  else
    n->global.inlined_to = e->caller;

  /* Recursively clone all bodies.  Non-zero aux means we've handled
     this edge already; skip it to avoid confusing ourselves.  */
  for (step_edge = n->callees; step_edge; step_edge = step_edge->next_callee)
    if (!step_edge->inline_failed && !step_edge->callee->aux)
      cgraph_clone_inlined_nodes_1 (step_edge, duplicate);
}

/* E is expected to be an edge being inlined.  Clone destination node of
   the edge and redirect it to the new clone.
   DUPLICATE is used for bookeeping on whether we are actually creating new
   clones or re-using node originally representing out-of-line function call.
   */
void
cgraph_clone_inlined_nodes (struct cgraph_edge *e, bool duplicate)
{
  struct cgraph_node *next_node;
  struct cgraph_node *step_node;
  struct cgraph_node end_node;        /* A non-NULL end-of-chain marker.  */
  
  end_node.aux = (struct cgraph_node *)0;
  already_cloned = &end_node;
  cgraph_clone_inlined_nodes_1 (e, duplicate);
  for (step_node = already_cloned; step_node != &end_node; step_node = next_node)
    {
      next_node = step_node->aux;
      step_node->aux = (struct cgraph_node *)0;
    }
  already_cloned = (struct cgraph_node *)0;
}

/* Mark edge E as inlined and update callgraph accordingly.  */

void
cgraph_mark_inline_edge (struct cgraph_edge *e)
{
  int old_insns = 0, new_insns = 0;
  struct cgraph_node *to = NULL, *what;
  bool simple_recursion = (e->callee == e->caller);

  if (!e->inline_failed)
    abort ();
  e->inline_failed = NULL;

  if (!e->callee->global.inlined && flag_unit_at_a_time)
    DECL_POSSIBLY_INLINED (e->callee->decl) = true;
  e->callee->global.inlined = true;

  cgraph_clone_inlined_nodes (e, true);

  what = e->callee;
  if (simple_recursion)
    {
      struct cgraph_edge* e2;
      /* New edge from original to copy should be marked !inline_failed, but
	 edge(s) back from copy to original (which we just created by
	 cloning the original->copy edge) should not be.  Indirect
	 recursion doesn't have the problem. */
      for (e2 = what->callees; e2; e2 = e2->next_callee)
	if (e2->callee == e->caller)
	  e2->inline_failed = N_("function not considered for inlining");
    }

  /* Install the just-cloned cgraph_node on the list.  */

  /* Now update size of caller and all functions caller is inlined into.  */
  for (;e && !e->inline_failed; e = e->caller->callers)
    {
      old_insns = e->caller->global.insns;
      new_insns = cgraph_estimate_size_after_inlining (1, e->caller,
						       what);
      if (new_insns < 0)
	abort ();
      to = e->caller;
      to->global.insns = new_insns;
    }
  if (what->global.inlined_to != to)
    abort ();
  overall_insns += new_insns - old_insns;
  ncalls_inlined++;
}

/* Mark all calls of EDGE->CALLEE inlined into EDGE->CALLER.
   Return following unredirected edge in the list of callers
   of EDGE->CALLEE  */

static struct cgraph_edge *
cgraph_mark_inline (struct cgraph_edge *edge)
{
  struct cgraph_node *to = edge->caller;
  struct cgraph_node *what = edge->callee;
  struct cgraph_edge *e, *next;
  int times = 0;

  /* Look for all calls, mark them inline and clone recursively
     all inlined functions.  */
  for (e = what->callers; e; e = next)
    {
      next = e->next_caller;
      if (e->caller == to && e->inline_failed)
	{
          cgraph_mark_inline_edge (e);
	  if (e == edge)
	    edge = next;
	  times ++;
	}
    }
  if (!times)
    abort ();
  return edge;
}

/* Return false when inlining WHAT into TO is not good idea
   as it would cause too large growth of function bodies.  */

static bool
cgraph_check_inline_limits (struct cgraph_node *to, struct cgraph_node *what,
			    const char **reason)
{
  int times = 0;
  struct cgraph_edge *e;
  int newsize;
  int limit;

  if (to->global.inlined_to)
    to = to->global.inlined_to;

  for (e = to->callees; e; e = e->next_callee)
    if (e->callee == what)
      times++;

  /* When inlining large function body called once into small function,
     take the inlined function as base for limiting the growth.  */
  if (to->local.self_insns > what->local.self_insns)
    limit = to->local.self_insns;
  else
    limit = what->local.self_insns;

  limit += limit * PARAM_VALUE (PARAM_LARGE_FUNCTION_GROWTH) / 100;

  newsize = cgraph_estimate_size_after_inlining (times, to, what);
  if (newsize > PARAM_VALUE (PARAM_LARGE_FUNCTION_INSNS)
      && newsize > limit)
    {
      if (reason)
        *reason = N_("--param large-function-growth limit reached");
      return false;
    }
  return true;
}

/* Return true when function N is small enough to be inlined.  */

static bool
cgraph_default_inline_p (struct cgraph_node *n)
{
  if (!DECL_INLINE (n->decl) || !DECL_SAVED_TREE (n->decl))
    return false;
  if (DECL_DECLARED_INLINE_P (n->decl))
    return n->global.insns < MAX_INLINE_INSNS_SINGLE;
  else
    return n->global.insns < MAX_INLINE_INSNS_AUTO;
}

/* Return true when inlining WHAT would create recursive inlining.
   We call recursive inlining all cases where same function appears more than
   once in the single recursion nest path in the inline graph.  */

static bool
cgraph_recursive_inlining_p (struct cgraph_node *to,
			     struct cgraph_node *what,
			     const char **reason)
{
  bool recursive;
  if (to->global.inlined_to)
    recursive = what->decl == to->global.inlined_to->decl;
  else
    recursive = what->decl == to->decl;
  /* Marking recursive function inline has sane semantic and thus we should
     not warn on it.  */
  if (recursive && reason)
    *reason = (what->local.disregard_inline_limits
	       ? N_("recursive inlining") : "");
  return recursive;
}

/* Recompute heap nodes for each of callees.  */
static void
update_callee_keys (fibheap_t heap, struct fibnode **heap_node,
		    struct cgraph_node *node)
{
  struct cgraph_edge *e;

  for (e = node->callees; e; e = e->next_callee)
    if (e->inline_failed && heap_node[e->callee->uid])
      fibheap_replace_key (heap, heap_node[e->callee->uid],
			   cgraph_estimate_growth (e->callee));
    else if (!e->inline_failed)
      update_callee_keys (heap, heap_node, e->callee);
}

/* Enqueue all recursive calls from NODE into queue linked via aux pointers
   in between FIRST and LAST.  WHERE is used for bookkeeping while looking
   int calls inlined within NODE.  */
static void
lookup_recursive_calls (struct cgraph_node *node, struct cgraph_node *where,
			struct cgraph_edge **first, struct cgraph_edge **last)
{
  struct cgraph_edge *e;
  for (e = where->callees; e; e = e->next_callee)
    if (e->callee == node)
      {
	if (!*first)
	  *first = e;
	else
	  (*last)->aux = e;
	*last = e;
      }
  for (e = where->callees; e; e = e->next_callee)
    if (!e->inline_failed)
      lookup_recursive_calls (node, e->callee, first, last);
}

/* Decide on recursive inlining: in the case function has recursive calls,
   inline until body size reaches given argument.  */
static void
cgraph_decide_recursive_inlining (struct cgraph_node *node)
{
  int limit = PARAM_VALUE (PARAM_MAX_INLINE_INSNS_RECURSIVE_AUTO);
  int max_depth = PARAM_VALUE (PARAM_MAX_INLINE_RECURSIVE_DEPTH_AUTO);
  struct cgraph_edge *first_call = NULL, *last_call = NULL;
  struct cgraph_edge *last_in_current_depth;
  struct cgraph_edge *e;
  struct cgraph_node *master_clone;
  int depth = 0;
  int n = 0;

  if (DECL_DECLARED_INLINE_P (node->decl))
    {
      limit = PARAM_VALUE (PARAM_MAX_INLINE_INSNS_RECURSIVE);
      max_depth = PARAM_VALUE (PARAM_MAX_INLINE_RECURSIVE_DEPTH);
    }

  /* Make sure that function is small enought to be considered for inlining.  */
  if (!max_depth
      || cgraph_estimate_size_after_inlining (1, node, node)  >= limit)
    return;
  lookup_recursive_calls (node, node, &first_call, &last_call);
  if (!first_call)
    return;

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, 
	     "\nPerforming recursive inlining on %s\n",
	     cgraph_node_name (node));

  /* We need original clone to copy around.  */
  master_clone = cgraph_clone_node (node);
  master_clone->needed = true;
  for (e = master_clone->callees; e; e = e->next_callee)
    if (!e->inline_failed)
      cgraph_clone_inlined_nodes (e, true);

  /* Do the inlining and update list of recursive call during process.  */
  last_in_current_depth = last_call;
  while (first_call
	 && cgraph_estimate_size_after_inlining (1, node, master_clone) <= limit)
    {
      struct cgraph_edge *curr = first_call;

      first_call = first_call->aux;
      curr->aux = NULL;

      cgraph_redirect_edge_callee (curr, master_clone);
      cgraph_mark_inline_edge (curr);
      lookup_recursive_calls (node, curr->callee, &first_call, &last_call);

      if (last_in_current_depth
	  && ++depth >= max_depth)
	break;
      n++;
    }

  /* Cleanup queue pointers.  */
  while (first_call)
    {
      struct cgraph_edge *next = first_call->aux;
      first_call->aux = NULL;
      first_call = next;
    }
  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, 
	     "\n   Inlined %i times, body grown from %i to %i insns\n", n,
	     master_clone->global.insns, node->global.insns);

  /* Remove master clone we used for inlining.  We rely that clones inlined
     into master clone gets queued just before master clone so we don't
     need recursion.  */
  for (node = cgraph_nodes; node != master_clone;
       node = node->next)
    if (node->global.inlined_to == master_clone)
      cgraph_remove_node (node);
  cgraph_remove_node (master_clone);
}

/* Set inline_failed for all callers of given function to REASON.  */

static void
cgraph_set_inline_failed (struct cgraph_node *node, const char *reason)
{
  struct cgraph_edge *e;

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "Inlining failed: %s\n", reason);
  for (e = node->callers; e; e = e->next_caller)
    if (e->inline_failed)
      e->inline_failed = reason;
}

static cgraph_desirability_type
cgraph_desirability (struct cgraph_edge *edge)
{
  HOST_WIDEST_INT desire;
  HOST_WIDEST_INT denominator;
  struct cgraph_node *callee = edge->callee;
  
  if (!callee->local.inlinable)
    return 0;
  
  if (callee->local.disregard_inline_limits)
    return MAX_DESIRABILITY;
  
  /* FIXME: Ideally we'd replace this with a more sophisticated
     "temperature" sort of metric.  */
  denominator = callee->local.self_insns + callee->insn_growth;
  if (denominator)
    desire = edge->count / denominator;
  else
    desire = 0;
  return desire;
}

static struct cgraph_edge *
cgraph_pick_most_desirable_edge (struct cgraph_node *node)
{
  struct cgraph_edge *step_edge;
  struct cgraph_edge *most_desirable_edge;
  HOST_WIDEST_INT highest_desire;
  
  highest_desire = 0;
  most_desirable_edge = (struct cgraph_edge *)0;
  for (step_edge = node->callees;
       step_edge;
       step_edge = step_edge->next_callee)
    {
      if (step_edge->inline_failed
	  && (step_edge->desirability > highest_desire))
	{
	  highest_desire = step_edge->desirability;
	  most_desirable_edge = step_edge;
	}
    }
  return most_desirable_edge;
}


static void
cgraph_profile_driven_inlining (void)
{
  struct cgraph_node *step_node;
  struct cgraph_edge *step_edge;
  HOST_WIDEST_INT highest_desire;
  struct cgraph_edge *most_desirable_edge;
  struct cgraph_node *most_desirable_node;
  
  /* Pass 1: compute desirability of all CALL_EXPRs.  */
  for (step_node = cgraph_nodes;
       step_node;
       step_node = step_node->next)
    {
      highest_desire = 0;
      most_desirable_edge = (struct cgraph_edge *)0;
      for (step_edge = step_node->callees;
	   step_edge;
	   step_edge = step_edge->next_callee)
	{
	  step_edge->desirability = cgraph_desirability (step_edge);
	  if (step_edge->desirability > highest_desire)
	    {
	      highest_desire = step_edge->desirability;
	      most_desirable_edge = step_edge;
	    }
	}
      step_node->most_desirable = most_desirable_edge;
    }
  /* Pass 2: pick edges to inline.  */
  while (overall_insns <= max_insns)
    {
      highest_desire = 0;
      most_desirable_node = (struct cgraph_node *)0;
      for (step_node = cgraph_nodes;
	   step_node;
	   step_node = step_node->next)
	{
	  if (step_node->most_desirable
	      && (step_node->most_desirable->desirability > highest_desire))
	    {
	      highest_desire = step_node->most_desirable->desirability;
	      most_desirable_node = step_node;
	    }
	}
      /* If we found a suitable CALL_EXPR, record our choice, else
	 abandon the search.  */
      if (most_desirable_node)
	{
	  cgraph_mark_inline_edge (most_desirable_node->most_desirable);
	  most_desirable_node->most_desirable = cgraph_pick_most_desirable_edge (most_desirable_node);
	}
      else
        break;
    }
}

/* We use greedy algorithm for inlining of small functions:
   All inline candidates are put into prioritized heap based on estimated
   growth of the overall number of instructions and then update the estimates.

   INLINED and INLINED_CALEES are just pointers to arrays large enough
   to be passed to cgraph_inlined_into and cgraph_inlined_callees.  */

static void
cgraph_decide_inlining_of_small_functions (void)
{
  struct cgraph_node *node;
  fibheap_t heap = fibheap_new ();
  struct fibnode **heap_node =
    xcalloc (cgraph_max_uid, sizeof (struct fibnode *));

  /* Put all inline candidates into the heap.  */

  for (node = cgraph_nodes; node; node = node->next)
    {
      if (!node->local.inlinable || !node->callers
	  || node->local.disregard_inline_limits)
	continue;

      if (!cgraph_default_inline_p (node))
	{
	  cgraph_set_inline_failed (node,
	    N_("--param max-inline-insns-single limit reached"));
	  continue;
	}
      heap_node[node->uid] =
	fibheap_insert (heap, cgraph_estimate_growth (node), node);
    }

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\nDeciding on smaller functions:\n");
  while (overall_insns <= max_insns && (node = fibheap_extract_min (heap)))
    {
      struct cgraph_edge *e, *next;
      int old_insns = overall_insns;

      heap_node[node->uid] = NULL;
      if (cgraph_dump_file)
	fprintf (cgraph_dump_file, 
		 "\nConsidering %s with %i insns\n"
		 " Estimated growth is %+i insns.\n",
		 cgraph_node_name (node), node->global.insns,
		 cgraph_estimate_growth (node));
      if (!cgraph_default_inline_p (node))
	{
	  cgraph_set_inline_failed (node,
	    N_("--param max-inline-insns-single limit reached after inlining into the callee"));
	  continue;
	}
      for (e = node->callers; e; e = next)
	{
	  next = e->next_caller;
	  if (e->inline_failed)
	    {
	      struct cgraph_node *where;

	      if (cgraph_recursive_inlining_p (e->caller, e->callee,
				      	       &e->inline_failed)
		  || !cgraph_check_inline_limits (e->caller, e->callee,
			  			  &e->inline_failed))
		{
		  if (cgraph_dump_file)
		    fprintf (cgraph_dump_file, " Not inlining into %s:%s.\n",
			     cgraph_node_name (e->caller), e->inline_failed);
		  continue;
		}
	      next = cgraph_mark_inline (e);
	      where = e->caller;
	      if (where->global.inlined_to)
		where = where->global.inlined_to;

	      if (heap_node[where->uid])
		fibheap_replace_key (heap, heap_node[where->uid],
				     cgraph_estimate_growth (where));

	      if (cgraph_dump_file)
		fprintf (cgraph_dump_file, 
			 " Inlined into %s which now has %i insns.\n",
			 cgraph_node_name (e->caller),
			 e->caller->global.insns);
	    }
	}

      cgraph_decide_recursive_inlining (node);

      /* Similarly all functions called by the function we just inlined
         are now called more times; update keys.  */
      update_callee_keys (heap, heap_node, node);

      if (cgraph_dump_file)
	fprintf (cgraph_dump_file, 
		 " Inlined for a net change of %+i insns.\n",
		 overall_insns - old_insns);
    }
  while ((node = fibheap_extract_min (heap)) != NULL)
    if (!node->local.disregard_inline_limits)
      cgraph_set_inline_failed (node, N_("--param inline-unit-growth limit reached"));
  fibheap_delete (heap);
  free (heap_node);
}

/* Decide on the inlining.  We do so in the topological order to avoid
   expenses on updating data structures.  */

static void
cgraph_decide_inlining (void)
{
  struct cgraph_node *node;
  int nnodes;
  struct cgraph_node **order =
    xcalloc (cgraph_n_nodes, sizeof (struct cgraph_node *));
  int old_insns = 0;
  int i;

  for (node = cgraph_nodes; node; node = node->next)
    initial_insns += node->local.self_insns;
  overall_insns = initial_insns;

  max_insns = ((HOST_WIDEST_INT) overall_insns
	       * (100 + PARAM_VALUE (PARAM_INLINE_UNIT_GROWTH)) / 100);

  nnodes = cgraph_postorder (order);

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file,
	     "\nDeciding on inlining.  Starting with %i insns.\n",
	     initial_insns);

  for (node = cgraph_nodes; node; node = node->next)
    node->aux = 0;

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\nInlining always_inline functions:\n");

  /* In the first pass mark all always_inline edges.  Do this with a priority
     so none of our later choices will make this impossible.  */
  for (i = nnodes - 1; i >= 0; i--)
    {
      struct cgraph_edge *e, *next;

      node = order[i];

      if (!node->local.disregard_inline_limits)
	continue;
      if (cgraph_dump_file)
	fprintf (cgraph_dump_file,
		 "\nConsidering %s %i insns (always inline)\n",
		 cgraph_node_name (node), node->global.insns);
      old_insns = overall_insns;
      for (e = node->callers; e; e = next)
	{
	  next = e->next_caller;
	  if (!e->inline_failed)
	    continue;
	  if (cgraph_recursive_inlining_p (e->caller, e->callee,
				  	   &e->inline_failed))
	    continue;
	  cgraph_mark_inline_edge (e);
	  if (cgraph_dump_file)
	    fprintf (cgraph_dump_file, 
		     " Inlined into %s which now has %i insns.\n",
		     cgraph_node_name (e->caller),
		     e->caller->global.insns);
	}
      if (cgraph_dump_file)
	fprintf (cgraph_dump_file, 
		 " Inlined for a net change of %+i insns.\n",
		 overall_insns - old_insns);
    }

  if (!flag_really_no_inline)
    {
      /* If we have profiling information, use it to choose inlining candidates.  */
      if (profile_info)
	cgraph_profile_driven_inlining ();
      else
	cgraph_decide_inlining_of_small_functions ();

      if (cgraph_dump_file)
	fprintf (cgraph_dump_file, "\nDeciding on functions called once:\n");

      /* And finally decide what functions are called once.  */

      for (i = nnodes - 1; i >= 0; i--)
	{
	  node = order[i];

	  if (node->callers && !node->callers->next_caller && !node->needed
	      && node->local.inlinable && node->callers->inline_failed
	      && !DECL_EXTERNAL (node->decl) && !DECL_COMDAT (node->decl))
	    {
	      bool ok = true;
	      struct cgraph_node *node1;

	      /* Verify that we won't duplicate the caller.  */
	      for (node1 = node->callers->caller;
		   node1->callers && !node1->callers->inline_failed
		   && ok; node1 = node1->callers->caller)
		if (node1->callers->next_caller || node1->needed)
		  ok = false;
	      if (ok)
		{
		  if (cgraph_dump_file)
		    fprintf (cgraph_dump_file,
			     "\nConsidering %s %i insns.\n"
			     " Called once from %s %i insns.\n",
			     cgraph_node_name (node), node->global.insns,
			     cgraph_node_name (node->callers->caller),
			     node->callers->caller->global.insns);

		  old_insns = overall_insns;

		  if (cgraph_check_inline_limits (node->callers->caller, node,
					  	  NULL))
		    {
		      cgraph_mark_inline (node->callers);
		      if (cgraph_dump_file)
			fprintf (cgraph_dump_file,
				 " Inlined into %s which now has %i insns"
				 " for a net change of %+i insns.\n",
				 cgraph_node_name (node->callers->caller),
				 node->callers->caller->global.insns,
				 overall_insns - old_insns);
		    }
		  else
		    {
		      if (cgraph_dump_file)
			fprintf (cgraph_dump_file,
				 " Inline limit reached, not inlined.\n");
		    }
		}
	    }
	}
    }

  /* We will never output extern functions we didn't inline. 
     ??? Perhaps we can prevent accounting of growth of external
     inline functions.  */
  cgraph_remove_unreachable_nodes ();

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file,
	     "\nInlined %i calls, eliminated %i functions, "
	     "%i insns turned to %i insns.\n\n",
	     ncalls_inlined, nfunctions_inlined, initial_insns,
	     overall_insns);
  free (order);
}

/* Decide on the inlining.  We do so in the topological order to avoid
   expenses on updating data structures.  */

static void
cgraph_decide_inlining_incrementally (struct cgraph_node *node)
{
  struct cgraph_edge *e;

  /* First of all look for always inline functions.  */
  for (e = node->callees; e; e = e->next_callee)
    if (e->callee->local.disregard_inline_limits
	&& e->inline_failed
        && !cgraph_recursive_inlining_p (node, e->callee, &e->inline_failed)
	/* ??? It is possible that renaming variable removed the function body
	   in duplicate_decls. See gcc.c-torture/compile/20011119-2.c  */
	&& DECL_SAVED_TREE (e->callee->decl))
      cgraph_mark_inline (e);

  /* Now do the automatic inlining.  */
  if (!flag_really_no_inline)
    for (e = node->callees; e; e = e->next_callee)
      if (e->callee->local.inlinable
	  && e->inline_failed
	  && !e->callee->local.disregard_inline_limits
	  && !cgraph_recursive_inlining_p (node, e->callee, &e->inline_failed)
	  && cgraph_check_inline_limits (node, e->callee, &e->inline_failed)
	  && DECL_SAVED_TREE (e->callee->decl))
	{
	  if (cgraph_default_inline_p (e->callee))
	    cgraph_mark_inline (e);
	  else
	    e->inline_failed
	      = N_("--param max-inline-insns-single limit reached");
	}
}


/* Return true when CALLER_DECL should be inlined into CALLEE_DECL.  */

bool
cgraph_inline_p (struct cgraph_edge *e, const char **reason)
{
  *reason = e->inline_failed;
  return !e->inline_failed;
}

/* Expand all functions that must be output.

   Attempt to topologically sort the nodes so function is output when
   all called functions are already assembled to allow data to be
   propagated across the callgraph.  Use a stack to get smaller distance
   between a function and its callees (later we may choose to use a more
   sophisticated algorithm for function reordering; we will likely want
   to use subsections to make the output functions appear in top-down
   order).  */

static void
cgraph_expand_all_functions (void)
{
  struct cgraph_node *node;
  struct cgraph_node **order =
    xcalloc (cgraph_n_nodes, sizeof (struct cgraph_node *));
  int order_pos = 0, new_order_pos = 0;
  int i;

  cgraph_mark_functions_to_output ();

  order_pos = cgraph_postorder (order);
  if (order_pos != cgraph_n_nodes)
    abort ();

  /* Garbage collector may remove inline clones we eliminate during
     optimization.  So we must be sure to not reference them.  */
  for (i = 0; i < order_pos; i++)
    if (order[i]->output)
      order[new_order_pos++] = order[i];

  for (i = new_order_pos - 1; i >= 0; i--)
    {
      node = order[i];
      if (node->output)
	{
	  if (!node->reachable)
	    abort ();
	  node->output = 0;
	  cgraph_expand_function (node);
	}
    }
  free (order);
}

/* Mark all local functions.

   A local function is one whose calls can occur only in the
   current compilation unit and all its calls are explicit,
   so we can change its calling convention.
   We simply mark all static functions whose address is not taken
   as local.  */

static void
cgraph_mark_local_functions (void)
{
  struct cgraph_node *node;

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\nMarking local functions:");

  /* Figure out functions we want to assemble.  */
  for (node = cgraph_nodes; node; node = node->next)
    {
      node->local.local = (!node->needed
		           && DECL_SAVED_TREE (node->decl)
		           && !TREE_PUBLIC (node->decl));
      if (cgraph_dump_file && node->local.local)
	fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
    }
  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\n\n");
}

/* Return true when function body of DECL still needs to be kept around
   for later re-use.  */
bool
cgraph_preserve_function_body_p (tree decl)
{
  struct cgraph_node *node;
  /* Keep the body; we're going to dump it.  */
  if (dump_enabled_p (TDI_all))
    return true;
  if (!cgraph_global_info_ready)
    return (DECL_INLINE (decl) && !flag_really_no_inline);
  /* Look if there is any clone around.  */
  for (node = cgraph_node (decl); node; node = node->next_clone)
    if (node->global.inlined_to)
      return true;
  return false;
}

/* Perform simple optimizations based on callgraph.  */

void
cgraph_optimize (void)
{
#ifdef ENABLE_CHECKING
  verify_cgraph ();
#endif
  if (!flag_unit_at_a_time)
    return;
  timevar_push (TV_CGRAPHOPT);
  if (!quiet_flag)
    fprintf (stderr, "Performing intraprocedural optimizations\n");

  cgraph_mark_local_functions ();
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Marked ");
      dump_cgraph (cgraph_dump_file);
    }

  if (flag_inline_trees)
    cgraph_decide_inlining ();
  cgraph_global_info_ready = true;
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Optimized ");
      dump_cgraph (cgraph_dump_file);
    }
  timevar_pop (TV_CGRAPHOPT);

  /* Output everything.  */
  if (!quiet_flag)
    fprintf (stderr, "Assembling functions:\n");
#ifdef ENABLE_CHECKING
  verify_cgraph ();
#endif
#if 0
  if (flag_peel_structs)
    cgraph_peel_structs ();
#endif
  cgraph_expand_all_functions ();
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "\nFinal ");
      dump_cgraph (cgraph_dump_file);
    }
#ifdef ENABLE_CHECKING
  verify_cgraph ();
  /* Double check that all inline clones are gone and that all
     function bodies have been released from memory.  */
  if (flag_unit_at_a_time
      && !dump_enabled_p (TDI_all)
      && !(sorrycount || errorcount))
    {
      struct cgraph_node *node;
      bool error_found = false;

      for (node = cgraph_nodes; node; node = node->next)
	if (node->analyzed
	    && (node->global.inlined_to
	        || DECL_SAVED_TREE (node->decl)))
	  {
	    error_found = true;
	    dump_cgraph_node (stderr, node);
 	  }
      if (error_found)
	internal_error ("Nodes with no released memory found.");
    }
#endif
}

/* Generate and emit a static constructor or destructor.  WHICH must be
   one of 'I' or 'D'.  BODY should be a STATEMENT_LIST containing 
   GENERIC statements.  */

void
cgraph_build_static_cdtor (char which, tree body, int priority)
{
  static int counter = 0;
  char which_buf[16];
  tree decl, name, resdecl;

  sprintf (which_buf, "%c_%d", which, counter++);
  name = get_file_function_name_long (which_buf);

  decl = build_decl (FUNCTION_DECL, name,
		     build_function_type (void_type_node, void_list_node));
  current_function_decl = decl;

  resdecl = build_decl (RESULT_DECL, NULL_TREE, void_type_node);
  DECL_ARTIFICIAL (resdecl) = 1;
  DECL_IGNORED_P (resdecl) = 1;
  DECL_RESULT (decl) = resdecl;

  allocate_struct_function (decl);

  TREE_STATIC (decl) = 1;
  TREE_USED (decl) = 1;
  DECL_ARTIFICIAL (decl) = 1;
  DECL_IGNORED_P (decl) = 1;
  DECL_NO_INSTRUMENT_FUNCTION_ENTRY_EXIT (decl) = 1;
  DECL_SAVED_TREE (decl) = body;
  TREE_PUBLIC (decl) = ! targetm.have_ctors_dtors;
  DECL_UNINLINABLE (decl) = 1;

  DECL_INITIAL (decl) = make_node (BLOCK);
  TREE_USED (DECL_INITIAL (decl)) = 1;

  DECL_SOURCE_LOCATION (decl) = input_location;
  cfun->function_end_locus = input_location;

  if (which == 'I')
    DECL_STATIC_CONSTRUCTOR (decl) = 1;
  else if (which == 'D')
    DECL_STATIC_DESTRUCTOR (decl) = 1;
  else
    abort ();

  gimplify_function_tree (decl);

  cgraph_build_cfg (decl);

  /* ??? We will get called LATE in the compilation process.  */
  if (cgraph_global_info_ready)
    tree_rest_of_compilation (decl, false);
  else
    cgraph_finalize_function (decl, 0);
  
  if (targetm.have_ctors_dtors)
    {
      void (*fn) (rtx, int);

      if (which == 'I')
	fn = targetm.asm_out.constructor;
      else
	fn = targetm.asm_out.destructor;
      fn (XEXP (DECL_RTL (decl), 0), priority);
    }
}

/* A function used to throw (we thought) and now we know better.
   It is possible that functions that call this one have
   already built CFGs and region info that is now wrong.
   Fix these. */
void cgraph_change_to_nothrow (tree funcdecl)
{
  struct cgraph_node *node = cgraph_node (funcdecl);
  struct cgraph_edge *cge;
  struct function *ifun;
  tree call, caller_decl;
  basic_block bb;
  for (cge = node->callers; cge; cge = cge->next_caller)
    {
      call = cge->call_expr;
      caller_decl = cge->caller->decl;
      ifun = DECL_STRUCT_FUNCTION (caller_decl);
      if (ifun && (ifun->cfg || ifun->eh))
        {
	  push_cfun (ifun);
	  /* This part is currently s l o w, as there is no
	     pointer up to the bb node.  */
	  FOR_EACH_BB (bb)
	    {
	      block_stmt_iterator bsi = bsi_last (bb);
	      tree stmt;
	      if (bsi_end_p (bsi))
		continue;
	      stmt = bsi_stmt (bsi);
	      if (stmt 
		  && (stmt == call 
		      || (TREE_CODE (stmt) == MODIFY_EXPR
			  && TREE_OPERAND (stmt, 1) == call)
		      || (TREE_CODE (stmt) == RETURN_EXPR
			  && TREE_OPERAND (stmt, 0)
			  && TREE_CODE (TREE_OPERAND (stmt, 0)) == MODIFY_EXPR
			  && TREE_OPERAND (TREE_OPERAND (stmt, 0), 1) == call)))
		{
		  /* found it. */
		  edge e, next_e;
		  for (e = bb->succ; e; e = next_e)
		    {
		      next_e = e->succ_next;
		      if (e->flags & EDGE_EH)
			remove_edge (e);
		    }
		  remove_stmt_from_eh_region_fn (ifun, stmt);
		  break;
		}
	    }
	  pop_cfun ();
	}
    }
}
