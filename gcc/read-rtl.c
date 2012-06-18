/* RTL reader for GCC.
   Copyright (C) 1987, 1988, 1991, 1994, 1997, 1998, 1999, 2000, 2001, 2002,
   2003, 2004, 2005, 2007, 2008, 2010
   Free Software Foundation, Inc.

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

#include "bconfig.h"

/* Disable rtl checking; it conflicts with the iterator handling.  */
#undef ENABLE_RTL_CHECKING

#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "obstack.h"
#include "hashtab.h"
#include "read-md.h"
#include "gensupport.h"

/* One element in a singly-linked list of (integer, string) pairs.  */
struct map_value {
  struct map_value *next;
  int number;
  const char *string;
};

/* Maps an iterator or attribute name to a list of (integer, string) pairs.
   The integers are iterator values; the strings are either C conditions
   or attribute values.  */
struct mapping {
  /* The name of the iterator or attribute.  */
  const char *name;

  /* The group (modes or codes) to which the iterator or attribute belongs.  */
  struct iterator_group *group;

  /* The list of (integer, string) pairs.  */
  struct map_value *values;

  /* For iterators, records the current value of the iterator.  */
  struct map_value *current_value;
};

/* Vector definitions for the above.  */
typedef struct mapping *mapping_ptr;
DEF_VEC_P (mapping_ptr);
DEF_VEC_ALLOC_P (mapping_ptr, heap);

/* A structure for abstracting the common parts of iterators.  */
struct iterator_group {
  /* Tables of "mapping" structures, one for attributes and one for
     iterators.  */
  htab_t attrs, iterators;

  /* Treat the given string as the name of a standard mode, etc., and
     return its integer value.  */
  int (*find_builtin) (const char *);

  /* Make the given pointer use the given iterator value.  */
  void (*apply_iterator) (void *, int);
};

/* Records one use of an iterator.  */
struct iterator_use {
  /* The iterator itself.  */
  struct mapping *iterator;

  /* The location of the use, as passed to the apply_iterator callback.  */
  void *ptr;
};

/* Vector definitions for the above.  */
typedef struct iterator_use iterator_use;
DEF_VEC_O (iterator_use);
DEF_VEC_ALLOC_O (iterator_use, heap);

/* Records one use of an attribute (the "<[iterator:]attribute>" syntax)
   in a non-string rtx field.  */
struct attribute_use {
  /* The group that describes the use site.  */
  struct iterator_group *group;

  /* The name of the attribute, possibly with an "iterator:" prefix.  */
  const char *value;

  /* The location of the use, as passed to GROUP's apply_iterator callback.  */
  void *ptr;
};

/* Vector definitions for the above.  */
typedef struct attribute_use attribute_use;
DEF_VEC_O (attribute_use);
DEF_VEC_ALLOC_O (attribute_use, heap);

static void validate_const_int (const char *);
static rtx read_rtx_code (const char *);
static rtx read_nested_rtx (void);
static rtx read_rtx_variadic (rtx);

/* The mode and code iterator structures.  */
static struct iterator_group modes, codes;

/* All iterators used in the current rtx.  */
static VEC (mapping_ptr, heap) *current_iterators;

/* The list of all iterator uses in the current rtx.  */
static VEC (iterator_use, heap) *iterator_uses;

/* The list of all attribute uses in the current rtx.  */
static VEC (attribute_use, heap) *attribute_uses;

/* Implementations of the iterator_group callbacks for modes.  */

static int
find_mode (const char *name)
{
  int i;

  for (i = 0; i < NUM_MACHINE_MODES; i++)
    if (strcmp (GET_MODE_NAME (i), name) == 0)
      return i;

  fatal_with_file_and_line ("unknown mode `%s'", name);
}

static void
apply_mode_iterator (void *loc, int mode)
{
  PUT_MODE ((rtx) loc, (enum machine_mode) mode);
}

/* Implementations of the iterator_group callbacks for codes.  */

