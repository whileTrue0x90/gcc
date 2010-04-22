/* Input functions for reading LTO sections.

   Copyright 2009 Free Software Foundation, Inc.
   Contributed by Kenneth Zadeck <zadeck@naturalbridge.com>

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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "toplev.h"
#include "tree.h"
#include "expr.h"
#include "flags.h"
#include "params.h"
#include "input.h"
#include "varray.h"
#include "hashtab.h"
#include "basic-block.h"
#include "tree-flow.h"
#include "cgraph.h"
#include "function.h"
#include "ggc.h"
#include "diagnostic.h"
#include "except.h"
#include "vec.h"
#include "timevar.h"
#include "output.h"
#include "lto-streamer.h"
#include "lto-compress.h"

/* Section names.  These must correspond to the values of
   enum lto_section_type.  */
const char *lto_section_name[LTO_N_SECTION_TYPES] =
{
  "decls",
  "function_body",
  "static_initializer",
  "cgraph",
  "ipa_pure_const",
  "ipa_reference",
  "symtab",
  "opts"
};

unsigned char
lto_input_1_unsigned (struct lto_input_block *ib)
{
  if (ib->p >= ib->len)
    internal_error ("bytecode stream: trying to read %d bytes "
		    "after the end of the input buffer", ib->p - ib->len);

  return (ib->data[ib->p++]);
}


/* Read an ULEB128 Number of IB.  */

unsigned HOST_WIDE_INT
lto_input_uleb128 (struct lto_input_block *ib)
{
  unsigned HOST_WIDE_INT result = 0;
  int shift = 0;
  unsigned HOST_WIDE_INT byte;

  while (true)
    {
      byte = lto_input_1_unsigned (ib);
      result |= (byte & 0x7f) << shift;
      shift += 7;
      if ((byte & 0x80) == 0)
	return result;
    }
}

/* HOST_WIDEST_INT version of lto_input_uleb128.  IB is as in
   lto_input_uleb128.  */

unsigned HOST_WIDEST_INT
lto_input_widest_uint_uleb128 (struct lto_input_block *ib)
{
  unsigned HOST_WIDEST_INT result = 0;
  int shift = 0;
  unsigned HOST_WIDEST_INT byte;

  while (true)
    {
      byte = lto_input_1_unsigned (ib);
      result |= (byte & 0x7f) << shift;
      shift += 7;
      if ((byte & 0x80) == 0)
	return result;
    }
}

/* Read an SLEB128 Number of IB.  */

HOST_WIDE_INT
lto_input_sleb128 (struct lto_input_block *ib)
{
  HOST_WIDE_INT result = 0;
  int shift = 0;
  unsigned HOST_WIDE_INT byte;

  while (true)
    {
      byte = lto_input_1_unsigned (ib);
      result |= (byte & 0x7f) << shift;
      shift += 7;
      if ((byte & 0x80) == 0)
	{
	  if ((shift < HOST_BITS_PER_WIDE_INT) && (byte & 0x40))
	    result |= - ((HOST_WIDE_INT)1 << shift);

	  return result;
	}
    }
}


/* Hooks so that the ipa passes can call into the lto front end to get
   sections.  */

static struct lto_file_decl_data ** file_decl_data;
static lto_get_section_data_f* get_section_f;
static lto_free_section_data_f* free_section_f;


/* This is called from the lto front end to set up the hooks that are
   used by the ipa passes to get the data that they will
   deserialize.  */

void
lto_set_in_hooks (struct lto_file_decl_data ** data,
		  lto_get_section_data_f* get_f,
		  lto_free_section_data_f* free_f)
{
  file_decl_data = data;
  get_section_f = get_f;
  free_section_f = free_f;
}


/* Return an array of file decl datas for all of the files passed to
   this compilation.  */

struct lto_file_decl_data **
lto_get_file_decl_data (void)
{
  gcc_assert (file_decl_data);
  return file_decl_data;
}

/* Buffer structure for accumulating data from compression callbacks.  */

struct lto_buffer
{
  char *data;
  size_t length;
};

/* Compression callback, append LENGTH bytes from DATA to the buffer pointed
   to by OPAQUE.  */

static void
lto_append_data (const char *data, unsigned length, void *opaque)
{
  struct lto_buffer *buffer = (struct lto_buffer *) opaque;

  buffer->data = (char *) xrealloc (buffer->data, buffer->length + length);
  memcpy (buffer->data + buffer->length, data, length);
  buffer->length += length;
}

