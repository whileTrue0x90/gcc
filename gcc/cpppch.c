/* Part of CPP library.  (Precompiled header reading/writing.)
   Copyright (C) 2000, 2001, 2002 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "cpplib.h"
#include "cpphash.h"
#include "intl.h"
#include "hashtab.h"
#include "mkdeps.h"

static int write_macdef PARAMS ((cpp_reader *, cpp_hashnode *, void *));
static int save_idents PARAMS ((cpp_reader *, cpp_hashnode *, void *));
static hashval_t hashmem PARAMS ((const void *, size_t));
static hashval_t cpp_string_hash PARAMS ((const void *));
static int cpp_string_eq PARAMS ((const void *, const void *));
static int count_defs PARAMS ((cpp_reader *, cpp_hashnode *, void *));
static int write_defs PARAMS ((cpp_reader *, cpp_hashnode *, void *));
static int save_macros PARAMS ((cpp_reader *, cpp_hashnode *, void *));
static int reset_ht PARAMS ((cpp_reader *, cpp_hashnode *, void *));

/* This structure represents a macro definition on disk.  */
struct macrodef_struct 
{
  unsigned int definition_length;
  unsigned short name_length;
  unsigned short flags;
};

/* This is how we write out a macro definition.  
   Suitable for being called by cpp_forall_identifiers.  */

static int
write_macdef (pfile, hn, file_p)
     cpp_reader *pfile;
     cpp_hashnode *hn;
     void *file_p;
{
  FILE *f = (FILE *) file_p;
  switch (hn->type)
    {
    case NT_VOID:
      if (! (hn->flags & NODE_POISONED))
	return 1;
      
    case NT_MACRO:
      if ((hn->flags & NODE_BUILTIN))
	return 1;

      {
	struct macrodef_struct s;
	const unsigned char *defn;

	s.name_length = NODE_LEN (hn);
	s.flags = hn->flags & NODE_POISONED;

	if (hn->type == NT_MACRO)
	  {
	    defn = cpp_macro_definition (pfile, hn);
	    s.definition_length = ustrlen (defn);
	  }
	else
	  {
	    defn = NODE_NAME (hn);
	    s.definition_length = s.name_length;
	  }
	
	if (fwrite (&s, sizeof (s), 1, f) != 1
	    || fwrite (defn, 1, s.definition_length, f) != s.definition_length)
	  {
	    cpp_errno (pfile, DL_ERROR, "while writing precompiled header");
	    return 0;
	  }
      }
      return 1;
      
    case NT_ASSERTION:
      /* Not currently implemented.  */
      return 1;

    default:
      abort ();
    }
}

/* This structure records the names of the defined macros.
   It's also used as a callback structure for size_initial_idents
   and save_idents.  */

struct cpp_savedstate
{
  /* A hash table of the defined identifiers.  */
  htab_t definedhash;
  /* The size of the definitions of those identifiers (the size of
     'definedstrs').  */
  size_t hashsize;
  /* Space for the next definition.  Definitions are null-terminated
     strings.  */
  unsigned char *definedstrs;
};

/* Save this identifier into the state: put it in the hash table,
   put the definition in 'definedstrs'.  */

static int
save_idents (pfile, hn, ss_p)
     cpp_reader *pfile ATTRIBUTE_UNUSED;
     cpp_hashnode *hn;
     void *ss_p;
{
  struct cpp_savedstate *const ss = (struct cpp_savedstate *)ss_p;
  
  if (hn->type != NT_VOID)
    {
      struct cpp_string news;
      void **slot;

      news.len = NODE_LEN (hn);
      news.text= NODE_NAME (hn);
      slot = htab_find_slot (ss->definedhash, &news, INSERT);
      if (*slot == NULL)
	{
	  struct cpp_string *sp;
	  unsigned char *text;
	  
	  sp = xmalloc (sizeof (struct cpp_string));
	  *slot = sp;

	  sp->len = NODE_LEN (hn);
	  sp->text = text = xmalloc (NODE_LEN (hn));
	  memcpy (text, NODE_NAME (hn), NODE_LEN (hn));
	}
    }

  return 1;
}

/* Hash some memory in a generic way.  */

static hashval_t
hashmem (p_p, sz)
     const void *p_p;
     size_t sz;
{
  const unsigned char *p = (const unsigned char *)p_p;
  size_t i;
  hashval_t h;
  
  h = 0;
  for (i = 0; i < sz; i++)
    h = h * 67 - (*p++ - 113);
  return h;
}

/* Hash a cpp string for the hashtable machinery.  */