static int
find_code (const char *name)
{
  int i;

  for (i = 0; i < NUM_RTX_CODE; i++)
    if (strcmp (GET_RTX_NAME (i), name) == 0)
      return i;

  fatal_with_file_and_line ("unknown rtx code `%s'", name);
}

static void
apply_code_iterator (void *loc, int code)
{
  PUT_CODE ((rtx) loc, (enum rtx_code) code);
}

/* Map attribute string P to its current value.  Return null if the attribute
   isn't known.  */

static struct map_value *
map_attr_string (const char *p)
{
  const char *attr;
  struct mapping *iterator;
  unsigned int i;
  struct mapping *m;
  struct map_value *v;
  int iterator_name_len;

  /* Peel off any "iterator:" prefix.  Set ATTR to the start of the
     attribute name.  */
  attr = strchr (p, ':');
  if (attr == 0)
    {
      iterator_name_len = -1;
      attr = p;
    }
  else
    {
      iterator_name_len = attr - p;
      attr++;
    }

  FOR_EACH_VEC_ELT (mapping_ptr, current_iterators, i, iterator)
    {
      /* If an iterator name was specified, check that it matches.  */
      if (iterator_name_len >= 0
	  && (strncmp (p, iterator->name, iterator_name_len) != 0
	      || iterator->name[iterator_name_len] != 0))
	continue;

      /* Find the attribute specification.  */
      m = (struct mapping *) htab_find (iterator->group->attrs, &attr);
      if (m)
	/* Find the attribute value associated with the current
	   iterator value.  */
	for (v = m->values; v; v = v->next)
	  if (v->number == iterator->current_value->number)
	    return v;
    }
  return NULL;
}

/* Apply the current iterator values to STRING.  Return the new string
   if any changes were needed, otherwise return STRING itself.  */

static const char *
apply_iterator_to_string (const char *string)
{
  char *base, *copy, *p, *start, *end;
  struct map_value *v;

  if (string == 0)
    return string;

  base = p = copy = ASTRDUP (string);
  while ((start = strchr (p, '<')) && (end = strchr (start, '>')))
    {
      p = start + 1;

      *end = 0;
      v = map_attr_string (p);
      *end = '>';
      if (v == 0)
	continue;

      /* Add everything between the last copied byte and the '<',
	 then add in the attribute value.  */
      obstack_grow (&string_obstack, base, start - base);
      obstack_grow (&string_obstack, v->string, strlen (v->string));
      base = end + 1;
    }
  if (base != copy)
    {
      obstack_grow (&string_obstack, base, strlen (base) + 1);
      copy = XOBFINISH (&string_obstack, char *);
      copy_md_ptr_loc (copy, string);
      return copy;
    }
  return string;
}

/* Return a deep copy of X, substituting the current iterator
   values into any strings.  */

static rtx
copy_rtx_for_iterators (rtx original)
{
  const char *format_ptr;
  int i, j;
  rtx x;

  if (original == 0)
    return original;

  /* Create a shallow copy of ORIGINAL.  */
  x = rtx_alloc (GET_CODE (original));
  memcpy (x, original, RTX_CODE_SIZE (GET_CODE (original)));

  /* Change each string and recursively change each rtx.  */
  format_ptr = GET_RTX_FORMAT (GET_CODE (original));
  for (i = 0; format_ptr[i] != 0; i++)
    switch (format_ptr[i])
      {
      case 'T':
	XTMPL (x, i) = apply_iterator_to_string (XTMPL (x, i));
	break;

      case 'S':
      case 's':
	XSTR (x, i) = apply_iterator_to_string (XSTR (x, i));
	break;

      case 'e':
	XEXP (x, i) = copy_rtx_for_iterators (XEXP (x, i));
	break;

      case 'V':
      case 'E':
	if (XVEC (original, i))
	  {
	    XVEC (x, i) = rtvec_alloc (XVECLEN (original, i));
	    for (j = 0; j < XVECLEN (x, i); j++)
	      XVECEXP (x, i, j)
		= copy_rtx_for_iterators (XVECEXP (original, i, j));
	  }
	break;

      default:
	break;
      }
  return x;
}