/* Header placed in returned uncompressed data streams.  Allows the
   uncompressed allocated data to be mapped back to the underlying
   compressed data for use with free_section_f.  */

struct lto_data_header
{
  const char *data;
  size_t len;
};

/* Return a char pointer to the start of a data stream for an LTO pass
   or function.  FILE_DATA indicates where to obtain the data.
   SECTION_TYPE is the type of information to be obtained.  NAME is
   the name of the function and is only used when finding a function
   body; otherwise it is NULL.  LEN is the size of the data
   returned.  */

const char *
lto_get_section_data (struct lto_file_decl_data *file_data,
		      enum lto_section_type section_type,
		      const char *name,
		      size_t *len)
{
  const char *data = (get_section_f) (file_data, section_type, name, len);
  const size_t header_length = sizeof (struct lto_data_header);
  struct lto_data_header *header;
  struct lto_buffer buffer;
  struct lto_compression_stream *stream;
  lto_stats.section_size[section_type] += *len;

  if (data == NULL)
    return NULL;

  /* FIXME lto: WPA mode does not write compressed sections, so for now
     suppress uncompression if flag_ltrans.  */
  if (flag_ltrans)
    return data;

  /* Create a mapping header containing the underlying data and length,
     and prepend this to the uncompression buffer.  The uncompressed data
     then follows, and a pointer to the start of the uncompressed data is
     returned.  */
  header = (struct lto_data_header *) xmalloc (header_length);
  header->data = data;
  header->len = *len;

  buffer.data = (char *) header;
  buffer.length = header_length;

  stream = lto_start_uncompression (lto_append_data, &buffer);
  lto_uncompress_block (stream, data, *len);
  lto_end_uncompression (stream);

  *len = buffer.length - header_length;
  return buffer.data + header_length;
}


/* Free the data found from the above call.  The first three
   parameters are the same as above.  DATA is the data to be freed and
   LEN is the length of that data.  */

void
lto_free_section_data (struct lto_file_decl_data *file_data,
		       enum lto_section_type section_type,
		       const char *name,
		       const char *data,
		       size_t len)
{
  const size_t header_length = sizeof (struct lto_data_header);
  const char *real_data = data - header_length;
  const struct lto_data_header *header
    = (const struct lto_data_header *) real_data;

  gcc_assert (free_section_f);

  /* FIXME lto: WPA mode does not write compressed sections, so for now
     suppress uncompression mapping if flag_ltrans.  */
  if (flag_ltrans)
    {
      (free_section_f) (file_data, section_type, name, data, len);
      return;
    }

  /* The underlying data address has been extracted from the mapping header.
     Free that, then free the allocated uncompression buffer.  */
  (free_section_f) (file_data, section_type, name, header->data, header->len);
  free (CONST_CAST (char *, real_data));
}


/* Load a section of type SECTION_TYPE from FILE_DATA, parse the
   header and then return an input block pointing to the section.  The
   raw pointer to the section is returned in DATAR and LEN.  These are
   used to free the section.  Return NULL if the section is not present.  */

struct lto_input_block *
lto_create_simple_input_block (struct lto_file_decl_data *file_data,
			       enum lto_section_type section_type,
			       const char **datar, size_t *len)
{
  const char *data = lto_get_section_data (file_data, section_type, NULL, len);
  const struct lto_simple_header * header
    = (const struct lto_simple_header *) data;

  struct lto_input_block* ib_main;
  int32_t main_offset = sizeof (struct lto_simple_header);

  if (!data)
    return NULL;

  ib_main = XNEW (struct lto_input_block);

  *datar = data;
  LTO_INIT_INPUT_BLOCK_PTR (ib_main, data + main_offset,
			    0, header->main_size);

  return ib_main;
}


/* Close the section returned from a call to
   LTO_CREATE_SIMPLE_INPUT_BLOCK.  IB is the input block returned from
   that call.  The FILE_DATA and SECTION_TYPE are the same as what was
   passed to that call and the DATA and LEN are what was returned from
   that call.  */

void
lto_destroy_simple_input_block (struct lto_file_decl_data *file_data,
				enum lto_section_type section_type,
				struct lto_input_block *ib,
				const char *data, size_t len)
{
  free (ib);
  lto_free_section_data (file_data, section_type, NULL, data, len);
}

/*****************************************************************************/
/* Record renamings of static declarations                                   */
/*****************************************************************************/

struct lto_renaming_slot
{
  const char *old_name;
  const char *new_name;
};

/* Returns a hash code for P.  */

