/* Declaration statement matcher
   Copyright (C) 2002 Free Software Foundation, Inc.
   Contributed by Andy Vaught

This file is part of GNU G95.

GNU G95 is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU G95 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU G95; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


#include "config.h"
#include "gfortran.h"
#include "match.h"
#include "parse.h"
#include <string.h>


/* This flag is set if a an old-style length selector is matched
   during an type-declaration statement. */

static int old_char_selector;

/* When variables aquire types and attributes from a declaration
   statement, they get them from the following static variables.  The
   first part of a declaration sets these variables and the second
   part copies these into symbol structures.  */

static gfc_typespec current_ts;

static symbol_attribute current_attr;
static gfc_array_spec *current_as;
static int colon_seen;

/* gfc_new_block points to the symbol of a newly matched block.  */

gfc_symbol *gfc_new_block;


/* Match an intent specification.  Since this can only happen after an
   INTENT word, a legal intent-spec must follow.  */

static sym_intent
match_intent_spec (void)
{

  if (gfc_match (" ( in out )") == MATCH_YES)
    return INTENT_INOUT;
  if (gfc_match (" ( in )") == MATCH_YES)
    return INTENT_IN;
  if (gfc_match (" ( out )") == MATCH_YES)
    return INTENT_OUT;

  gfc_error ("Bad INTENT specification at %C");
  return INTENT_UNKNOWN;
}


/* Matches a character length specification, which is either a
   specification expression or a '*'.  */

static match
char_len_param_value (gfc_expr ** expr)
{

  if (gfc_match_char ('*') == MATCH_YES)
    {
      *expr = NULL;
      return MATCH_YES;
    }

  return gfc_match_expr (expr);
}


/* A character length is a '*' followed by a literal integer or a
   char_len_param_value in parenthesis.  */

static match
match_char_length (gfc_expr ** expr)
{
  int length;
  match m;

  m = gfc_match_char ('*');
  if (m != MATCH_YES)
    return m;

  m = gfc_match_small_literal_int (&length);
  if (m == MATCH_ERROR)
    return m;

  if (m == MATCH_YES)
    {
      *expr = gfc_int_expr (length);
      return m;
    }

  if (gfc_match_char ('(') == MATCH_NO)
    goto syntax;

  m = char_len_param_value (expr);
  if (m == MATCH_ERROR)
    return m;
  if (m == MATCH_NO)
    goto syntax;

  if (gfc_match_char (')') == MATCH_NO)
    {
      gfc_free_expr (*expr);
      *expr = NULL;
      goto syntax;
    }

  return MATCH_YES;

syntax:
  gfc_error ("Syntax error in character length specification at %C");
  return MATCH_ERROR;
}


/* Special subroutine for finding a symbol.  If we're compiling a
   function or subroutine and the parent compilation unit is an
   interface, then check to see if the name we've been given is the
   name of the interface (located in another namespace).  If so,
   return that symbol.  If not, use gfc_get_symbol().  */

static int
find_special (const char *name, gfc_symbol ** result)
{
  gfc_state_data *s;

  if (gfc_current_state () != COMP_SUBROUTINE
      && gfc_current_state () != COMP_FUNCTION)
    goto normal;

  s = gfc_state_stack->previous;
  if (s == NULL)
    goto normal;

  if (s->state != COMP_INTERFACE)
    goto normal;
  if (s->sym == NULL)
    goto normal;		/* Nameless interface */

  if (strcmp (name, s->sym->name) == 0)
    {
      *result = s->sym;
      return 0;
    }

normal:
  return gfc_get_symbol (name, NULL, result);
}


/* Special subroutine for getting a symbol node associated with a
   procedure name, used in SUBROUTINE and FUNCTION statements.  The
   symbol is created in the parent using with symtree node in the
   child unit pointing to the symbol.  If the current namespace has no
   parent, then the symbol is just created in the current unit.  */

static int
get_proc_name (const char *name, gfc_symbol ** result)
{
  gfc_symtree *st;
  gfc_symbol *sym;
  int rc;

  if (gfc_current_ns->parent == NULL)
    return gfc_get_symbol (name, NULL, result);

  rc = gfc_get_symbol (name, gfc_current_ns->parent, result);
  if (*result == NULL)
    return rc;

  /* Deal with ENTRY problem */

  st = gfc_new_symtree (&gfc_current_ns->sym_root, name);

  sym = *result;
  st->n.sym = sym;
  sym->refs++;

  /* See if the procedure should be a module procedure */

  if (sym->ns->proc_name != NULL
      && sym->ns->proc_name->attr.flavor == FL_MODULE
      && sym->attr.proc != PROC_MODULE
      && gfc_add_procedure (&sym->attr, PROC_MODULE, NULL) == FAILURE)
    rc = 2;

  return rc;
}


/* Function called by variable_decl() that adds a name to the symbol
   table.  */

static try
build_sym (const char *name, gfc_charlen * cl,
	   gfc_array_spec ** as, locus * var_locus)
{
  symbol_attribute attr;
  gfc_symbol *sym;

  if (find_special (name, &sym))
    return FAILURE;

  /* Start updating the symbol table.  Add basic type attribute
     if present.  */
  if (current_ts.type != BT_UNKNOWN
      &&(sym->attr.implicit_type == 0
	 || !gfc_compare_types (&sym->ts, &current_ts))
      && gfc_add_type (sym, &current_ts, var_locus) == FAILURE)
    return FAILURE;

  if (sym->ts.type == BT_CHARACTER)
    sym->ts.cl = cl;

  /* Add dimension attribute if present.  */
  if (gfc_set_array_spec (sym, *as, var_locus) == FAILURE)
    return FAILURE;
  *as = NULL;

  /* Add attribute to symbol.  The copy is so that we can reset the
     dimension attribute.  */
  attr = current_attr;
  attr.dimension = 0;

  if (gfc_copy_attr (&sym->attr, &attr, var_locus) == FAILURE)
    return FAILURE;

  return SUCCESS;
}


/* Function called by variable_decl() that adds an initialization
   expression to a symbol.  */

static try
add_init_expr_to_sym (const char *name, gfc_expr ** initp,
		      locus * var_locus)
{
  symbol_attribute attr;
  gfc_symbol *sym;
  gfc_expr *init;

  init = *initp;
  if (find_special (name, &sym))
    return FAILURE;

  attr = sym->attr;

  /* If this symbol is confirming an implicit parameter type,
     then an initialization expression is not allowed.  */
  if (attr.flavor == FL_PARAMETER
      && sym->value != NULL
      && *initp != NULL)
    {
      gfc_error ("Initializer not allowed for PARAMETER '%s' at %C",
		 sym->name);
      return FAILURE;
    }

  if (init == NULL)
    {
      /* An initializer is required for PARAMETER declarations.  */
      if (attr.flavor == FL_PARAMETER)
	{
	  gfc_error ("PARAMETER at %L is missing an initializer", var_locus);
	  return FAILURE;
	}
    }
  else
    {
      /* If a variable appears in a DATA block, it cannot have an
         initializer.  */
      if (sym->attr.data)
	{
	  gfc_error
	    ("Variable '%s' at %C with an initializer already appears "
	     "in a DATA statement", sym->name);
	  return FAILURE;
	}

      /* Checking a derived type parameter has to be put off until later.  */
      if (sym->ts.type != BT_DERIVED && init->ts.type != BT_DERIVED
	  && gfc_check_assign_symbol (sym, init) == FAILURE)
	return FAILURE;

      /* Add initializer.  Make sure we keep the ranks sane.  */
      if (sym->attr.dimension && init->rank == 0)
	init->rank = sym->as->rank;

      sym->value = init;
      *initp = NULL;
    }

  return SUCCESS;
}


/* Function called by variable_decl() that adds a name to a structure
   being built.  */

