/* Callgraph based interprocedural optimizations.
   Copyright (C) 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
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
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/* This module implements main driver of compilation process as well as
   few basic interprocedural optimizers.

   The main scope of this file is to act as an interface in between
   tree based frontends and the backend (and middle end)

   The front-end is supposed to use following functionality:

    - cgraph_finalize_function

      This function is called once front-end has parsed whole body of function
      and it is certain that the function body nor the declaration will change.

      (There is one exception needed for implementing GCC extern inline
	function.)

    - varpool_finalize_variable

      This function has same behavior as the above but is used for static
      variables.

    - cgraph_finalize_compilation_unit

      This function is called once (source level) compilation unit is finalized
      and it will no longer change.

      In the unit-at-a-time the call-graph construction and local function
      analysis takes place here.  Bodies of unreachable functions are released
      to conserve memory usage.

      The function can be called multiple times when multiple source level
      compilation units are combined (such as in C frontend)

    - cgraph_optimize

      In this unit-at-a-time compilation the intra procedural analysis takes
      place here.  In particular the static functions whose address is never
      taken are marked as local.  Backend can then use this information to
      modify calling conventions, do better inlining or similar optimizations.

    - cgraph_mark_needed_node
    - varpool_mark_needed_node

      When function or variable is referenced by some hidden way the call-graph
      data structure must be updated accordingly by this function.
      There should be little need to call this function and all the references
      should be made explicit to cgraph code.  At present these functions are
      used by C++ frontend to explicitly mark the keyed methods.

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
      varpool_assemble_pending_variables, cgraph_finalize_function,
      varpool_finalize_function, cgraph_optimize can cause arbitrarily
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

	The intra-procedural information is produced and its existence
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
	is completely relied upn assemble_variable to mark them.  */


#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "rtl.h"
#include "tree-flow.h"
#include "tree-inline.h"
#include "langhooks.h"
#include "pointer-set.h"
#include "toplev.h"
#include "flags.h"
#include "ggc.h"
#include "debug.h"
#include "target.h"
#include "cgraph.h"
#include "diagnostic.h"
#include "timevar.h"
#include "params.h"
#include "fibheap.h"
#include "c-common.h"
#include "intl.h"
#include "function.h"
#include "ipa-prop.h"
#include "tree-gimple.h"
#include "tree-pass.h"
#include "output.h"

static void cgraph_expand_all_functions (void);
static void cgraph_mark_functions_to_output (void);
static void cgraph_expand_function (struct cgraph_node *);
static void cgraph_output_pending_asms (void);

static FILE *cgraph_dump_file;

static GTY (()) tree static_ctors;
static GTY (()) tree static_dtors;

/* When target does not have ctors and dtors, we call all constructor
   and destructor by special initialization/destruction functio
   recognized by collect2.  
   
   When we are going to build this function, collect all constructors and
   destructors and turn them into normal functions.  */

static void
record_cdtor_fn (tree fndecl)
{
  if (targetm.have_ctors_dtors)
    return;

  if (DECL_STATIC_CONSTRUCTOR (fndecl))
    {
      static_ctors = tree_cons (NULL_TREE, fndecl, static_ctors);
      DECL_STATIC_CONSTRUCTOR (fndecl) = 0;
      cgraph_mark_reachable_node (cgraph_node (fndecl));
    }
  if (DECL_STATIC_DESTRUCTOR (fndecl))
    {
      static_dtors = tree_cons (NULL_TREE, fndecl, static_dtors);
      DECL_STATIC_DESTRUCTOR (fndecl) = 0;
      cgraph_mark_reachable_node (cgraph_node (fndecl));
    }
}

/* Synthesize a function which calls all the global ctors or global
   dtors in this file.  This is only used for targets which do not
   support .ctors/.dtors sections.  */
static void
build_cdtor (int method_type, tree cdtors)
{
  tree body = 0;

  if (!cdtors)
    return;

  for (; cdtors; cdtors = TREE_CHAIN (cdtors))
    append_to_statement_list (build_function_call_expr (TREE_VALUE (cdtors), 0),
			      &body);

  cgraph_build_static_cdtor (method_type, body, DEFAULT_INIT_PRIORITY);
}

/* Generate functions to call static constructors and destructors
   for targets that do not support .ctors/.dtors sections.  These
   functions have magic names which are detected by collect2.  */

static void
cgraph_build_cdtor_fns (void)
{
  if (!targetm.have_ctors_dtors)
    {
      build_cdtor ('I', static_ctors); 
      static_ctors = NULL_TREE;
      build_cdtor ('D', static_dtors); 
      static_dtors = NULL_TREE;
    }
  else
    {
      gcc_assert (!static_ctors);
      gcc_assert (!static_dtors);
    }
}

/* Determine if function DECL is needed.  That is, visible to something
   either outside this translation unit, something magic in the system
   configury, or (if not doing unit-at-a-time) to something we havn't
   seen yet.  */

