/* Tree lowering pass.  This pass simplifies the tree representation built
   by the C-based front ends.  The structure of simplified, or
   language-independent, trees is dictated by the grammar described in this
   file.
   Copyright (C) 2002 Free Software Foundation, Inc.
   Lowering of expressions contributed by Sebastian Pop <s.pop@laposte.net>
   Re-written to support lowering of whole function trees, documentation
   and miscellaneous cleanups by Diego Novillo <dnovillo@redhat.com>

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

#include "config.h"
#include "system.h"
#include "tree.h"
#include "errors.h"
#include "varray.h"
#include "c-tree.h"
#include "c-common.h"
#include "tree-simple.h"
#include "tree-inline.h"
#include "diagnostic.h"

/** The simplification pass converts the language-dependent trees
    (ld-trees) emitted by the parser into language-independent trees
    (li-trees) that are the target of SSA analysis and transformations.  

    Language-independent trees are based on the SIMPLE intermediate
    representation used in the McCAT compiler framework:

    "Designing the McCAT Compiler Based on a Family of Structured
    Intermediate Representations,"
    L. Hendren, C. Donawa, M. Emami, G. Gao, Justiani, and B. Sridharan,
    Proceedings of the 5th International Workshop on Languages and
    Compilers for Parallel Computing, no. 757 in Lecture Notes in
    Computer Science, New Haven, Connecticut, pp. 406-420,
    Springer-Verlag, August 3-5, 1992.

    http://www-acaps.cs.mcgill.ca/info/McCAT/McCAT.html   */

/* {{{ Local declarations.  */

static void simplify_stmt            PARAMS ((tree));
static void simplify_for_stmt        PARAMS ((tree, tree *, tree *));
static void simplify_while_stmt      PARAMS ((tree, tree *));
static void simplify_do_stmt         PARAMS ((tree));
static void simplify_if_stmt         PARAMS ((tree, tree *));
static void simplify_switch_stmt     PARAMS ((tree, tree *));
static void simplify_return_stmt     PARAMS ((tree, tree *));
static void simplify_expr            PARAMS ((tree *, tree *, tree *,
                                              int (*) PARAMS ((tree)), tree));
static void simplify_array_ref       PARAMS ((tree *, tree *, tree *, tree));
static void simplify_self_mod_expr   PARAMS ((tree *, tree *, tree *, tree));
static void simplify_component_ref   PARAMS ((tree *, tree *, tree *, tree));
static void simplify_call_expr       PARAMS ((tree *, tree *, tree *, tree));
static void simplify_tree_list       PARAMS ((tree *, tree *, tree *, tree));
static void simplify_cond_expr       PARAMS ((tree *, tree *, tree));
static void simplify_modify_expr     PARAMS ((tree *, tree *, tree *, tree));
static void simplify_boolean_expr    PARAMS ((tree *, tree *, tree));
static void simplify_compound_expr   PARAMS ((tree *, tree *, tree *, tree));
static void simplify_expr_wfl        PARAMS ((tree *, tree *, tree *,
                                              int (*) PARAMS ((tree)), tree));
static void simplify_lvalue_expr     PARAMS ((tree *, tree *, tree *, tree));
static void make_type_writable       PARAMS ((tree));
static tree add_tree                 PARAMS ((tree, tree *));
static tree insert_before_continue   PARAMS ((tree, tree));
static void insert_before_first      PARAMS ((tree, tree));
static tree tree_last_decl           PARAMS ((tree));
static tree convert_to_stmt_chain    PARAMS ((tree, tree));
static int  stmt_has_effect          PARAMS ((tree));
static int  expr_has_effect          PARAMS ((tree));
static tree mostly_copy_tree_r       PARAMS ((tree *, int *, void *));

/* Local variables.  */
static FILE *dump_file;
static int dump_flags;

/* Used to keep track of statement expressions.  Incremented each time we
   start processing a statement expression.  When simplifying statement
   expressions, we need to keep some statements with no effect because they
   might represent the return value of the statement expression.  */
static int stmt_expr_level;

/* }}} */


/* Simplification of statement trees.  */

/** {{{ c_simplify_function_tree ()

    Entry point to the simplification pass.  FNDECL is the FUNCTION_DECL
    node for the function we want to simplify.  */

int
c_simplify_function_tree (fndecl)
     tree fndecl;
{
  tree fnbody;

  fnbody = COMPOUND_BODY (DECL_SAVED_TREE (fndecl));
  if (fnbody == NULL)
    return 1;

  /* Debugging dumps.  */
  dump_file = dump_begin (TDI_simple, &dump_flags);
  if (dump_file)
    {
      fprintf (dump_file, "\n%s()    (ORIGINAL)\n",
	       IDENTIFIER_POINTER (DECL_NAME (fndecl)));

      if (dump_flags & TDF_UNPARSE)
	print_c_tree (dump_file, fnbody);
      else
	dump_node (fnbody, TDF_SLIM | dump_flags, dump_file);
    }

  /* Create a new binding level for the temporaries created by the
     simplification process.  */
  pushlevel (0);

  /* Simplify the function's body.  */
  stmt_expr_level = 0;
  simplify_stmt (fnbody);

  /* Declare the new temporary variables.  */
  declare_tmp_vars (getdecls(), fnbody);

  /* Restore the binding level.  */
  poplevel (1, 1, 0);

  /* Debugging dump after simplification.  */
  if (dump_file)
    {
      fprintf (dump_file, "\n%s()    (SIMPLIFIED)\n",
	       IDENTIFIER_POINTER (DECL_NAME (fndecl)));

      if (dump_flags & TDF_UNPARSE)
	print_c_tree (dump_file, fnbody);
      else
	dump_node (fnbody, TDF_SLIM | dump_flags, dump_file);

      dump_end (TDI_simple, dump_file);
    }

  return 1;
}

/* }}} */

/** {{{ simplify_stmt ()
  
    Entry point for the tree lowering pass.  Recursively scan
    STMT and convert it to a SIMPLE tree.  */

static void 
simplify_stmt (stmt)
     tree stmt;
{
  tree prev;

  /* PRE and POST are tree chains that contain the side-effects of the
     simplified tree.  For instance, given the expression tree:

     		c = ++a * 3 + b++;

     After simplification, the tree will be re-written as:

     		a = a + 1;
		t1 = a * 3;	<-- PRE
     		c = t1 + b;
     		b = b + 1;	<-- POST  */

  prev = stmt;
  while (stmt && stmt != error_mark_node)
    {
      tree next, pre, post;
      int keep_stmt_p, stmt_was_null;

      pre = NULL;
      post = NULL;
      stmt_was_null = 0;
      next = TREE_CHAIN (stmt);
      
      if (dump_file && (dump_flags & TDF_DETAILS))
	{
	  fprintf (dump_file, "# %d\nORIGINAL:\n", STMT_LINENO (stmt));
	  print_c_node (dump_file, stmt);
	  fprintf (dump_file, "\n");
	}

      switch (TREE_CODE (stmt))
	{
	case COMPOUND_STMT:
	  simplify_stmt (COMPOUND_BODY (stmt));
	  prev = stmt;
	  stmt = next;
	  continue;
	  
	case FOR_STMT:
	  simplify_for_stmt (stmt, &pre, &post);
	  break;
	  
	case WHILE_STMT:
	  simplify_while_stmt (stmt, &pre);
	  break;

	case DO_STMT:
	  simplify_do_stmt (stmt);
	  break;

	case IF_STMT:
	  simplify_if_stmt (stmt, &pre);
	  break;
	  
	case SWITCH_STMT:
	  simplify_switch_stmt (stmt, &pre);
	  break;

	case EXPR_STMT:
	  /* Simplification of a statement expression will nullify the
	     statement if all its side effects are moved to PRE and POST.
	     In this case we will not want to emit the simplified
	     statement.  However, if the statement was already null before
	     simplification, we should leave it to avoid changing the
	     semantics of the program.  */
	  if (!expr_has_effect (EXPR_STMT_EXPR (stmt)))
	    stmt_was_null = 1;

	  walk_tree (&EXPR_STMT_EXPR (stmt), mostly_copy_tree_r, NULL, NULL);
	  simplify_expr (&EXPR_STMT_EXPR (stmt), &pre, &post, is_simple_expr,
	                 stmt);
	  break;

	case RETURN_STMT:
	  simplify_return_stmt (stmt, &pre);
	  break;

	/* Contrary to the original SIMPLE grammar, we do not convert
	   declaration initializers into SIMPLE assignments because this
	   breaks several C semantics (static variables, read-only
	   initializers, dynamic arrays, etc).  */
	case DECL_STMT:
	  break;

	/* Statements that need no simplification.  */
	case FILE_STMT:
	case LABEL_STMT:
	case GOTO_STMT:
	case ASM_STMT:
	case CASE_LABEL:
	case CONTINUE_STMT:
	case BREAK_STMT:
	case SCOPE_STMT:
	  prev = stmt;
	  stmt = next;
	  continue;

	default:
	  prep_stmt (stmt);
	  fprintf (stderr, "unhandled statement node in simplify_stmt ():\n");
	  debug_tree (stmt);
	  abort ();
	  break;
	}

      /* PRE and POST contain a list of expressions for all the
	 side-effects in STMT.  Each expression must be converted into a
	 statement and chained so that:

	 	PREV -> STMT -> NEXT

         is re-written as:

	 	PREV -> PRE -> STMT -> POST -> NEXT

	 However, if STMT has been nullified, it is bypassed.  */

      pre = convert_to_stmt_chain (pre, stmt);
      post = convert_to_stmt_chain (post, stmt);

      /* Before re-chaining the side effects, determine if we are going to
	 keep the original statement or not.  If the statement had no
	 effect before simplification, we emit it anyway to avoid changing
	 the semantics of the original program.  */
      keep_stmt_p = (stmt_was_null || stmt_has_effect (stmt));
      
      TREE_CHAIN (prev) = NULL_TREE;
      TREE_CHAIN (stmt) = NULL_TREE;

      /* Dump the side-effects and the simplified statement.  */
      if (dump_file && (dump_flags & TDF_DETAILS))
	{
	  fprintf (dump_file, "# %d\nPRE:\n", STMT_LINENO (stmt));
	  print_c_tree (dump_file, pre);
	  fprintf (dump_file, "\n");

	  fprintf (dump_file, "SIMPLIFIED:\n");
	  print_c_tree (dump_file, stmt);
	  fprintf (dump_file, "\n");

	  fprintf (dump_file, "POST:\n");
	  print_c_tree (dump_file, post);
	  fprintf (dump_file, "\n");
	}

      chainon (prev, pre);

      if (keep_stmt_p)
	{
	  chainon (prev, stmt);
	  chainon (stmt, post);
	}
      else
	chainon (prev, post);

      /* Next iteration.  Re-set PREV to the last statement of the chain
	 PREV -> PRE -> STMT -> POST.  */
      prev = tree_last (prev);
      stmt = next;
      TREE_CHAIN (prev) = next;
    }
}

