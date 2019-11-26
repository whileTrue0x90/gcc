/* "Supergraph" classes that combine CFGs and callgraph into one digraph.
   Copyright (C) 2019 Free Software Foundation, Inc.
   Contributed by David Malcolm <dmalcolm@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "gcc-plugin.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tm.h"
#include "toplev.h"
#include "hash-table.h"
#include "vec.h"
#include "ggc.h"
#include "basic-block.h"
#include "gimple-fold.h"
#include "tree-eh.h"
#include "gimple-expr.h"
#include "is-a.h"
#include "timevar.h"
#include "gimple.h"
#include "gimple-iterator.h"
#include "gimple-pretty-print.h"
#include "tree-pretty-print.h"
#include "graphviz.h"
#include "cgraph.h"
#include "tree-dfa.h"
#include "analyzer/analyzer.h"
#include "analyzer/supergraph.h"
#include "analyzer/analyzer-logging.h"

/* Get the cgraph_edge, but only if there's an underlying function body.  */

cgraph_edge *
supergraph_call_edge (function *fun, gimple *stmt)
{
  gcall *call = dyn_cast<gcall *> (stmt);
  if (!call)
    return NULL;
  cgraph_edge *edge = cgraph_node::get (fun->decl)->get_edge (stmt);
  if (!edge)
    return NULL;
  if (!edge->callee)
    return NULL; /* e.g. for a function pointer.  */
  if (!edge->callee->get_fun ())
    return NULL;
  return edge;
}

/* supergraph's ctor.  Walk the callgraph, building supernodes for each
   CFG basic block, splitting the basic blocks at callsites.  Join
   together the supernodes with interprocedural and intraprocedural
   superedges as appropriate.  */