/* Return a condition that must satisfy both ORIGINAL and EXTRA.  If ORIGINAL
   has the form "&& ..." (as used in define_insn_and_splits), assume that
   EXTRA is already satisfied.  Empty strings are treated like "true".  */

static const char *
add_condition_to_string (const char *original, const char *extra)
{
  if (original != 0 && original[0] == '&' && original[1] == '&')
    return original;
  return join_c_conditions (original, extra);
}

/* Like add_condition, but applied to all conditions in rtx X.  */

static void
add_condition_to_rtx (rtx x, const char *extra)
{
  switch (GET_CODE (x))
    {
    case DEFINE_INSN:
    case DEFINE_EXPAND:
      XSTR (x, 2) = add_condition_to_string (XSTR (x, 2), extra);
      break;

    case DEFINE_SPLIT:
    case DEFINE_PEEPHOLE:
    case DEFINE_PEEPHOLE2:
    case DEFINE_COND_EXEC:
      XSTR (x, 1) = add_condition_to_string (XSTR (x, 1), extra);
      break;

    case DEFINE_INSN_AND_SPLIT:
      XSTR (x, 2) = add_condition_to_string (XSTR (x, 2), extra);
      XSTR (x, 4) = add_condition_to_string (XSTR (x, 4), extra);
      break;

    default:
      break;
    }
}

/* Apply the current iterator values to all attribute_uses.  */

static void
apply_attribute_uses (void)
{
  struct map_value *v;
  attribute_use *ause;
  unsigned int i;

  FOR_EACH_VEC_ELT (attribute_use, attribute_uses, i, ause)
    {
      v = map_attr_string (ause->value);
      if (!v)
	fatal_with_file_and_line ("unknown iterator value `%s'", ause->value);
      ause->group->apply_iterator (ause->ptr,
				   ause->group->find_builtin (v->string));
    }
}

/* A htab_traverse callback for iterators.  Add all used iterators
   to current_iterators.  */

static int
add_current_iterators (void **slot, void *data ATTRIBUTE_UNUSED)
{
  struct mapping *iterator;

  iterator = (struct mapping *) *slot;
  if (iterator->current_value)
    VEC_safe_push (mapping_ptr, heap, current_iterators, iterator);
  return 1;
}

/* Expand all iterators in the current rtx, which is given as ORIGINAL.
   Build a list of expanded rtxes in the EXPR_LIST pointed to by QUEUE.  */

static void
apply_iterators (rtx original, rtx *queue)
{
  unsigned int i;
  const char *condition;
  iterator_use *iuse;
  struct mapping *iterator;
  struct map_value *v;
  rtx x;

  if (VEC_empty (iterator_use, iterator_uses))
    {
      /* Raise an error if any attributes were used.  */
      apply_attribute_uses ();
      XEXP (*queue, 0) = original;
      XEXP (*queue, 1) = NULL_RTX;
      return;
    }

  /* Clear out the iterators from the previous run.  */
  FOR_EACH_VEC_ELT (mapping_ptr, current_iterators, i, iterator)
    iterator->current_value = NULL;
  VEC_truncate (mapping_ptr, current_iterators, 0);

  /* Mark the iterators that we need this time.  */
  FOR_EACH_VEC_ELT (iterator_use, iterator_uses, i, iuse)
    iuse->iterator->current_value = iuse->iterator->values;

  /* Get the list of iterators that are in use, preserving the
     definition order within each group.  */
  htab_traverse (modes.iterators, add_current_iterators, NULL);
  htab_traverse (codes.iterators, add_current_iterators, NULL);
  gcc_assert (!VEC_empty (mapping_ptr, current_iterators));

  for (;;)
    {
      /* Apply the current iterator values.  Accumulate a condition to
	 say when the resulting rtx can be used.  */
      condition = NULL;
      FOR_EACH_VEC_ELT (iterator_use, iterator_uses, i, iuse)
	{
	  v = iuse->iterator->current_value;
	  iuse->iterator->group->apply_iterator (iuse->ptr, v->number);
	  condition = join_c_conditions (condition, v->string);
	}
      apply_attribute_uses ();
      x = copy_rtx_for_iterators (original);
      add_condition_to_rtx (x, condition);

      /* Add the new rtx to the end of the queue.  */
      XEXP (*queue, 0) = x;
      XEXP (*queue, 1) = NULL_RTX;

      /* Lexicographically increment the iterator value sequence.
	 That is, cycle through iterator values, starting from the right,
	 and stopping when one of them doesn't wrap around.  */
      i = VEC_length (mapping_ptr, current_iterators);
      for (;;)
	{
	  if (i == 0)
	    return;
	  i--;
	  iterator = VEC_index (mapping_ptr, current_iterators, i);
	  iterator->current_value = iterator->current_value->next;
	  if (iterator->current_value)
	    break;
	  iterator->current_value = iterator->values;
	}

      /* At least one more rtx to go.  Allocate room for it.  */
      XEXP (*queue, 1) = rtx_alloc (EXPR_LIST);
      queue = &XEXP (*queue, 1);
    }
}