static try
build_struct (const char *name, gfc_charlen * cl, gfc_expr ** init,
	      gfc_array_spec ** as)
{
  gfc_component *c;

  /* If the current symbol is of the same derived type that we're
     constructing, it must have the pointer attribute.  */
  if (current_ts.type == BT_DERIVED
      && current_ts.derived == gfc_current_block ()
      && current_attr.pointer == 0)
    {
      gfc_error ("Component at %C must have the POINTER attribute");
      return FAILURE;
    }

  if (gfc_current_block ()->attr.pointer
      && (*as)->rank != 0)
    {
      if ((*as)->type != AS_DEFERRED && (*as)->type != AS_EXPLICIT)
	{
	  gfc_error ("Array component of structure at %C must have explicit "
		     "or deferred shape");
	  return FAILURE;
	}
    }

  if (gfc_add_component (gfc_current_block (), name, &c) == FAILURE)
    return FAILURE;

  c->ts = current_ts;
  c->ts.cl = cl;
  gfc_set_component_attr (c, &current_attr);

  c->initializer = *init;
  *init = NULL;

  c->as = *as;
  if (c->as != NULL)
    c->dimension = 1;
  *as = NULL;

  /* Check array components.  */
  if (!c->dimension)
    return SUCCESS;

  if (c->pointer)
    {
      if (c->as->type != AS_DEFERRED)
	{
	  gfc_error ("Pointer array component of structure at %C "
		     "must have a deferred shape");
	  return FAILURE;
	}
    }
  else
    {
      if (c->as->type != AS_EXPLICIT)
	{
	  gfc_error
	    ("Array component of structure at %C must have an explicit "
	     "shape");
	  return FAILURE;
	}
    }

  return SUCCESS;
}


/* Match a 'NULL()', and possibly take care of some side effects.  */

match
gfc_match_null (gfc_expr ** result)
{
  gfc_symbol *sym;
  gfc_expr *e;
  match m;

  m = gfc_match (" null ( )");
  if (m != MATCH_YES)
    return m;

  /* The NULL symbol now has to be/become an intrinsic function.  */
  if (gfc_get_symbol ("null", NULL, &sym))
    {
      gfc_error ("NULL() initialization at %C is ambiguous");
      return MATCH_ERROR;
    }

  gfc_intrinsic_symbol (sym);

  if (sym->attr.proc != PROC_INTRINSIC
      && (gfc_add_procedure (&sym->attr, PROC_INTRINSIC, NULL) == FAILURE
	  || gfc_add_function (&sym->attr, NULL) == FAILURE))
    return MATCH_ERROR;

  e = gfc_get_expr ();
  e->where = *gfc_current_locus ();
  e->expr_type = EXPR_NULL;
  e->ts.type = BT_UNKNOWN;

  *result = e;

  return MATCH_YES;
}


/* Get an expression for a default initializer.  */
static gfc_expr *
default_initializer (void)
{
  gfc_constructor *tail;
  gfc_expr *init;
  gfc_component *c;

  init = NULL;

  /* First see if we have a default initializer.  */
  for (c = current_ts.derived->components; c; c = c->next)
    {
      if (c->initializer && init == NULL)
        init = gfc_get_expr ();
    }

  if (init == NULL)
    return NULL;

  init->expr_type = EXPR_STRUCTURE;
  init->ts = current_ts;
  init->where = current_ts.derived->declared_at;
  tail = NULL;
  for (c = current_ts.derived->components; c; c = c->next)
    {
      if (tail == NULL)
        init->value.constructor = tail = gfc_get_constructor ();
      else
        {
          tail->next = gfc_get_constructor ();
          tail = tail->next;
        }

      if (c->initializer)
        tail->expr = gfc_copy_expr (c->initializer);
    }
  return init;
}


/* Match a variable name with an optional initializer.  When this
   subroutine is called, a variable is expected to be parsed next.
   Depending on what is happening at the moment, updates either the
   symbol table or the current interface.  */

static match
variable_decl (void)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_expr *initializer, *char_len;
  gfc_array_spec *as;
  gfc_charlen *cl;
  locus var_locus;
  match m;
  try t;

  initializer = NULL;
  as = NULL;

  /* When we get here, we've just matched a list of attributes and
     maybe a type and a double colon.  The next thing we expect to see
     is the name of the symbol.  */
  m = gfc_match_name (name);
  if (m != MATCH_YES)
    goto cleanup;

  var_locus = *gfc_current_locus ();

  /* Now we could see the optional array spec. or character length.  */
  m = gfc_match_array_spec (&as);
  if (m == MATCH_ERROR)
    goto cleanup;
  if (m == MATCH_NO)
    as = gfc_copy_array_spec (current_as);

  char_len = NULL;
  cl = NULL;

  if (current_ts.type == BT_CHARACTER)
    {
      switch (match_char_length (&char_len))
	{
	case MATCH_YES:
	  cl = gfc_get_charlen ();
	  cl->next = gfc_current_ns->cl_list;
	  gfc_current_ns->cl_list = cl;

	  cl->length = char_len;
	  break;

	case MATCH_NO:
	  cl = current_ts.cl;
	  break;

	case MATCH_ERROR:
	  goto cleanup;
	}
    }

  /* OK, we've successfully matched the declaration.  Now put the
     symbol in the current namespace, because it might be used in the
     optional intialization expression for this symbol, e.g. this is
     perfectly legal:

     integer, parameter :: i = huge(i)

     This is only true for parameters or variables of a basic type.
     For components of derived types, it is not true, so we don't
     create a symbol for those yet.  If we fail to create the symbol,
     bail out.  */
  if (gfc_current_state () != COMP_DERIVED
      && build_sym (name, cl, &as, &var_locus) == FAILURE)
    {
      m = MATCH_ERROR;
      goto cleanup;
    }

  /* In functions that have a RESULT variable defined, the function
     name always refers to function calls.  Therefore, the name is
     not allowed to appear in specification statements.  */
  if (gfc_current_state () == COMP_FUNCTION
      && gfc_current_block () != NULL
      && gfc_current_block ()->result != NULL
      && gfc_current_block ()->result != gfc_current_block ()
      && strcmp (gfc_current_block ()->name, name) == 0)
    {
      gfc_error ("Function name '%s' not allowed at %C", name);
      m = MATCH_ERROR;
      goto cleanup;
    }

  /* The double colon must be present in order to have initializers.
     Otherwise the statement is ambiguous with an assignment statement.  */
  if (colon_seen)
    {
      if (gfc_match (" =>") == MATCH_YES)
	{

	  if (!current_attr.pointer)
	    {
	      gfc_error ("Initialization at %C isn't for a pointer variable");
	      m = MATCH_ERROR;
	      goto cleanup;
	    }

	  m = gfc_match_null (&initializer);
	  if (m == MATCH_NO)
	    {
	      gfc_error ("Pointer initialization requires a NULL at %C");
	      m = MATCH_ERROR;
	    }

	  if (gfc_pure (NULL))
	    {
	      gfc_error
		("Initialization of pointer at %C is not allowed in a "
		 "PURE procedure");
	      m = MATCH_ERROR;
	    }

	  if (m != MATCH_YES)
	    goto cleanup;

	  initializer->ts = current_ts;

	}
      else if (gfc_match_char ('=') == MATCH_YES)
	{
	  if (current_attr.pointer)
	    {
	      gfc_error
		("Pointer initialization at %C requires '=>', not '='");
	      m = MATCH_ERROR;
	      goto cleanup;
	    }

	  m = gfc_match_init_expr (&initializer);
	  if (m == MATCH_NO)
	    {
	      gfc_error ("Expected an initialization expression at %C");
	      m = MATCH_ERROR;
	    }

	  if (current_attr.flavor != FL_PARAMETER && gfc_pure (NULL))
	    {
	      gfc_error
		("Initialization of variable at %C is not allowed in a "
		 "PURE procedure");
	      m = MATCH_ERROR;
	    }

	  if (m != MATCH_YES)
	    goto cleanup;
	}
      else if (current_ts.type == BT_DERIVED)
        {
          initializer = default_initializer ();
        }
    }

  /* Add the initializer.  Note that it is fine if &initializer is
     NULL here, because we sometimes also need to check if a
     declaration *must* have an initialization expression.  */
  if (gfc_current_state () != COMP_DERIVED)
    t = add_init_expr_to_sym (name, &initializer, &var_locus);
  else
    t = build_struct (name, cl, &initializer, &as);

  m = (t == SUCCESS) ? MATCH_YES : MATCH_ERROR;

cleanup:
  /* Free stuff up and return.  */
  gfc_free_expr (initializer);
  gfc_free_array_spec (as);

  return m;
}


/* Match an extended-f77 kind specification.  */

match
gfc_match_old_kind_spec (gfc_typespec * ts)
{
  match m;

  if (gfc_match_char ('*') != MATCH_YES)
    return MATCH_NO;

  m = gfc_match_small_literal_int (&ts->kind);
  if (m != MATCH_YES)
    return MATCH_ERROR;

  /* Massage the kind numbers for complex types.  */
  if (ts->type == BT_COMPLEX && ts->kind == 8)
    ts->kind = 4;
  if (ts->type == BT_COMPLEX && ts->kind == 16)
    ts->kind = 8;

  if (gfc_validate_kind (ts->type, ts->kind) == -1)
    {
      gfc_error ("Old-style kind %d not supported for type %s at %C",
		 ts->kind, gfc_basic_typename (ts->type));

      return MATCH_ERROR;
    }

  return MATCH_YES;
}