/* }}} */

/** {{{ simplify_for_stmt ()

    Simplify a FOR_STMT node.  This will convert:

    	for (init; cond; expr)
	  {
	    body;
	  }

    into

    	pre_init_s;
	init_s;
	post_init_s;
	pre_cond_s;
	for ( ; cond_s; )
	  {
	    post_cond_s;
	    body;
	    pre_expr_s;
	    expr_s;
	    post_expr_s;
	    pre_cond_s;
	  }
	post_cond_s;

    where INIT_S, COND_S and EXPR_S are the simplified versions of INIT,
    COND and EXPR respectively.  PRE_*_S and POST_*_S are the side-effects
    for each expression in the header.

    Note that the above form is the more general case, other variations are
    possible if any of the PRE_*_S or POST_*S expressions are missing.

    The order in which the different pieces are simplified is also
    important.  Simplification should be done in the same order in which
    the loop will execute at runtime.

    PRE_P points to the list where side effects that must happen before
	STMT should be store.  These are all the expressions in
	FOR_INIT_STMT and the pre side-effects of the loop conditional.

    POST_P points to the list where side effects that must happen after
	STMT should be stored.  These are the post side effects for the
	loop conditional.  */

static void
simplify_for_stmt (stmt, pre_p, post_p)
     tree stmt;
     tree *pre_p;
     tree *post_p;
{
  int init_is_simple, cond_is_simple, expr_is_simple;
  tree pre_init_s, init_s, post_init_s;
  tree pre_cond_s, cond_s, post_cond_s;
  tree pre_expr_s, expr_s, post_expr_s;

  /* Make sure that the loop body has a scope.  */
  tree_build_scope (&FOR_BODY (stmt));

  init_s = EXPR_STMT_EXPR (FOR_INIT_STMT (stmt));
  cond_s = FOR_COND (stmt);
  expr_s = FOR_EXPR (stmt);

  /* Check if we need to do anything.  */
  init_is_simple = (init_s == NULL_TREE || is_simple_exprseq (init_s));
  cond_is_simple = (cond_s == NULL_TREE || is_simple_condexpr (cond_s));
  expr_is_simple = (expr_s == NULL_TREE || is_simple_exprseq (expr_s));

  if (init_is_simple && cond_is_simple && expr_is_simple)
    {
      /* Nothing to do, simplify the body and return.  */
      simplify_stmt (FOR_BODY (stmt));
      return;
    }

  /* Unshare the header expressions.  */
  walk_tree (&init_s, mostly_copy_tree_r, NULL, NULL);
  walk_tree (&cond_s, mostly_copy_tree_r, NULL, NULL);
  walk_tree (&expr_s, mostly_copy_tree_r, NULL, NULL);

  pre_init_s = NULL_TREE;
  post_init_s = NULL_TREE;
  pre_cond_s = NULL_TREE;
  post_cond_s = NULL_TREE;
  pre_expr_s = NULL_TREE;
  post_expr_s = NULL_TREE;

  /* Simplify FOR_INIT_STMT.  Note that we always simplify it, even if it's
     in SIMPLE form already.  This is because we need to insert PRE_COND_S
     right after the initialization statements, and if PRE_COND_S contains
     statement trees, we cannot add them to a COMPOUND_EXPR:

	BEFORE				AFTER

					pre_init_s;
					init_s;
					post_cond_s;
					pre_cond_s;
	for (init; cond; ...)		for ( ; cond_s; ...)

     FIXME: Since FOR_INIT_STMT can be a COMPOUND_EXPR, it should be possible
	    to emit PRE_INIT_S, INIT_S, POST_COND_S and PRE_COND_S into a
	    COMPOUND_EXPR inside FOR_INIT_STMT.  However, this is not
	    possible if any of these elements contains statement trees.  */
  simplify_expr (&init_s, &pre_init_s, &post_init_s, is_simple_expr, stmt);

  /* Simplify FOR_COND.  */
  if (!cond_is_simple)
    simplify_expr (&cond_s, &pre_cond_s, &post_cond_s, is_simple_condexpr,
	           stmt);

  /* Simplify the body of the loop.  */
  simplify_stmt (FOR_BODY (stmt));

  /* Simplify FOR_EXPR.  Note that if FOR_EXPR needs to be simplified,
     it's converted into a simple_expr because we need to move it out of
     the loop header (see previous FIXME note for future enhancement).  */
  if (!expr_is_simple)
    simplify_expr (&expr_s, &pre_expr_s, &post_expr_s, is_simple_expr, stmt);
  

  /* Now that all the components are simplified, we have to build a new
     loop with all the side-effects in the right spots:

    	pre_init_s;
	init_s;
	post_init_s;
	pre_cond_s;
	for ( ; cond_s; )
	  {
	    post_cond_s;
	    body;
	    pre_expr_s;
	    expr_s;
	    post_expr_s;
	    pre_cond_s;
	  }
	post_cond_s;

     The above is the more general case, which produces a for() loop that
     doesn't resemble the original.  To minimize shape changes, we try to
     insert expressions in FOR_INIT_STMT and FOR_EXPR.  */

  /* Link PRE_INIT_S, INIT_S, POST_INIT_S and a copy of PRE_COND_S to make
     up a new FOR_INIT_STMT.  If the last tree in the list is an expression
     tree, it is emmitted inside FOR_INIT_STMT.  We emit a copy of
     PRE_COND_S because we also need to emit it at every wrap-around point
     in the loop body.  */
  add_tree (pre_init_s, pre_p);
  add_tree (init_s, pre_p);
  add_tree (post_init_s, pre_p);
  add_tree (deep_copy_list (pre_cond_s), pre_p);

  if (*pre_p)
    {
      tree prev, last, op;

      prev = NULL_TREE;
      for (op = *pre_p; TREE_CHAIN (op); op = TREE_CHAIN (op))
	prev = op;
      
      last = TREE_VALUE ((prev) ? TREE_CHAIN (prev) : *pre_p);

      if (!statement_code_p (TREE_CODE (last)))
	{
	  /* The last statement is an expression, emit it inside
	    FOR_INIT_STMT and remove the expression from PRE_P.  */
	  EXPR_STMT_EXPR (FOR_INIT_STMT (stmt)) = last;
	  if (prev)
	    TREE_CHAIN (prev) = NULL_TREE;
	  else
	    *pre_p = NULL_TREE;
	}
      else
	{
	  /* The last statement is not an expression, nullify FOR_INIT_STMT.
	    All the expressions in FOR_INIT_STMT and PRE_COND_S have been
	    emitted inside PRE_P already.  */
	  EXPR_STMT_EXPR (FOR_INIT_STMT (stmt)) = NULL_TREE;
	}
    }

  /* Build the new FOR_COND.  */
  {
    FOR_COND (stmt) = cond_s;

    /* Insert one copy of POST_COND_S in the loop body and another copy in
       POST_P.  */
    insert_before_first (convert_to_stmt_chain (deep_copy_list (post_cond_s),
					        stmt),
			 FOR_BODY (stmt));
    add_tree (post_cond_s, post_p);
  }


  /* Link PRE_EXPR_S, EXPR_S, POST_EXPR_S and a copy of PRE_COND_S to emit
     before every wrap-around point inside the loop body.  If the last tree
     in the list is an expression tree, it is emmitted inside FOR_EXPR.  */
  {
    tree expr_chain;

    expr_chain = pre_expr_s;
    add_tree (expr_s, &expr_chain);
    add_tree (post_expr_s, &expr_chain);
    add_tree (deep_copy_list (pre_cond_s), &expr_chain);

    if (expr_chain)
      {
	tree prev, op, last, stmt_chain;

	prev = NULL_TREE;
	for (op = expr_chain; TREE_CHAIN (op); op = TREE_CHAIN (op))
	  prev = op;
	last = TREE_VALUE ((prev) ? TREE_CHAIN (prev) : expr_chain);

	if (!statement_code_p (TREE_CODE (last)))
	  {
	    /* The last statement is an expression, emit it inside
	      FOR_EXPR and remove the expression from EXPR_CHAIN.  */
	    FOR_EXPR (stmt) = last;
	    if (prev)
	      TREE_CHAIN (prev) = NULL_TREE;
	    else
	      expr_chain = NULL_TREE;
	  }
	else
	  {
	    /* The last statement is not an expression, nullify FOR_EXPR.  */
	    FOR_EXPR (stmt) = NULL_TREE;
	  }

	stmt_chain = convert_to_stmt_chain (deep_copy_list (expr_chain), stmt);
	insert_before_continue_end (stmt_chain, FOR_BODY (stmt),
				    STMT_LINENO (stmt));
      }
  }
}