/* Add a new "mapping" structure to hashtable TABLE.  NAME is the name
   of the mapping and GROUP is the group to which it belongs.  */

static struct mapping *
add_mapping (struct iterator_group *group, htab_t table, const char *name)
{
  struct mapping *m;
  void **slot;

  m = XNEW (struct mapping);
  m->name = xstrdup (name);
  m->group = group;
  m->values = 0;
  m->current_value = NULL;

  slot = htab_find_slot (table, m, INSERT);
  if (*slot != 0)
    fatal_with_file_and_line ("`%s' already defined", name);

  *slot = m;
  return m;
}

/* Add the pair (NUMBER, STRING) to a list of map_value structures.
   END_PTR points to the current null terminator for the list; return
   a pointer the new null terminator.  */

static struct map_value **
add_map_value (struct map_value **end_ptr, int number, const char *string)
{
  struct map_value *value;

  value = XNEW (struct map_value);
  value->next = 0;
  value->number = number;
  value->string = string;

  *end_ptr = value;
  return &value->next;
}

/* Do one-time initialization of the mode and code attributes.  */

static void
initialize_iterators (void)
{
  struct mapping *lower, *upper;
  struct map_value **lower_ptr, **upper_ptr;
  char *copy, *p;
  int i;

  modes.attrs = htab_create (13, leading_string_hash, leading_string_eq_p, 0);
  modes.iterators = htab_create (13, leading_string_hash,
				 leading_string_eq_p, 0);
  modes.find_builtin = find_mode;
  modes.apply_iterator = apply_mode_iterator;

  codes.attrs = htab_create (13, leading_string_hash, leading_string_eq_p, 0);
  codes.iterators = htab_create (13, leading_string_hash,
				 leading_string_eq_p, 0);
  codes.find_builtin = find_code;
  codes.apply_iterator = apply_code_iterator;

  lower = add_mapping (&modes, modes.attrs, "mode");
  upper = add_mapping (&modes, modes.attrs, "MODE");
  lower_ptr = &lower->values;
  upper_ptr = &upper->values;
  for (i = 0; i < MAX_MACHINE_MODE; i++)
    {
      copy = xstrdup (GET_MODE_NAME (i));
      for (p = copy; *p != 0; p++)
	*p = TOLOWER (*p);

      upper_ptr = add_map_value (upper_ptr, i, GET_MODE_NAME (i));
      lower_ptr = add_map_value (lower_ptr, i, copy);
    }

  lower = add_mapping (&codes, codes.attrs, "code");
  upper = add_mapping (&codes, codes.attrs, "CODE");
  lower_ptr = &lower->values;
  upper_ptr = &upper->values;
  for (i = 0; i < NUM_RTX_CODE; i++)
    {
      copy = xstrdup (GET_RTX_NAME (i));
      for (p = copy; *p != 0; p++)
	*p = TOUPPER (*p);

      lower_ptr = add_map_value (lower_ptr, i, GET_RTX_NAME (i));
      upper_ptr = add_map_value (upper_ptr, i, copy);
    }
}