/* Match a kind specification.  Since kinds are generally optional, we
   usually return MATCH_NO if something goes wrong.  If a "kind="
   string is found, then we know we have an error.  */

match
gfc_match_kind_spec (gfc_typespec * ts)
{
  locus where;
  gfc_expr *e;
  match m, n;
  const char *msg;

  m = MATCH_NO;
  e = NULL;

  where = *gfc_current_locus ();

  if (gfc_match_char ('(') == MATCH_NO)
    return MATCH_NO;

  /* Also gobbles optional text.  */
  if (gfc_match (" kind = ") == MATCH_YES)
    m = MATCH_ERROR;

  n = gfc_match_init_expr (&e);
  if (n == MATCH_NO)
    gfc_error ("Expected initialization expression at %C");
  if (n != MATCH_YES)
    return MATCH_ERROR;

  if (e->rank != 0)
    {
      gfc_error ("Expected scalar initialization expression at %C");
      m = MATCH_ERROR;
      goto no_match;
    }

  msg = gfc_extract_int (e, &ts->kind);
  if (msg != NULL)
    {
      gfc_error (msg);
      m = MATCH_ERROR;
      goto no_match;
    }

  gfc_free_expr (e);
  e = NULL;

  if (gfc_validate_kind (ts->type, ts->kind) == -1)
    {
      gfc_error ("Kind %d not supported for type %s at %C", ts->kind,
		 gfc_basic_typename (ts->type));

      m = MATCH_ERROR;
      goto no_match;
    }

  if (gfc_match_char (')') != MATCH_YES)
    {
      gfc_error ("Missing right paren at %C");
      goto no_match;
    }

  return MATCH_YES;

no_match:
  gfc_free_expr (e);
  gfc_set_locus (&where);
  return m;
}


/* Match the various kind/length specifications in a CHARACTER
   declaration.  We don't return MATCH_NO.  */

static match
match_char_spec (gfc_typespec * ts)
{
  int i, kind, seen_length;
  gfc_charlen *cl;
  gfc_expr *len;
  match m;

  kind = gfc_default_character_kind ();
  len = NULL;
  seen_length = 0;

  /* Try the old-style specification first.  */
  old_char_selector = 0;

  m = match_char_length (&len);
  if (m != MATCH_NO)
    {
      if (m == MATCH_YES)
	old_char_selector = 1;
      seen_length = 1;
      goto done;
    }

  m = gfc_match_char ('(');
  if (m != MATCH_YES)
    {
      m = MATCH_YES;	/* character without length is a single char */
      goto done;
    }

  /* Try the weird case:  ( KIND = <int> [ , LEN = <len-param> ] )   */
  if (gfc_match (" kind =") == MATCH_YES)
    {
      m = gfc_match_small_int (&kind);
      if (m == MATCH_ERROR)
	goto done;
      if (m == MATCH_NO)
	goto syntax;

      if (gfc_match (" , len =") == MATCH_NO)
	goto rparen;

      m = char_len_param_value (&len);
      if (m == MATCH_NO)
	goto syntax;
      if (m == MATCH_ERROR)
	goto done;
      seen_length = 1;

      goto rparen;
    }

  /* Try to match ( LEN = <len-param> ) or ( LEN = <len-param>, KIND = <int> )  */
  if (gfc_match (" len =") == MATCH_YES)
    {
      m = char_len_param_value (&len);
      if (m == MATCH_NO)
	goto syntax;
      if (m == MATCH_ERROR)
	goto done;
      seen_length = 1;

      if (gfc_match_char (')') == MATCH_YES)
	goto done;

      if (gfc_match (" , kind =") != MATCH_YES)
	goto syntax;

      gfc_match_small_int (&kind);

      if (gfc_validate_kind (BT_CHARACTER, kind) == -1)
	{
	  gfc_error ("Kind %d is not a CHARACTER kind at %C", kind);
	  return MATCH_YES;
	}

      goto rparen;
    }

  /* Try to match   ( <len-param> ) or ( <len-param> , [ KIND = ] <int> )  */
  m = char_len_param_value (&len);
  if (m == MATCH_NO)
    goto syntax;
  if (m == MATCH_ERROR)
    goto done;
  seen_length = 1;

  m = gfc_match_char (')');
  if (m == MATCH_YES)
    goto done;

  if (gfc_match_char (',') != MATCH_YES)
    goto syntax;

  gfc_match (" kind =");	/* Gobble optional text */

  m = gfc_match_small_int (&kind);
  if (m == MATCH_ERROR)
    goto done;
  if (m == MATCH_NO)
    goto syntax;

rparen:
  /* Require a right-paren at this point.  */
  m = gfc_match_char (')');
  if (m == MATCH_YES)
    goto done;

syntax:
  gfc_error ("Syntax error in CHARACTER declaration at %C");
  m = MATCH_ERROR;

done:
  if (m == MATCH_YES && gfc_validate_kind (BT_CHARACTER, kind) == -1)
    {
      gfc_error ("Kind %d is not a CHARACTER kind at %C", kind);
      m = MATCH_ERROR;
    }

  if (m != MATCH_YES)
    {
      gfc_free_expr (len);
      return m;
    }

  /* Do some final massaging of the length values.  */
  cl = gfc_get_charlen ();
  cl->next = gfc_current_ns->cl_list;
  gfc_current_ns->cl_list = cl;

  if (seen_length == 0)
    cl->length = gfc_int_expr (1);
  else
    {
      if (len == NULL || gfc_extract_int (len, &i) != NULL || i >= 0)
	cl->length = len;
      else
	{
	  gfc_free_expr (len);
	  cl->length = gfc_int_expr (0);
	}
    }

  ts->cl = cl;
  ts->kind = kind;

  return MATCH_YES;
}


/* Matches a type specification.  If successful, sets the ts structure
   to the matched specification.  This is necessary for FUNCTION and
   IMPLICIT statements.

   If kind_flag is nonzero, then we check for the optional kind
   specification.  Not doing so is needed for matching an IMPLICIT
   statement correctly.  */

match
gfc_match_type_spec (gfc_typespec * ts, int kind_flag)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_symbol *sym;
  match m;

  gfc_clear_ts (ts);

  if (gfc_match (" integer") == MATCH_YES)
    {
      ts->type = BT_INTEGER;
      ts->kind = gfc_default_integer_kind ();
      goto get_kind;
    }

  if (gfc_match (" character") == MATCH_YES)
    {
      ts->type = BT_CHARACTER;
      return match_char_spec (ts);
    }

  if (gfc_match (" real") == MATCH_YES)
    {
      ts->type = BT_REAL;
      ts->kind = gfc_default_real_kind ();
      goto get_kind;
    }

  if (gfc_match (" double precision") == MATCH_YES)
    {
      ts->type = BT_REAL;
      ts->kind = gfc_default_double_kind ();
      return MATCH_YES;
    }

  if (gfc_match (" complex") == MATCH_YES)
    {
      ts->type = BT_COMPLEX;
      ts->kind = gfc_default_complex_kind ();
      goto get_kind;
    }

  if (gfc_match (" double complex") == MATCH_YES)
    {
      ts->type = BT_COMPLEX;
      ts->kind = gfc_default_double_kind ();
      return MATCH_YES;
    }

  if (gfc_match (" logical") == MATCH_YES)
    {
      ts->type = BT_LOGICAL;
      ts->kind = gfc_default_logical_kind ();
      goto get_kind;
    }

  m = gfc_match (" type ( %n )", name);
  if (m != MATCH_YES)
    return m;

  /* Search for the name but allow the components to be defined later.  */
  if (gfc_get_ha_symbol (name, &sym))
    {
      gfc_error ("Type name '%s' at %C is ambiguous", name);
      return MATCH_ERROR;
    }

  if (sym->attr.flavor != FL_DERIVED
      && gfc_add_flavor (&sym->attr, FL_DERIVED, NULL) == FAILURE)
    return MATCH_ERROR;

  ts->type = BT_DERIVED;
  ts->kind = 0;
  ts->derived = sym;

  return MATCH_YES;