supergraph::supergraph (logger *logger)
{
  auto_client_timevar tv ("building supergraph");

  LOG_FUNC (logger);

  /* First pass: make supernodes.  */
  {
    /* Sort the cgraph_nodes?  */
    cgraph_node *node;
    FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node)
    {
      function *fun = node->get_fun ();

      /* Ensure that EDGE_DFS_BACK is correct for every CFG edge in
	 the supergraph (by doing it per-function).  */
      auto_cfun sentinel (fun);
      mark_dfs_back_edges ();

      const int start_idx = m_nodes.length ();

      basic_block bb;
      FOR_ALL_BB_FN (bb, fun)
	{
	  /* The initial supernode for the BB gets the phi nodes (if any).  */
	  supernode *node_for_stmts = add_node (fun, bb, NULL, phi_nodes (bb));
	  m_bb_to_initial_node.put (bb, node_for_stmts);
	  for (gphi_iterator gpi = gsi_start_phis (bb); !gsi_end_p (gpi);
	       gsi_next (&gpi))
	    {
	      gimple *stmt = gsi_stmt (gpi);
	      m_stmt_to_node_t.put (stmt, node_for_stmts);
	    }

	  /* Append statements from BB to the current supernode, splitting
	     them into a new supernode at each call site; such call statements
	     appear in both supernodes (representing call and return).  */
	  gimple_stmt_iterator gsi;
	  for (gsi = gsi_start_bb (bb); !gsi_end_p (gsi); gsi_next (&gsi))
	    {
	      gimple *stmt = gsi_stmt (gsi);
	      node_for_stmts->m_stmts.safe_push (stmt);
	      m_stmt_to_node_t.put (stmt, node_for_stmts);
	      if (cgraph_edge *edge = supergraph_call_edge (fun, stmt))
		{
		  m_cgraph_edge_to_caller_prev_node.put(edge, node_for_stmts);
		  node_for_stmts = add_node (fun, bb, as_a <gcall *> (stmt), NULL);
		  m_cgraph_edge_to_caller_next_node.put (edge, node_for_stmts);
		}
	    }

	  m_bb_to_final_node.put (bb, node_for_stmts);
	}

      const unsigned num_snodes = m_nodes.length () - start_idx;
      m_function_to_num_snodes.put (fun, num_snodes);

      if (logger)
	{
	  const int end_idx = m_nodes.length () - 1;
	  logger->log ("SN: %i...%i: function %qD",
		       start_idx, end_idx, fun->decl);
	}
    }
  }

  /* Second pass: make superedges.  */
  {
    /* Make superedges for CFG edges.  */
    for (bb_to_node_t::iterator iter = m_bb_to_final_node.begin ();
	 iter != m_bb_to_final_node.end ();
	 ++iter)
      {
	basic_block bb = (*iter).first;
	supernode *src_supernode = (*iter).second;

	::edge cfg_edge;
	int idx;
	if (bb->succs)
	  FOR_EACH_VEC_ELT (*bb->succs, idx, cfg_edge)
	    {
	      basic_block dest_cfg_block = cfg_edge->dest;
	      supernode *dest_supernode
		= *m_bb_to_initial_node.get (dest_cfg_block);
	      cfg_superedge *cfg_sedge
		= add_cfg_edge (src_supernode, dest_supernode, cfg_edge, idx);
	      m_cfg_edge_to_cfg_superedge.put (cfg_edge, cfg_sedge);
	    }
      }

    /* Make interprocedural superedges for calls.  */
    {
      for (cgraph_edge_to_node_t::iterator iter
	     = m_cgraph_edge_to_caller_prev_node.begin ();
	   iter != m_cgraph_edge_to_caller_prev_node.end ();
	   ++iter)
	{
	  cgraph_edge *edge = (*iter).first;
	  supernode *caller_prev_supernode = (*iter).second;
	  basic_block callee_cfg_block
	    = ENTRY_BLOCK_PTR_FOR_FN (edge->callee->get_fun ());
	  supernode *callee_supernode
	    = *m_bb_to_initial_node.get (callee_cfg_block);
	  call_superedge *sedge
	    = add_call_superedge (caller_prev_supernode,
				  callee_supernode,
				  edge);
	  m_cgraph_edge_to_call_superedge.put (edge, sedge);
	}
    }

    /* Make interprocedural superedges for returns.  */
    {
      for (cgraph_edge_to_node_t::iterator iter
	     = m_cgraph_edge_to_caller_next_node.begin ();
	   iter != m_cgraph_edge_to_caller_next_node.end ();
	   ++iter)
	{
	  cgraph_edge *edge = (*iter).first;
	  supernode *caller_next_supernode = (*iter).second;
	  basic_block callee_cfg_block
	    = EXIT_BLOCK_PTR_FOR_FN (edge->callee->get_fun ());
	  supernode *callee_supernode
	    = *m_bb_to_initial_node.get (callee_cfg_block);
	  return_superedge *sedge
	    = add_return_superedge (callee_supernode,
				    caller_next_supernode,
				    edge);
	  m_cgraph_edge_to_return_superedge.put (edge, sedge);
	}
    }

    /* Make intraprocedural superedges linking the two halves of a call.  */
    {
      for (cgraph_edge_to_node_t::iterator iter
	     = m_cgraph_edge_to_caller_prev_node.begin ();
	   iter != m_cgraph_edge_to_caller_prev_node.end ();
	   ++iter)
	{
	  cgraph_edge *edge = (*iter).first;
	  supernode *caller_prev_supernode = (*iter).second;
	  supernode *caller_next_supernode
	    = *m_cgraph_edge_to_caller_next_node.get (edge);
	  superedge *sedge
	    = new callgraph_superedge (caller_prev_supernode,
				       caller_next_supernode,
				       SUPEREDGE_INTRAPROCEDURAL_CALL,
				       edge);
	  add_edge (sedge);
	  m_cgraph_edge_to_intraproc_superedge.put (edge, sedge);
	}

    }
  }
}

/* Dump this graph in .dot format to PP, using DUMP_ARGS.
   Cluster the supernodes by function, then by BB from original CFG.  */