/* Provide a version of a function to read a long long if the system does
   not provide one.  */
#if HOST_BITS_PER_WIDE_INT > HOST_BITS_PER_LONG && !defined(HAVE_ATOLL) && !defined(HAVE_ATOQ)
HOST_WIDE_INT atoll (const char *);

HOST_WIDE_INT
atoll (const char *p)
{
  int neg = 0;
  HOST_WIDE_INT tmp_wide;

  while (ISSPACE (*p))
    p++;
  if (*p == '-')
    neg = 1, p++;
  else if (*p == '+')
    p++;

  tmp_wide = 0;
  while (ISDIGIT (*p))
    {
      HOST_WIDE_INT new_wide = tmp_wide*10 + (*p - '0');
      if (new_wide < tmp_wide)
	{
	  /* Return INT_MAX equiv on overflow.  */
	  tmp_wide = (~(unsigned HOST_WIDE_INT) 0) >> 1;
	  break;
	}
      tmp_wide = new_wide;
      p++;
    }

  if (neg)
    tmp_wide = -tmp_wide;
  return tmp_wide;
}
#endif

/* Process a define_conditions directive, starting with the optional
   space after the "define_conditions".  The directive looks like this:

     (define_conditions [
        (number "string")
        (number "string")
        ...
     ])

   It's not intended to appear in machine descriptions.  It is
   generated by (the program generated by) genconditions.c, and
   slipped in at the beginning of the sequence of MD files read by
   most of the other generators.  */
static void
read_conditions (void)
{
  int c;

  c = read_skip_spaces ();
  if (c != '[')
    fatal_expected_char ('[', c);

  while ( (c = read_skip_spaces ()) != ']')
    {
      struct md_name name;
      char *expr;
      int value;

      if (c != '(')
	fatal_expected_char ('(', c);

      read_name (&name);
      validate_const_int (name.string);
      value = atoi (name.string);

      c = read_skip_spaces ();
      if (c != '"')
	fatal_expected_char ('"', c);
      expr = read_quoted_string ();

      c = read_skip_spaces ();
      if (c != ')')
	fatal_expected_char (')', c);

      add_c_test (expr, value);
    }
}

static void
validate_const_int (const char *string)
{
  const char *cp;
  int valid = 1;

  cp = string;
  while (*cp && ISSPACE (*cp))
    cp++;
  if (*cp == '-' || *cp == '+')
    cp++;
  if (*cp == 0)
    valid = 0;
  for (; *cp; cp++)
    if (! ISDIGIT (*cp))
      valid = 0;
  if (!valid)
    fatal_with_file_and_line ("invalid decimal constant \"%s\"\n", string);
}

/* Record that PTR uses iterator ITERATOR.  */

static void
record_iterator_use (struct mapping *iterator, void *ptr)
{
  struct iterator_use *iuse;

  iuse = VEC_safe_push (iterator_use, heap, iterator_uses, NULL);
  iuse->iterator = iterator;
  iuse->ptr = ptr;
}

/* Record that PTR uses attribute VALUE, which must match a built-in
   value from group GROUP.  */

static void
record_attribute_use (struct iterator_group *group, void *ptr,
		      const char *value)
{
  struct attribute_use *ause;

  ause = VEC_safe_push (attribute_use, heap, attribute_uses, NULL);
  ause->group = group;
  ause->value = value;
  ause->ptr = ptr;
}

/* Interpret NAME as either a built-in value, iterator or attribute
   for group GROUP.  PTR is the value to pass to GROUP's apply_iterator
   callback.  */

static void
record_potential_iterator_use (struct iterator_group *group, void *ptr,
			       const char *name)
{
  struct mapping *m;
  size_t len;

  len = strlen (name);
  if (name[0] == '<' && name[len - 1] == '>')
    {
      /* Copy the attribute string into permanent storage, without the
	 angle brackets around it.  */
      obstack_grow0 (&string_obstack, name + 1, len - 2);
      record_attribute_use (group, ptr, XOBFINISH (&string_obstack, char *));
    }
  else
    {
      m = (struct mapping *) htab_find (group->iterators, &name);
      if (m != 0)
	record_iterator_use (m, ptr);
      else
	group->apply_iterator (ptr, group->find_builtin (name));
    }
}