static bool
decide_is_function_needed (struct cgraph_node *node, tree decl)
{
  tree origin;
  if (MAIN_NAME_P (DECL_NAME (decl))
      && TREE_PUBLIC (decl))
    {
      node->local.externally_visible = true;
      return true;
    }

  /* If the user told us it is used, then it must be so.  */
  if (node->local.externally_visible)
    return true;

  if (!flag_unit_at_a_time && lookup_attribute ("used", DECL_ATTRIBUTES (decl)))
    return true;

  /* ??? If the assembler name is set by hand, it is possible to assemble
     the name later after finalizing the function and the fact is noticed
     in assemble_name then.  This is arguably a bug.  */
  if (DECL_ASSEMBLER_NAME_SET_P (decl)
      && TREE_SYMBOL_REFERENCED (DECL_ASSEMBLER_NAME (decl)))
    return true;

  /* With -fkeep-inline-functions we are keeping all inline functions except
     for extern inline ones.  */
  if (flag_keep_inline_functions
      && DECL_DECLARED_INLINE_P (decl)
      && !DECL_EXTERNAL (decl)
      && !lookup_attribute ("always_inline", DECL_ATTRIBUTES (decl)))
     return true;

  /* If we decided it was needed before, but at the time we didn't have
     the body of the function available, then it's still needed.  We have
     to go back and re-check its dependencies now.  */
  if (node->needed)
    return true;

  /* Externally visible functions must be output.  The exception is
     COMDAT functions that must be output only when they are needed.

     When not optimizing, also output the static functions. (see
     PR24561), but don't do so for always_inline functions, functions
     declared inline and nested functions.  These was optimized out
     in the original implementation and it is unclear whether we want
     to change the behavior here.  */
  if (((TREE_PUBLIC (decl)
	|| (!optimize && !node->local.disregard_inline_limits
	    && !DECL_DECLARED_INLINE_P (decl)
	    && !node->origin))
      && !flag_whole_program)
      && !DECL_COMDAT (decl) && !DECL_EXTERNAL (decl))
    return true;

  /* Constructors and destructors are reachable from the runtime by
     some mechanism.  */
  if (DECL_STATIC_CONSTRUCTOR (decl) || DECL_STATIC_DESTRUCTOR (decl))
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
  for (origin = decl_function_context (decl); origin;
       origin = decl_function_context (origin))
    if (DECL_EXTERNAL (origin))
      return false;
  /* We want to emit COMDAT functions only when absolutely necessary.  */
  if (DECL_COMDAT (decl))
    return false;
  if (!DECL_INLINE (decl)
      || (!node->local.disregard_inline_limits
	  /* When declared inline, defer even the uninlinable functions.
	     This allows them to be eliminated when unused.  */
	  && !DECL_DECLARED_INLINE_P (decl)
	  && (!node->local.inlinable || !cgraph_default_inline_p (node, NULL))))
    return true;

  return false;
}

/* Process CGRAPH_NEW_FUNCTIONS and perform actions necessary to add these
   functions into callgraph in a way so they look like ordinary reachable
   functions inserted into callgraph already at construction time.  */

bool
cgraph_process_new_functions (void)
{
  bool output = false;
  tree fndecl;
  struct cgraph_node *node;

  /*  Note that this queue may grow as its being processed, as the new
      functions may generate new ones.  */
  while (cgraph_new_nodes)
    {
      node = cgraph_new_nodes;
      fndecl = node->decl;
      cgraph_new_nodes = cgraph_new_nodes->next_needed;
      switch (cgraph_state)
	{
	case CGRAPH_STATE_CONSTRUCTION:
	  /* At construction time we just need to finalize function and move
	     it into reachable functions list.  */

	  node->next_needed = NULL;
	  node->needed = node->reachable = false;
	  cgraph_finalize_function (fndecl, false);
	  cgraph_mark_reachable_node (node);
	  output = true;
	  break;

	case CGRAPH_STATE_IPA:
	case CGRAPH_STATE_IPA_SSA:
	  /* When IPA optimization already started, do all essential
	     transformations that has been already performed on the whole
	     cgraph but not on this function.  */

	  tree_register_cfg_hooks ();
	  if (!node->analyzed)
	    cgraph_analyze_function (node);
	  push_cfun (DECL_STRUCT_FUNCTION (fndecl));
	  current_function_decl = fndecl;
	  node->local.inlinable = tree_inlinable_function_p (fndecl);
	  node->local.self_insns = estimate_num_insns (fndecl,
						       &eni_inlining_weights);
	  node->local.disregard_inline_limits
	    = lang_hooks.tree_inlining.disregard_inline_limits (fndecl);
	  /* Inlining characteristics are maintained by the
	     cgraph_mark_inline.  */
	  node->global.insns = node->local.self_insns;
	  if (flag_really_no_inline && !node->local.disregard_inline_limits)
	     node->local.inlinable = 0;
	  if ((cgraph_state == CGRAPH_STATE_IPA_SSA
	      && !gimple_in_ssa_p (DECL_STRUCT_FUNCTION (fndecl)))
	      /* When not optimizing, be sure we run early local passes anyway
		 to expand OMP.  */
	      || !optimize)
	    execute_pass_list (pass_early_local_passes.sub);
	  free_dominance_info (CDI_POST_DOMINATORS);
	  free_dominance_info (CDI_DOMINATORS);
	  pop_cfun ();
	  current_function_decl = NULL;
	  break;

	case CGRAPH_STATE_EXPANSION:
	  /* Functions created during expansion shall be compiled
	     directly.  */
	  node->output = 0;
	  cgraph_expand_function (node);
	  break;

	default:
	  gcc_unreachable ();
	  break;
	}
    }
  return output;
}

/* When not doing unit-at-a-time, output all functions enqueued.
   Return true when such a functions were found.  */