/* }}} */

/** {{{ simplify_while_stmt ()

    Simplify a WHILE_STMT node.  This will convert:

    	while (cond)
	  {
	    body;
	  }

    into

	pre_cond_s;
	T = cond_s;
	post_cond_s;
    	while (T)
	  {
	    body;
	    pre_cond_s;
	    T = cond_s;
	    post_cond_s;
	  }

    where COND_S is the simplified version of the loop predicate.
    PRE_COND_S and POST_COND_S are the pre and post side-effects produced
    by the simplification of the conditional.
    
    Both PRE and POST side-effects will be replicated at every wrap-around
    point inside the loop's body.

    The order in which the different pieces are simplified is also
    important.  Simplification should be done in the same order in which
    the loop will execute at runtime.

    STMT is the statement to simplify.

    PRE_P points to the list where side effects that must happen before
	STMT should be stored.  */

static void
simplify_while_stmt (stmt, pre_p)
     tree stmt;
     tree *pre_p;
{
  tree post, cond_s, stmt_chain;

  post = NULL_TREE;

  /* Make sure that the loop body has a scope.  */
  tree_build_scope (&WHILE_BODY (stmt));
  
  /* Check wether the loop condition is already simplified.  */
  if (is_simple_condexpr (WHILE_COND (stmt)))
    {
      /* Nothing to do.  Simplify the body and return.  */
      simplify_stmt (WHILE_BODY (stmt)); 
      return;
    }
    
  /* Simplify the loop conditional.  Note that we simplify the conditional
     into a SIMPLE identifier so that the pre and post side-effects of the
     conditional can be computed before the loop header.  */
  cond_s = WHILE_COND (stmt);
  walk_tree (&cond_s, mostly_copy_tree_r, NULL, NULL);
  simplify_expr (&cond_s, pre_p, &post, is_simple_id, stmt);
  WHILE_COND (stmt) = build (NE_EXPR, TREE_TYPE (WHILE_COND (stmt)),
	                     cond_s, integer_zero_node);
  add_tree (post, pre_p);

  /* Simplify the body of the loop.  */
  simplify_stmt (WHILE_BODY (stmt)); 

  /* Insert all the side-effects for the conditional before every
     wrap-around point in the loop body (i.e., before every first-level
     CONTINUE and before the end of the body).  */
  stmt_chain = convert_to_stmt_chain (deep_copy_list (*pre_p), stmt);
  insert_before_continue_end (stmt_chain, WHILE_BODY (stmt),
                              STMT_LINENO (stmt));
}

/* }}} */

/** {{{ simplify_do_stmt ()

    Simplify a DO_STMT node.  This will convert:

	do
	  {
	    body;
	  }
    	while (cond);

    into

    	do
	  {
	    body;
	    pre_cond_s;
	    T = cond_s;
	    post_cond_s;
	  }
    	while (T);

    where COND_S is the simplified version of the loop predicate.
    PRE_COND_S and POST_COND_S are the pre and post side-effects produced
    by the simplification of the conditional.

    Both PRE and POST side-effects will be replicated at every wrap-around
    point inside the loop's body.

    The order in which the different pieces are simplified is also
    important.  Simplification should be done in the same order in which
    the loop will execute at runtime.

    STMT is the statement to simplify.  */

static void
simplify_do_stmt (stmt)
     tree stmt;
{
  tree cond_s, pre_cond_s, post_cond_s, stmt_chain;

  /* Make sure that the loop body has a scope.  */
  tree_build_scope (&DO_BODY (stmt));

  /* Simplify the loop's body.  */
  simplify_stmt (DO_BODY (stmt));

  /* Check wether the loop condition is already simplified.  */
  if (is_simple_condexpr (DO_COND (stmt)))
    return;

  /* Simplify the loop conditional.  Note that we simplify the conditional
     into a SIMPLE identifier so that the pre and post side-effects of the
     conditional can be computed before the loop header.  */
  pre_cond_s = NULL;
  post_cond_s = NULL;
  cond_s = DO_COND (stmt);
  walk_tree (&cond_s, mostly_copy_tree_r, NULL, NULL);
  simplify_expr (&cond_s, &pre_cond_s, &post_cond_s, is_simple_id, stmt);
  DO_COND (stmt) = build (NE_EXPR, TREE_TYPE (DO_COND (stmt)), cond_s, 
                          integer_zero_node);

  /* Insert all the side-effects for the conditional before every
     wrap-around point in the loop body (i.e., before every first-level
     CONTINUE and before the end of the body).  */
  add_tree (post_cond_s, &pre_cond_s);
  stmt_chain = convert_to_stmt_chain (deep_copy_list (pre_cond_s), stmt);
  insert_before_continue_end (stmt_chain, DO_BODY (stmt), STMT_LINENO (stmt));
}

/* }}} */

/** {{{ simplify_if_stmt ()
    
    Simplify an IF_STMT.  This will convert:

    	if (cond)
	  {
	    then_clause;
	  }
	else
	  {
	    else_clause;
	  }

    into

	pre_cond_s;
	T = cond_s;
	post_cond_s;
    	if (T)
	  {
	    then_clause;
	  }
	else
	  {
	    else_clause;
	  }

    where COND_S is the simplified version of the predicate. PRE_COND_S and
    POST_COND_S are the pre and post side-effects produced by the
    simplification of the conditional.

    The order in which the different pieces are simplified is also
    important.  Simplification should be done in the same order in which
    the loop will execute at runtime.

    STMT is the statement to simplify.

    PRE_P points to the list where side effects that must happen before
	STMT should be stored.  */

static void
simplify_if_stmt (stmt, pre_p)
     tree stmt;
     tree *pre_p;
{
  tree cond_s, post_cond_s = NULL_TREE;

  /* Make sure each clause is contained inside a scope.  */
  if (THEN_CLAUSE (stmt))
    tree_build_scope (&THEN_CLAUSE (stmt));

  if (ELSE_CLAUSE (stmt))
    tree_build_scope (&ELSE_CLAUSE (stmt));
      		
  /* Nothing to do if the condition is simple already.  */
  if (is_simple_condexpr (IF_COND (stmt)))
    {
      if (THEN_CLAUSE (stmt))
	simplify_stmt (THEN_CLAUSE (stmt));

      if (ELSE_CLAUSE (stmt))
	simplify_stmt (ELSE_CLAUSE (stmt));

      return;
    }

  /* Simplify the conditional.  Force simplification to produce a SIMPLE
     identifier so that all the pre and post side-effects for the
     conditional can be evaluated before the if().  */
  cond_s = IF_COND (stmt);
  walk_tree (&cond_s, mostly_copy_tree_r, NULL, NULL);
  simplify_expr (&cond_s, pre_p, &post_cond_s, is_simple_id, stmt);
  IF_COND (stmt) = build (NE_EXPR, TREE_TYPE (cond_s), cond_s,
			  integer_zero_node);

  /* Chain pre and post side-effects together.  Since the simplification of
     the conditional has produced an identifier, it is safe to emit the
     side-effects before the if() statement.  */
  add_tree (post_cond_s, pre_p);

  /* Simplify each of the clauses.  */
  if (THEN_CLAUSE (stmt))
    simplify_stmt (THEN_CLAUSE (stmt));

  if (ELSE_CLAUSE (stmt))
    simplify_stmt (ELSE_CLAUSE (stmt));
}

/* }}} */

/** {{{ simplify_switch_stmt ()

    Simplify a SWITCH_STMT.  This will convert:

    	switch (cond)
	  {
	    body;
	  }

    into

	pre_cond_s;
	T = cond_s;
	post_cond_s;
    	switch (T)
	  {
	    body;
	  }

    where COND_S is the simplified version of the predicate. PRE_COND_S and
    POST_COND_S are the pre and post side-effects produced by the
    simplification of the conditional.
    
    The order in which the different pieces are simplified is also
    important.  Simplification should be done in the same order in which
    the loop will execute at runtime.

    STMT is the statement to simplify.

    PRE_P points to the list where side effects that must happen before
	STMT should be stored.  */