static hashval_t
cpp_string_hash (a_p)
     const void *a_p;
{
  const struct cpp_string *a = (const struct cpp_string *) a_p;
  return hashmem (a->text, a->len);
}

/* Compare two cpp strings for the hashtable machinery.  */

static int
cpp_string_eq (a_p, b_p)
     const void *a_p;
     const void *b_p;
{
  const struct cpp_string *a = (const struct cpp_string *) a_p;
  const struct cpp_string *b = (const struct cpp_string *) b_p;
  return (a->len == b->len
	  && memcmp (a->text, b->text, a->len) == 0);
}

/* Save the current definitions of the cpp_reader for dependency
   checking purposes.  When writing a precompiled header, this should
   be called at the same point in the compilation as cpp_valid_state
   would be called when reading the precompiled header back in.  */

int
cpp_save_state (r, f)
     cpp_reader *r;
     FILE *f;
{
  /* Save the list of non-void identifiers for the dependency checking.  */
  r->savedstate = xmalloc (sizeof (struct cpp_savedstate));
  r->savedstate->definedhash = htab_create (100, cpp_string_hash, 
					    cpp_string_eq, NULL);
  cpp_forall_identifiers (r, save_idents, r->savedstate);
  
  /* Write out the list of defined identifiers.  */
  cpp_forall_identifiers (r, write_macdef, f);

  return 0;
}

/* Calculate the 'hashsize' field of the saved state.  */

static int
count_defs (pfile, hn, ss_p)
     cpp_reader *pfile ATTRIBUTE_UNUSED;
     cpp_hashnode *hn;
     void *ss_p;
{
  struct cpp_savedstate *const ss = (struct cpp_savedstate *)ss_p;
  
  switch (hn->type)
    {
    case NT_MACRO:
      if (hn->flags & NODE_BUILTIN)
	return 1;
      
      /* else fall through.  */

    case NT_VOID:
      {
	struct cpp_string news;
	void **slot;
	
	news.len = NODE_LEN (hn);
	news.text = NODE_NAME (hn);
	slot = htab_find (ss->definedhash, &news);
	if (slot == NULL)
	  ss->hashsize += NODE_LEN (hn) + 1;
      }
      return 1;

    case NT_ASSERTION:
      /* Not currently implemented.  */
      return 1;

    default:
      abort ();
    }
}

/* Write the identifiers into 'definedstrs' of the state.  */

static int
write_defs (pfile, hn, ss_p)
     cpp_reader *pfile ATTRIBUTE_UNUSED;
     cpp_hashnode *hn;
     void *ss_p;
{
  struct cpp_savedstate *const ss = (struct cpp_savedstate *)ss_p;
  
  switch (hn->type)
    {
    case NT_MACRO:
      if (hn->flags & NODE_BUILTIN)
	return 1;
      
      /* else fall through.  */

    case NT_VOID:
      {
	struct cpp_string news;
	void **slot;
	
	news.len = NODE_LEN (hn);
	news.text = NODE_NAME (hn);
	slot = htab_find (ss->definedhash, &news);
	if (slot == NULL)
	  {
	    memcpy (ss->definedstrs, NODE_NAME (hn), NODE_LEN (hn));
	    ss->definedstrs[NODE_LEN (hn)] = 0;
	    ss->definedstrs += NODE_LEN (hn) + 1;
	  }
      }
      return 1;

    case NT_ASSERTION:
      /* Not currently implemented.  */
      return 1;

    default:
      abort ();
    }
}

/* Write out the remainder of the dependency information.  This should be
   called after the PCH is ready to be saved.  */

int
cpp_write_pch_deps (r, f)
     cpp_reader *r;
     FILE *f;
{
  struct macrodef_struct z;
  struct cpp_savedstate *const ss = r->savedstate;
  unsigned char *definedstrs;
  
  ss->hashsize = 0;
  
  /* Write out the list of identifiers which have been seen and
     weren't defined to anything previously.  */
  cpp_forall_identifiers (r, count_defs, ss);
  definedstrs = ss->definedstrs = xmalloc (ss->hashsize);
  cpp_forall_identifiers (r, write_defs, ss);
  memset (&z, 0, sizeof (z));
  z.definition_length = ss->hashsize;
  if (fwrite (&z, sizeof (z), 1, f) != 1
      || fwrite (definedstrs, ss->hashsize, 1, f) != 1)
    {
      cpp_errno (r, DL_ERROR, "while writing precompiled header");
      return -1;
    }
  free (definedstrs);

  /* Free the saved state.  */
  free (ss);
  r->savedstate = NULL;
  return 0;
}

/* Write out the definitions of the preprocessor, in a form suitable for
   cpp_read_state.  */