static bool
cgraph_assemble_pending_functions (void)
{
  bool output = false;

  if (flag_unit_at_a_time)
    return false;

  cgraph_output_pending_asms ();

  while (cgraph_nodes_queue)
    {
      struct cgraph_node *n = cgraph_nodes_queue;

      cgraph_nodes_queue = cgraph_nodes_queue->next_needed;
      n->next_needed = NULL;
      if (!n->global.inlined_to
	  && !n->alias
	  && !DECL_EXTERNAL (n->decl))
	{
	  cgraph_expand_function (n);
	  output = true;
	}
      output |= cgraph_process_new_functions ();
    }

  return output;
}


/* As an GCC extension we allow redefinition of the function.  The
   semantics when both copies of bodies differ is not well defined.
   We replace the old body with new body so in unit at a time mode
   we always use new body, while in normal mode we may end up with
   old body inlined into some functions and new body expanded and
   inlined in others.

   ??? It may make more sense to use one body for inlining and other
   body for expanding the function but this is difficult to do.  */

static void
cgraph_reset_node (struct cgraph_node *node)
{
  /* If node->output is set, then this is a unit-at-a-time compilation
     and we have already begun whole-unit analysis.  This is *not*
     testing for whether we've already emitted the function.  That
     case can be sort-of legitimately seen with real function
     redefinition errors.  I would argue that the front end should
     never present us with such a case, but don't enforce that for now.  */
  gcc_assert (!node->output);

  /* Reset our data structures so we can analyze the function again.  */
  memset (&node->local, 0, sizeof (node->local));
  memset (&node->global, 0, sizeof (node->global));
  memset (&node->rtl, 0, sizeof (node->rtl));
  node->analyzed = false;
  node->local.redefined_extern_inline = true;
  node->local.finalized = false;

  if (!flag_unit_at_a_time)
    {
      struct cgraph_node *n, *next;

      for (n = cgraph_nodes; n; n = next)
	{
	  next = n->next;
	  if (n->global.inlined_to == node)
	    cgraph_remove_node (n);
	}
    }

  cgraph_node_remove_callees (node);

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

static void
cgraph_lower_function (struct cgraph_node *node)
{
  if (node->lowered)
    return;
  tree_lowering_passes (node->decl);
  node->lowered = true;
}

/* DECL has been parsed.  Take it, queue it, compile it at the whim of the
   logic in effect.  If NESTED is true, then our caller cannot stand to have
   the garbage collector run at the moment.  We would need to either create
   a new GC context, or just not compile right now.  */

void
cgraph_finalize_function (tree decl, bool nested)
{
  struct cgraph_node *node = cgraph_node (decl);

  if (node->local.finalized)
    cgraph_reset_node (node);

  node->pid = cgraph_max_pid ++;
  notice_global_symbol (decl);
  node->decl = decl;
  node->local.finalized = true;
  node->lowered = DECL_STRUCT_FUNCTION (decl)->cfg != NULL;
  record_cdtor_fn (node->decl);
  if (node->nested)
    lower_nested_functions (decl);
  gcc_assert (!node->nested);

  /* If not unit at a time, then we need to create the call graph
     now, so that called functions can be queued and emitted now.  */
  if (!flag_unit_at_a_time)
    cgraph_analyze_function (node);

  if (decide_is_function_needed (node, decl))
    cgraph_mark_needed_node (node);

  /* Since we reclaim unreachable nodes at the end of every language
     level unit, we need to be conservative about possible entry points
     there.  */
  if ((TREE_PUBLIC (decl) && !DECL_COMDAT (decl) && !DECL_EXTERNAL (decl)))
    cgraph_mark_reachable_node (node);

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

/* Verify cgraph nodes of given cgraph node.  */
void
verify_cgraph_node (struct cgraph_node *node)
{
  struct cgraph_edge *e;
  struct cgraph_node *main_clone;
  struct function *this_cfun = DECL_STRUCT_FUNCTION (node->decl);
  basic_block this_block;
  block_stmt_iterator bsi;
  bool error_found = false;

  if (errorcount || sorrycount)
    return;

  timevar_push (TV_CGRAPH_VERIFY);
  for (e = node->callees; e; e = e->next_callee)
    if (e->aux)
      {
	error ("aux field set for edge %s->%s",
	       cgraph_node_name (e->caller), cgraph_node_name (e->callee));
	error_found = true;
      }
  if (node->count < 0)
    {
      error ("Execution count is negative");
      error_found = true;
    }
  for (e = node->callers; e; e = e->next_caller)
    {
      if (e->count < 0)
	{
	  error ("caller edge count is negative");
	  error_found = true;
	}
      if (e->frequency < 0)
	{
	  error ("caller edge frequency is negative");
	  error_found = true;
	}
      if (e->frequency > CGRAPH_FREQ_MAX)
	{
	  error ("caller edge frequency is too large");
	  error_found = true;
	}
      if (!e->inline_failed)
	{
	  if (node->global.inlined_to
	      != (e->caller->global.inlined_to
		  ? e->caller->global.inlined_to : e->caller))
	    {
	      error ("inlined_to pointer is wrong");
	      error_found = true;
	    }
	  if (node->callers->next_caller)
	    {
	      error ("multiple inline callers");
	      error_found = true;
	    }
	}
      else
	if (node->global.inlined_to)
	  {
	    error ("inlined_to pointer set for noninline callers");
	    error_found = true;
	  }
    }
  if (!node->callers && node->global.inlined_to)
    {
      error ("inlined_to pointer is set but no predecessors found");
      error_found = true;
    }
  if (node->global.inlined_to == node)
    {
      error ("inlined_to pointer refers to itself");
      error_found = true;
    }

  for (main_clone = cgraph_node (node->decl); main_clone;
       main_clone = main_clone->next_clone)
    if (main_clone == node)
      break;
  if (!cgraph_node (node->decl))
    {
      error ("node not found in cgraph_hash");
      error_found = true;
    }

  if (node->analyzed
      && DECL_SAVED_TREE (node->decl) && !TREE_ASM_WRITTEN (node->decl)
      && (!DECL_EXTERNAL (node->decl) || node->global.inlined_to))
    {
      if (this_cfun->cfg)
	{
	  /* The nodes we're interested in are never shared, so walk
	     the tree ignoring duplicates.  */
	  struct pointer_set_t *visited_nodes = pointer_set_create ();
	  /* Reach the trees by walking over the CFG, and note the
	     enclosing basic-blocks in the call edges.  */
	  FOR_EACH_BB_FN (this_block, this_cfun)
	    for (bsi = bsi_start (this_block); !bsi_end_p (bsi); bsi_next (&bsi))
	      {
		tree stmt = bsi_stmt (bsi);
		tree call = get_call_expr_in (stmt);
		tree decl;
		if (call && (decl = get_callee_fndecl (call)))
		  {
		    struct cgraph_edge *e = cgraph_edge (node, stmt);
		    if (e)
		      {
			if (e->aux)
			  {
			    error ("shared call_stmt:");
			    debug_generic_stmt (stmt);
			    error_found = true;
			  }
			if (e->callee->decl != cgraph_node (decl)->decl
			    && e->inline_failed)
			  {
			    error ("edge points to wrong declaration:");
			    debug_tree (e->callee->decl);
			    fprintf (stderr," Instead of:");
			    debug_tree (decl);
			  }
			e->aux = (void *)1;
		      }
		    else
		      {
			error ("missing callgraph edge for call stmt:");
			debug_generic_stmt (stmt);
			error_found = true;
		      }
		  }
	      }
	  pointer_set_destroy (visited_nodes);
	}
      else
	/* No CFG available?!  */
	gcc_unreachable ();

      for (e = node->callees; e; e = e->next_callee)
	{
	  if (!e->aux)
	    {
	      error ("edge %s->%s has no corresponding call_stmt",
		     cgraph_node_name (e->caller),
		     cgraph_node_name (e->callee));
	      debug_generic_stmt (e->call_stmt);
	      error_found = true;
	    }
	  e->aux = 0;
	}
    }
  if (error_found)
    {
      dump_cgraph_node (stderr, node);
      internal_error ("verify_cgraph_node failed");
    }
  timevar_pop (TV_CGRAPH_VERIFY);
}

/* Verify whole cgraph structure.  */
void
verify_cgraph (void)
{
  struct cgraph_node *node;

  if (sorrycount || errorcount)
    return;

  for (node = cgraph_nodes; node; node = node->next)
    verify_cgraph_node (node);
}

/* Output all asm statements we have stored up to be output.  */

static void
cgraph_output_pending_asms (void)
{
  struct cgraph_asm_node *can;

  if (errorcount || sorrycount)
    return;

  for (can = cgraph_asm_nodes; can; can = can->next)
    assemble_asm (can->asm_str);
  cgraph_asm_nodes = NULL;
}

/* Analyze the function scheduled to be output.  */
void
cgraph_analyze_function (struct cgraph_node *node)
{
  tree decl = node->decl;

  current_function_decl = decl;
  push_cfun (DECL_STRUCT_FUNCTION (decl));
  cgraph_lower_function (node);

  node->local.estimated_self_stack_size = estimated_stack_frame_size ();
  node->global.estimated_stack_size = node->local.estimated_self_stack_size;
  node->global.stack_frame_offset = 0;
  node->local.inlinable = tree_inlinable_function_p (decl);
  if (!flag_unit_at_a_time)
    node->local.self_insns = estimate_num_insns (decl, &eni_inlining_weights);
  if (node->local.inlinable)
    node->local.disregard_inline_limits
      = lang_hooks.tree_inlining.disregard_inline_limits (decl);
  if (flag_really_no_inline && !node->local.disregard_inline_limits)
    node->local.inlinable = 0;
  /* Inlining characteristics are maintained by the cgraph_mark_inline.  */
  node->global.insns = node->local.self_insns;
  if (!flag_unit_at_a_time)
    {
      bitmap_obstack_initialize (NULL);
      tree_register_cfg_hooks ();
      execute_pass_list (pass_early_local_passes.sub);
      free_dominance_info (CDI_POST_DOMINATORS);
      free_dominance_info (CDI_DOMINATORS);
      bitmap_obstack_release (NULL);
    }

  node->analyzed = true;
  pop_cfun ();
  current_function_decl = NULL;
}

/* Look for externally_visible and used attributes and mark cgraph nodes
   accordingly.

   We cannot mark the nodes at the point the attributes are processed (in
   handle_*_attribute) because the copy of the declarations available at that
   point may not be canonical.  For example, in:

    void f();
    void f() __attribute__((used));

   the declaration we see in handle_used_attribute will be the second
   declaration -- but the front end will subsequently merge that declaration
   with the original declaration and discard the second declaration.

   Furthermore, we can't mark these nodes in cgraph_finalize_function because:

    void f() {}
    void f() __attribute__((externally_visible));

   is valid.

   So, we walk the nodes at the end of the translation unit, applying the
   attributes at that point.  */

static void
process_function_and_variable_attributes (struct cgraph_node *first,
                                          struct varpool_node *first_var)
{
  struct cgraph_node *node;
  struct varpool_node *vnode;

  for (node = cgraph_nodes; node != first; node = node->next)
    {
      tree decl = node->decl;
      if (lookup_attribute ("used", DECL_ATTRIBUTES (decl)))
	{
	  mark_decl_referenced (decl);
	  if (node->local.finalized)
	     cgraph_mark_needed_node (node);
	}
      if (lookup_attribute ("externally_visible", DECL_ATTRIBUTES (decl)))
	{
	  if (! TREE_PUBLIC (node->decl))
	    warning (OPT_Wattributes,
		     "%J%<externally_visible%> attribute have effect only on public objects",
		     node->decl);
	  else
	    {
	      if (node->local.finalized)
		cgraph_mark_needed_node (node);
	      node->local.externally_visible = true;
	    }
	}
    }
  for (vnode = varpool_nodes; vnode != first_var; vnode = vnode->next)
    {
      tree decl = vnode->decl;
      if (lookup_attribute ("used", DECL_ATTRIBUTES (decl)))
	{
	  mark_decl_referenced (decl);
	  if (vnode->finalized)
	    varpool_mark_needed_node (vnode);
	}
      if (lookup_attribute ("externally_visible", DECL_ATTRIBUTES (decl)))
	{
	  if (! TREE_PUBLIC (vnode->decl))
	    warning (OPT_Wattributes,
		     "%J%<externally_visible%> attribute have effect only on public objects",
		     vnode->decl);
	  else
	    {
	      if (vnode->finalized)
		varpool_mark_needed_node (vnode);
	      vnode->externally_visible = true;
	    }
	}
    }
}

/* Process CGRAPH_NODES_NEEDED queue, analyze each function (and transitively
   each reachable functions) and build cgraph.
   The function can be called multiple times after inserting new nodes
   into beginning of queue.  Just the new part of queue is re-scanned then.  */

static void
cgraph_analyze_functions (void)
{
  /* Keep track of already processed nodes when called multiple times for
     intermodule optimization.  */
  static struct cgraph_node *first_analyzed;
  struct cgraph_node *first_processed = first_analyzed;
  static struct varpool_node *first_analyzed_var;
  struct cgraph_node *node, *next;

  process_function_and_variable_attributes (first_processed,
					    first_analyzed_var);
  first_processed = cgraph_nodes;
  first_analyzed_var = varpool_nodes;
  varpool_analyze_pending_decls ();
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Initial entry points:");
      for (node = cgraph_nodes; node != first_analyzed; node = node->next)
	if (node->needed && DECL_SAVED_TREE (node->decl))
	  fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
      fprintf (cgraph_dump_file, "\n");
    }
  cgraph_process_new_functions ();

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
	 weak alias attribute to kill its body. See
	 gcc.c-torture/compile/20011119-1.c  */
      if (!DECL_SAVED_TREE (decl))
	{
	  cgraph_reset_node (node);
	  continue;
	}

      gcc_assert (!node->analyzed && node->reachable);
      gcc_assert (DECL_SAVED_TREE (decl));

      cgraph_analyze_function (node);

      for (edge = node->callees; edge; edge = edge->next_callee)
	if (!edge->callee->reachable)
	  cgraph_mark_reachable_node (edge->callee);

      /* We finalize local static variables during constructing callgraph
         edges.  Process their attributes too.  */
      process_function_and_variable_attributes (first_processed,
						first_analyzed_var);
      first_processed = cgraph_nodes;
      first_analyzed_var = varpool_nodes;
      varpool_analyze_pending_decls ();
      cgraph_process_new_functions ();
    }

  /* Collect entry points to the unit.  */
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Unit entry points:");
      for (node = cgraph_nodes; node != first_analyzed; node = node->next)
	if (node->needed && DECL_SAVED_TREE (node->decl))
	  fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
      fprintf (cgraph_dump_file, "\n\nInitial ");
      dump_cgraph (cgraph_dump_file);
    }

  if (cgraph_dump_file)
    fprintf (cgraph_dump_file, "\nReclaiming functions:");

  for (node = cgraph_nodes; node != first_analyzed; node = next)
    {
      tree decl = node->decl;
      next = node->next;

      if (node->local.finalized && !DECL_SAVED_TREE (decl))
	cgraph_reset_node (node);

      if (!node->reachable && DECL_SAVED_TREE (decl))
	{
	  if (cgraph_dump_file)
	    fprintf (cgraph_dump_file, " %s", cgraph_node_name (node));
	  cgraph_remove_node (node);
	  continue;
	}
      else
	node->next_needed = NULL;
      gcc_assert (!node->local.finalized || DECL_SAVED_TREE (decl));
      gcc_assert (node->analyzed == node->local.finalized);
    }
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "\n\nReclaimed ");
      dump_cgraph (cgraph_dump_file);
    }
  first_analyzed = cgraph_nodes;
  ggc_collect ();
}