static void
simplify_switch_stmt (stmt, pre_p)
     tree stmt;
     tree *pre_p;
{
  tree post_cond_s;


  if (is_simple_val (SWITCH_COND (stmt)))
    {
      /* Nothing to do.  Simplify the body and return.  */
      simplify_stmt (SWITCH_BODY (stmt));
      return;
    }

  /* Simplify the conditional.  Force simplification to produce a SIMPLE
     identifier so that all the pre and post side-effects for the
     conditional can be evaluated before the switch().  */
  post_cond_s = NULL_TREE;
  walk_tree (&SWITCH_COND (stmt), mostly_copy_tree_r, NULL, NULL);
  simplify_expr (&SWITCH_COND (stmt), pre_p, &post_cond_s, is_simple_id, stmt);

  /* Chain pre and post side-effects together.  Since the simplification of
     the conditional has produced an identifier, it is safe to emit the
     side-effects before the switch() statement.  */
  add_tree (post_cond_s, pre_p);

  simplify_stmt (SWITCH_BODY (stmt));
}

/* }}} */

/** {{{ simplify_return_stmt ()

    Simplify a RETURN_STMT.  If the expression to be returned is not a
    SIMPLE value, it is assigned to a new temporary and the statement is
    re-written to return the temporary.

    PRE_P points to the list where side effects that must happen before
	STMT should be stored.  */

static void
simplify_return_stmt (stmt, pre_p)
     tree stmt;
     tree *pre_p;
{
  if (!VOID_TYPE_P (TREE_TYPE (TREE_TYPE (current_function_decl)))
      && RETURN_EXPR (stmt))
    {
      tree expr, tmp, ret_expr, post;
      
      /* A return expression is represented by a MODIFY_EXPR node that
	 assigns the return value into a RESULT_DECL.  */
      if (TREE_CODE (RETURN_EXPR (stmt)) != MODIFY_EXPR)
	abort ();

      ret_expr = TREE_OPERAND (RETURN_EXPR (stmt), 1);

      if (is_simple_val (ret_expr))
	return;

      /* Create the assignment 'T = ret_expr'.  */
      tmp = create_tmp_var (TREE_TYPE (ret_expr));
      expr = build_modify_expr (tmp, NOP_EXPR, ret_expr);

      /* Simplify it.  */
      post = NULL_TREE;
      walk_tree (&expr, mostly_copy_tree_r, NULL, NULL);
      simplify_expr (&expr, pre_p, &post, is_simple_expr, stmt);

      /* Emit the expression and its post side-effects into PRE_P.  */
      add_tree (expr, pre_p);
      add_tree (post, pre_p);

      /* Replace the return value with the new temporary.  */
      TREE_OPERAND (RETURN_EXPR (stmt), 1) = tmp;
    }
}

/* }}} */


/* Simplification of expression trees.  */

/** {{{ simplify_expr ()

    Simplifies the expression tree pointed by EXPR_P.

    PRE_P points to the list where side effects that must happen before
	EXPR should be stored.

    POST_P points to the list where side effects that must happen after
	EXPR should be stored.

    SIMPLE_TEST_F points to a function that takes a tree T and
	returns nonzero if T is in the SIMPLE form requested by the
	caller.  The SIMPLE predicates are in tree-simple.c.

	This test is used twice.  Before simplification, the test is
	invoked to determine whether *EXPR_P is already simple enough.  If
	that fails, *EXPR_P is simplified according to its code and
	SIMPLE_TEST_F is called again.  If the test still fails, then a new
	temporary variable is created and assigned the value of the
	simplified expression.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_expr (expr_p, pre_p, post_p, simple_test_f, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     int (*simple_test_f) PARAMS ((tree));
     tree stmt;
{
  if (simple_test_f == NULL)
    abort ();

  if ((*simple_test_f) (*expr_p))
    return;

  /* First deal with the special cases.  */
  switch (TREE_CODE (*expr_p))
    {
    case POSTINCREMENT_EXPR:
    case POSTDECREMENT_EXPR:
    case PREINCREMENT_EXPR:
    case PREDECREMENT_EXPR:
      simplify_self_mod_expr (expr_p, pre_p, post_p, stmt);
      break;

    case ARRAY_REF:
      simplify_array_ref (expr_p, pre_p, post_p, stmt);
      break;

    case COMPONENT_REF:
      simplify_component_ref (expr_p, pre_p, post_p, stmt);
      break;
      
    case COND_EXPR:
      simplify_cond_expr (expr_p, pre_p, stmt);
      break;

    case CALL_EXPR:
      simplify_call_expr (expr_p, pre_p, post_p, stmt);
      break;

    case TREE_LIST:
      simplify_tree_list (expr_p, pre_p, post_p, stmt);
      break;

    case COMPOUND_EXPR:
      simplify_compound_expr (expr_p, pre_p, post_p, stmt);
      break;

    case MODIFY_EXPR:
      simplify_modify_expr (expr_p, pre_p, post_p, stmt);
      break;

    case TRUTH_ANDIF_EXPR:
    case TRUTH_ORIF_EXPR:
      simplify_boolean_expr (expr_p, pre_p, stmt);
      break;

    case TRUTH_NOT_EXPR:
      {
	tree t = TREE_OPERAND (*expr_p, 0);
	simplify_expr (&t, pre_p, post_p, is_simple_id, stmt);
	*expr_p = build (EQ_EXPR, TREE_TYPE (*expr_p), t, integer_zero_node);
	break;
      }

    /* Address expressions must not be simplified.  If they are, we may
       end up taking the address of a temporary, instead of the address
       of the original object.  */
    case ADDR_EXPR:
      break;

    /* va_arg expressions should also be left alone to avoid confusing the
       vararg code.  FIXME: Is this really necessary?  */
    case VA_ARG_EXPR:
      break;

    case NOP_EXPR:
    case CONVERT_EXPR:
    case FIX_TRUNC_EXPR:
    case FIX_CEIL_EXPR:
    case FIX_FLOOR_EXPR:
    case FIX_ROUND_EXPR:
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p,
	             is_simple_varname, stmt);
      break;

    case INDIRECT_REF:
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, is_simple_id,
	             stmt);
      break;

    case NEGATE_EXPR:
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, is_simple_val,
	             stmt);
      break;

    /* Constants need not be simplified.  */
    case INTEGER_CST:
    case REAL_CST:
    case STRING_CST:
    case COMPLEX_CST:
      break;

    /* Do not simplify compound literals.  FIXME: Maybe we should?  */
    case COMPOUND_LITERAL_EXPR:
      break;

    /* Do not simplify constructor expressions.  FIXME: Maybe we should?  */
    case CONSTRUCTOR:
      break;

    /* The following are special cases that are not handled by the original
       SIMPLE grammar.  */
    case STMT_EXPR:
      stmt_expr_level++;
      simplify_stmt (STMT_EXPR_STMT (*expr_p));
      stmt_expr_level--;
      break;

    /* SAVE_EXPR nodes are converted into a SIMPLE identifier and
       eliminated.  */
    case SAVE_EXPR:
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, is_simple_id,
	             stmt);
      *expr_p = TREE_OPERAND (*expr_p, 0);
      break;

    case EXPR_WITH_FILE_LOCATION:
      simplify_expr_wfl (expr_p, pre_p, post_p, simple_test_f, stmt);
      break;

    /* FIXME: This breaks stage2.  I still haven't figured out why.  When
	      fixing remember to undo a similar change in
	      is_simple_unary_expr.  */
    case BIT_FIELD_REF:
#if 0
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, is_simple_id);
      simplify_expr (&TREE_OPERAND (*expr_p, 1), pre_p, post_p, is_simple_val);
      simplify_expr (&TREE_OPERAND (*expr_p, 2), pre_p, post_p, is_simple_val);
#endif
      break;

    case NON_LVALUE_EXPR:
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, simple_test_f,
	             stmt);
      break;

    /* If *EXPR_P does not need to be special-cased, handle it according to
       its class.  */
    default:
      {
	if (TREE_CODE_CLASS (TREE_CODE (*expr_p)) == '1')
	  {
	    simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p,
	                   is_simple_val, stmt);
	  }
	else if (TREE_CODE_CLASS (TREE_CODE (*expr_p)) == '2'
	         || TREE_CODE_CLASS (TREE_CODE (*expr_p)) == '<'
		 || TREE_CODE (*expr_p) == TRUTH_AND_EXPR
		 || TREE_CODE (*expr_p) == TRUTH_OR_EXPR
		 || TREE_CODE (*expr_p) == TRUTH_XOR_EXPR)
	  {
	    simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p,
			   is_simple_val, stmt);

	    simplify_expr (&TREE_OPERAND (*expr_p, 1), pre_p, post_p,
			   is_simple_val, stmt);
	  }
	else
	  {
	    fprintf (stderr, "unhandled expression in simplify_expr ():\n");
	    debug_tree (*expr_p);
	    abort ();
	  }
      }
    }

  /* Test the simplified expression, if it's sufficiently simple already,
     return.  */
  if ((*simple_test_f) (*expr_p))
    return;
  
  /* Otherwise, we need to create a new temporary to hold the simplified
     expression.  At this point, the expression should be simple enough to
     qualify as a SIMPLE assignment RHS.  Otherwise, simplification has
     failed.  */
  if (!is_simple_rhs (*expr_p))
    {
      fprintf (stderr, "Expression is not a SIMPLE RHS:\n");
      debug_tree (*expr_p);
      fprintf (stderr, "\n");
      abort ();
    }

  /* If the simplified expression can be assigned into a new temporary TMP,
     do so and replace the original expression with TMP.  */
  if (!VOID_TYPE_P (TREE_TYPE (*expr_p)))
    {
      tree tmp = create_tmp_var (TREE_TYPE (*expr_p));
      add_tree (build_modify_expr (tmp, NOP_EXPR, *expr_p), pre_p);
      *expr_p = tmp;
    }
}