/* Finish reading a declaration of the form:

       (define... <name> [<value1> ... <valuen>])

   from the MD file, where each <valuei> is either a bare symbol name or a
   "(<name> <string>)" pair.  The "(define..." part has already been read.

   Represent the declaration as a "mapping" structure; add it to TABLE
   (which belongs to GROUP) and return it.  */

static struct mapping *
read_mapping (struct iterator_group *group, htab_t table)
{
  struct md_name name;
  struct mapping *m;
  struct map_value **end_ptr;
  const char *string;
  int number, c;

  /* Read the mapping name and create a structure for it.  */
  read_name (&name);
  m = add_mapping (group, table, name.string);

  c = read_skip_spaces ();
  if (c != '[')
    fatal_expected_char ('[', c);

  /* Read each value.  */
  end_ptr = &m->values;
  c = read_skip_spaces ();
  do
    {
      if (c != '(')
	{
	  /* A bare symbol name that is implicitly paired to an
	     empty string.  */
	  unread_char (c);
	  read_name (&name);
	  string = "";
	}
      else
	{
	  /* A "(name string)" pair.  */
	  read_name (&name);
	  string = read_string (false);
	  c = read_skip_spaces ();
	  if (c != ')')
	    fatal_expected_char (')', c);
	}
      number = group->find_builtin (name.string);
      end_ptr = add_map_value (end_ptr, number, string);
      c = read_skip_spaces ();
    }
  while (c != ']');

  return m;
}

/* Check newly-created code iterator ITERATOR to see whether every code has the
   same format.  */

static void
check_code_iterator (struct mapping *iterator)
{
  struct map_value *v;
  enum rtx_code bellwether;

  bellwether = (enum rtx_code) iterator->values->number;
  for (v = iterator->values->next; v != 0; v = v->next)
    if (strcmp (GET_RTX_FORMAT (bellwether), GET_RTX_FORMAT (v->number)) != 0)
      fatal_with_file_and_line ("code iterator `%s' combines "
				"different rtx formats", iterator->name);
}

/* Read an rtx-related declaration from the MD file, given that it
   starts with directive name RTX_NAME.  Return true if it expands to
   one or more rtxes (as defined by rtx.def).  When returning true,
   store the list of rtxes as an EXPR_LIST in *X.  */

bool
read_rtx (const char *rtx_name, rtx *x)
{
  static rtx queue_head;

  /* Do one-time initialization.  */
  if (queue_head == 0)
    {
      initialize_iterators ();
      queue_head = rtx_alloc (EXPR_LIST);
    }

  /* Handle various rtx-related declarations that aren't themselves
     encoded as rtxes.  */
  if (strcmp (rtx_name, "define_conditions") == 0)
    {
      read_conditions ();
      return false;
    }
  if (strcmp (rtx_name, "define_mode_attr") == 0)
    {
      read_mapping (&modes, modes.attrs);
      return false;
    }
  if (strcmp (rtx_name, "define_mode_iterator") == 0)
    {
      read_mapping (&modes, modes.iterators);
      return false;
    }
  if (strcmp (rtx_name, "define_code_attr") == 0)
    {
      read_mapping (&codes, codes.attrs);
      return false;
    }
  if (strcmp (rtx_name, "define_code_iterator") == 0)
    {
      check_code_iterator (read_mapping (&codes, codes.iterators));
      return false;
    }

  apply_iterators (read_rtx_code (rtx_name), &queue_head);
  VEC_truncate (iterator_use, iterator_uses, 0);
  VEC_truncate (attribute_use, attribute_uses, 0);

  *x = queue_head;
  return true;
}

/* Subroutine of read_rtx and read_nested_rtx.  CODE_NAME is the name of
   either an rtx code or a code iterator.  Parse the rest of the rtx and
   return it.  */