/* Analyze the whole compilation unit once it is parsed completely.  */

void
cgraph_finalize_compilation_unit (void)
{
  if (errorcount || sorrycount)
    return;

  finish_aliases_1 ();

  if (!flag_unit_at_a_time)
    {
      cgraph_output_pending_asms ();
      cgraph_assemble_pending_functions ();
      varpool_output_debug_info ();
      return;
    }

  if (!quiet_flag)
    {
      fprintf (stderr, "\nAnalyzing compilation unit\n");
      fflush (stderr);
    }

  timevar_push (TV_CGRAPH);
  cgraph_analyze_functions ();
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

      gcc_assert (!node->output);

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
      else
	{
	  /* We should've reclaimed all functions that are not needed.  */
#ifdef ENABLE_CHECKING
	  if (!node->global.inlined_to && DECL_SAVED_TREE (decl)
	      && !DECL_EXTERNAL (decl))
	    {
	      dump_cgraph_node (stderr, node);
	      internal_error ("failed to reclaim unneeded function");
	    }
#endif
	  gcc_assert (node->global.inlined_to || !DECL_SAVED_TREE (decl)
		      || DECL_EXTERNAL (decl));

	}

    }
}

/* Expand function specified by NODE.  */

static void
cgraph_expand_function (struct cgraph_node *node)
{
  enum debug_info_type save_write_symbols = NO_DEBUG;
  const struct gcc_debug_hooks *save_debug_hooks = NULL;
  tree decl = node->decl;

  /* We ought to not compile any inline clones.  */
  gcc_assert (!node->global.inlined_to);

  if (flag_unit_at_a_time)
    announce_function (decl);

  gcc_assert (node->lowered);

  if (DECL_IGNORED_P (decl))
    {
      save_write_symbols = write_symbols;
      write_symbols = NO_DEBUG;
      save_debug_hooks = debug_hooks;
      debug_hooks = &do_nothing_debug_hooks;
    }

  /* Generate RTL for the body of DECL.  */
  lang_hooks.callgraph.expand_function (decl);

  /* Make sure that BE didn't give up on compiling.  */
  /* ??? Can happen with nested function of extern inline.  */
  gcc_assert (TREE_ASM_WRITTEN (node->decl));

  if (DECL_IGNORED_P (decl))
    {
      write_symbols = save_write_symbols;
      debug_hooks = save_debug_hooks;
    }

  current_function_decl = NULL;
  if (!cgraph_preserve_function_body_p (node->decl))
    {
      cgraph_release_function_body (node);
      /* Eliminate all call edges.  This is important so the call_expr no longer
	 points to the dead function body.  */
      cgraph_node_remove_callees (node);
    }

  cgraph_function_flags_ready = true;
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
  struct cgraph_node **order = XCNEWVEC (struct cgraph_node *, cgraph_n_nodes);
  int order_pos = 0, new_order_pos = 0;
  int i;

  order_pos = cgraph_postorder (order);
  gcc_assert (order_pos == cgraph_n_nodes);

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
	  gcc_assert (node->reachable);
	  node->output = 0;
	  cgraph_expand_function (node);
	}
    }
  cgraph_process_new_functions ();

  free (order);

}

