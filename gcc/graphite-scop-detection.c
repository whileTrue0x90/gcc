/* Detection of Static Control Parts (SCoP) for Graphite.
   Copyright (C) 2009 Free Software Foundation, Inc.
   Contributed by Sebastian Pop <sebastian.pop@amd.com> and
   Tobias Grosser <grosser@fim.uni-passau.de>.

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
#include "ggc.h"
#include "tree.h"
#include "rtl.h"
#include "basic-block.h"
#include "diagnostic.h"
#include "tree-flow.h"
#include "toplev.h"
#include "tree-dump.h"
#include "timevar.h"
#include "cfgloop.h"
#include "tree-chrec.h"
#include "tree-data-ref.h"
#include "tree-scalar-evolution.h"
#include "tree-pass.h"
#include "domwalk.h"
#include "value-prof.h"
#include "pointer-set.h"
#include "gimple.h"
#include "sese.h"

#ifdef HAVE_cloog
#include "cloog/cloog.h"
#include "ppl_c.h"
#include "graphite-ppl.h"
#include "graphite.h"
#include "graphite-poly.h"
#include "graphite-scop-detection.h"
#include "graphite-data-ref.h"

/* The type of the analyzed basic block.  */

typedef enum gbb_type {
  GBB_UNKNOWN,
  GBB_LOOP_SING_EXIT_HEADER,
  GBB_LOOP_MULT_EXIT_HEADER,
  GBB_LOOP_EXIT,
  GBB_COND_HEADER,
  GBB_SIMPLE,
  GBB_LAST
} gbb_type;

/* Detect the type of BB.  Loop headers are only marked, if they are
   new.  This means their loop_father is different to LAST_LOOP.
   Otherwise they are treated like any other bb and their type can be
   any other type.  */

static gbb_type
get_bb_type (basic_block bb, struct loop *last_loop)
{
  VEC (basic_block, heap) *dom;
  int nb_dom, nb_suc;
  struct loop *loop = bb->loop_father;

  /* Check, if we entry into a new loop. */
  if (loop != last_loop)
    {
      if (single_exit (loop) != NULL)
        return GBB_LOOP_SING_EXIT_HEADER;
      else if (loop->num != 0)
        return GBB_LOOP_MULT_EXIT_HEADER;
      else
	return GBB_COND_HEADER;
    }

  dom = get_dominated_by (CDI_DOMINATORS, bb);
  nb_dom = VEC_length (basic_block, dom);
  VEC_free (basic_block, heap, dom);

  if (nb_dom == 0)
    return GBB_LAST;

  nb_suc = VEC_length (edge, bb->succs);

  if (nb_dom == 1 && nb_suc == 1)
    return GBB_SIMPLE;

  return GBB_COND_HEADER;
}

/* A SCoP detection region, defined using bbs as borders. 
   All control flow touching this region, comes in passing basic_block ENTRY and
   leaves passing basic_block EXIT.  By using bbs instead of edges for the
   borders we are able to represent also regions that do not have a single
   entry or exit edge.
   But as they have a single entry basic_block and a single exit basic_block, we
   are able to generate for every sd_region a single entry and exit edge.

   1   2
    \ /
     3	<- entry
     |
     4
    / \			This region contains: {3, 4, 5, 6, 7, 8}
   5   6
   |   |
   7   8
    \ /
     9	<- exit  */


typedef struct sd_region_p
{
  /* The entry bb dominates all bbs in the sd_region.  It is part of the
     region.  */
  basic_block entry;

  /* The exit bb postdominates all bbs in the sd_region, but is not 
     part of the region.  */
  basic_block exit;
} sd_region;

DEF_VEC_O(sd_region);
DEF_VEC_ALLOC_O(sd_region, heap);


/* Moves the scops from SOURCE to TARGET and clean up SOURCE.  */

static void
move_sd_regions (VEC (sd_region, heap) **source, VEC (sd_region, heap) **target)
{
  sd_region *s;
  int i;

  for (i = 0; VEC_iterate (sd_region, *source, i, s); i++)
    VEC_safe_push (sd_region, heap, *target, s);
  
  VEC_free (sd_region, heap, *source);
}

/* Return true when EXPR can be represented in the polyhedral model.  

   This means an expression can be represented, if it is linear with
   respect to the loops and the strides are non parametric.  */

static bool
graphite_can_represent_scev (tree scev, int loop)
{
  tree zero; 

  if (chrec_contains_undetermined (scev))
    return false;

  if (evolution_function_is_invariant_p (scev, loop))
    return true;

  zero = fold_convert (chrec_type (scev), integer_zero_node);
  scev = chrec_replace_initial_condition (scev, zero);

  if (evolution_function_is_affine_multivariate_p (scev, loop)
      && !chrec_contains_symbols (scev))
    return true;

  return false;
}