static rtx
read_rtx_code (const char *code_name)
{
  int i;
  RTX_CODE code;
  struct mapping *iterator;
  const char *format_ptr;
  struct md_name name;
  rtx return_rtx;
  int c;
  int tmp_int;
  HOST_WIDE_INT tmp_wide;

  /* Linked list structure for making RTXs: */
  struct rtx_list
    {
      struct rtx_list *next;
      rtx value;		/* Value of this node.  */
    };

  /* If this code is an iterator, build the rtx using the iterator's
     first value.  */
  iterator = (struct mapping *) htab_find (codes.iterators, &code_name);
  if (iterator != 0)
    code = (enum rtx_code) iterator->values->number;
  else
    code = (enum rtx_code) codes.find_builtin (code_name);

  /* If we end up with an insn expression then we free this space below.  */
  return_rtx = rtx_alloc (code);
  format_ptr = GET_RTX_FORMAT (code);
  PUT_CODE (return_rtx, code);

  if (iterator)
    record_iterator_use (iterator, return_rtx);

  /* If what follows is `: mode ', read it and
     store the mode in the rtx.  */

  i = read_skip_spaces ();
  if (i == ':')
    {
      read_name (&name);
      record_potential_iterator_use (&modes, return_rtx, name.string);
    }
  else
    unread_char (i);

  for (i = 0; format_ptr[i] != 0; i++)
    switch (format_ptr[i])
      {
	/* 0 means a field for internal use only.
	   Don't expect it to be present in the input.  */
      case '0':
	break;

      case 'e':
      case 'u':
	XEXP (return_rtx, i) = read_nested_rtx ();
	break;

      case 'V':
	/* 'V' is an optional vector: if a closeparen follows,
	   just store NULL for this element.  */
	c = read_skip_spaces ();
	unread_char (c);
	if (c == ')')
	  {
	    XVEC (return_rtx, i) = 0;
	    break;
	  }
	/* Now process the vector.  */

      case 'E':
	{
	  /* Obstack to store scratch vector in.  */
	  struct obstack vector_stack;
	  int list_counter = 0;
	  rtvec return_vec = NULL_RTVEC;

	  c = read_skip_spaces ();
	  if (c != '[')
	    fatal_expected_char ('[', c);

	  /* Add expressions to a list, while keeping a count.  */
	  obstack_init (&vector_stack);
	  while ((c = read_skip_spaces ()) && c != ']')
	    {
	      if (c == EOF)
		fatal_expected_char (']', c);
	      unread_char (c);
	      list_counter++;
	      obstack_ptr_grow (&vector_stack, read_nested_rtx ());
	    }
	  if (list_counter > 0)
	    {
	      return_vec = rtvec_alloc (list_counter);
	      memcpy (&return_vec->elem[0], obstack_finish (&vector_stack),
		      list_counter * sizeof (rtx));
	    }
	  else if (format_ptr[i] == 'E')
	    fatal_with_file_and_line ("vector must have at least one element");
	  XVEC (return_rtx, i) = return_vec;
	  obstack_free (&vector_stack, NULL);
	  /* close bracket gotten */
	}
	break;

      case 'S':
      case 'T':
      case 's':
	{
	  char *stringbuf;
	  int star_if_braced;

	  c = read_skip_spaces ();
	  unread_char (c);
	  if (c == ')')
	    {
	      /* 'S' fields are optional and should be NULL if no string
		 was given.  Also allow normal 's' and 'T' strings to be
		 omitted, treating them in the same way as empty strings.  */
	      XSTR (return_rtx, i) = (format_ptr[i] == 'S' ? NULL : "");
	      break;
	    }

	  /* The output template slot of a DEFINE_INSN,
	     DEFINE_INSN_AND_SPLIT, or DEFINE_PEEPHOLE automatically
	     gets a star inserted as its first character, if it is
	     written with a brace block instead of a string constant.  */
	  star_if_braced = (format_ptr[i] == 'T');

	  stringbuf = read_string (star_if_braced);

	  /* For insn patterns, we want to provide a default name
	     based on the file and line, like "*foo.md:12", if the
	     given name is blank.  These are only for define_insn and
	     define_insn_and_split, to aid debugging.  */
	  if (*stringbuf == '\0'
	      && i == 0
	      && (GET_CODE (return_rtx) == DEFINE_INSN
		  || GET_CODE (return_rtx) == DEFINE_INSN_AND_SPLIT))
	    {
	      char line_name[20];
	      const char *fn = (read_md_filename ? read_md_filename : "rtx");
	      const char *slash;
	      for (slash = fn; *slash; slash ++)
		if (*slash == '/' || *slash == '\\' || *slash == ':')
		  fn = slash + 1;
	      obstack_1grow (&string_obstack, '*');
	      obstack_grow (&string_obstack, fn, strlen (fn));
	      sprintf (line_name, ":%d", read_md_lineno);
	      obstack_grow (&string_obstack, line_name, strlen (line_name)+1);
	      stringbuf = XOBFINISH (&string_obstack, char *);
	    }

	  if (star_if_braced)
	    XTMPL (return_rtx, i) = stringbuf;
	  else
	    XSTR (return_rtx, i) = stringbuf;
	}
	break;

      case 'w':
	read_name (&name);
	validate_const_int (name.string);
#if HOST_BITS_PER_WIDE_INT == HOST_BITS_PER_INT
	tmp_wide = atoi (name.string);
#else
#if HOST_BITS_PER_WIDE_INT == HOST_BITS_PER_LONG
	tmp_wide = atol (name.string);
#else
	/* Prefer atoll over atoq, since the former is in the ISO C99 standard.
	   But prefer not to use our hand-rolled function above either.  */
#if defined(HAVE_ATOLL) || !defined(HAVE_ATOQ)
	tmp_wide = atoll (name.string);
#else
	tmp_wide = atoq (name.string);
#endif
#endif
#endif
	XWINT (return_rtx, i) = tmp_wide;
	break;

      case 'i':
      case 'n':
	read_name (&name);
	validate_const_int (name.string);
	tmp_int = atoi (name.string);
	XINT (return_rtx, i) = tmp_int;
	break;

      default:
	gcc_unreachable ();
      }

  c = read_skip_spaces ();
  /* Syntactic sugar for AND and IOR, allowing Lisp-like
     arbitrary number of arguments for them.  */
  if (c == '('
      && (GET_CODE (return_rtx) == AND
	  || GET_CODE (return_rtx) == IOR))
    return read_rtx_variadic (return_rtx);

  unread_char (c);
  return return_rtx;
}