get_kind:
  /* For all types except double, derived and character, look for an
     optional kind specifier.  MATCH_NO is actually OK at this point.  */
  if (kind_flag == 0)
    return MATCH_YES;

  m = gfc_match_kind_spec (ts);
  if (m == MATCH_NO && ts->type != BT_CHARACTER)
    m = gfc_match_old_kind_spec (ts);

  if (m == MATCH_NO)
    m = MATCH_YES;		/* No kind specifier found.  */

  return m;
}


/* Matches an attribute specification including array specs.  If
   successful, leaves the variables current_attr and current_as
   holding the specification.  Also sets the colon_seen variable for
   later use by matchers associated with initializations.

   This subroutine is a little tricky in the sense that we don't know
   if we really have an attr-spec until we hit the double colon.
   Until that time, we can only return MATCH_NO.  This forces us to
   check for duplicate specification at this level.  */

static match
match_attr_spec (void)
{

  /* Modifiers that can exist in a type statement.  */
  typedef enum
  { GFC_DECL_BEGIN = 0,
    DECL_ALLOCATABLE = GFC_DECL_BEGIN, DECL_DIMENSION, DECL_EXTERNAL,
    DECL_IN, DECL_OUT, DECL_INOUT, DECL_INTRINSIC, DECL_OPTIONAL,
    DECL_PARAMETER, DECL_POINTER, DECL_PRIVATE, DECL_PUBLIC, DECL_SAVE,
    DECL_TARGET, DECL_COLON, DECL_NONE,
    GFC_DECL_END /* Sentinel */
  }
  decl_types;

/* GFC_DECL_END is the sentinel, index starts at 0.  */
#define NUM_DECL GFC_DECL_END

  static mstring decls[] = {
    minit (", allocatable", DECL_ALLOCATABLE),
    minit (", dimension", DECL_DIMENSION),
    minit (", external", DECL_EXTERNAL),
    minit (", intent ( in )", DECL_IN),
    minit (", intent ( out )", DECL_OUT),
    minit (", intent ( in out )", DECL_INOUT),
    minit (", intrinsic", DECL_INTRINSIC),
    minit (", optional", DECL_OPTIONAL),
    minit (", parameter", DECL_PARAMETER),
    minit (", pointer", DECL_POINTER),
    minit (", private", DECL_PRIVATE),
    minit (", public", DECL_PUBLIC),
    minit (", save", DECL_SAVE),
    minit (", target", DECL_TARGET),
    minit ("::", DECL_COLON),
    minit (NULL, DECL_NONE)
  };

  locus start, seen_at[NUM_DECL];
  int seen[NUM_DECL];
  decl_types d;
  const char *attr;
  match m;
  try t;

  gfc_clear_attr (&current_attr);
  start = *gfc_current_locus ();

  current_as = NULL;
  colon_seen = 0;

  /* See if we get all of the keywords up to the final double colon.  */
  for (d = GFC_DECL_BEGIN; d != GFC_DECL_END; d++)
    seen[d] = 0;

  for (;;)
    {
      d = (decl_types) gfc_match_strings (decls);
      if (d == DECL_NONE || d == DECL_COLON)
	break;

      seen[d]++;
      seen_at[d] = *gfc_current_locus ();

      if (d == DECL_DIMENSION)
	{
	  m = gfc_match_array_spec (&current_as);

	  if (m == MATCH_NO)
	    {
	      gfc_error ("Missing dimension specification at %C");
	      m = MATCH_ERROR;
	    }

	  if (m == MATCH_ERROR)
	    goto cleanup;
	}
    }

  /* No double colon, so assume that we've been looking at something
     else the whole time.  */
  if (d == DECL_NONE)
    {
      m = MATCH_NO;
      goto cleanup;
    }

  /* Since we've seen a double colon, we have to be looking at an
     attr-spec.  This means that we can now issue errors.  */
  for (d = GFC_DECL_BEGIN; d != GFC_DECL_END; d++)
    if (seen[d] > 1)
      {
	switch (d)
	  {
	  case DECL_ALLOCATABLE:
	    attr = "ALLOCATABLE";
	    break;
	  case DECL_DIMENSION:
	    attr = "DIMENSION";
	    break;
	  case DECL_EXTERNAL:
	    attr = "EXTERNAL";
	    break;
	  case DECL_IN:
	    attr = "INTENT (IN)";
	    break;
	  case DECL_OUT:
	    attr = "INTENT (OUT)";
	    break;
	  case DECL_INOUT:
	    attr = "INTENT (IN OUT)";
	    break;
	  case DECL_INTRINSIC:
	    attr = "INTRINSIC";
	    break;
	  case DECL_OPTIONAL:
	    attr = "OPTIONAL";
	    break;
	  case DECL_PARAMETER:
	    attr = "PARAMETER";
	    break;
	  case DECL_POINTER:
	    attr = "POINTER";
	    break;
	  case DECL_PRIVATE:
	    attr = "PRIVATE";
	    break;
	  case DECL_PUBLIC:
	    attr = "PUBLIC";
	    break;
	  case DECL_SAVE:
	    attr = "SAVE";
	    break;
	  case DECL_TARGET:
	    attr = "TARGET";
	    break;
	  default:
	    attr = NULL;	/* This shouldn't happen */
	  }

	gfc_error ("Duplicate %s attribute at %L", attr, &seen_at[d]);
	m = MATCH_ERROR;
	goto cleanup;
      }

  /* Now that we've dealt with duplicate attributes, add the attributes
     to the current attribute.  */
  for (d = GFC_DECL_BEGIN; d != GFC_DECL_END; d++)
    {
      if (seen[d] == 0)
	continue;

      if (gfc_current_state () == COMP_DERIVED
	  && d != DECL_DIMENSION && d != DECL_POINTER
	  && d != DECL_COLON && d != DECL_NONE)
	{

	  gfc_error ("Attribute at %L is not allowed in a TYPE definition",
		     &seen_at[d]);
	  m = MATCH_ERROR;
	  goto cleanup;
	}

      switch (d)
	{
	case DECL_ALLOCATABLE:
	  t = gfc_add_allocatable (&current_attr, &seen_at[d]);
	  break;

	case DECL_DIMENSION:
	  t = gfc_add_dimension (&current_attr, &seen_at[d]);
	  break;

	case DECL_EXTERNAL:
	  t = gfc_add_external (&current_attr, &seen_at[d]);
	  break;

	case DECL_IN:
	  t = gfc_add_intent (&current_attr, INTENT_IN, &seen_at[d]);
	  break;

	case DECL_OUT:
	  t = gfc_add_intent (&current_attr, INTENT_OUT, &seen_at[d]);
	  break;

	case DECL_INOUT:
	  t = gfc_add_intent (&current_attr, INTENT_INOUT, &seen_at[d]);
	  break;

	case DECL_INTRINSIC:
	  t = gfc_add_intrinsic (&current_attr, &seen_at[d]);
	  break;

	case DECL_OPTIONAL:
	  t = gfc_add_optional (&current_attr, &seen_at[d]);
	  break;

	case DECL_PARAMETER:
	  t = gfc_add_flavor (&current_attr, FL_PARAMETER, &seen_at[d]);
	  break;

	case DECL_POINTER:
	  t = gfc_add_pointer (&current_attr, &seen_at[d]);
	  break;

	case DECL_PRIVATE:
	  t = gfc_add_access (&current_attr, ACCESS_PRIVATE, &seen_at[d]);
	  break;

	case DECL_PUBLIC:
	  t = gfc_add_access (&current_attr, ACCESS_PUBLIC, &seen_at[d]);
	  break;

	case DECL_SAVE:
	  t = gfc_add_save (&current_attr, &seen_at[d]);
	  break;

	case DECL_TARGET:
	  t = gfc_add_target (&current_attr, &seen_at[d]);
	  break;

	default:
	  gfc_internal_error ("match_attr_spec(): Bad attribute");
	}

      if (t == FAILURE)
	{
	  m = MATCH_ERROR;
	  goto cleanup;
	}
    }

  colon_seen = 1;
  return MATCH_YES;

cleanup:
  gfc_set_locus (&start);
  gfc_free_array_spec (current_as);
  current_as = NULL;
  return m;
}


/* Match a data declaration statement.  */