/* }}} */

/** {{{ simplify_array_ref ()

    Re-write the ARRAY_REF node pointed by EXPR_P.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.

    FIXME: ARRAY_REF currently doesn't accept a pointer as the array
    argument, so this simplification uses an INDIRECT_REF of ARRAY_TYPE.
    ARRAY_REF should be extended.  */

static void
simplify_array_ref (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  tree base, array;
  varray_type dim_stack;

  if (TREE_CODE (*expr_p) != ARRAY_REF)
    abort ();

  VARRAY_GENERIC_PTR_INIT (dim_stack, 10, "dim_stack");

  /* Create a stack with all the dimensions of the array so that they can
     be simplified from left to right.  */
  base = *expr_p;
  VARRAY_PUSH_GENERIC_PTR (dim_stack, (PTR)&(TREE_OPERAND (*expr_p, 1)));
  while (TREE_CODE (TREE_OPERAND (base, 0)) == ARRAY_REF)
    {
      base = TREE_OPERAND (base, 0);
      VARRAY_PUSH_GENERIC_PTR (dim_stack, (PTR)&(TREE_OPERAND (base, 1)));
    }

  /* After the loop above, 'base' contains the leftmost ARRAY_REF,
     and 'dim_stack' is a stack of pointers to all the dimensions in left
     to right order (the leftmost dimension is at the top of the stack).

     Simplify the base, and then each of the dimensions from left to
     right.  */

  array = TREE_OPERAND (base, 0);
  if (TREE_CODE (TREE_TYPE (array)) != ARRAY_TYPE)
    abort ();
  if (! is_simple_arraybase (array))
    {
      array = fold (build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (array)),
			    array));
      simplify_expr (&array, pre_p, post_p, is_simple_id, stmt);
      array = build_indirect_ref (array, "");
      TREE_OPERAND (base, 0) = array;
    }

  while (VARRAY_ACTIVE_SIZE (dim_stack) > 0)
    {
      tree *dim_p = (tree *)VARRAY_TOP_GENERIC_PTR (dim_stack);
      VARRAY_POP (dim_stack);
      simplify_expr (dim_p, pre_p, post_p, is_simple_val, stmt);
    }

  VARRAY_FREE (dim_stack);
}

/* }}} */

/** {{{ simplify_self_mod_expr ()

    Simplify the self modifying expression pointed by EXPR_P (++, --, +=, -=).

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_self_mod_expr (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  enum tree_code code;
  tree lhs, lvalue, rhs, t1;

  code = TREE_CODE (*expr_p);

  if (code != POSTINCREMENT_EXPR
      && code != POSTDECREMENT_EXPR
      && code != PREINCREMENT_EXPR
      && code != PREDECREMENT_EXPR)
    abort ();

  /* Simplify the LHS into a SIMPLE lvalue.  We need to deep copy the first
     operand because it will be simplified twice.  Once to convert it into
     a SIMPLE lvalue and the second time when we simplify the resulting
     binary expression on the RHS of the assignment.  */
  lvalue = TREE_OPERAND (*expr_p, 0);
  walk_tree (&lvalue, copy_tree_r, NULL, NULL);
  simplify_lvalue_expr (&lvalue, pre_p, post_p, stmt);

  /* Determine whether we need to create a PLUS or a MINUS operation.  */
  lhs = TREE_OPERAND (*expr_p, 0);
  rhs = TREE_OPERAND (*expr_p, 1);
  if (code == PREINCREMENT_EXPR || code == POSTINCREMENT_EXPR)
    t1 = build (PLUS_EXPR, TREE_TYPE (*expr_p), lhs, rhs);
  else
    t1 = build (MINUS_EXPR, TREE_TYPE (*expr_p), lhs, rhs);

  /* If LHS is not a SIMPLE identifier, the resulting binary expression
     will not be in simple form.  */
  simplify_expr (&t1, pre_p, post_p, is_simple_binary_expr, stmt);

  /* Determine whether the new assignment should go before or after
     the simplified expression.  */
  if (code == PREINCREMENT_EXPR || code == PREDECREMENT_EXPR)
    add_tree (build_modify_expr (lvalue, NOP_EXPR, t1), pre_p);
  else
    add_tree (build_modify_expr (lvalue, NOP_EXPR, t1), post_p);

  /* Replace the original expression with the LHS of the assignment.  */
  *expr_p = lvalue;
}

/* }}} */

/** {{{ simplify_component_ref ()
    
    Simplify the COMPONENT_REF node pointed by EXPR_P.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_component_ref (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  if (TREE_CODE (*expr_p) != COMPONENT_REF)
    abort ();

  simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p,
                 is_simple_compref_lhs, stmt);

  simplify_expr (&TREE_OPERAND (*expr_p, 1), pre_p, post_p, is_simple_id,
                 stmt);
}

/* }}} */

/** {{{ simplify_call_expr ()

    Simplify the CALL_EXPR node pointed by EXPR_P.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_call_expr (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  tree id;

  if (TREE_CODE (*expr_p) != CALL_EXPR)
    abort ();

  /* Do not simplify calls to builtin functions as they may require
     specific tree nodes (e.g., __builtin_stdarg_start).
     FIXME: We should identify which builtins can be simplified safely.  */
  id = get_callee_fndecl (*expr_p);
  if (id && DECL_BUILT_IN (id))
    return;

  simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, is_simple_id,
                 stmt);
  simplify_expr (&TREE_OPERAND (*expr_p, 1), pre_p, post_p, is_simple_arglist,
                 stmt);
}

/* }}} */

/** {{{ simplify_tree_list ()

    Simplify the TREE_LIST node pointed by EXPR_P.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_tree_list (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  tree op;

  if (TREE_CODE (*expr_p) != TREE_LIST)
    abort ();

  for (op = *expr_p; op; op = TREE_CHAIN (op))
    simplify_expr (&TREE_VALUE (op), pre_p, post_p, is_simple_val, stmt);
}

/* }}} */

/** {{{ simplify_cond_expr ()

    Convert the conditional expression pointed by EXPR_P '(p) ? a : b;'
    into

    if (p)			if (p)
      t1 = a;			  a;
    else		or	else
      t1 = b;			  b;
    t1;				(void)0;

    The second form is used when *EXPR_P is of type void.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_cond_expr (expr_p, pre_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree stmt;
{
  tree t_then, t_else, tmp, pred, tval, fval, new_if, expr_type;

  if (TREE_CODE (*expr_p) != COND_EXPR)
    abort ();

  expr_type = TREE_TYPE (*expr_p);

  if (!VOID_TYPE_P (expr_type))
    tmp = create_tmp_var (TREE_TYPE (*expr_p));
  else
    tmp = void_zero_node;

  pred = TREE_OPERAND (*expr_p, 0);
  tval = TREE_OPERAND (*expr_p, 1);
  fval = TREE_OPERAND (*expr_p, 2);

  /* Build the THEN_CLAUSE 't1 = a;' or 'a;'.  */
  if (!VOID_TYPE_P (expr_type))
    t_then = build_stmt (EXPR_STMT, build_modify_expr (tmp, NOP_EXPR, tval));
  else
    t_then = build_stmt (EXPR_STMT, tval);
  STMT_LINENO (t_then) = STMT_LINENO (stmt);
  tree_build_scope (&t_then);

  /* Build the ELSE_CLAUSE 't1 = b;' or 'b;'.  */
  if (!VOID_TYPE_P (expr_type))
    t_else = build_stmt (EXPR_STMT, build_modify_expr (tmp, NOP_EXPR, fval));
  else
    t_else = build_stmt (EXPR_STMT, fval);
  STMT_LINENO (t_else) = STMT_LINENO (stmt);
  tree_build_scope (&t_else);

  /* Build a new IF_STMT, simplify it and insert it in the PRE_P chain.  */
  new_if = build_stmt (IF_STMT, pred, t_then, t_else);
  STMT_LINENO (new_if) = STMT_LINENO (stmt);
  simplify_if_stmt (new_if, pre_p);
  add_tree (new_if, pre_p);

  /* Replace the original expression with the new temporary.  */
  *expr_p = tmp;
}

/* }}} */

/** {{{ simplify_modify_expr ()

    Simplify the MODIFY_EXPR node pointed by EXPR_P.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_modify_expr (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  if (TREE_CODE (*expr_p) != MODIFY_EXPR)
    abort ();

  simplify_lvalue_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, stmt);
  simplify_expr (&TREE_OPERAND (*expr_p, 1), pre_p, post_p, is_simple_rhs,
                 stmt);

  add_tree (*expr_p, pre_p);
  *expr_p = TREE_OPERAND (*expr_p, 0);
}

/* }}} */