/* Return true when EXPR can be represented in the polyhedral model.  

   This means an expression can be represented, if it is linear with
   respect to the loops and the strides are non parametric.  */

static bool
graphite_can_represent_expr (basic_block scop_entry, struct loop *loop,
			     tree expr)
{
  tree scev = analyze_scalar_evolution (loop, expr);

  scev = instantiate_scev (scop_entry, loop, scev);

  return graphite_can_represent_scev (scev, loop->num);
}

/* Return false if the tree_code of the operand OP or any of its operands
   is component_ref.  */

static bool
exclude_component_ref (tree op) 
{
  int i;
  int len;

  if (op)
    {
      if (TREE_CODE (op) == COMPONENT_REF)
	return false;
      else
	{
	  len = TREE_OPERAND_LENGTH (op);	  
	  for (i = 0; i < len; ++i)
	    {
	      if (!exclude_component_ref (TREE_OPERAND (op, i)))
		return false;
	    }
	}
    }

  return true;
}

/* Return true if we can create an affine data-ref for OP in STMT.  */

static bool
stmt_simple_memref_p (struct loop *loop, gimple stmt, tree op)
{
  data_reference_p dr;
  unsigned int i;
  VEC(tree,heap) *fns;
  tree t;
  bool res = true;

  dr = create_data_ref (loop, op, stmt, true);
  fns = DR_ACCESS_FNS (dr);

  for (i = 0; VEC_iterate (tree, fns, i, t); i++)
    if (!graphite_can_represent_scev (t, loop->num))
      {
	res = false;
	break;
      }

  free_data_ref (dr);
  return res;
}

/* Return true if the operand OP is simple.  */

static bool
is_simple_operand (loop_p loop, gimple stmt, tree op) 
{
  /* It is not a simple operand when it is a declaration,  */
  if (DECL_P (op)
      /* or a structure,  */
      || AGGREGATE_TYPE_P (TREE_TYPE (op))
      /* or a memory access that cannot be analyzed by the data
	 reference analysis.  */
      || ((handled_component_p (op) || INDIRECT_REF_P (op))
	  && !stmt_simple_memref_p (loop, stmt, op)))
    return false;

  return exclude_component_ref (op);
}

/* Return true only when STMT is simple enough for being handled by
   Graphite.  This depends on SCOP_ENTRY, as the parametetrs are
   initialized relatively to this basic block.  */

static bool
stmt_simple_for_scop_p (basic_block scop_entry, gimple stmt)
{
  basic_block bb = gimple_bb (stmt);
  struct loop *loop = bb->loop_father;

  /* GIMPLE_ASM and GIMPLE_CALL may embed arbitrary side effects.
     Calls have side-effects, except those to const or pure
     functions.  */
  if (gimple_has_volatile_ops (stmt)
      || (gimple_code (stmt) == GIMPLE_CALL
	  && !(gimple_call_flags (stmt) & (ECF_CONST | ECF_PURE)))
      || (gimple_code (stmt) == GIMPLE_ASM))
    return false;

  switch (gimple_code (stmt))
    {
    case GIMPLE_RETURN:
    case GIMPLE_LABEL:
      return true;

    case GIMPLE_COND:
      {
	tree op;
	ssa_op_iter op_iter;
        enum tree_code code = gimple_cond_code (stmt);

        /* We can only handle this kind of conditional expressions.  
           For inequalities like "if (i != 3 * k)" we need unions of
           polyhedrons.  Expressions like  "if (a)" or "if (a == 15)" need
           them for the else branch.  */
        if (!(code == LT_EXPR
	      || code == GT_EXPR
              || code == LE_EXPR
	      || code == GE_EXPR))
          return false;

	if (!scop_entry)
	  return false;

	FOR_EACH_SSA_TREE_OPERAND (op, stmt, op_iter, SSA_OP_ALL_USES)
	  if (!graphite_can_represent_expr (scop_entry, loop, op))
	    return false;

	return true;
      }

    case GIMPLE_ASSIGN:
      {
	enum tree_code code = gimple_assign_rhs_code (stmt);

	switch (get_gimple_rhs_class (code))
	  {
	  case GIMPLE_UNARY_RHS:
	  case GIMPLE_SINGLE_RHS:
	    return (is_simple_operand (loop, stmt, gimple_assign_lhs (stmt))
				       
		    && is_simple_operand (loop, stmt,
					  gimple_assign_rhs1 (stmt)));
					  

	  case GIMPLE_BINARY_RHS:
	    return (is_simple_operand (loop, stmt,
				       gimple_assign_lhs (stmt))
		    && is_simple_operand (loop, stmt,
					  gimple_assign_rhs1 (stmt))
		    && is_simple_operand (loop, stmt,
					  gimple_assign_rhs2 (stmt)));

	  case GIMPLE_INVALID_RHS:
	  default:
	    gcc_unreachable ();
	  }
      }

    case GIMPLE_CALL:
      {
	size_t i;
	size_t n = gimple_call_num_args (stmt);
	tree lhs = gimple_call_lhs (stmt);

	if (lhs && !is_simple_operand (loop, stmt, lhs))
	  return false;

	for (i = 0; i < n; i++)
	  if (!is_simple_operand (loop, stmt,
				  gimple_call_arg (stmt, i)))
	    return false;

	return true;
      }

    default:
      /* These nodes cut a new scope.  */
      return false;
    }

  return false;
}