void
supergraph::dump_dot_to_pp (pretty_printer *pp,
			    const dump_args_t &dump_args) const
{
  graphviz_out gv (pp);

  pp_string (pp, "digraph \"");
  pp_write_text_to_stream (pp);
  pp_string (pp, "supergraph");
  pp_write_text_as_dot_label_to_stream (pp, /*for_record=*/false);
  pp_string (pp, "\" {\n");
  gv.indent ();

  gv.println ("overlap=false;");
  gv.println ("compound=true;");

  /* TODO: maybe (optionally) sub-subdivide by TU, for LTO; see also:
     https://gcc-python-plugin.readthedocs.io/en/latest/_images/sample-supergraph.png
  */

  /* Break out the supernodes into clusters by function.  */
  {
    cgraph_node *node;
    FOR_EACH_FUNCTION_WITH_GIMPLE_BODY (node)
    {
      function *fun = node->get_fun ();
      const char *funcname = function_name (fun);
      gv.println ("subgraph \"cluster_%s\" {",
		  funcname);
      gv.indent ();
      pp_printf (pp,
		 ("style=\"dashed\";"
		  " color=\"black\";"
		  " label=\"%s\";\n"),
		 funcname);

      /* Break out the nodes into clusters by BB from original CFG.  */
      {
	basic_block bb;
	FOR_ALL_BB_FN (bb, fun)
	  {
	    if (dump_args.m_flags & SUPERGRAPH_DOT_SHOW_BBS)
	      {
		gv.println ("subgraph \"cluster_%s_bb_%i\" {",
			    funcname, bb->index);
		gv.indent ();
		pp_printf (pp,
			   ("style=\"dashed\";"
			    " color=\"black\";"
			    " label=\"bb: %i\";\n"),
			   bb->index);
	      }

	    // TODO: maybe keep an index per-function/per-bb to speed this up???
	    int i;
	    supernode *n;
	    FOR_EACH_VEC_ELT (m_nodes, i, n)
	      if (n->m_fun == fun && n->m_bb == bb)
		n->dump_dot (&gv, dump_args);

	    if (dump_args.m_flags & SUPERGRAPH_DOT_SHOW_BBS)
	      {
		/* Terminate per-bb "subgraph" */
		gv.outdent ();
		gv.println ("}");
	      }
	  }
      }

      /* Add an invisible edge from ENTRY to EXIT, to improve the graph layout.  */
      pp_string (pp, "\t");
      get_node_for_function_entry (fun)->dump_dot_id (pp);
      pp_string (pp, ":s -> ");
      get_node_for_function_exit (fun)->dump_dot_id (pp);
      pp_string (pp, ":n [style=\"invis\",constraint=true];\n");

      /* Terminate per-function "subgraph" */
      gv.outdent ();
      gv.println ("}");
    }
  }

  /* Superedges.  */
  int i;
  superedge *e;
  FOR_EACH_VEC_ELT (m_edges, i, e)
    e->dump_dot (&gv, dump_args);

  /* Terminate "digraph" */
  gv.outdent ();
  gv.println ("}");
}

/* Dump this graph in .dot format to FP, using DUMP_ARGS.  */

void
supergraph::dump_dot_to_file (FILE *fp, const dump_args_t &dump_args) const
{
  pretty_printer *pp = global_dc->printer->clone ();
  pp_show_color (pp) = 0;
  /* %qE in logs for SSA_NAMEs should show the ssa names, rather than
     trying to prettify things by showing the underlying var.  */
  pp_format_decoder (pp) = default_tree_printer;

  pp->buffer->stream = fp;
  dump_dot_to_pp (pp, dump_args);
  pp_flush (pp);
  delete pp;
}

/* Dump this graph in .dot format to PATH, using DUMP_ARGS.  */

void
supergraph::dump_dot (const char *path, const dump_args_t &dump_args) const
{
  FILE *fp = fopen (path, "w");
  dump_dot_to_file (fp, dump_args);
  fclose (fp);
}

/* Create a supernode for BB within FUN and add it to this supergraph.

   If RETURNING_CALL is non-NULL, the supernode represents the resumption
   of the basic block after returning from that call.

   If PHI_NODES is non-NULL, this is the initial supernode for the basic
   block, and is responsible for any handling of the phi nodes.  */

supernode *
supergraph::add_node (function *fun, basic_block bb, gcall *returning_call,
		      gimple_seq phi_nodes)
{
  supernode *n = new supernode (fun, bb, returning_call, phi_nodes,
				m_nodes.length ());
  m_nodes.safe_push (n);
  return n;
}

/* Create a new cfg_superedge from SRC to DEST for the underlying CFG edge E,
   adding it to this supergraph.

   If the edge is for a switch statement, create a switch_cfg_superedge
   subclass using IDX (the index of E within the out-edges from SRC's
   underlying basic block).  */