/* This is used to sort the node types by the cgraph order number.  */

struct cgraph_order_sort
{
  enum { ORDER_UNDEFINED = 0, ORDER_FUNCTION, ORDER_VAR, ORDER_ASM } kind;
  union
  {
    struct cgraph_node *f;
    struct varpool_node *v;
    struct cgraph_asm_node *a;
  } u;
};

/* Output all functions, variables, and asm statements in the order
   according to their order fields, which is the order in which they
   appeared in the file.  This implements -fno-toplevel-reorder.  In
   this mode we may output functions and variables which don't really
   need to be output.  */

static void
cgraph_output_in_order (void)
{
  int max;
  size_t size;
  struct cgraph_order_sort *nodes;
  int i;
  struct cgraph_node *pf;
  struct varpool_node *pv;
  struct cgraph_asm_node *pa;

  max = cgraph_order;
  size = max * sizeof (struct cgraph_order_sort);
  nodes = (struct cgraph_order_sort *) alloca (size);
  memset (nodes, 0, size);

  varpool_analyze_pending_decls ();

  for (pf = cgraph_nodes; pf; pf = pf->next)
    {
      if (pf->output)
	{
	  i = pf->order;
	  gcc_assert (nodes[i].kind == ORDER_UNDEFINED);
	  nodes[i].kind = ORDER_FUNCTION;
	  nodes[i].u.f = pf;
	}
    }

  for (pv = varpool_nodes_queue; pv; pv = pv->next_needed)
    {
      i = pv->order;
      gcc_assert (nodes[i].kind == ORDER_UNDEFINED);
      nodes[i].kind = ORDER_VAR;
      nodes[i].u.v = pv;
    }

  for (pa = cgraph_asm_nodes; pa; pa = pa->next)
    {
      i = pa->order;
      gcc_assert (nodes[i].kind == ORDER_UNDEFINED);
      nodes[i].kind = ORDER_ASM;
      nodes[i].u.a = pa;
    }

  for (i = 0; i < max; ++i)
    {
      switch (nodes[i].kind)
	{
	case ORDER_FUNCTION:
	  nodes[i].u.f->output = 0;
	  cgraph_expand_function (nodes[i].u.f);
	  break;

	case ORDER_VAR:
	  varpool_assemble_decl (nodes[i].u.v);
	  break;

	case ORDER_ASM:
	  assemble_asm (nodes[i].u.a->asm_str);
	  break;

	case ORDER_UNDEFINED:
	  break;

	default:
	  gcc_unreachable ();
	}
    }

  cgraph_asm_nodes = NULL;
}