/* Returns the statement of BB that contains a harmful operation: that
   can be a function call with side effects, the induction variables
   are not linear with respect to SCOP_ENTRY, etc.  The current open
   scop should end before this statement.  */

static gimple
harmful_stmt_in_bb (basic_block scop_entry, basic_block bb)
{
  gimple_stmt_iterator gsi;
  gimple stmt;

  for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi))
    if (!stmt_simple_for_scop_p (scop_entry, gsi_stmt (gsi)))
      return gsi_stmt (gsi);

  stmt = last_stmt (bb);
  if (stmt && gimple_code (stmt) == GIMPLE_COND)
    {
      tree lhs = gimple_cond_lhs (stmt);
      tree rhs = gimple_cond_rhs (stmt);

      if (TREE_CODE (TREE_TYPE (lhs)) == REAL_TYPE
	  || TREE_CODE (TREE_TYPE (rhs)) == REAL_TYPE)
	return stmt;
    }

  return NULL;
}

/* Returns the number of reduction phi nodes in LOOP.  */

int
nb_reductions_in_loop (loop_p loop)
{
  int res = 0;
  gimple_stmt_iterator gsi;

  for (gsi = gsi_start_phis (loop->header); !gsi_end_p (gsi); gsi_next (&gsi))
    {
      gimple phi = gsi_stmt (gsi);
      tree scev;
      affine_iv iv;

      if (!is_gimple_reg (PHI_RESULT (phi)))
	continue;

      scev = analyze_scalar_evolution (loop, PHI_RESULT (phi));
      scev = instantiate_parameters (loop, scev);
      if (!simple_iv (loop, phi, PHI_RESULT (phi), &iv, true))
	res++;
    }

  return res;
}

/* Return true when it is not possible to represent LOOP in the
   polyhedral representation.  */

static bool
graphite_cannot_represent_loop (basic_block scop_entry, loop_p loop)
{
  tree niter = number_of_latch_executions (loop);

  /* The upper bound of LOOP should be known.  */
  return (chrec_contains_undetermined (niter)

	  /* Upper bound should be linear.   */
	  || !graphite_can_represent_expr (scop_entry, loop, niter)

	  /* Code generation does not deal with scalar reductions.  */
	  || nb_reductions_in_loop (loop) > 0);
}

/* Store information needed by scopdet_* functions.  */

struct scopdet_info
{
  /* Where the last open scop would stop if the current BB is harmful.  */
  basic_block last;

  /* Where the next scop would start if the current BB is harmful.  */
  basic_block next;

  /* The bb or one of its children contains open loop exits.  That means
     loop exit nodes that are not surrounded by a loop dominated by bb.  */ 
  bool exits;

  /* The bb or one of its children contains only structures we can handle.  */ 
  bool difficult;
};

static struct scopdet_info build_scops_1 (basic_block, VEC (sd_region, heap) **,
                                          loop_p);

/* Calculates BB infos. If bb is difficult we add valid SCoPs dominated by BB
   to SCOPS.  TYPE is the gbb_type of BB.  */