match
gfc_match_data_decl (void)
{
  gfc_symbol *sym;
  match m;

  m = gfc_match_type_spec (&current_ts, 1);
  if (m != MATCH_YES)
    return m;

  if (current_ts.type == BT_DERIVED && gfc_current_state () != COMP_DERIVED)
    {
      sym = gfc_use_derived (current_ts.derived);

      if (sym == NULL)
	{
	  m = MATCH_ERROR;
	  goto cleanup;
	}

      current_ts.derived = sym;
    }

  m = match_attr_spec ();
  if (m == MATCH_ERROR)
    {
      m = MATCH_NO;
      goto cleanup;
    }

  if (current_ts.type == BT_DERIVED && current_ts.derived->components == NULL)
    {

      if (current_attr.pointer && gfc_current_state () == COMP_DERIVED)
	goto ok;

      if (gfc_find_symbol (current_ts.derived->name,
			   current_ts.derived->ns->parent, 1, &sym) == 0)
	goto ok;

      /* Hope that an ambiguous symbol is itself masked by a type definition.  */
      if (sym != NULL && sym->attr.flavor == FL_DERIVED)
	goto ok;

      gfc_error ("Derived type at %C has not been previously defined");
      m = MATCH_ERROR;
      goto cleanup;
    }

ok:
  /* If we have an old-style character declaration, and no new-style
     attribute specifications, then there a comma is optional between
     the type specification and the variable list.  */
  if (m == MATCH_NO && current_ts.type == BT_CHARACTER && old_char_selector)
    gfc_match_char (',');

  /* Give the types/attributes to symbols that follow.  */
  for (;;)
    {
      m = variable_decl ();
      if (m == MATCH_ERROR)
	goto cleanup;
      if (m == MATCH_NO)
	break;

      if (gfc_match_eos () == MATCH_YES)
	goto cleanup;
      if (gfc_match_char (',') != MATCH_YES)
	break;
    }

  gfc_error ("Syntax error in data declaration at %C");
  m = MATCH_ERROR;

cleanup:
  gfc_free_array_spec (current_as);
  current_as = NULL;
  return m;
}


/* Match a prefix associated with a function or subroutine
   declaration.  If the typespec pointer is nonnull, then a typespec
   can be matched.  Note that if nothing matches, MATCH_YES is
   returned (the null string was matched).  */

static match
match_prefix (gfc_typespec * ts)
{
  int seen_type;

  gfc_clear_attr (&current_attr);
  seen_type = 0;

loop:
  if (!seen_type && ts != NULL
      && gfc_match_type_spec (ts, 1) == MATCH_YES
      && gfc_match_space () == MATCH_YES)
    {

      seen_type = 1;
      goto loop;
    }

  if (gfc_match ("elemental% ") == MATCH_YES)
    {
      if (gfc_add_elemental (&current_attr, NULL) == FAILURE)
	return MATCH_ERROR;

      goto loop;
    }

  if (gfc_match ("pure% ") == MATCH_YES)
    {
      if (gfc_add_pure (&current_attr, NULL) == FAILURE)
	return MATCH_ERROR;

      goto loop;
    }

  if (gfc_match ("recursive% ") == MATCH_YES)
    {
      if (gfc_add_recursive (&current_attr, NULL) == FAILURE)
	return MATCH_ERROR;

      goto loop;
    }

  /* At this point, the next item is not a prefix.  */
  return MATCH_YES;
}


/* Copy attributes matched by match_prefix() to attributes on a symbol.  */

static try
copy_prefix (symbol_attribute * dest, locus * where)
{

  if (current_attr.pure && gfc_add_pure (dest, where) == FAILURE)
    return FAILURE;

  if (current_attr.elemental && gfc_add_elemental (dest, where) == FAILURE)
    return FAILURE;

  if (current_attr.recursive && gfc_add_recursive (dest, where) == FAILURE)
    return FAILURE;

  return SUCCESS;
}


/* Match a formal argument list.  */

match
gfc_match_formal_arglist (gfc_symbol * progname, int st_flag, int null_flag)
{
  gfc_formal_arglist *head, *tail, *p, *q;
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_symbol *sym;
  match m;

  head = tail = NULL;

  if (gfc_match_char ('(') != MATCH_YES)
    {
      if (null_flag)
	goto ok;
      return MATCH_NO;
    }

  if (gfc_match_char (')') == MATCH_YES)
    goto ok;

  for (;;)
    {
      if (gfc_match_char ('*') == MATCH_YES)
	sym = NULL;
      else
	{
	  m = gfc_match_name (name);
	  if (m != MATCH_YES)
	    goto cleanup;

	  if (gfc_get_symbol (name, NULL, &sym))
	    goto cleanup;
	}

      p = gfc_get_formal_arglist ();

      if (head == NULL)
	head = tail = p;
      else
	{
	  tail->next = p;
	  tail = p;
	}

      tail->sym = sym;

      /* We don't add the VARIABLE flavor because the name could be a
         dummy procedure.  We don't apply these attributes to formal
         arguments of statement functions.  */
      if (sym != NULL && !st_flag
	  && (gfc_add_dummy (&sym->attr, NULL) == FAILURE
	      || gfc_missing_attr (&sym->attr, NULL) == FAILURE))
	{
	  m = MATCH_ERROR;
	  goto cleanup;
	}

      /* The name of a program unit can be in a different namespace,
         so check for it explicitly.  After the statement is accepted,
         the name is checked for especially in gfc_get_symbol().  */
      if (gfc_new_block != NULL && sym != NULL
	  && strcmp (sym->name, gfc_new_block->name) == 0)
	{
	  gfc_error ("Name '%s' at %C is the name of the procedure",
		     sym->name);
	  m = MATCH_ERROR;
	  goto cleanup;
	}

      if (gfc_match_char (')') == MATCH_YES)
	goto ok;

      m = gfc_match_char (',');
      if (m != MATCH_YES)
	{
	  gfc_error ("Unexpected junk in formal argument list at %C");
	  goto cleanup;
	}
    }

ok:
  /* Check for duplicate symbols in the formal argument list.  */
  if (head != NULL)
    {
      for (p = head; p->next; p = p->next)
	{
	  if (p->sym == NULL)
	    continue;

	  for (q = p->next; q; q = q->next)
	    if (p->sym == q->sym)
	      {
		gfc_error
		  ("Duplicate symbol '%s' in formal argument list at %C",
		   p->sym->name);

		m = MATCH_ERROR;
		goto cleanup;
	      }
	}
    }

  if (gfc_add_explicit_interface (progname, IFSRC_DECL, head, NULL) ==
      FAILURE)
    {
      m = MATCH_ERROR;
      goto cleanup;
    }

  return MATCH_YES;

cleanup:
  gfc_free_formal_arglist (head);
  return m;
}


/* Match a RESULT specification following a function declaration or
   ENTRY statement.  Also matches the end-of-statement.  */

static match
match_result (gfc_symbol * function, gfc_symbol ** result)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_symbol *r;
  match m;

  if (gfc_match (" result (") != MATCH_YES)
    return MATCH_NO;

  m = gfc_match_name (name);
  if (m != MATCH_YES)
    return m;

  if (gfc_match (" )%t") != MATCH_YES)
    {
      gfc_error ("Unexpected junk following RESULT variable at %C");
      return MATCH_ERROR;
    }

  if (strcmp (function->name, name) == 0)
    {
      gfc_error
	("RESULT variable at %C must be different than function name");
      return MATCH_ERROR;
    }

  if (gfc_get_symbol (name, NULL, &r))
    return MATCH_ERROR;

  if (gfc_add_flavor (&r->attr, FL_VARIABLE, NULL) == FAILURE
      || gfc_add_result (&r->attr, NULL) == FAILURE)
    return MATCH_ERROR;

  *result = r;

  return MATCH_YES;
}


/* Match a function declaration.  */