/** {{{ simplify_boolean_expr ()

    Simplify TRUTH_ANDIF_EXPR and TRUTH_ORIF_EXPR expressions.  EXPR_P
    points to the expression to simplify.

    Expressions of the form 'a && b' are simplified to:

    	T = a;
	if (T)
	  T = b;

    Expressions of the form 'a || b' are simplified to:

	T = a;
	if (!T)
	  T = b;

    In both cases, the expression is re-written as 'T != 0'.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_boolean_expr (expr_p, pre_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree stmt;
{
  enum tree_code code;
  tree t, lhs, rhs, if_body, if_cond, mod_expr, if_stmt;

  code = TREE_CODE (*expr_p);
  if (code != TRUTH_ANDIF_EXPR && code != TRUTH_ORIF_EXPR)
    abort ();

  lhs = TREE_OPERAND (*expr_p, 0);
  rhs = TREE_OPERAND (*expr_p, 1);

  /* Build 'T = a'  */
  t = create_tmp_var (TREE_TYPE (*expr_p));
  mod_expr = build_modify_expr (t, NOP_EXPR, lhs);

  /* Build the body for the if() statement that conditionally evaluates the
     RHS of the expression.  Note that we first build the assignment
     surrounded by a new scope so that its simplified form is computed
     inside the new scope.  */
  if_body = build_stmt (EXPR_STMT, build_modify_expr (t, NOP_EXPR, rhs));
  STMT_LINENO (if_body) = STMT_LINENO (stmt);
  tree_build_scope (&if_body);

  /* Build the statement 'if (T = a <comp> 0) T = b;'.  Where <comp> is
     NE_EXPR if we are processing && and EQ_EXPR if we are processing ||.

     Note that we are deliberately creating a non SIMPLE statement to
     explicitly expose the sequence points to the simplifier.  When the
     resulting if() statement is simplified, the side effects for the LHS
     of 'a && b' will be inserted before the evaluation of 'b'.  */
  if (code == TRUTH_ANDIF_EXPR)
    if_cond = build (NE_EXPR, TREE_TYPE (*expr_p), mod_expr, integer_zero_node);
  else
    if_cond = build (EQ_EXPR, TREE_TYPE (*expr_p), mod_expr, integer_zero_node);

  if_stmt = build_stmt (IF_STMT, if_cond, if_body, NULL_TREE);
  STMT_LINENO (if_stmt) = STMT_LINENO (stmt);

  /* Simplify the IF_STMT and insert it in the PRE_P chain.  */
  simplify_if_stmt (if_stmt, pre_p);
  add_tree (if_stmt, pre_p);

  /* Re-write the original expression to evaluate 'T != 0'.  */
  *expr_p = build (NE_EXPR, TREE_TYPE (*expr_p), t, integer_zero_node);
}

/* }}} */

/** {{{ simplify_compound_expr ()

    Simplifies an expression sequence. This function simplifies each
    expression and re-writes the original expression with the last
    expression of the sequence in SIMPLE form.

    PRE_P points to the list where the side effects for all the expressions
	in the sequence will be emitted.

    POST_P points to the list where the post side effects for the last
	expression in the sequence are emitted.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_compound_expr (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  tree *expr_s, *pre_expr_s, *post_expr_s;
  tree t, ret;
  int i, num;

  if (TREE_CODE (*expr_p) != COMPOUND_EXPR)
    abort ();

  /* Count all the expressions in the sequence.  */
  num = 2;
  t = *expr_p;
  while (TREE_OPERAND (t, 1)
         && TREE_CODE (TREE_OPERAND (t, 1)) == COMPOUND_EXPR)
    {
      num++;
      t = TREE_OPERAND (t, 1);
    }

  /* Collect all the expressions in the sequence into the EXPR_S array.  */
  expr_s = (tree *) xmalloc (num * sizeof (tree));
  memset (expr_s, 0, num * sizeof (tree));

  pre_expr_s = (tree *) xmalloc (num * sizeof (tree));
  memset (pre_expr_s, 0, num * sizeof (tree));

  post_expr_s = (tree *) xmalloc (num * sizeof (tree));
  memset (post_expr_s, 0, num * sizeof (tree));

  t = *expr_p;
  for (i = 0; i < num; i++)
    {
      if (i < num - 1)
	{
	  expr_s[i] = TREE_OPERAND (t, 0);
	  t = TREE_OPERAND (t, 1);
	}
      else
	expr_s[i] = t;
    }


  /* Simplify each expression in the array.  Add all the side effects and
     the simplified expressions to PRE_P.  POST_P will contain the post
     side-effects of the last expression in the sequence.  After
     simplification, we return the last expression of the sequence.  */
  for (i = 0; i < num; i++)
    {
      simplify_expr (&expr_s[i], &pre_expr_s[i], &post_expr_s[i],
	             is_simple_expr, stmt);

      /* Add the side-effects and the simplified expression to PRE_P.  
	 This is necessary because the comma operator represents a sequence
	 point.  */
      add_tree (pre_expr_s[i], pre_p);

      if (i < num - 1)
	{
	  add_tree (expr_s[i], pre_p);
	  add_tree (post_expr_s[i], pre_p);
	}
    }

  ret = expr_s[num - 1];
  add_tree (post_expr_s[num - 1], post_p);

  free (expr_s);
  free (pre_expr_s);
  free (post_expr_s);

  *expr_p = ret;
}

/* }}} */

/** {{{ simplify_expr_wfl ()

    Simplify an EXPR_WITH_FILE_LOCATION.  EXPR_P points tothe expression to
    simplify.
    
    After simplification, all the nodes in PRE_P and POST_P are wrapped
    inside a EXPR_WITH_FILE_LOCATION node to preserve the original
    semantics.  The simplified expression is also returned inside an
    EXPR_WITH_FILE_LOCATION node.

    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    SIMPLE_TEST_F points to a function that takes a tree T and
	returns nonzero if T is in the SIMPLE form requested by the
	caller.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_expr_wfl (expr_p, pre_p, post_p, simple_test_f, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     int (*simple_test_f) PARAMS ((tree));
     tree stmt;
{
  tree op;
  const char *file;
  int line, col;

  if (TREE_CODE (*expr_p) != EXPR_WITH_FILE_LOCATION)
    abort ();

  simplify_expr (&EXPR_WFL_NODE (*expr_p), pre_p, post_p, simple_test_f, stmt);

  file = EXPR_WFL_FILENAME (*expr_p);
  line = EXPR_WFL_LINENO (*expr_p);
  col = EXPR_WFL_COLNO (*expr_p);

  for (op = *pre_p; op; op = TREE_CHAIN (op))
    TREE_VALUE (op) = build_expr_wfl (TREE_VALUE (op), file, line, col);

  for (op = *post_p; op; op = TREE_CHAIN (op))
    TREE_VALUE (op) = build_expr_wfl (TREE_VALUE (op), file, line, col);
}

/* }}} */

/** {{{ simplify_lvalue_expr ()

    Simplify the node pointed by EXPR_P into a SIMPLE lvalue.  The
    simplification is done so that the simplified expression can be used on
    the LHS of an assignment that modifies the same memory location than
    the original expression.
    
    PRE_P points to the list where side effects that must happen before
	*EXPR_P should be stored.

    POST_P points to the list where side effects that must happen after
    	*EXPR_P should be stored.

    STMT is the statement tree that contains EXPR.  It's used in cases
	where simplifying an expression requires creating new statement
	trees.  */

static void
simplify_lvalue_expr (expr_p, pre_p, post_p, stmt)
     tree *expr_p;
     tree *pre_p;
     tree *post_p;
     tree stmt;
{
  if (is_simple_modify_expr_lhs (*expr_p))
    return;

  if (TREE_CODE (*expr_p) == INDIRECT_REF)
    {
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p, is_simple_id,
	             stmt);
    }
  else if (TREE_CODE (*expr_p) == COMPONENT_REF
	   && TREE_CODE (TREE_OPERAND (*expr_p, 0)) == INDIRECT_REF)
    {
      tree t = TREE_OPERAND (*expr_p, 0);
      simplify_expr (&TREE_OPERAND (t, 0), pre_p, post_p, is_simple_id, stmt);
    }
  else if (TREE_CODE (*expr_p) == ARRAY_REF)
    {
      simplify_expr (expr_p, pre_p, post_p, is_simple_varname, stmt);
    }
  else if (TREE_CODE (*expr_p) == COMPONENT_REF)
    {
      /* Load the address of the base structure or union into a
	 temporary.  Since address expressions are never simplified, we
	 don't need to simplify the new lvalue.  */
      tree lvalue, tmp, base;
      
      base = TREE_OPERAND (*expr_p, 0);
      lvalue = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (base)), base);
      tmp = create_tmp_var (TREE_TYPE (lvalue));
      add_tree (build_modify_expr (tmp, NOP_EXPR, lvalue), pre_p);
      
      /* Re-write the base of the structure with a reference to the new
	 temporary.  */
      TREE_OPERAND (*expr_p, 0) = build1 (INDIRECT_REF, TREE_TYPE (base), tmp);
    }
  else if (TREE_CODE (*expr_p) == REALPART_EXPR
	   || TREE_CODE (*expr_p) == IMAGPART_EXPR)
    {
      simplify_expr (&TREE_OPERAND (*expr_p, 0), pre_p, post_p,
		     is_simple_varname, stmt);
    }
  else
    abort ();
}