static struct scopdet_info 
scopdet_basic_block_info (basic_block bb, VEC (sd_region, heap) **scops,
			  gbb_type type)
{
  struct loop *loop = bb->loop_father;
  struct scopdet_info result;
  gimple stmt;

  /* XXX: ENTRY_BLOCK_PTR could be optimized in later steps.  */
  basic_block entry_block = ENTRY_BLOCK_PTR;
  stmt = harmful_stmt_in_bb (entry_block, bb);
  result.difficult = (stmt != NULL);
  result.last = NULL;

  switch (type)
    {
    case GBB_LAST:
      result.next = NULL;
      result.exits = false;
      result.last = bb;

      /* Mark bbs terminating a SESE region difficult, if they start
	 a condition.  */
      if (VEC_length (edge, bb->succs) > 1)
	result.difficult = true; 

      break;

    case GBB_SIMPLE:
      result.next = single_succ (bb);
      result.exits = false;
      result.last = bb;
      break;

    case GBB_LOOP_SING_EXIT_HEADER:
      {
        VEC (sd_region, heap) *regions = VEC_alloc (sd_region, heap,3);
        struct scopdet_info sinfo;

        sinfo = build_scops_1 (bb, &regions, loop);
	
        result.last = single_exit (bb->loop_father)->src;
        result.next = single_exit (bb->loop_father)->dest;

        /* If we do not dominate result.next, remove it.  It's either
           the EXIT_BLOCK_PTR, or another bb dominates it and will
           call the scop detection for this bb.  */
        if (!dominated_by_p (CDI_DOMINATORS, result.next, bb))
	  result.next = NULL;

	if (result.last->loop_father != loop)
	  result.next = NULL;

        if (graphite_cannot_represent_loop (entry_block, loop))
          result.difficult = true;

        if (sinfo.difficult)
          move_sd_regions (&regions, scops);
        else 
          VEC_free (sd_region, heap, regions);

        result.exits = false;
        result.difficult |= sinfo.difficult;
        break;
      }

    case GBB_LOOP_MULT_EXIT_HEADER:
      {
        /* XXX: For now we just do not join loops with multiple exits.  If the 
           exits lead to the same bb it may be possible to join the loop.  */
        VEC (sd_region, heap) *regions = VEC_alloc (sd_region, heap, 3);
        VEC (edge, heap) *exits = get_loop_exit_edges (loop);
        edge e;
        int i;
        build_scops_1 (bb, &regions, loop);

	/* Scan the code dominated by this loop.  This means all bbs, that are
	   are dominated by a bb in this loop, but are not part of this loop.
	   
	   The easiest case:
	     - The loop exit destination is dominated by the exit sources.  
	 
	   TODO: We miss here the more complex cases:
		  - The exit destinations are dominated by another bb inside the
		    loop.
		  - The loop dominates bbs, that are not exit destinations.  */
        for (i = 0; VEC_iterate (edge, exits, i, e); i++)
          if (e->src->loop_father == loop
	      && dominated_by_p (CDI_DOMINATORS, e->dest, e->src))
	    {
	      /* Pass loop_outer to recognize e->dest as loop header in
		 build_scops_1.  */
	      if (e->dest->loop_father->header == e->dest)
		build_scops_1 (e->dest, &regions,
			       loop_outer (e->dest->loop_father));
	      else
		build_scops_1 (e->dest, &regions, e->dest->loop_father);
	    }

        result.next = NULL; 
        result.last = NULL;
        result.difficult = true;
        result.exits = false;
        move_sd_regions (&regions, scops);
        VEC_free (edge, heap, exits);
        break;
      }
    case GBB_COND_HEADER:
      {
	VEC (sd_region, heap) *regions = VEC_alloc (sd_region, heap, 3);
	struct scopdet_info sinfo;
	VEC (basic_block, heap) *dominated;
	int i;
	basic_block dom_bb;
	basic_block last_bb = NULL;
	edge e;
	result.exits = false;
 
	/* First check the successors of BB, and check if it is possible to join
	   the different branches.  */
	for (i = 0; VEC_iterate (edge, bb->succs, i, e); i++)
	  {
	    /* Ignore loop exits.  They will be handled after the loop body.  */
	    if (is_loop_exit (loop, e->dest))
	      {
		result.exits = true;
		continue;
	      }

	    /* Do not follow edges that lead to the end of the
	       conditions block.  For example, in

               |   0
	       |  /|\
	       | 1 2 |
	       | | | |
	       | 3 4 |
	       |  \|/
               |   6

	       the edge from 0 => 6.  Only check if all paths lead to
	       the same node 6.  */

	    if (!single_pred_p (e->dest))
	      {
		/* Check, if edge leads directly to the end of this
		   condition.  */
		if (!last_bb)
		  {
		    last_bb = e->dest;
		  }

		if (e->dest != last_bb)
		  result.difficult = true;

		continue;
	      }

	    if (!dominated_by_p (CDI_DOMINATORS, e->dest, bb))
	      {
		result.difficult = true;
		continue;
	      }

	    sinfo = build_scops_1 (e->dest, &regions, loop);

	    result.exits |= sinfo.exits;
	    result.last = sinfo.last;
	    result.difficult |= sinfo.difficult; 

	    /* Checks, if all branches end at the same point. 
	       If that is true, the condition stays joinable.
	       Have a look at the example above.  */
	    if (sinfo.last && single_succ_p (sinfo.last))
	      {
		basic_block next_tmp = single_succ (sinfo.last);
                  
		if (!last_bb)
		    last_bb = next_tmp;

		if (next_tmp != last_bb)
		  result.difficult = true;
	      }
	    else
	      result.difficult = true;
	  }

	/* If the condition is joinable.  */
	if (!result.exits && !result.difficult)
	  {
	    /* Only return a next pointer if we dominate this pointer.
	       Otherwise it will be handled by the bb dominating it.  */ 
	    if (dominated_by_p (CDI_DOMINATORS, last_bb, bb) && last_bb != bb)
	      result.next = last_bb;
	    else
	      result.next = NULL; 

	    VEC_free (sd_region, heap, regions);
	    break;
	  }

	/* Scan remaining bbs dominated by BB.  */
	dominated = get_dominated_by (CDI_DOMINATORS, bb);

	for (i = 0; VEC_iterate (basic_block, dominated, i, dom_bb); i++)
	  {
	    /* Ignore loop exits: they will be handled after the loop body.  */
	    if (loop_depth (find_common_loop (loop, dom_bb->loop_father))
		< loop_depth (loop))
	      {
		result.exits = true;
		continue;
	      }

	    /* Ignore the bbs processed above.  */
	    if (single_pred_p (dom_bb) && single_pred (dom_bb) == bb)
	      continue;

	    if (loop_depth (loop) > loop_depth (dom_bb->loop_father))
	      sinfo = build_scops_1 (dom_bb, &regions, loop_outer (loop));
	    else
	      sinfo = build_scops_1 (dom_bb, &regions, loop);
                                           
                                     
	    result.exits |= sinfo.exits; 
	    result.difficult = true;
	    result.last = NULL;
	  }

	VEC_free (basic_block, heap, dominated);

	result.next = NULL; 
	move_sd_regions (&regions, scops);

	break;
      }

    default:
      gcc_unreachable ();
    }

  return result;
}