int
cpp_write_pch_state (r, f)
     cpp_reader *r;
     FILE *f;
{
  struct macrodef_struct z;

  /* Write out the list of defined identifiers.  */
  cpp_forall_identifiers (r, write_macdef, f);
  memset (&z, 0, sizeof (z));
  if (fwrite (&z, sizeof (z), 1, f) != 1)
    {
      cpp_errno (r, DL_ERROR, "while writing precompiled header");
      return -1;
    }

  if (!r->deps)
    r->deps = deps_init ();

  if (deps_save (r->deps, f) != 0)
    {
      cpp_errno (r, DL_ERROR, "while writing precompiled header");
      return -1;
    }

  return 0;
}

/* Return nonzero if FD is a precompiled header which is consistent
   with the preprocessor's current definitions.  It will be consistent
   when:

   - anything that was defined just before the PCH was generated 
     is defined the same way now; and
   - anything that was not defined then, but is defined now, was not
     used by the PCH.

   NAME is used to print warnings if `warn_invalid_pch' is set in the
   reader's flags.
*/

int
cpp_valid_state (r, name, fd)
     cpp_reader *r;
     const char *name;
     int fd;
{
  struct macrodef_struct m;
  size_t namebufsz = 256;
  unsigned char *namebuf = xmalloc (namebufsz);
  unsigned char *undeftab = NULL;
  unsigned int i;
  
  /* Read in the list of identifiers that must be defined
     Check that they are defined in the same way.  */
  for (;;)
    {
      cpp_hashnode *h;
      const unsigned char *newdefn;
      
      if (read (fd, &m, sizeof (m)) != sizeof (m))
	goto error;
      
      if (m.name_length == 0)
	break;

      if (m.definition_length > namebufsz)
	{
	  free (namebuf);
	  namebufsz = m.definition_length + 256;
	  namebuf = xmalloc (namebufsz);
	}
      
      if ((size_t)read (fd, namebuf, m.definition_length) 
	  != m.definition_length)
	goto error;
      
      h = cpp_lookup (r, namebuf, m.name_length);
      if (m.flags & NODE_POISONED
	  || h->type != NT_MACRO
	  || h->flags & NODE_POISONED)
	{
	  if (CPP_OPTION (r, warn_invalid_pch))
	    cpp_error (r, DL_WARNING,
		       "%s: not used because `%.*s' not defined",
		       name, m.name_length, namebuf);
	  goto fail;
	}

      newdefn = cpp_macro_definition (r, h);
      
      if (m.definition_length != ustrlen (newdefn)
	  || memcmp (namebuf, newdefn, m.definition_length) != 0)
	{
	  if (CPP_OPTION (r, warn_invalid_pch))
	    cpp_error (r, DL_WARNING, 
	       "%s: not used because `%.*s' defined as `%s' not `%.*s'",
		       name, m.name_length, namebuf, newdefn + m.name_length,
		       m.definition_length - m.name_length,
		       namebuf +  m.name_length);
	  goto fail;
	}
    }
  free (namebuf);
  namebuf = NULL;

  /* Read in the list of identifiers that must not be defined.
     Check that they really aren't.  */
  undeftab = xmalloc (m.definition_length);
  if ((size_t) read (fd, undeftab, m.definition_length) != m.definition_length)
    goto error;
  for (i = 0; i < m.definition_length; )
    {
      int l = ustrlen (undeftab + i);
      cpp_hashnode *h;
      h = cpp_lookup (r, undeftab + i, l);
      if (h->type != NT_VOID
	  || h->flags & NODE_POISONED)
	{
	  if (CPP_OPTION (r, warn_invalid_pch))
	    cpp_error (r, DL_WARNING, "%s: not used because `%s' is defined",
		       name, undeftab + i);
	  goto fail;
	}
      i += l + 1;
    }
  free (undeftab);

  /* We win!  */
  return 0;

 error:
  cpp_errno (r, DL_ERROR, "while reading precompiled header");
  return -1;

 fail:
  if (namebuf != NULL)
    free (namebuf);
  if (undeftab != NULL)
    free (undeftab);
  return 1;
}

/* Save all the existing macros and assertions.  
   This code assumes that there might be hundreds, but not thousands of
   existing definitions.  */

struct save_macro_data {
  struct save_macro_data *next;
  size_t count;
  struct cpp_hashnode macs[64];
};