/* Return true when function body of DECL still needs to be kept around
   for later re-use.  */
bool
cgraph_preserve_function_body_p (tree decl)
{
  struct cgraph_node *node;
  if (!cgraph_global_info_ready)
    return (flag_really_no_inline
	    ? lang_hooks.tree_inlining.disregard_inline_limits (decl)
	    : DECL_INLINE (decl));
  /* Look if there is any clone around.  */
  for (node = cgraph_node (decl); node; node = node->next_clone)
    if (node->global.inlined_to)
      return true;
  return false;
}

static void
ipa_passes (void)
{
  cfun = NULL;
  current_function_decl = NULL;
  tree_register_cfg_hooks ();
  bitmap_obstack_initialize (NULL);
  execute_ipa_pass_list (all_ipa_passes);
  bitmap_obstack_release (NULL);
}

/* Perform simple optimizations based on callgraph.  */

void
cgraph_optimize (void)
{
  if (errorcount || sorrycount)
    return;

#ifdef ENABLE_CHECKING
  verify_cgraph ();
#endif

  /* Call functions declared with the "constructor" or "destructor"
     attribute.  */
  cgraph_build_cdtor_fns ();
  if (!flag_unit_at_a_time)
    {
      cgraph_assemble_pending_functions ();
      cgraph_process_new_functions ();
      cgraph_state = CGRAPH_STATE_FINISHED;
      cgraph_output_pending_asms ();
      varpool_assemble_pending_decls ();
      varpool_output_debug_info ();
      return;
    }

  /* Frontend may output common variables after the unit has been finalized.
     It is safe to deal with them here as they are always zero initialized.  */
  varpool_analyze_pending_decls ();
  cgraph_analyze_functions ();

  timevar_push (TV_CGRAPHOPT);
  if (pre_ipa_mem_report)
    {
      fprintf (stderr, "Memory consumption before IPA\n");
      dump_memory_report (false);
    }
  if (!quiet_flag)
    fprintf (stderr, "Performing interprocedural optimizations\n");
  cgraph_state = CGRAPH_STATE_IPA;
    
  /* Don't run the IPA passes if there was any error or sorry messages.  */
  if (errorcount == 0 && sorrycount == 0)
    ipa_passes ();

  /* This pass remove bodies of extern inline functions we never inlined.
     Do this later so other IPA passes see what is really going on.  */
  cgraph_remove_unreachable_nodes (false, dump_file);
  cgraph_global_info_ready = true;
  if (cgraph_dump_file)
    {
      fprintf (cgraph_dump_file, "Optimized ");
      dump_cgraph (cgraph_dump_file);
      dump_varpool (cgraph_dump_file);
    }
  if (post_ipa_mem_report)
    {
      fprintf (stderr, "Memory consumption after IPA\n");
      dump_memory_report (false);
    }
  timevar_pop (TV_CGRAPHOPT);

  /* Output everything.  */
  if (!quiet_flag)
    fprintf (stderr, "Assembling functions:\n");
#ifdef ENABLE_CHECKING
  verify_cgraph ();
#endif

  cgraph_mark_functions_to_output ();

  cgraph_state = CGRAPH_STATE_EXPANSION;
  if (!flag_toplevel_reorder)
    cgraph_output_in_order ();
  else
    {
      cgraph_output_pending_asms ();

      cgraph_expand_all_functions ();
      varpool_remove_unreferenced_decls ();

      varpool_assemble_pending_decls ();
      varpool_output_debug_info ();
    }
  cgraph_process_new_functions ();
  cgraph_state = CGRAPH_STATE_FINISHED;

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
	internal_error ("nodes with no released memory found");
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
  name = get_file_function_name (which_buf);

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

  switch (which)
    {
    case 'I':
      DECL_STATIC_CONSTRUCTOR (decl) = 1;
      decl_init_priority_insert (decl, priority);
      break;
    case 'D':
      DECL_STATIC_DESTRUCTOR (decl) = 1;
      decl_fini_priority_insert (decl, priority);
      break;
    default:
      gcc_unreachable ();
    }

  gimplify_function_tree (decl);

  cgraph_add_new_function (decl, false);
  cgraph_mark_needed_node (cgraph_node (decl));
}