/* Creates the SCoPs and writes entry and exit points for every SCoP.  */

static struct scopdet_info 
build_scops_1 (basic_block current, VEC (sd_region, heap) **scops, loop_p loop)
{
  bool in_scop = false;
  sd_region open_scop;
  struct scopdet_info sinfo;

  /* Initialize result.  */ 
  struct scopdet_info result;
  result.exits = false;
  result.difficult = false;
  result.next = NULL;
  result.last = NULL;
  open_scop.entry = NULL;
  open_scop.exit = NULL;
  sinfo.last = NULL;

  /* Loop over the dominance tree.  If we meet a difficult bb, close
     the current SCoP.  Loop and condition header start a new layer,
     and can only be added if all bbs in deeper layers are simple.  */
  while (current != NULL)
    {
      sinfo = scopdet_basic_block_info (current, scops,
					get_bb_type (current, loop));

      if (!in_scop && !(sinfo.exits || sinfo.difficult))
        {
	  open_scop.entry = current;
	  open_scop.exit = NULL;
          in_scop = true;
        }
      else if (in_scop && (sinfo.exits || sinfo.difficult))
        {
	  open_scop.exit = current;
          VEC_safe_push (sd_region, heap, *scops, &open_scop); 
          in_scop = false;
        }

      result.difficult |= sinfo.difficult;
      result.exits |= sinfo.exits;

      current = sinfo.next;
    }

  /* Try to close open_scop, if we are still in an open SCoP.  */
  if (in_scop)
    {
      int i;
      edge e;

	for (i = 0; VEC_iterate (edge, sinfo.last->succs, i, e); i++)
	  if (dominated_by_p (CDI_POST_DOMINATORS, sinfo.last, e->dest))
            open_scop.exit = e->dest;

        if (!open_scop.exit && open_scop.entry != sinfo.last)
	  open_scop.exit = sinfo.last;

	if (open_scop.exit)
	  VEC_safe_push (sd_region, heap, *scops, &open_scop);
      
    }

  result.last = sinfo.last;
  return result;
}

/* Checks if a bb is contained in REGION.  */

static bool
bb_in_sd_region (basic_block bb, sd_region *region)
{
  return dominated_by_p (CDI_DOMINATORS, bb, region->entry)
	 && !(dominated_by_p (CDI_DOMINATORS, bb, region->exit)
	      && !dominated_by_p (CDI_DOMINATORS, region->entry,
				  region->exit));
}

/* Returns the single entry edge of REGION, if it does not exits NULL.  */

static edge
find_single_entry_edge (sd_region *region)
{
  edge e;
  edge_iterator ei;
  edge entry = NULL;

  FOR_EACH_EDGE (e, ei, region->entry->preds)
    if (!bb_in_sd_region (e->src, region))
      {
	if (entry)
	  {
	    entry = NULL;
	    break;
	  }

	else
	  entry = e;
      }

  return entry;
}