/* }}} */


/* Code generation.  */

/** {{{ tree_build_scope ()
   
   Replaces T; by a COMPOUND_STMT containing {T;}.  */

void
tree_build_scope (t)
     tree *t;
{
  tree comp_stmt, start_scope, end_scope;

  /* If T already has a proper scope, do nothing.  */
  if (*t
      && TREE_CODE (*t) == COMPOUND_STMT
      && COMPOUND_BODY (*t))
    return;

  /* Create a new empty scope.  */
  comp_stmt = make_node (COMPOUND_STMT);

  start_scope = make_node (SCOPE_STMT);
  SCOPE_BEGIN_P (start_scope) = 1;

  end_scope = make_node (SCOPE_STMT);
  SCOPE_BEGIN_P (end_scope) = 0;

  COMPOUND_BODY (comp_stmt) = start_scope;

  if (*t)
    {
      /* If T is not empty, insert it inside the newly created scope.  Note
	 that we can't just join TREE_CHAIN(*T) to the closing scope
	 because even if T wasn't inside a scope, it might be a list of
	 statements.  */
      TREE_CHAIN (start_scope) = *t;
      chainon (*t, end_scope);
    }
  else
    {
      /* T is empty.  Simply join the start/end nodes.  */
      TREE_CHAIN (start_scope) = end_scope;
    }

  /* Set T to the newly constructed scope.  */
  *t = comp_stmt;
}

/* }}} */

/** {{{ add_tree ()

    Add T to the list container pointed by LIST_P.  If T is a TREE_LIST
    node, it is linked-in directly.  If T is an expression with no effects,
    it is ignored.
    
    Return the newly added list node or NULL_TREE if T was not added to
    LIST_P.  */

static tree
add_tree (t, list_p)
    tree t;
    tree *list_p;
{
  tree n;
  
  if (t == NULL_TREE)
    return NULL_TREE;

  if (TREE_CODE (t) != TREE_LIST)
    {
      /* Do nothing if T has no effect.  */
      if (statement_code_p (TREE_CODE (t)))
	{
	  if (!stmt_has_effect (t))
	    return NULL_TREE;
	}
      else
	{
	  if (!expr_has_effect (t))
	    return NULL_TREE;
	}

      /* Deep-copy expressions before adding them to the list.  */
      if (!statement_code_p (TREE_CODE (t)))
	walk_tree (&t, copy_tree_r, NULL, NULL);

      n = build_tree_list (NULL_TREE, t);
    }
  else
    n = t;

  *list_p = chainon (*list_p, n);

  return n;
}

/* }}} */

/** {{{ insert_before_continue_end ()
    
    Insert the REEVAL list before CONTINUE_STMTs and at the end of the loop
    body BODY.  Set the line number of the REEVAL list to LINE.  */

void
insert_before_continue_end (reeval, body, line)
     tree reeval;
     tree body;
     int line;
{
  tree last, beforelast;

  if (reeval == NULL_TREE)
    return;

  /* Update the line number information.  */
  {
    tree it;
    for (it = reeval; TREE_CHAIN (it); it = TREE_CHAIN (it))
      update_line_number (it, line);
  }

  /* Make sure that the loop body has a scope.  */
  tree_build_scope (&body);

  /* Insert the reevaluation list before every CONTINUE_STMT.  */
  /*  TREE_CHAIN (reeval) = NULL_TREE; */
  beforelast = insert_before_continue (body, reeval);
  last = TREE_CHAIN (beforelast);
  
  /* If the last statement of the WHILE_BODY is not a CONTINUE_STMT, 
     then insert reeval at the end of the loop block.  */
  if (TREE_CODE (beforelast) != CONTINUE_STMT)
    {
      TREE_CHAIN (beforelast) = deep_copy_list (reeval);
      beforelast = tree_last (beforelast);
      TREE_CHAIN (beforelast) = last;
    }
}

/* }}} */

/** {{{ insert_before_continue ()
    
    Insert the statement list REEVAL before each CONTINUE_STMT in the block
    pointed to by NODE.  At the end returns a pointer to the beforelast
    node in the block NODE.  The caller can insert then the last loop
    reevaluation at the end of the loop block.  */

static tree
insert_before_continue (node, reeval)
     tree node;
     tree reeval;
{
  tree next;

  if (reeval == NULL_TREE || node == NULL_TREE)
    return NULL_TREE;
  
  if (TREE_CODE (node) == COMPOUND_STMT)
    node = COMPOUND_BODY (node);
  
  next = TREE_CHAIN (node);
  if (next == NULL_TREE)
    return NULL_TREE;

  /* Walk through each statement in the given block up to the last one, 
     searching for CONTINUE_STMTs.  */
  for ( ;TREE_CHAIN (next) != NULL_TREE; 
	node = TREE_CHAIN (node), next = TREE_CHAIN (next))
    {
      switch (TREE_CODE (next))
	{
	case CONTINUE_STMT:
	  /* Insert the reeval of statements before continue.  */
	  TREE_CHAIN (node) = deep_copy_list (reeval);
	  node = tree_last (node);
	  TREE_CHAIN (node) = next;
	  break;
	  
	case IF_STMT:
	  /* Be sure that the THEN_CLAUSE has a scope.  */
	  tree_build_scope (&THEN_CLAUSE (next));

	  /* Insert the code REEVAL in the block of the THEN_CLAUSE.  */
	  insert_before_continue (COMPOUND_BODY (THEN_CLAUSE (next)), reeval);
	  
	  /* Same thing for the ELSE_CLAUSE.  */
	  if (ELSE_CLAUSE (TREE_CHAIN (node)))
	    {
	      tree_build_scope (&ELSE_CLAUSE (next));
	      insert_before_continue (COMPOUND_BODY (ELSE_CLAUSE (next)), reeval);
	    }
	  break;

	case SWITCH_STMT:
	  /* Be sure that the SWITCH_BODY has a scope.  */
	  tree_build_scope (&SWITCH_BODY (next));

	  /* Insert the code REEVAL in the SWITCH_BODY.  */
	  insert_before_continue (COMPOUND_BODY (SWITCH_BODY (next)), reeval);
	  break;

	case COMPOUND_STMT:
	  /* Insert in the inner block.  */
	  insert_before_continue (COMPOUND_BODY (next), reeval);
	  break;

	default:
	  /* Don't enter in sub loops...  The continue statement has an effect 
	     only at a depth 1.  */
	  break;
	}
    }
  return node;
}

/* }}} */

/** {{{ insert_before_first ()

    Insert statement T before the first statement of the compound statement
    BODY.  */

static void
insert_before_first (t, body)
     tree t;
     tree body;
{
  tree first_stmt;

  if (TREE_CODE (body) != COMPOUND_STMT)
    abort ();

  if (t == NULL)
    return;

  first_stmt = COMPOUND_BODY (body);
  chainon (t, TREE_CHAIN (first_stmt));
  TREE_CHAIN (first_stmt) = t;
}

/* }}} */


/* Miscellaneous helpers.  */

/** {{{ create_tmp_var ()

   Create a new temporary variable declaration of type TYPE.  Returns the
   newly created decl and pushes it into the current binding.  */

tree
create_tmp_var (type)
     tree type;
{
  static unsigned int id_num = 1;
  char *tmp_name;
  tree tmp_var;

  ASM_FORMAT_PRIVATE_NAME (tmp_name, "T", id_num++);

  /* If the type is an array, use TYPE_POINTER_TO to create a valid pointer
     that can be used in the LHS of an assignment.  */
  if (TREE_CODE (type) == ARRAY_TYPE)
    type = TYPE_POINTER_TO (TREE_TYPE (type));

  tmp_var = build_decl (VAR_DECL, get_identifier (tmp_name), type);

  /* The variable was declared by the compiler.  */
  DECL_ARTIFICIAL (tmp_var) = 1;

  /* Make the variable writable.  */
  TREE_READONLY (tmp_var) = 0;

  /* Make the type of the variable writable.  */
  make_type_writable (tmp_var);

  DECL_EXTERNAL (tmp_var) = 0;
  TREE_STATIC (tmp_var) = 0;
  TREE_USED (tmp_var) = 1;

  pushdecl (tmp_var);

  return tmp_var;
}

/* }}} */

/** {{{ make_type_writable ()

    Change the flags for the type of the node T to make it writtable.  */