static hashval_t
hash_name (const void *p)
{
  const struct lto_renaming_slot *ds = (const struct lto_renaming_slot *) p;
  return (hashval_t) htab_hash_string (ds->new_name);
}

/* Returns nonzero if P1 and P2 are equal.  */

static int
eq_name (const void *p1, const void *p2)
{
  const struct lto_renaming_slot *s1 =
    (const struct lto_renaming_slot *) p1;
  const struct lto_renaming_slot *s2 =
    (const struct lto_renaming_slot *) p2;

  return strcmp (s1->new_name, s2->new_name) == 0;
}

/* Free a renaming table entry.  */

static void
renaming_slot_free (void *slot)
{
  struct lto_renaming_slot *s = (struct lto_renaming_slot *) slot;

  free (CONST_CAST (void *, (const void *) s->old_name));
  free (CONST_CAST (void *, (const void *) s->new_name));
  free ((void *) s);
}

/* Create an empty hash table for recording declaration renamings.  */

htab_t
lto_create_renaming_table (void)
{
  return htab_create (37, hash_name, eq_name, renaming_slot_free);
}

/* Record a declaration name mapping OLD_NAME -> NEW_NAME.  DECL_DATA
   holds the renaming hash table to use.  */

void
lto_record_renamed_decl (struct lto_file_decl_data *decl_data,
			 const char *old_name, const char *new_name)
{
  void **slot;
  struct lto_renaming_slot r_slot;

  r_slot.new_name = new_name;
  slot = htab_find_slot (decl_data->renaming_hash_table, &r_slot, INSERT);
  if (*slot == NULL)
    {
      struct lto_renaming_slot *new_slot = XNEW (struct lto_renaming_slot);
      new_slot->old_name = xstrdup (old_name);
      new_slot->new_name = xstrdup (new_name);
      *slot = new_slot;
    }
  else
    gcc_unreachable ();
}


/* Given a string NAME, return the string that it has been mapped to
   by lto_record_renamed_decl.  If NAME was not renamed, it is
   returned unchanged.  DECL_DATA holds the renaming hash table to use.  */

const char *
lto_get_decl_name_mapping (struct lto_file_decl_data *decl_data,
			   const char *name)
{
  htab_t renaming_hash_table = decl_data->renaming_hash_table;
  struct lto_renaming_slot *slot;
  struct lto_renaming_slot r_slot;

  r_slot.new_name = name;
  slot = (struct lto_renaming_slot *) htab_find (renaming_hash_table, &r_slot);
  if (slot)
    return slot->old_name;
  else
    return name;
}

/*****************************************************************************/
/* Input decl state object.                                                  */
/*****************************************************************************/

/* Return a newly created in-decl state object. */

struct lto_in_decl_state *
lto_new_in_decl_state (void)
{
  struct lto_in_decl_state *state;

  state = ((struct lto_in_decl_state *) xmalloc (sizeof (*state)));
  memset (state, 0, sizeof (*state));
  return state;
}

/* Delete STATE and its components. */

void
lto_delete_in_decl_state (struct lto_in_decl_state *state)
{
  int i;

  for (i = 0; i < LTO_N_DECL_STREAMS; i++)
    if (state->streams[i].trees)
      free (state->streams[i].trees);
  free (state);
}

/* Hashtable helpers. lto_in_decl_states are hash by their function decls. */

hashval_t
lto_hash_in_decl_state (const void *p)
{
  const struct lto_in_decl_state *state = (const struct lto_in_decl_state *) p;
  return htab_hash_pointer (state->fn_decl);
}

/* Return true if the fn_decl field of the lto_in_decl_state pointed to by
   P1 equals to the function decl P2. */

int
lto_eq_in_decl_state (const void *p1, const void *p2)
{
  const struct lto_in_decl_state *state1 =
   (const struct lto_in_decl_state *) p1;
  const struct lto_in_decl_state *state2 =
   (const struct lto_in_decl_state *) p2;
  return state1->fn_decl == state2->fn_decl;
}


/* Search the in-decl state of a function FUNC contained in the file
   associated with FILE_DATA.  Return NULL if not found.  */

struct lto_in_decl_state*
lto_get_function_in_decl_state (struct lto_file_decl_data *file_data,
				tree func)
{
  struct lto_in_decl_state temp;
  void **slot;

  temp.fn_decl = func;
  slot = htab_find_slot (file_data->function_decl_states, &temp, NO_INSERT);
  return slot? ((struct lto_in_decl_state*) *slot) : NULL;
}