cfg_superedge *
supergraph::add_cfg_edge (supernode *src, supernode *dest, ::edge e, int idx)
{
  /* Special-case switch edges.  */
  gimple *stmt = src->get_last_stmt ();
  cfg_superedge *new_edge;
  if (stmt && stmt->code == GIMPLE_SWITCH)
    new_edge = new switch_cfg_superedge (src, dest, e, idx);
  else
    new_edge = new cfg_superedge (src, dest, e);
  add_edge (new_edge);
  return new_edge;
}

/* Create and add a call_superedge representing an interprocedural call
   from SRC to DEST, using CEDGE.  */

call_superedge *
supergraph::add_call_superedge (supernode *src, supernode *dest,
				cgraph_edge *cedge)
{
  call_superedge *new_edge = new call_superedge (src, dest, cedge);
  add_edge (new_edge);
  return new_edge;
}

/* Create and add a return_superedge representing returning from an
   interprocedural call, returning from SRC to DEST, using CEDGE.  */

return_superedge *
supergraph::add_return_superedge (supernode *src, supernode *dest,
				  cgraph_edge *cedge)
{
  return_superedge *new_edge = new return_superedge (src, dest, cedge);
  add_edge (new_edge);
  return new_edge;
}

/* Implementation of dnode::dump_dot vfunc for supernodes.

   Write a cluster for the node, and within it a .dot node showing
   the phi nodes and stmts.  Call into any node annotator from ARGS to
   potentially add other records to the cluster.  */

void
supernode::dump_dot (graphviz_out *gv, const dump_args_t &args) const
{
  gv->println ("subgraph cluster_node_%i {",
	       m_index);
  gv->indent ();

  gv->println("style=\"solid\";");
  gv->println("color=\"black\";");
  gv->println("fillcolor=\"lightgrey\";");
  gv->println("label=\"sn: %i\";", m_index);

  pretty_printer *pp = gv->get_pp ();

  if (args.m_node_annotator)
    args.m_node_annotator->add_node_annotations (gv, *this);

  gv->write_indent ();
  dump_dot_id (pp);
  pp_printf (pp,
	     " [shape=none,margin=0,style=filled,fillcolor=%s,label=<",
	     "lightgrey");
  pp_string (pp, "<TABLE BORDER=\"0\">");
  pp_write_text_to_stream (pp);

  if (m_returning_call)
    {
      gv->begin_tr ();
      pp_string (pp, "returning call: ");
      gv->end_tr ();

      gv->begin_tr ();
      pp_gimple_stmt_1 (pp, m_returning_call, 0, (dump_flags_t)0);
      pp_write_text_as_html_like_dot_to_stream (pp);
      gv->end_tr ();

      if (args.m_node_annotator)
	args.m_node_annotator->add_stmt_annotations (gv, m_returning_call);
      pp_newline (pp);
    }

  if (entry_p ())
    {
      pp_string (pp, "<TR><TD>ENTRY</TD></TR>");
      pp_newline (pp);
    }

  if (return_p ())
    {
      pp_string (pp, "<TR><TD>EXIT</TD></TR>");
      pp_newline (pp);
    }

  /* Phi nodes.  */
  for (gphi_iterator gpi = const_cast<supernode *> (this)->start_phis ();
       !gsi_end_p (gpi); gsi_next (&gpi))
    {
      const gimple *stmt = gsi_stmt (gpi);
      gv->begin_tr ();
      pp_gimple_stmt_1 (pp, stmt, 0, (dump_flags_t)0);
      pp_write_text_as_html_like_dot_to_stream (pp);
      gv->end_tr ();

      if (args.m_node_annotator)
	args.m_node_annotator->add_stmt_annotations (gv, stmt);

      pp_newline (pp);
    }

  /* Statements.  */
  int i;
  gimple *stmt;
  FOR_EACH_VEC_ELT (m_stmts, i, stmt)
    {
      gv->begin_tr ();
      pp_gimple_stmt_1 (pp, stmt, 0, (dump_flags_t)0);
      pp_write_text_as_html_like_dot_to_stream (pp);
      gv->end_tr ();

      if (args.m_node_annotator)
	args.m_node_annotator->add_stmt_annotations (gv, stmt);

      pp_newline (pp);
    }

  pp_string (pp, "</TABLE>>];\n\n");
  pp_flush (pp);

  /* Terminate "subgraph" */
  gv->outdent ();
  gv->println ("}");
}

