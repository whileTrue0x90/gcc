/* Language-specific hook definitions for C front end.
   Copyright (C) 1991, 1995, 1997, 1998 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#include "config.h"
#include "system.h"
#include "tree.h"
#include "input.h"
#include "c-tree.h"
#include "ggc.h"

/* Each of the functions defined here
   is an alternative to a function in objc-actions.c.  */
   
int
lang_decode_option (p)
     char *p;
{
  return c_decode_option (p);
}

void
lang_init ()
{
#if !USE_CPPLIB
  /* the beginning of the file is a new line; check for # */
  /* With luck, we discover the real source file's name from that
     and put it in input_filename.  */
  ungetc (check_newline (), finput);
#endif
}

void
lang_finish ()
{
}

char *
lang_identify ()
{
  return "c";
}

void
print_lang_statistics ()
{
}

/* used by print-tree.c */

void
lang_print_xnode (file, node, indent)
     FILE *file;
     tree node;
     int indent;
{
}

/* Used by c-lex.c, but only for objc.  */

tree
lookup_interface (arg)
     tree arg;
{
  return 0;
}

tree
is_class_name (arg)
    tree arg;
{
  return 0;
}

void
maybe_objc_check_decl (decl)
     tree decl;
{
}

int
maybe_objc_comptypes (lhs, rhs, reflexive)
     tree lhs, rhs;
     int reflexive;
{
  return -1;
}

tree
maybe_objc_method_name (decl)
    tree decl;
{
  return 0;
}

tree
maybe_building_objc_message_expr ()
{
  return 0;
}

int
recognize_objc_keyword ()
{
  return 0;
}

tree
build_objc_string (len, str)
    int len;
    char *str;
{
  abort ();
  return NULL_TREE;
}

void
GNU_xref_begin ()
{
  fatal ("GCC does not yet support XREF");
}

void
GNU_xref_end ()
{
  fatal ("GCC does not yet support XREF");
}

/* Called at end of parsing, but before end-of-file processing.  */

void
finish_file ()
{
#ifndef ASM_OUTPUT_CONSTRUCTOR
  extern tree static_ctors;
#endif
#ifndef ASM_OUTPUT_DESTRUCTOR
  extern tree static_dtors;
#endif
  extern tree build_function_call                 PROTO((tree, tree));
#if !defined(ASM_OUTPUT_CONSTRUCTOR) || !defined(ASM_OUTPUT_DESTRUCTOR)
  tree void_list_node = build_tree_list (NULL_TREE, void_type_node);
#endif
#ifndef ASM_OUTPUT_CONSTRUCTOR
  if (static_ctors)
    {
      tree fnname = get_file_function_name ('I');
      start_function (void_list_node,
		      build_parse_node (CALL_EXPR, fnname, void_list_node,
					NULL_TREE),
		      NULL_TREE, NULL_TREE, 0);
      fnname = DECL_ASSEMBLER_NAME (current_function_decl);
      store_parm_decls ();

      for (; static_ctors; static_ctors = TREE_CHAIN (static_ctors))
	expand_expr_stmt (build_function_call (TREE_VALUE (static_ctors),
					       NULL_TREE));

      finish_function (0);

      assemble_constructor (IDENTIFIER_POINTER (fnname));
    }
#endif
#ifndef ASM_OUTPUT_DESTRUCTOR
  if (static_dtors)
    {
      tree fnname = get_file_function_name ('D');
      start_function (void_list_node,
		      build_parse_node (CALL_EXPR, fnname, void_list_node,
					NULL_TREE),
		      NULL_TREE, NULL_TREE, 0);
      fnname = DECL_ASSEMBLER_NAME (current_function_decl);
      store_parm_decls ();

      for (; static_dtors; static_dtors = TREE_CHAIN (static_dtors))
	expand_expr_stmt (build_function_call (TREE_VALUE (static_dtors),
					       NULL_TREE));

      finish_function (0);

      assemble_destructor (IDENTIFIER_POINTER (fnname));
    }
#endif
}

void
lang_mark_false_label_stack (arg)
     void *arg;
{
  /* C doesn't use false_label_stack.  It better be NULL.  */
  if (*(void **)arg != NULL)
    abort();
}

void lang_mark_tree (t)
     tree t;
{
  if (TREE_CODE (t) == IDENTIFIER_NODE)
    {
      struct lang_identifier *i = (struct lang_identifier *) t;
      ggc_mark_tree (i->global_value);
      ggc_mark_tree (i->local_value);
      ggc_mark_tree (i->label_value);
      ggc_mark_tree (i->implicit_decl);
      ggc_mark_tree (i->error_locus);
      ggc_mark_tree (i->limbo_value);
    }
}

void lang_cleanup_tree (t)
     tree t;
{
  if (TREE_CODE_CLASS (TREE_CODE (t)) == 't'
      && TYPE_LANG_SPECIFIC (t) != NULL)
    {
#if 0
      /* This is currently allocated with an obstack.  This will change.  */
      free (TYPE_LANG_SPECIFIC (t));
#endif
    }
}