match
gfc_match_function_decl (void)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_symbol *sym, *result;
  locus old_loc;
  match m;

  if (gfc_current_state () != COMP_NONE
      && gfc_current_state () != COMP_INTERFACE
      && gfc_current_state () != COMP_CONTAINS)
    return MATCH_NO;

  gfc_clear_ts (&current_ts);

  old_loc = *gfc_current_locus ();

  m = match_prefix (&current_ts);
  if (m != MATCH_YES)
    {
      gfc_set_locus (&old_loc);
      return m;
    }

  if (gfc_match ("function% %n", name) != MATCH_YES)
    {
      gfc_set_locus (&old_loc);
      return MATCH_NO;
    }

  if (get_proc_name (name, &sym))
    return MATCH_ERROR;
  gfc_new_block = sym;

  m = gfc_match_formal_arglist (sym, 0, 0);
  if (m == MATCH_NO)
    gfc_error ("Expected formal argument list in function definition at %C");
  else if (m == MATCH_ERROR)
    goto cleanup;

  result = NULL;

  if (gfc_match_eos () != MATCH_YES)
    {
      /* See if a result variable is present.  */
      m = match_result (sym, &result);
      if (m == MATCH_NO)
	gfc_error ("Unexpected junk after function declaration at %C");

      if (m != MATCH_YES)
	{
	  m = MATCH_ERROR;
	  goto cleanup;
	}
    }

  /* Make changes to the symbol.  */
  m = MATCH_ERROR;

  if (gfc_add_function (&sym->attr, NULL) == FAILURE)
    goto cleanup;

  if (gfc_missing_attr (&sym->attr, NULL) == FAILURE
      || copy_prefix (&sym->attr, &sym->declared_at) == FAILURE)
    goto cleanup;

  if (current_ts.type != BT_UNKNOWN && sym->ts.type != BT_UNKNOWN)
    {
      gfc_error ("Function '%s' at %C already has a type of %s", name,
		 gfc_basic_typename (sym->ts.type));
      goto cleanup;
    }

  if (result == NULL)
    {
      sym->ts = current_ts;
      sym->result = sym;
    }
  else
    {
      result->ts = current_ts;
      sym->result = result;
    }

  return MATCH_YES;

cleanup:
  gfc_set_locus (&old_loc);
  return m;
}


/* Match an ENTRY statement.  */

match
gfc_match_entry (void)
{
  gfc_symbol *function, *result, *entry;
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_compile_state state;
  match m;

  m = gfc_match_name (name);
  if (m != MATCH_YES)
    return m;

  if (get_proc_name (name, &entry))
    return MATCH_ERROR;

  gfc_enclosing_unit (&state);
  switch (state)
    {
    case COMP_SUBROUTINE:
      m = gfc_match_formal_arglist (entry, 0, 1);
      if (m != MATCH_YES)
	return MATCH_ERROR;

      if (gfc_current_state () != COMP_SUBROUTINE)
	goto exec_construct;

      if (gfc_add_entry (&entry->attr, NULL) == FAILURE
	  || gfc_add_subroutine (&entry->attr, NULL) == FAILURE)
	return MATCH_ERROR;

      break;

    case COMP_FUNCTION:
      m = gfc_match_formal_arglist (entry, 0, 0);
      if (m != MATCH_YES)
	return MATCH_ERROR;

      if (gfc_current_state () != COMP_FUNCTION)
	goto exec_construct;
      function = gfc_state_stack->sym;

      result = NULL;

      if (gfc_match_eos () == MATCH_YES)
	{
	  if (gfc_add_entry (&entry->attr, NULL) == FAILURE
	      || gfc_add_function (&entry->attr, NULL) == FAILURE)
	    return MATCH_ERROR;

	  entry->result = function->result;

	}
      else
	{
	  m = match_result (function, &result);
	  if (m == MATCH_NO)
	    gfc_syntax_error (ST_ENTRY);
	  if (m != MATCH_YES)
	    return MATCH_ERROR;

	  if (gfc_add_result (&result->attr, NULL) == FAILURE
	      || gfc_add_entry (&entry->attr, NULL) == FAILURE
	      || gfc_add_function (&entry->attr, NULL) == FAILURE)
	    return MATCH_ERROR;
	}

      if (function->attr.recursive && result == NULL)
	{
	  gfc_error ("RESULT attribute required in ENTRY statement at %C");
	  return MATCH_ERROR;
	}

      break;

    default:
      goto exec_construct;
    }

  if (gfc_match_eos () != MATCH_YES)
    {
      gfc_syntax_error (ST_ENTRY);
      return MATCH_ERROR;
    }

  return MATCH_YES;

exec_construct:
  gfc_error ("ENTRY statement at %C cannot appear within %s",
	     gfc_state_name (gfc_current_state ()));

  return MATCH_ERROR;
}


/* Match a subroutine statement, including optional prefixes.  */

match
gfc_match_subroutine (void)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_symbol *sym;
  match m;

  if (gfc_current_state () != COMP_NONE
      && gfc_current_state () != COMP_INTERFACE
      && gfc_current_state () != COMP_CONTAINS)
    return MATCH_NO;

  m = match_prefix (NULL);
  if (m != MATCH_YES)
    return m;

  m = gfc_match ("subroutine% %n", name);
  if (m != MATCH_YES)
    return m;

  if (get_proc_name (name, &sym))
    return MATCH_ERROR;
  gfc_new_block = sym;

  if (gfc_add_subroutine (&sym->attr, NULL) == FAILURE)
    return MATCH_ERROR;

  if (gfc_match_formal_arglist (sym, 0, 1) != MATCH_YES)
    return MATCH_ERROR;

  if (gfc_match_eos () != MATCH_YES)
    {
      gfc_syntax_error (ST_SUBROUTINE);
      return MATCH_ERROR;
    }

  if (copy_prefix (&sym->attr, &sym->declared_at) == FAILURE)
    return MATCH_ERROR;

  return MATCH_YES;
}


/* Match any of the various end-block statements.  Returns the type of
   END to the caller.  The END INTERFACE, END IF, END DO and END
   SELECT statements cannot be replaced by a single END statement.  */

match
gfc_match_end (gfc_statement * st)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_compile_state state;
  locus old_loc;
  const char *block_name;
  const char *target;
  match m;

  old_loc = *gfc_current_locus ();
  if (gfc_match ("end") != MATCH_YES)
    return MATCH_NO;

  state = gfc_current_state ();
  block_name =
    gfc_current_block () == NULL ? NULL : gfc_current_block ()->name;

  if (state == COMP_CONTAINS)
    {
      state = gfc_state_stack->previous->state;
      block_name = gfc_state_stack->previous->sym == NULL ? NULL
	: gfc_state_stack->previous->sym->name;
    }

  switch (state)
    {
    case COMP_NONE:
    case COMP_PROGRAM:
      *st = ST_END_PROGRAM;
      target = " program";
      break;

    case COMP_SUBROUTINE:
      *st = ST_END_SUBROUTINE;
      target = " subroutine";
      break;

    case COMP_FUNCTION:
      *st = ST_END_FUNCTION;
      target = " function";
      break;

    case COMP_BLOCK_DATA:
      *st = ST_END_BLOCK_DATA;
      target = " block data";
      break;

    case COMP_MODULE:
      *st = ST_END_MODULE;
      target = " module";
      break;

    case COMP_INTERFACE:
      *st = ST_END_INTERFACE;
      target = " interface";
      break;

    case COMP_DERIVED:
      *st = ST_END_TYPE;
      target = " type";
      break;

    case COMP_IF:
      *st = ST_ENDIF;
      target = " if";
      break;

    case COMP_DO:
      *st = ST_ENDDO;
      target = " do";
      break;

    case COMP_SELECT:
      *st = ST_END_SELECT;
      target = " select";
      break;

    case COMP_FORALL:
      *st = ST_END_FORALL;
      target = " forall";
      break;

    case COMP_WHERE:
      *st = ST_END_WHERE;
      target = " where";
      break;

    default:
      gfc_error ("Unexpected END statement at %C");
      goto cleanup;
    }

  if (gfc_match_eos () == MATCH_YES)
    {

      if (*st == ST_ENDIF || *st == ST_ENDDO || *st == ST_END_SELECT
	  || *st == ST_END_INTERFACE || *st == ST_END_FORALL
	  || *st == ST_END_WHERE)
	{

	  gfc_error ("%s statement expected at %C",
		     gfc_ascii_statement (*st));
	  goto cleanup;
	}

      return MATCH_YES;
    }

  /* Verify that we've got the sort of end-block that we're expecting.  */
  if (gfc_match (target) != MATCH_YES)
    {
      gfc_error ("Expecting %s statement at %C", gfc_ascii_statement (*st));
      goto cleanup;
    }

  /* If we're at the end, make sure a block name wasn't required.  */
  if (gfc_match_eos () == MATCH_YES)
    {

      if (*st != ST_ENDDO && *st != ST_ENDIF && *st != ST_END_SELECT)
	return MATCH_YES;

      if (gfc_current_block () == NULL)
	return MATCH_YES;

      gfc_error ("Expected block name of '%s' in %s statement at %C",
		 block_name, gfc_ascii_statement (*st));

      return MATCH_ERROR;
    }

  /* END INTERFACE has a special handler for its several possible endings.  */
  if (*st == ST_END_INTERFACE)
    return gfc_match_end_interface ();

  /* We haven't hit the end of statement, so what is left must be an end-name.  */
  m = gfc_match_space ();
  if (m == MATCH_YES)
    m = gfc_match_name (name);

  if (m == MATCH_NO)
    gfc_error ("Expected terminating name at %C");
  if (m != MATCH_YES)
    goto cleanup;

  if (block_name == NULL)
    goto syntax;

  if (strcmp (name, block_name) != 0)
    {
      gfc_error ("Expected label '%s' for %s statement at %C", block_name,
		 gfc_ascii_statement (*st));
      goto cleanup;
    }

  if (gfc_match_eos () == MATCH_YES)
    return MATCH_YES;