/* Write an ID for this node to PP, for use in .dot output.  */

void
supernode::dump_dot_id (pretty_printer *pp) const
{
  pp_printf (pp, "node_%i", m_index);
}

/* Get a location_t for the start of this supernode.  */

location_t
supernode::get_start_location () const
{
  if (m_returning_call && m_returning_call->location != UNKNOWN_LOCATION)
    return m_returning_call->location;

  int i;
  gimple *stmt;
  FOR_EACH_VEC_ELT (m_stmts, i, stmt)
    if (stmt->location != UNKNOWN_LOCATION)
      return stmt->location;

  if (entry_p ())
    {
      // TWEAK: show the decl instead; this leads to more readable output:
      return DECL_SOURCE_LOCATION (m_fun->decl);

      return m_fun->function_start_locus;
    }
  if (return_p ())
    return m_fun->function_end_locus;

  return UNKNOWN_LOCATION;
}

/* Get a location_t for the end of this supernode.  */

location_t
supernode::get_end_location () const
{
  int i;
  gimple *stmt;
  FOR_EACH_VEC_ELT_REVERSE (m_stmts, i, stmt)
    if (stmt->location != UNKNOWN_LOCATION)
      return stmt->location;

  if (m_returning_call && m_returning_call->location != UNKNOWN_LOCATION)
    return m_returning_call->location;

  if (entry_p ())
    return m_fun->function_start_locus;
  if (return_p ())
    return m_fun->function_end_locus;

  return UNKNOWN_LOCATION;
}

/* Given STMT within this supernode, return its index within m_stmts.  */

unsigned int
supernode::get_stmt_index (const gimple *stmt) const
{
  unsigned i;
  gimple *iter_stmt;
  FOR_EACH_VEC_ELT (m_stmts, i, iter_stmt)
    if (iter_stmt == stmt)
      return i;
  gcc_unreachable ();
}

/* Implementation of dedge::dump_dot for superedges.
   Write a .dot edge to GV representing this superedge.  */

void
superedge::dump_dot (graphviz_out *gv, const dump_args_t &) const
{
  const char *style = "\"solid,bold\"";
  const char *color = "black";
  int weight = 10;
  const char *constraint = "true";

  switch (m_kind)
    {
    default:
      gcc_unreachable ();
    case SUPEREDGE_CFG_EDGE:
      break;
    case SUPEREDGE_CALL:
      color = "red";
      break;
    case SUPEREDGE_RETURN:
      color = "green";
      break;
    case SUPEREDGE_INTRAPROCEDURAL_CALL:
      style = "\"dotted\"";
      break;
    }

  /* Adapted from graph.c:draw_cfg_node_succ_edges.  */
  if (::edge cfg_edge = get_any_cfg_edge ())
    {
      if (cfg_edge->flags & EDGE_FAKE)
	{
	  style = "dotted";
	  color = "green";
	  weight = 0;
	}
      else if (cfg_edge->flags & EDGE_DFS_BACK)
	{
	  style = "\"dotted,bold\"";
	  color = "blue";
	  weight = 10;
	}
      else if (cfg_edge->flags & EDGE_FALLTHRU)
	{
	  color = "blue";
	  weight = 100;
	}

      if (cfg_edge->flags & EDGE_ABNORMAL)
	color = "red";
    }

  gv->write_indent ();

  pretty_printer *pp = gv->get_pp ();

  m_src->dump_dot_id (pp);
  pp_string (pp, " -> ");
  m_dest->dump_dot_id (pp);
  pp_printf (pp,
	     (" [style=%s, color=%s, weight=%d, constraint=%s,"
	      " ltail=\"cluster_node_%i\", lhead=\"cluster_node_%i\""
	      " headlabel=\""),
	     style, color, weight, constraint,
	     m_src->m_index, m_dest->m_index);

  dump_label_to_pp (pp, false);

  pp_printf (pp, "\"];\n");
}

/* If this is an intraprocedural superedge, return the associated
   CFG edge.  Otherwise, return NULL.  */

::edge
superedge::get_any_cfg_edge () const
{
  if (const cfg_superedge *sub = dyn_cast_cfg_superedge ())
    return sub->get_cfg_edge ();
  return NULL;
}