/* Returns the single exit edge of REGION, if it does not exits NULL.  */

static edge
find_single_exit_edge (sd_region *region)
{
  edge e;
  edge_iterator ei;
  edge exit = NULL;

  FOR_EACH_EDGE (e, ei, region->exit->preds)
    if (bb_in_sd_region (e->src, region))
      {
	if (exit)
	  {
	    exit = NULL;
	    break;
	  }

	else
	  exit = e;
      }

  return exit;
}

/* Create a single entry edge for REGION.  */

static void
create_single_entry_edge (sd_region *region)
{
  if (find_single_entry_edge (region))
    return;

  /* There are multiple predecessors for bb_3 

  |  1  2
  |  | /
  |  |/
  |  3	<- entry
  |  |\
  |  | |
  |  4 ^
  |  | |
  |  |/
  |  5

  There are two edges (1->3, 2->3), that point from outside into the region,
  and another one (5->3), a loop latch, lead to bb_3.

  We split bb_3.
  
  |  1  2
  |  | /
  |  |/
  |3.0
  |  |\     (3.0 -> 3.1) = single entry edge
  |3.1 |  	<- entry
  |  | |
  |  | |
  |  4 ^ 
  |  | |
  |  |/
  |  5

  If the loop is part of the SCoP, we have to redirect the loop latches.

  |  1  2
  |  | /
  |  |/
  |3.0
  |  |      (3.0 -> 3.1) = entry edge
  |3.1  	<- entry
  |  |\
  |  | |
  |  4 ^
  |  | |
  |  |/
  |  5  */

  if (region->entry->loop_father->header != region->entry
      || dominated_by_p (CDI_DOMINATORS,
			 loop_latch_edge (region->entry->loop_father)->src,
			 region->exit))
    {
      edge forwarder = split_block_after_labels (region->entry);
      region->entry = forwarder->dest;
    }
  else
    /* This case is never executed, as the loop headers seem always to have a
       single edge pointing from outside into the loop.  */
    gcc_unreachable ();
      
#ifdef ENABLE_CHECKING
  gcc_assert (find_single_entry_edge (region));
#endif
}

/* Check if the sd_region, mentioned in EDGE, has no exit bb.  */

static bool
sd_region_without_exit (edge e)
{
  sd_region *r = (sd_region *) e->aux;

  if (r)
    return r->exit == NULL;
  else
    return false;
}

/* Create a single exit edge for REGION.  */

static void
create_single_exit_edge (sd_region *region)
{
  edge e;
  edge_iterator ei;
  edge forwarder = NULL;
  basic_block exit;
  
  if (find_single_exit_edge (region))
    return;

  /* We create a forwarder bb (5) for all edges leaving this region
     (3->5, 4->5).  All other edges leading to the same bb, are moved
     to a new bb (6).  If these edges where part of another region (2->5)
     we update the region->exit pointer, of this region.

     To identify which edge belongs to which region we depend on the e->aux
     pointer in every edge.  It points to the region of the edge or to NULL,
     if the edge is not part of any region.

     1 2 3 4   	1->5 no region, 		2->5 region->exit = 5,
      \| |/    	3->5 region->exit = NULL, 	4->5 region->exit = NULL
        5	<- exit

     changes to

     1 2 3 4   	1->6 no region, 			2->6 region->exit = 6,
     | | \/	3->5 no region,				4->5 no region, 
     | |  5
      \| /	5->6 region->exit = 6
	6 

     Now there is only a single exit edge (5->6).  */
  exit = region->exit;
  region->exit = NULL;
  forwarder = make_forwarder_block (exit, &sd_region_without_exit, NULL);
  
  /* Unmark the edges, that are no longer exit edges.  */
  FOR_EACH_EDGE (e, ei, forwarder->src->preds)
    if (e->aux)
      e->aux = NULL;

  /* Mark the new exit edge.  */ 
  single_succ_edge (forwarder->src)->aux = region;

  /* Update the exit bb of all regions, where exit edges lead to
     forwarder->dest.  */
  FOR_EACH_EDGE (e, ei, forwarder->dest->preds)
    if (e->aux)
      ((sd_region *) e->aux)->exit = forwarder->dest;

#ifdef ENABLE_CHECKING
  gcc_assert (find_single_exit_edge (region));
#endif
}

/* Unmark the exit edges of all REGIONS.  
   See comment in "create_single_exit_edge". */