/* Read a nested rtx construct from the MD file and return it.  */

static rtx
read_nested_rtx (void)
{
  struct md_name name;
  int c;
  rtx return_rtx;

  c = read_skip_spaces ();
  if (c != '(')
    fatal_expected_char ('(', c);

  read_name (&name);
  if (strcmp (name.string, "nil") == 0)
    return_rtx = NULL;
  else
    return_rtx = read_rtx_code (name.string);

  c = read_skip_spaces ();
  if (c != ')')
    fatal_expected_char (')', c);

  return return_rtx;
}

/* Mutually recursive subroutine of read_rtx which reads
   (thing x1 x2 x3 ...) and produces RTL as if
   (thing x1 (thing x2 (thing x3 ...)))  had been written.
   When called, FORM is (thing x1 x2), and the file position
   is just past the leading parenthesis of x3.  Only works
   for THINGs which are dyadic expressions, e.g. AND, IOR.  */
static rtx
read_rtx_variadic (rtx form)
{
  char c = '(';
  rtx p = form, q;

  do
    {
      unread_char (c);

      q = rtx_alloc (GET_CODE (p));
      PUT_MODE (q, GET_MODE (p));

      XEXP (q, 0) = XEXP (p, 1);
      XEXP (q, 1) = read_nested_rtx ();

      XEXP (p, 1) = q;
      p = q;
      c = read_skip_spaces ();
    }
  while (c == '(');
  unread_char (c);
  return form;
}