/* If this is an interprocedural superedge, return the associated
   cgraph_edge *.  Otherwise, return NULL.  */

cgraph_edge *
superedge::get_any_callgraph_edge () const
{
  if (const callgraph_superedge *sub = dyn_cast_callgraph_superedge ())
    return sub->m_cedge;
  return NULL;
}

/* Build a description of this superedge (e.g. "true" for the true
   edge of a conditional, or "case 42:" for a switch case).

   The caller is responsible for freeing the result.

   If USER_FACING is false, the result also contains any underlying
   CFG edge flags. e.g. " (flags FALLTHRU | DFS_BACK)".  */

char *
superedge::get_description (bool user_facing) const
{
  pretty_printer pp;
  dump_label_to_pp (&pp, user_facing);
  return xstrdup (pp_formatted_text (&pp));
}

/* Implementation of superedge::dump_label_to_pp for non-switch CFG
   superedges.

   For true/false edges, print "true" or "false" to PP.

   If USER_FACING is false, also print flags on the underlying CFG edge to
   PP.  */

void
cfg_superedge::dump_label_to_pp (pretty_printer *pp,
				 bool user_facing) const
{
  if (true_value_p ())
    pp_printf (pp, "true");
  else if (false_value_p ())
    pp_printf (pp, "false");

  if (user_facing)
    return;

  /* Express edge flags as a string with " | " separator.
     e.g. " (flags FALLTHRU | DFS_BACK)".  */
  if (get_flags ())
    {
      pp_string (pp, " (flags ");
      bool seen_flag = false;
#define DEF_EDGE_FLAG(NAME,IDX)			\
  do {						\
    if (get_flags () & EDGE_##NAME)			\
      {						\
	if (seen_flag)				\
	  pp_string (pp, " | ");			\
	pp_printf (pp, "%s", (#NAME));		\
	seen_flag = true;			\
      }						\
  } while (0);
#include "cfg-flags.def"
#undef DEF_EDGE_FLAG
      pp_string (pp, ")");
    }

  /* Otherwise, no label.  */
}

/* Get the phi argument for PHI for this CFG edge.  */

tree
cfg_superedge::get_phi_arg (const gphi *phi) const
{
  size_t index = m_cfg_edge->dest_idx;
  return gimple_phi_arg_def (phi, index);
}

/* Implementation of superedge::dump_label_to_pp for CFG superedges for
   "switch" statements.

   Print "case VAL:", "case LOWER ... UPPER:", or "default:" to PP.  */

void
switch_cfg_superedge::dump_label_to_pp (pretty_printer *pp,
					bool user_facing ATTRIBUTE_UNUSED) const
{
  tree case_label = get_case_label ();
  gcc_assert (TREE_CODE (case_label) == CASE_LABEL_EXPR);
  tree lower_bound = CASE_LOW (case_label);
  tree upper_bound = CASE_HIGH (case_label);
  if (lower_bound)
    {
      pp_printf (pp, "case ");
      dump_generic_node (pp, lower_bound, 0, (dump_flags_t)0, false);
      if (upper_bound)
	{
	  pp_printf (pp, " ... ");
	  dump_generic_node (pp, upper_bound, 0, (dump_flags_t)0, false);
	}
      pp_printf (pp, ":");
    }
  else
    pp_printf (pp, "default:");
}

/* Get the case label for this "switch" superedge.  */

tree
switch_cfg_superedge::get_case_label () const
{
  return gimple_switch_label (get_switch_stmt (), m_idx);
}

/* Implementation of superedge::dump_label_to_pp for interprocedural
   superedges.  */

void
callgraph_superedge::dump_label_to_pp (pretty_printer *pp,
				       bool user_facing ATTRIBUTE_UNUSED) const
{
  switch (m_kind)
    {
    default:
    case SUPEREDGE_CFG_EDGE:
      gcc_unreachable ();

    case SUPEREDGE_CALL:
      pp_printf (pp, "call");
      break;

    case SUPEREDGE_RETURN:
      pp_printf (pp, "return");
      break;

    case SUPEREDGE_INTRAPROCEDURAL_CALL:
      pp_printf (pp, "intraproc link");
      break;
    }
}

/* Get the function that was called at this interprocedural call/return
   edge.  */

function *
callgraph_superedge::get_callee_function () const
{
  return m_cedge->callee->get_fun ();
}

/* Get the calling function at this interprocedural call/return edge.  */

function *
callgraph_superedge::get_caller_function () const
{
  return m_cedge->caller->get_fun ();
}

/* Get the fndecl that was called at this interprocedural call/return
   edge.  */

tree
callgraph_superedge::get_callee_decl () const
{
  return get_callee_function ()->decl;
}

/* Get the calling fndecl at this interprocedural call/return edge.  */

tree
callgraph_superedge::get_caller_decl () const
{
  return get_caller_function ()->decl;
}

/* Given PARM_TO_FIND, a PARM_DECL, identify its index (writing it
   to *OUT if OUT is non-NULL), and return the corresponding argument
   at the callsite.  */

tree
callgraph_superedge::get_arg_for_parm (tree parm_to_find,
				       callsite_expr *out) const
{
  gcc_assert  (TREE_CODE (parm_to_find) == PARM_DECL);

  tree callee = get_callee_decl ();

  int i = 0;
  for (tree iter_parm = DECL_ARGUMENTS (callee); iter_parm;
       iter_parm = DECL_CHAIN (iter_parm), ++i)
    {
      if (iter_parm == parm_to_find)
	{
	  if (out)
	    *out = callsite_expr::from_zero_based_param (i);
	  return gimple_call_arg (get_call_stmt (), i);
	}
    }

  /* Not found.  */
  return NULL_TREE;
}

/* Look for a use of ARG_TO_FIND as an argument at this callsite.
   If found, return the default SSA def of the corresponding parm within
   the callee, and if OUT is non-NULL, write the index to *OUT.
   Only the first match is handled.  */

tree
callgraph_superedge::get_parm_for_arg (tree arg_to_find,
				       callsite_expr *out) const
{
  tree callee = get_callee_decl ();

  int i = 0;
  for (tree iter_parm = DECL_ARGUMENTS (callee); iter_parm;
       iter_parm = DECL_CHAIN (iter_parm), ++i)
    {
      tree param = gimple_call_arg (get_call_stmt (), i);
      if (arg_to_find == param)
	{
	  if (out)
	    *out = callsite_expr::from_zero_based_param (i);
	  return ssa_default_def (get_callee_function (), iter_parm);
	}
    }

  /* Not found.  */
  return NULL_TREE;
}

/* Map caller_expr back to an expr within the callee, or return NULL_TREE.
   If non-NULL is returned, populate OUT.  */

tree
callgraph_superedge::map_expr_from_caller_to_callee (tree caller_expr,
						     callsite_expr *out) const
{
  /* Is it an argument (actual param)?  If so, convert to
     parameter (formal param).  */
  tree parm = get_parm_for_arg (caller_expr, out);
  if (parm)
    return parm;
  /* Otherwise try return value.  */
  if (caller_expr == gimple_call_lhs (get_call_stmt ()))
    {
      if (out)
	*out = callsite_expr::from_return_value ();
      return DECL_RESULT (get_callee_decl ());
    }

  return NULL_TREE;
}

/* Map callee_expr back to an expr within the caller, or return NULL_TREE.
   If non-NULL is returned, populate OUT.  */

tree
callgraph_superedge::map_expr_from_callee_to_caller (tree callee_expr,
						     callsite_expr *out) const
{
  if (callee_expr == NULL_TREE)
    return NULL_TREE;

  /* If it's a parameter (formal param), get the argument (actual param).  */
  if (TREE_CODE (callee_expr) == PARM_DECL)
    return get_arg_for_parm (callee_expr, out);

  /* Similar for the default SSA name of the PARM_DECL.  */
  if (TREE_CODE (callee_expr) == SSA_NAME
      && SSA_NAME_IS_DEFAULT_DEF (callee_expr)
      && TREE_CODE (SSA_NAME_VAR (callee_expr)) == PARM_DECL)
    return get_arg_for_parm (SSA_NAME_VAR (callee_expr), out);

  /* Otherwise try return value.  */
  if (callee_expr == DECL_RESULT (get_callee_decl ()))
    {
      if (out)
	*out = callsite_expr::from_return_value ();
      return gimple_call_lhs (get_call_stmt ());
    }

  return NULL_TREE;
}