static void 
make_type_writable (t)
     tree t;
{
  if (t == NULL_TREE)
    abort ();

  if (TYPE_READONLY (TREE_TYPE (t))
      || ((TREE_CODE (TREE_TYPE (t)) == RECORD_TYPE
	   || TREE_CODE (TREE_TYPE (t)) == UNION_TYPE)
	  && C_TYPE_FIELDS_READONLY (TREE_TYPE (t))))
    {
      /* Make a copy of the type declaration.  */
      TREE_TYPE (t) = build_type_copy (TREE_TYPE (t));
      TYPE_READONLY (TREE_TYPE (t)) = 0;
      
      /* If the type is a structure that contains a field readonly.  */
      if ((TREE_CODE (TREE_TYPE (t)) == RECORD_TYPE
	   || TREE_CODE (TREE_TYPE (t)) == UNION_TYPE)
	  && C_TYPE_FIELDS_READONLY (TREE_TYPE (t)))
	{
	  C_TYPE_FIELDS_READONLY (TREE_TYPE (t)) = 0;

	  /* Make the fields of the structure writable.  */
	  {
	    tree it;
	    it = TYPE_FIELDS (TREE_TYPE (t));
	    while (it)
	      {
		/* Make the field writable.  */
		TREE_READONLY (it) = 0;
		
		/* Make the type of the field writable.  */
		make_type_writable (it);
		it = TREE_CHAIN (it);
	      }
	  }
	}
    }
}

/* }}} */

/** {{{ declare_tmp_vars ()

    Declares all the variables in VARS in SCOPE.  Returns the last DECL_STMT
    emitted.  */

tree
declare_tmp_vars (vars, scope)
     tree vars;
     tree scope;
{
  tree t, last;

  /* Find the last declaration statement in the scope.  Add all the new
     declarations after it.  */
  last = tree_last_decl (scope);

  for (t = vars; t; t = TREE_CHAIN (t))
    {
      tree decl, tmp;

      decl = build_stmt (DECL_STMT, t);
      STMT_LINENO (decl) = STMT_LINENO (scope);

      tmp = TREE_CHAIN (last);
      TREE_CHAIN (last) = decl;
      TREE_CHAIN (decl) = tmp;

      last = decl;
    }

  return last;
}

/* }}} */

/** {{{ tree_last_decl ()
    
    Returns the last DECL_STMT in the scope SCOPE.  */

static tree
tree_last_decl (scope)
     tree scope;
{
  tree last;

  /* Be sure that we get a scope.  Ignore FILE_STMT nodes.  */
  while (TREE_CODE (scope) == FILE_STMT)
    scope = TREE_CHAIN (scope);

  if (!SCOPE_BEGIN_P (scope))
    abort ();

  /* Find the last declaration statement in the scope.  */
  last = scope;
  while (TREE_CHAIN (last) && TREE_CODE (TREE_CHAIN (last)) == DECL_STMT)
    last = TREE_CHAIN (last);

  return last;
}

/* }}} */

/** {{{ deep_copy_list ()
    
    Copy every statement from the chain CHAIN by calling deep_copy_node().
    Return the new chain.  */

tree
deep_copy_list (chain)
     tree chain;
{
  tree new_chain, res;

  if (chain == NULL_TREE)
    /* Nothing to copy.  */
    return NULL_TREE;
  
  new_chain = deep_copy_node (chain);
  res = new_chain;

  while (TREE_CHAIN (chain))
    {
      chain = TREE_CHAIN (chain);
      TREE_CHAIN (new_chain) = deep_copy_node (chain);
      new_chain = TREE_CHAIN (new_chain);
    }

  return res;
}

/* }}} */

/** {{{ deep_copy_node ()

    Create a deep copy of NODE.  The only nodes that are not deep copied
    are declarations, constants and types.  */

tree 
deep_copy_node (node)
     tree node;
{
  tree res;

  if (node == NULL_TREE)
    return NULL_TREE;

  switch (TREE_CODE (node))
    {
    case COMPOUND_STMT:
      res = build_stmt (COMPOUND_STMT, deep_copy_list (COMPOUND_BODY (node)));
      break;

    case FOR_STMT:
      res = build_stmt (FOR_STMT, 
			deep_copy_node (FOR_INIT_STMT (node)),
			deep_copy_node (FOR_COND (node)),
			deep_copy_node (FOR_EXPR (node)),
			deep_copy_node (FOR_BODY (node)));
      break;

    case WHILE_STMT:
      res = build_stmt (WHILE_STMT, 
			deep_copy_node (WHILE_COND (node)),
			deep_copy_node (WHILE_BODY (node)));
      break;

    case DO_STMT:
      res = build_stmt (DO_STMT, 
			deep_copy_node (DO_COND (node)),
			deep_copy_node (DO_BODY (node)));
      break;

    case IF_STMT:
      res = build_stmt (IF_STMT, 
			deep_copy_node (IF_COND (node)),
			deep_copy_node (THEN_CLAUSE (node)),
			deep_copy_node (ELSE_CLAUSE (node)));
      break;

    case SWITCH_STMT:
      res = build_stmt (SWITCH_STMT,
			deep_copy_node (SWITCH_COND (node)),
			deep_copy_node (SWITCH_BODY (node)));
      break;

    case EXPR_STMT:
      res = build_stmt (EXPR_STMT, deep_copy_node (EXPR_STMT_EXPR (node)));
      break;

    case DECL_STMT:
      res = build_stmt (DECL_STMT, DECL_STMT_DECL (node));
      break;

    case RETURN_STMT:
      res = build_stmt (RETURN_STMT, deep_copy_node (RETURN_EXPR (node)));
      break;

    case TREE_LIST:
      res = build_tree_list (deep_copy_node (TREE_PURPOSE (node)),
	                     deep_copy_node (TREE_VALUE (node)));
      break;

    case SCOPE_STMT:
      if (SCOPE_BEGIN_P (node))
	{
	  /* ??? The sub-blocks and supercontext for the scope's BLOCK_VARS
		 should be re-computed after copying.  */
	  res = build_stmt (SCOPE_STMT,
			    deep_copy_list (SCOPE_STMT_BLOCK (node)));
	  SCOPE_BEGIN_P (res) = 1;
	}
      else 
	{
	  res = build_stmt (SCOPE_STMT, NULL_TREE);
	  SCOPE_BEGIN_P (res) = 0;
	}
      break;

    default:
      walk_tree (&node, copy_tree_r, NULL, NULL);
      res = node;
      break;
    }
  
  /* Set the line number.  */
  if (statement_code_p (TREE_CODE (node)))
    STMT_LINENO (res) = STMT_LINENO (node);
  
  return res;
}

/* }}} */

/** {{{ update_line_number ()

    Updates the STMT_LINENO of each stmt in the tree t to the line number
    LINE.  Returns the last stmt in the tree chain.  */

tree
update_line_number (t, line)
     tree t;
     int line;
{
  if (t == NULL_TREE)
    return NULL_TREE;

  for (; TREE_CHAIN (t); t = TREE_CHAIN (t))
    STMT_LINENO (t) = line;
  STMT_LINENO (t) = line;
  return t;
}

/* }}} */

/** {{{ convert_to_stmt_chain ()

    Convert the list of expressions LIST into a list of statements.  Each
    statement in the new list gets line number information from STMT.  */

static tree
convert_to_stmt_chain (list, stmt)
     tree list;
     tree stmt;
{
  tree op, stmt_list;

  stmt_list = NULL;
  for (op = list; op; op = TREE_CHAIN (op))
    {
      tree t, n;
      
      /* Only create a new statement for expression trees.  */
      t = TREE_VALUE (op);
      n = (statement_code_p (TREE_CODE (t))) ? t : build_stmt (EXPR_STMT, t);
      STMT_LINENO (n) = STMT_LINENO (stmt);

      /* Only add statements that have an effect.  */
      if (stmt_has_effect (n))
	stmt_list = chainon (stmt_list, n);
    }

  return stmt_list;
}

/* }}} */

/** {{{ stmt_has_effect ()

    Return nonzero if STMT has some effect (i.e., if it's not of the form
    'a;' where a is a non-volatile variable).  */
    
static int
stmt_has_effect (stmt)
     tree stmt;
{
  if (TREE_CODE (stmt) != EXPR_STMT)
    return 1;
  else if (expr_has_effect (EXPR_STMT_EXPR (stmt)))
    return 1;
  else
    {
      /* The statement has no effect.  However, if we are simplifying a
	 statement expression '({ ... })' and this statement may be the
	 last statement in the statement expression body, then it may
	 represent the return value of the statement expression.  */
      if (stmt_expr_level > 0
	  && TREE_CHAIN (stmt)
	  && TREE_CODE (TREE_CHAIN (stmt)) == SCOPE_STMT
	  && SCOPE_END_P (TREE_CHAIN (stmt)))
	return 1;
    }

  return 0;
}

/* }}} */

/** {{{ expr_has_effect ()

    Return nonzero if EXPR has some effect (e.g., it's not a single
    non-volatile VAR_DECL).  */

static int
expr_has_effect (expr)
     tree expr;
{
  return (TREE_SIDE_EFFECTS (expr)
	  || (TREE_CODE (expr) == CONVERT_EXPR
	      && VOID_TYPE_P (TREE_TYPE (expr))));
}

/* }}} */

/** {{{ mostly_copy_tree_r ()

    Similar to copy_tree_r() but do not copy SAVE_EXPR nor STMT_EXPR nodes.  */

static tree
mostly_copy_tree_r (tp, walk_subtrees, data)
     tree *tp;
     int *walk_subtrees;
     void *data;
{
  if (TREE_CODE (*tp) == SAVE_EXPR
      || TREE_CODE (*tp) == STMT_EXPR)
    *walk_subtrees = 0;
  else
    copy_tree_r (tp, walk_subtrees, data);

  return NULL_TREE;
}

/* }}} */