syntax:
  gfc_syntax_error (*st);

cleanup:
  gfc_set_locus (&old_loc);
  return MATCH_ERROR;
}



/***************** Attribute declaration statements ****************/

/* Set the attribute of a single variable.  */

static match
attr_decl1 (void)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_array_spec *as;
  gfc_symbol *sym;
  locus var_locus;
  match m;

  as = NULL;

  m = gfc_match_name (name);
  if (m != MATCH_YES)
    goto cleanup;

  if (find_special (name, &sym))
    return MATCH_ERROR;

  var_locus = *gfc_current_locus ();

  /* Deal with possible array specification for certain attributes.  */
  if (current_attr.dimension
      || current_attr.allocatable
      || current_attr.pointer
      || current_attr.target)
    {
      m = gfc_match_array_spec (&as);
      if (m == MATCH_ERROR)
	goto cleanup;

      if (current_attr.dimension && m == MATCH_NO)
	{
	  gfc_error
	    ("Missing array specification at %L in DIMENSION statement",
	     &var_locus);
	  m = MATCH_ERROR;
	  goto cleanup;
	}

      if ((current_attr.allocatable || current_attr.pointer)
	  && (m == MATCH_YES) && (as->type != AS_DEFERRED))
	{
	  gfc_error ("Array specification must be deferred at %L",
		     &var_locus);
	  m = MATCH_ERROR;
	  goto cleanup;
	}
    }

  /* Update symbol table.  DIMENSION attribute is set in gfc_set_array_spec().  */
  if (current_attr.dimension == 0
      && gfc_copy_attr (&sym->attr, &current_attr, NULL) == FAILURE)
    {
      m = MATCH_ERROR;
      goto cleanup;
    }

  if (gfc_set_array_spec (sym, as, &var_locus) == FAILURE)
    {
      m = MATCH_ERROR;
      goto cleanup;
    }

  if ((current_attr.external || current_attr.intrinsic)
      && sym->attr.flavor != FL_PROCEDURE
      && gfc_add_flavor (&sym->attr, FL_PROCEDURE, NULL) == FAILURE)
    {
      m = MATCH_ERROR;
      goto cleanup;
    }

  return MATCH_YES;

cleanup:
  gfc_free_array_spec (as);
  return m;
}


/* Generic attribute declaration subroutine.  Used for attributes that
   just have a list of names.  */

static match
attr_decl (void)
{
  match m;

  /* Gobble the optional double colon, by simply ignoring the result
     of gfc_match().  */
  gfc_match (" ::");

  for (;;)
    {
      m = attr_decl1 ();
      if (m != MATCH_YES)
	break;

      if (gfc_match_eos () == MATCH_YES)
	{
	  m = MATCH_YES;
	  break;
	}

      if (gfc_match_char (',') != MATCH_YES)
	{
	  gfc_error ("Unexpected character in variable list at %C");
	  m = MATCH_ERROR;
	  break;
	}
    }

  return m;
}


match
gfc_match_external (void)
{

  gfc_clear_attr (&current_attr);
  gfc_add_external (&current_attr, NULL);

  return attr_decl ();
}



match
gfc_match_intent (void)
{
  sym_intent intent;

  intent = match_intent_spec ();
  if (intent == INTENT_UNKNOWN)
    return MATCH_ERROR;

  gfc_clear_attr (&current_attr);
  gfc_add_intent (&current_attr, intent, NULL);	/* Can't fail */

  return attr_decl ();
}


match
gfc_match_intrinsic (void)
{

  gfc_clear_attr (&current_attr);
  gfc_add_intrinsic (&current_attr, NULL);

  return attr_decl ();
}


match
gfc_match_optional (void)
{

  gfc_clear_attr (&current_attr);
  gfc_add_optional (&current_attr, NULL);

  return attr_decl ();
}


match
gfc_match_pointer (void)
{

  gfc_clear_attr (&current_attr);
  gfc_add_pointer (&current_attr, NULL);

  return attr_decl ();
}


match
gfc_match_allocatable (void)
{

  gfc_clear_attr (&current_attr);
  gfc_add_allocatable (&current_attr, NULL);

  return attr_decl ();
}


match
gfc_match_dimension (void)
{

  gfc_clear_attr (&current_attr);
  gfc_add_dimension (&current_attr, NULL);

  return attr_decl ();
}


match
gfc_match_target (void)
{

  gfc_clear_attr (&current_attr);
  gfc_add_target (&current_attr, NULL);

  return attr_decl ();
}


/* Match the list of entities being specified in a PUBLIC or PRIVATE
   statement.  */

static match
access_attr_decl (gfc_statement st)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  interface_type type;
  gfc_user_op *uop;
  gfc_symbol *sym;
  gfc_intrinsic_op operator;
  match m;

  if (gfc_match (" ::") == MATCH_NO && gfc_match_space () == MATCH_NO)
    goto done;

  for (;;)
    {
      m = gfc_match_generic_spec (&type, name, &operator);
      if (m == MATCH_NO)
	goto syntax;
      if (m == MATCH_ERROR)
	return MATCH_ERROR;

      switch (type)
	{
	case INTERFACE_NAMELESS:
	  goto syntax;

	case INTERFACE_GENERIC:
	  if (gfc_get_symbol (name, NULL, &sym))
	    goto done;

	  if (gfc_add_access (&sym->attr,
			      (st ==
			       ST_PUBLIC) ? ACCESS_PUBLIC : ACCESS_PRIVATE,
			      NULL) == FAILURE)
	    return MATCH_ERROR;

	  break;

	case INTERFACE_INTRINSIC_OP:
	  if (gfc_current_ns->operator_access[operator] == ACCESS_UNKNOWN)
	    {
	      gfc_current_ns->operator_access[operator] =
		(st == ST_PUBLIC) ? ACCESS_PUBLIC : ACCESS_PRIVATE;
	    }
	  else
	    {
	      gfc_error ("Access specification of the %s operator at %C has "
			 "already been specified", gfc_op2string (operator));
	      goto done;
	    }

	  break;

	case INTERFACE_USER_OP:
	  uop = gfc_get_uop (name);

	  if (uop->access == ACCESS_UNKNOWN)
	    {
	      uop->access =
		(st == ST_PUBLIC) ? ACCESS_PUBLIC : ACCESS_PRIVATE;
	    }
	  else
	    {
	      gfc_error
		("Access specification of the .%s. operator at %C has "
		 "already been specified", sym->name);
	      goto done;
	    }

	  break;
	}

      if (gfc_match_char (',') == MATCH_NO)
	break;
    }

  if (gfc_match_eos () != MATCH_YES)
    goto syntax;
  return MATCH_YES;

syntax:
  gfc_syntax_error (st);

done:
  return MATCH_ERROR;
}


/* The PRIVATE statement is a bit weird in that it can be a attribute
   declaration, but also works as a standlone statement inside of a
   type declaration or a module.  */

match
gfc_match_private (gfc_statement * st)
{

  if (gfc_match ("private") != MATCH_YES)
    return MATCH_NO;

  if (gfc_current_state () == COMP_DERIVED)
    {
      if (gfc_match_eos () == MATCH_YES)
	{
	  *st = ST_PRIVATE;
	  return MATCH_YES;
	}

      gfc_syntax_error (ST_PRIVATE);
      return MATCH_ERROR;
    }

  if (gfc_match_eos () == MATCH_YES)
    {
      *st = ST_PRIVATE;
      return MATCH_YES;
    }

  *st = ST_ATTR_DECL;
  return access_attr_decl (ST_PRIVATE);
}


match
gfc_match_public (gfc_statement * st)
{

  if (gfc_match ("public") != MATCH_YES)
    return MATCH_NO;

  if (gfc_match_eos () == MATCH_YES)
    {
      *st = ST_PUBLIC;
      return MATCH_YES;
    }

  *st = ST_ATTR_DECL;
  return access_attr_decl (ST_PUBLIC);
}