static void
unmark_exit_edges (VEC (sd_region, heap) *regions)
{
  int i;
  sd_region *s;
  edge e;
  edge_iterator ei;

  for (i = 0; VEC_iterate (sd_region, regions, i, s); i++)
    FOR_EACH_EDGE (e, ei, s->exit->preds)
      e->aux = NULL;
}


/* Mark the exit edges of all REGIONS.  
   See comment in "create_single_exit_edge". */

static void
mark_exit_edges (VEC (sd_region, heap) *regions)
{
  int i;
  sd_region *s;
  edge e;
  edge_iterator ei;

  for (i = 0; VEC_iterate (sd_region, regions, i, s); i++)
    FOR_EACH_EDGE (e, ei, s->exit->preds)
      if (bb_in_sd_region (e->src, s))
	e->aux = s;
}

/* Create for all scop regions a single entry and a single exit edge.  */

static void
create_sese_edges (VEC (sd_region, heap) *regions)
{
  int i;
  sd_region *s;

  for (i = 0; VEC_iterate (sd_region, regions, i, s); i++)
    create_single_entry_edge (s);

  mark_exit_edges (regions);

  for (i = 0; VEC_iterate (sd_region, regions, i, s); i++)
    create_single_exit_edge (s);

  unmark_exit_edges (regions);

  fix_loop_structure (NULL);

#ifdef ENABLE_CHECKING
  verify_loop_structure ();
  verify_dominators (CDI_DOMINATORS);
  verify_ssa (false);
#endif
}

/* Create graphite SCoPs from an array of scop detection REGIONS.  */

static void
build_graphite_scops (VEC (sd_region, heap) *regions,
		      VEC (scop_p, heap) **scops)
{
  int i;
  sd_region *s;

  for (i = 0; VEC_iterate (sd_region, regions, i, s); i++)
    {
      edge entry = find_single_entry_edge (s); 
      edge exit = find_single_exit_edge (s);
      scop_p scop = new_scop (new_sese (entry, exit));
      VEC_safe_push (scop_p, heap, *scops, scop);

      /* Are there overlapping SCoPs?  */
#ifdef ENABLE_CHECKING
	{
	  int j;
	  sd_region *s2;

	  for (j = 0; VEC_iterate (sd_region, regions, j, s2); j++)
	    if (s != s2)
	      gcc_assert (!bb_in_sd_region (s->entry, s2));
	}
#endif
    }
}

/* We limit all SCoPs to SCoPs, that are completely surrounded by a loop. 

   Example:

   for (i      |
     {         |
       for (j  |  SCoP 1
       for (k  |
     }         |

   * SCoP frontier, as this line is not surrounded by any loop. *

   for (l      |  SCoP 2

   This is necessary as scalar evolution and parameter detection need a
   outermost loop to initialize parameters correctly.  
  
   TODO: FIX scalar evolution and parameter detection to allow more flexible
         SCoP frontiers.  */

static void
limit_scops (VEC (scop_p, heap) **scops)
{
  VEC (sd_region, heap) *regions = VEC_alloc (sd_region, heap, 3);

  int i;
  scop_p scop;

  for (i = 0; VEC_iterate (scop_p, *scops, i, scop); i++)
    {
      int j;
      loop_p loop;
      sese region = SCOP_REGION (scop);
      build_scop_bbs (scop);
      build_sese_loop_nests (region);

      for (j = 0; VEC_iterate (loop_p, SESE_LOOP_NEST (region), j, loop); j++) 
        if (!loop_in_sese_p (loop_outer (loop), region))
          {
	    sd_region open_scop;
	    open_scop.entry = loop->header;
	    open_scop.exit = single_exit (loop)->dest;
	    VEC_safe_push (sd_region, heap, regions, &open_scop);
	  }
    }

  free_scops (*scops);
  *scops = VEC_alloc (scop_p, heap, 3);

  create_sese_edges (regions);
  build_graphite_scops (regions, scops);
  VEC_free (sd_region, heap, regions);
}

/* Find Static Control Parts (SCoP) in the current function and pushes
   them to SCOPS.  */

void
build_scops (VEC (scop_p, heap) **scops)
{
  struct loop *loop = current_loops->tree_root;
  VEC (sd_region, heap) *regions = VEC_alloc (sd_region, heap, 3);

  build_scops_1 (single_succ (ENTRY_BLOCK_PTR), &regions, loop);
  create_sese_edges (regions);
  build_graphite_scops (regions, scops);
  limit_scops (scops);
  VEC_free (sd_region, heap, regions);

  if (dump_file && (dump_flags & TDF_DETAILS))
    fprintf (dump_file, "\nnumber of SCoPs: %d\n", VEC_length (scop_p, *scops));
}