static int 
save_macros (r, h, data_p)
     cpp_reader *r ATTRIBUTE_UNUSED;
     cpp_hashnode *h;
     void *data_p;
{
  struct save_macro_data **data = (struct save_macro_data **)data_p;
  if (h->type != NT_VOID
      && (h->flags & NODE_BUILTIN) == 0)
    {
      cpp_hashnode *save;
      if ((*data)->count == ARRAY_SIZE ((*data)->macs))
	{
	  struct save_macro_data *d = *data;
	  *data = xmalloc (sizeof (struct save_macro_data));
	  (*data)->next = d;
	  (*data)->count = 0;
	}
      save = (*data)->macs + (*data)->count;
      (*data)->count++;
      memcpy (save, h, sizeof (struct cpp_hashnode));
      HT_STR (&save->ident) = xmemdup (HT_STR (HT_NODE (save)),
				       HT_LEN (HT_NODE (save)),
				       HT_LEN (HT_NODE (save)) + 1);
    }
  return 1;
}

void
cpp_prepare_state (r, data)
     cpp_reader *r;
     struct save_macro_data **data;
{
  struct save_macro_data *d = xmalloc (sizeof (struct save_macro_data));
  
  d->next = NULL;
  d->count = 0;
  cpp_forall_identifiers (r, save_macros, &d);
  *data = d;
}


/* Erase all the existing macros and assertions.  */

static int 
reset_ht (r, h, unused)
     cpp_reader *r ATTRIBUTE_UNUSED;
     cpp_hashnode *h;
     void *unused ATTRIBUTE_UNUSED;
{
  if (h->type != NT_VOID
      && (h->flags & NODE_BUILTIN) == 0)
    {
      h->type = NT_VOID;
      memset (&h->value, 0, sizeof (h->value));
    }
  return 1;
}

/* Given a precompiled header that was previously determined to be valid,
   apply all its definitions (and undefinitions) to the current state. 
   DEPNAME is passed to deps_restore.  */

int
cpp_read_state (r, name, f, data)
     cpp_reader *r;
     const char *name;
     FILE *f;
     struct save_macro_data *data;
{
  struct macrodef_struct m;
  size_t defnlen = 256;
  unsigned char *defn = xmalloc (defnlen);
  struct lexer_state old_state;
  struct save_macro_data *d;
  size_t i;
  int saved_line = r->line;

  /* First, erase all the existing hashtable entries for macros.  At
     this point, they're all from the PCH file, and their pointers
     won't be valid.  */
  cpp_forall_identifiers (r, reset_ht, NULL);

  /* Run through the carefully-saved macros, insert them.  */
  d = data;
  while (d)
    {
      struct save_macro_data *nextd;
      for (i = 0; i < d->count; i++)
	{
	  cpp_hashnode *h;
	  
	  h = cpp_lookup (r, HT_STR (HT_NODE (&d->macs[i])), 
			  HT_LEN (HT_NODE (&d->macs[i])));
	  h->type = d->macs[i].type;
	  h->flags = d->macs[i].flags;
	  h->value = d->macs[i].value;
	  free ((void *)HT_STR (HT_NODE (&d->macs[i])));
	}
      nextd = d->next;
      free (d);
      d = nextd;
    }

  old_state = r->state;

  r->state.in_directive = 1;
  r->state.prevent_expansion = 1;
  r->state.angled_headers = 0;

  /* Read in the identifiers that must be defined.  */
  for (;;)
    {
      cpp_hashnode *h;
      
      if (fread (&m, sizeof (m), 1, f) != 1)
	goto error;
      
      if (m.name_length == 0)
	break;

      if (defnlen < m.definition_length + 1)
	{
	  defnlen = m.definition_length + 256;
	  defn = xrealloc (defn, defnlen);
	}

      if (fread (defn, 1, m.definition_length, f) != m.definition_length)
	goto error;
      defn[m.definition_length] = '\0';
      
      h = cpp_lookup (r, defn, m.name_length);

      if (h->type == NT_MACRO)
	_cpp_free_definition (h);
      if (m.flags & NODE_POISONED)
	h->flags |= NODE_POISONED | NODE_DIAGNOSTIC;
      else if (m.name_length != m.definition_length)
	{
	  if (cpp_push_buffer (r, defn + m.name_length, 
			       m.definition_length - m.name_length, 
			       true, 1) != NULL)
	    {
	      if (!_cpp_create_definition (r, h))
		abort ();
	      _cpp_pop_buffer (r);
	    }
	  else
	    abort ();
	}
    }

  r->state = old_state;
  r->line = saved_line;
  free (defn);
  defn = NULL;

  if (deps_restore (r->deps, f, CPP_OPTION (r, restore_pch_deps) ? name : NULL)
      != 0)
    goto error;

  return 0;
  
 error:
  cpp_errno (r, DL_ERROR, "while reading precompiled header");
  return -1;
}