/* Workhorse for gfc_match_parameter.  */

static match
do_parm (void)
{
  gfc_symbol *sym;
  gfc_expr *init;
  match m;

  m = gfc_match_symbol (&sym, 0);
  if (m == MATCH_NO)
    gfc_error ("Expected variable name at %C in PARAMETER statement");

  if (m != MATCH_YES)
    return m;

  if (gfc_match_char ('=') == MATCH_NO)
    {
      gfc_error ("Expected = sign in PARAMETER statement at %C");
      return MATCH_ERROR;
    }

  m = gfc_match_init_expr (&init);
  if (m == MATCH_NO)
    gfc_error ("Expected expression at %C in PARAMETER statement");
  if (m != MATCH_YES)
    return m;

  if (sym->ts.type == BT_UNKNOWN
      && gfc_set_default_type (sym, 1, NULL) == FAILURE)
    {
      m = MATCH_ERROR;
      goto cleanup;
    }

  if (gfc_check_assign_symbol (sym, init) == FAILURE
      || gfc_add_flavor (&sym->attr, FL_PARAMETER, NULL) == FAILURE)
    {
      m = MATCH_ERROR;
      goto cleanup;
    }

  sym->value = init;
  return MATCH_YES;

cleanup:
  gfc_free_expr (init);
  return m;
}


/* Match a parameter statement, with the weird syntax that these have.  */

match
gfc_match_parameter (void)
{
  match m;

  if (gfc_match_char ('(') == MATCH_NO)
    return MATCH_NO;

  for (;;)
    {
      m = do_parm ();
      if (m != MATCH_YES)
	break;

      if (gfc_match (" )%t") == MATCH_YES)
	break;

      if (gfc_match_char (',') != MATCH_YES)
	{
	  gfc_error ("Unexpected characters in PARAMETER statement at %C");
	  m = MATCH_ERROR;
	  break;
	}
    }

  return m;
}


/* Save statements have a special syntax.  */

match
gfc_match_save (void)
{
  gfc_symbol *sym;
  match m;

  if (gfc_match_eos () == MATCH_YES)
    {
      if (gfc_current_ns->seen_save)
	{
	  gfc_error ("Blanket SAVE statement at %C follows previous "
		     "SAVE statement");

	  return MATCH_ERROR;
	}

      gfc_current_ns->save_all = gfc_current_ns->seen_save = 1;
      return MATCH_YES;
    }

  if (gfc_current_ns->save_all)
    {
      gfc_error ("SAVE statement at %C follows blanket SAVE statement");
      return MATCH_ERROR;
    }

  gfc_match (" ::");

  for (;;)
    {
      m = gfc_match_symbol (&sym, 0);
      switch (m)
	{
	case MATCH_YES:
	  if (gfc_add_save (&sym->attr, gfc_current_locus ()) == FAILURE)
	    return MATCH_ERROR;
	  goto next_item;

	case MATCH_NO:
	  break;

	case MATCH_ERROR:
	  return MATCH_ERROR;
	}

      m = gfc_match (" / %s /", &sym);
      if (m == MATCH_ERROR)
	return MATCH_ERROR;
      if (m == MATCH_NO)
	goto syntax;

      if (gfc_add_saved_common (&sym->attr, NULL) == FAILURE)
	return MATCH_ERROR;
      gfc_current_ns->seen_save = 1;

    next_item:
      if (gfc_match_eos () == MATCH_YES)
	break;
      if (gfc_match_char (',') != MATCH_YES)
	goto syntax;
    }

  return MATCH_YES;

syntax:
  gfc_error ("Syntax error in SAVE statement at %C");
  return MATCH_ERROR;
}


/* Match a module procedure statement.  Note that we have to modify
   symbols in the parent's namespace because the current one was there
   to receive symbols that are in a interface's formal argument list.  */

match
gfc_match_modproc (void)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  gfc_symbol *sym;
  match m;

  if (gfc_state_stack->state != COMP_INTERFACE
      || gfc_state_stack->previous == NULL
      || current_interface.type == INTERFACE_NAMELESS)
    {
      gfc_error
	("MODULE PROCEDURE at %C must be in a generic module interface");
      return MATCH_ERROR;
    }

  for (;;)
    {
      m = gfc_match_name (name);
      if (m == MATCH_NO)
	goto syntax;
      if (m != MATCH_YES)
	return MATCH_ERROR;

      if (gfc_get_symbol (name, gfc_current_ns->parent, &sym))
	return MATCH_ERROR;

      if (sym->attr.proc != PROC_MODULE
	  && gfc_add_procedure (&sym->attr, PROC_MODULE, NULL) == FAILURE)
	return MATCH_ERROR;

      if (gfc_add_interface (sym) == FAILURE)
	return MATCH_ERROR;

      if (gfc_match_eos () == MATCH_YES)
	break;
      if (gfc_match_char (',') != MATCH_YES)
	goto syntax;
    }

  return MATCH_YES;

syntax:
  gfc_syntax_error (ST_MODULE_PROC);
  return MATCH_ERROR;
}


/* Match the beginning of a derived type declaration.  If a type name
   was the result of a function, then it is possible to have a symbol
   already to be known as a derived type yet have no components.  */

match
gfc_match_derived_decl (void)
{
  char name[GFC_MAX_SYMBOL_LEN + 1];
  symbol_attribute attr;
  gfc_symbol *sym;
  match m;

  if (gfc_current_state () == COMP_DERIVED)
    return MATCH_NO;

  gfc_clear_attr (&attr);

loop:
  if (gfc_match (" , private") == MATCH_YES)
    {
      if (gfc_find_state (COMP_MODULE) == FAILURE)
	{
	  gfc_error
	    ("Derived type at %C can only be PRIVATE within a MODULE");
	  return MATCH_ERROR;
	}

      if (gfc_add_access (&attr, ACCESS_PRIVATE, NULL) == FAILURE)
	return MATCH_ERROR;
      goto loop;
    }

  if (gfc_match (" , public") == MATCH_YES)
    {
      if (gfc_find_state (COMP_MODULE) == FAILURE)
	{
	  gfc_error ("Derived type at %C can only be PUBLIC within a MODULE");
	  return MATCH_ERROR;
	}

      if (gfc_add_access (&attr, ACCESS_PUBLIC, NULL) == FAILURE)
	return MATCH_ERROR;
      goto loop;
    }

  if (gfc_match (" ::") != MATCH_YES && attr.access != ACCESS_UNKNOWN)
    {
      gfc_error ("Expected :: in TYPE definition at %C");
      return MATCH_ERROR;
    }

  m = gfc_match (" %n%t", name);
  if (m != MATCH_YES)
    return m;

  /* Make sure the name isn't the name of an intrinsic type.  The
     'double precision' type doesn't get past the name matcher.  */
  if (strcmp (name, "integer") == 0
      || strcmp (name, "real") == 0
      || strcmp (name, "character") == 0
      || strcmp (name, "logical") == 0
      || strcmp (name, "complex") == 0)
    {
      gfc_error
	("Type name '%s' at %C cannot be the same as an intrinsic type",
	 name);
      return MATCH_ERROR;
    }

  if (gfc_get_symbol (name, NULL, &sym))
    return MATCH_ERROR;

  if (sym->ts.type != BT_UNKNOWN)
    {
      gfc_error ("Derived type name '%s' at %C already has a basic type "
		 "of %s", sym->name, gfc_typename (&sym->ts));
      return MATCH_ERROR;
    }

  /* The symbol may already have the derived attribute without the
     components.  The ways this can happen is via a function
     definition, an INTRINSIC statement or a subtype in another
     derived type that is a pointer.  The first part of the AND clause
     is true if a the symbol is not the return value of a function. */
  if (sym->attr.flavor != FL_DERIVED
      && gfc_add_flavor (&sym->attr, FL_DERIVED, NULL) == FAILURE)
    return MATCH_ERROR;

  if (sym->components != NULL)
    {
      gfc_error
	("Derived type definition of '%s' at %C has already been defined",
	 sym->name);
      return MATCH_ERROR;
    }

  if (attr.access != ACCESS_UNKNOWN
      && gfc_add_access (&sym->attr, attr.access, NULL) == FAILURE)
    return MATCH_ERROR;

  gfc_new_block = sym;

  return MATCH_YES;
}