void
init_cgraph (void)
{
  cgraph_dump_file = dump_begin (TDI_cgraph, NULL);
}

/* The edges representing the callers of the NEW_VERSION node were
   fixed by cgraph_function_versioning (), now the call_expr in their
   respective tree code should be updated to call the NEW_VERSION.  */

static void
update_call_expr (struct cgraph_node *new_version)
{
  struct cgraph_edge *e;

  gcc_assert (new_version);
  for (e = new_version->callers; e; e = e->next_caller)
    /* Update the call expr on the edges
       to call the new version.  */
    TREE_OPERAND (CALL_EXPR_FN (get_call_expr_in (e->call_stmt)), 0) = new_version->decl;
}


/* Create a new cgraph node which is the new version of
   OLD_VERSION node.  REDIRECT_CALLERS holds the callers
   edges which should be redirected to point to
   NEW_VERSION.  ALL the callees edges of OLD_VERSION
   are cloned to the new version node.  Return the new
   version node.  */

static struct cgraph_node *
cgraph_copy_node_for_versioning (struct cgraph_node *old_version,
				 tree new_decl,
				 VEC(cgraph_edge_p,heap) *redirect_callers)
 {
   struct cgraph_node *new_version;
   struct cgraph_edge *e, *new_e;
   struct cgraph_edge *next_callee;
   unsigned i;

   gcc_assert (old_version);

   new_version = cgraph_node (new_decl);

   new_version->analyzed = true;
   new_version->local = old_version->local;
   new_version->global = old_version->global;
   new_version->rtl = new_version->rtl;
   new_version->reachable = true;
   new_version->count = old_version->count;

   /* Clone the old node callees.  Recursive calls are
      also cloned.  */
   for (e = old_version->callees;e; e=e->next_callee)
     {
       new_e = cgraph_clone_edge (e, new_version, e->call_stmt, 0, e->frequency,
				  e->loop_nest, true);
       new_e->count = e->count;
     }
   /* Fix recursive calls.
      If OLD_VERSION has a recursive call after the
      previous edge cloning, the new version will have an edge
      pointing to the old version, which is wrong;
      Redirect it to point to the new version. */
   for (e = new_version->callees ; e; e = next_callee)
     {
       next_callee = e->next_callee;
       if (e->callee == old_version)
	 cgraph_redirect_edge_callee (e, new_version);

       if (!next_callee)
	 break;
     }
   for (i = 0; VEC_iterate (cgraph_edge_p, redirect_callers, i, e); i++)
     {
       /* Redirect calls to the old version node to point to its new
	  version.  */
       cgraph_redirect_edge_callee (e, new_version);
     }

   return new_version;
 }

 /* Perform function versioning.
    Function versioning includes copying of the tree and
    a callgraph update (creating a new cgraph node and updating
    its callees and callers).

    REDIRECT_CALLERS varray includes the edges to be redirected
    to the new version.

    TREE_MAP is a mapping of tree nodes we want to replace with
    new ones (according to results of prior analysis).
    OLD_VERSION_NODE is the node that is versioned.
    It returns the new version's cgraph node.  */