/* Pretty print all SCoPs in DOT format and mark them with different colors.
   If there are not enough colors, paint later SCoPs gray.
   Special nodes:
   - "*" after the node number: entry of a SCoP,
   - "#" after the node number: exit of a SCoP,
   - "()" entry or exit not part of SCoP.  */

static void
dot_all_scops_1 (FILE *file, VEC (scop_p, heap) *scops)
{
  basic_block bb;
  edge e;
  edge_iterator ei;
  scop_p scop;
  const char* color;
  int i;

  /* Disable debugging while printing graph.  */
  int tmp_dump_flags = dump_flags;
  dump_flags = 0;

  fprintf (file, "digraph all {\n");

  FOR_ALL_BB (bb)
    {
      int part_of_scop = false;

      /* Use HTML for every bb label.  So we are able to print bbs
         which are part of two different SCoPs, with two different
         background colors.  */
      fprintf (file, "%d [label=<\n  <TABLE BORDER=\"0\" CELLBORDER=\"1\" ",
                     bb->index);
      fprintf (file, "CELLSPACING=\"0\">\n");

      /* Select color for SCoP.  */
      for (i = 0; VEC_iterate (scop_p, scops, i, scop); i++)
	{
	  sese region = SCOP_REGION (scop);
	  if (bb_in_sese_p (bb, region)
	      || (SESE_EXIT_BB (region) == bb)
	      || (SESE_ENTRY_BB (region) == bb))
	    {
	      switch (i % 17)
		{
		case 0: /* red */
		  color = "#e41a1c";
		  break;
		case 1: /* blue */
		  color = "#377eb8";
		  break;
		case 2: /* green */
		  color = "#4daf4a";
		  break;
		case 3: /* purple */
		  color = "#984ea3";
		  break;
		case 4: /* orange */
		  color = "#ff7f00";
		  break;
		case 5: /* yellow */
		  color = "#ffff33";
		  break;
		case 6: /* brown */
		  color = "#a65628";
		  break;
		case 7: /* rose */
		  color = "#f781bf";
		  break;
		case 8:
		  color = "#8dd3c7";
		  break;
		case 9:
		  color = "#ffffb3";
		  break;
		case 10:
		  color = "#bebada";
		  break;
		case 11:
		  color = "#fb8072";
		  break;
		case 12:
		  color = "#80b1d3";
		  break;
		case 13:
		  color = "#fdb462";
		  break;
		case 14:
		  color = "#b3de69";
		  break;
		case 15:
		  color = "#fccde5";
		  break;
		case 16:
		  color = "#bc80bd";
		  break;
		default: /* gray */
		  color = "#999999";
		}

	      fprintf (file, "    <TR><TD WIDTH=\"50\" BGCOLOR=\"%s\">", color);

	      if (!bb_in_sese_p (bb, region))
		fprintf (file, " ("); 

	      if (bb == SESE_ENTRY_BB (region)
		  && bb == SESE_EXIT_BB (region))
		fprintf (file, " %d*# ", bb->index);
	      else if (bb == SESE_ENTRY_BB (region))
		fprintf (file, " %d* ", bb->index);
	      else if (bb == SESE_EXIT_BB (region))
		fprintf (file, " %d# ", bb->index);
	      else
		fprintf (file, " %d ", bb->index);

	      if (!bb_in_sese_p (bb,region)) 
		fprintf (file, ")");

	      fprintf (file, "</TD></TR>\n");
	      part_of_scop  = true;
	    }

	  if (!part_of_scop)
	    {
	      fprintf (file, "    <TR><TD WIDTH=\"50\" BGCOLOR=\"#ffffff\">");
	      fprintf (file, " %d </TD></TR>\n", bb->index);
	    }
	

	  fprintf (file, "  </TABLE>>, shape=box, style=\"setlinewidth(0)\"]\n");
	}
    }

  FOR_ALL_BB (bb)
    {
      FOR_EACH_EDGE (e, ei, bb->succs)
	      fprintf (file, "%d -> %d;\n", bb->index, e->dest->index);
    }

  fputs ("}\n\n", file);

  /* Enable debugging again.  */
  dump_flags = tmp_dump_flags;
}

/* Display all SCoPs using dotty.  */

void
dot_all_scops (VEC (scop_p, heap) *scops)
{
  /* When debugging, enable the following code.  This cannot be used
     in production compilers because it calls "system".  */
#if 1
  FILE *stream = fopen ("/tmp/allscops.dot", "w");
  gcc_assert (stream);

  dot_all_scops_1 (stream, scops);
  fclose (stream);

  system ("dotty /tmp/allscops.dot");
#else
  dot_all_scops_1 (stderr, scops);
#endif
}

#endif