struct cgraph_node *
cgraph_function_versioning (struct cgraph_node *old_version_node,
			    VEC(cgraph_edge_p,heap) *redirect_callers,
			    varray_type tree_map)
{
  tree old_decl = old_version_node->decl;
  struct cgraph_node *new_version_node = NULL;
  tree new_decl;

  if (!tree_versionable_function_p (old_decl))
    return NULL;

  /* Make a new FUNCTION_DECL tree node for the
     new version. */
  new_decl = copy_node (old_decl);

  /* Create the new version's call-graph node.
     and update the edges of the new node. */
  new_version_node =
    cgraph_copy_node_for_versioning (old_version_node, new_decl,
				     redirect_callers);

  /* Copy the OLD_VERSION_NODE function tree to the new version.  */
  tree_function_versioning (old_decl, new_decl, tree_map, false);
  /* Update the call_expr on the edges to call the new version node. */
  update_call_expr (new_version_node);

  /* Update the new version's properties.
     Make The new version visible only within this translation unit.
     ??? We cannot use COMDAT linkage because there is no
     ABI support for this.  */
  DECL_EXTERNAL (new_version_node->decl) = 0;
  DECL_ONE_ONLY (new_version_node->decl) = 0;
  TREE_PUBLIC (new_version_node->decl) = 0;
  DECL_COMDAT (new_version_node->decl) = 0;
  new_version_node->local.externally_visible = 0;
  new_version_node->local.local = 1;
  new_version_node->lowered = true;
  return new_version_node;
}

/* Produce separate function body for inline clones so the offline copy can be
   modified without affecting them.  */
struct cgraph_node *
save_inline_function_body (struct cgraph_node *node)
{
  struct cgraph_node *first_clone;

  gcc_assert (node == cgraph_node (node->decl));

  cgraph_lower_function (node);

  /* In non-unit-at-a-time we construct full fledged clone we never output to
     assembly file.  This clone is pointed out by inline_decl of original function
     and inlining infrastructure knows how to deal with this.  */
  if (!flag_unit_at_a_time)
    {
      struct cgraph_edge *e;

      first_clone = cgraph_clone_node (node, node->count, 0, CGRAPH_FREQ_BASE,
				       false);
      first_clone->needed = 0;
      first_clone->reachable = 1;
      /* Recursively clone all bodies.  */
      for (e = first_clone->callees; e; e = e->next_callee)
	if (!e->inline_failed)
	  cgraph_clone_inlined_nodes (e, true, false);
    }
  else
    first_clone = node->next_clone;

  first_clone->decl = copy_node (node->decl);
  node->next_clone = NULL;
  if (!flag_unit_at_a_time)
    node->inline_decl = first_clone->decl;
  first_clone->prev_clone = NULL;
  cgraph_insert_node_to_hashtable (first_clone);
  gcc_assert (first_clone == cgraph_node (first_clone->decl));

  /* Copy the OLD_VERSION_NODE function tree to the new version.  */
  tree_function_versioning (node->decl, first_clone->decl, NULL, true);

  DECL_EXTERNAL (first_clone->decl) = 0;
  DECL_ONE_ONLY (first_clone->decl) = 0;
  TREE_PUBLIC (first_clone->decl) = 0;
  DECL_COMDAT (first_clone->decl) = 0;

  for (node = first_clone->next_clone; node; node = node->next_clone)
    node->decl = first_clone->decl;
#ifdef ENABLE_CHECKING
  verify_cgraph_node (first_clone);
#endif
  return first_clone;
}

#include "gt-cgraphunit.h"
