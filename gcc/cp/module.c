/* -*- C++ -*- modules.  Experimental!
   Copyright (C) 2017 Free Software Foundation, Inc.
   Written by Nathan Sidwell <nathan@acm.org>

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

/* MODULE_STAMP is a #define passed in from the Make file.  When
   present, it is used for version stamping the binary files, and
   indicates experimentalness of the module system.

   Comments in this file have a non-negligible chance of being wrong
   or at least inaccurate.  Due to (a) my misunderstanding, (b)
   ambiguities that I have interpretted differently to original intent
   (c) changes in the specification, (d) my poor wording.  */

#ifndef MODULE_STAMP
#error "Stahp! What are you doing? This is not ready yet."
#endif

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "cp-tree.h"
#include "stringpool.h"
#include "dumpfile.h"
#include "bitmap.h"
#include "cgraph.h"
#include "tree-iterator.h"

/* Id for dumping the class heirarchy.  */
int module_dump_id;
 
/* State of a particular module. */
struct GTY(()) module_state
{
  /* We always import & export ourselves.  */
  bitmap imports;	/* Transitive modules we're importing.  */
  bitmap exports;	/* Subset of that, that we're exporting.  */
  tree name;		/* Name of the module.  */
  vec<tree, va_gc> *name_parts;  /* Split parts of name.  */
  int direct_import;	/* Direct import/rexport of main module.  */
  /* Don't need to record the module's index, yet.  */
  unsigned crc;		/* CRC we saw reading it in. */
  unsigned HOST_WIDE_INT stamp;	/* Timestamp we saw reading it in.  */

 public:
  module_state ();

 public:
  void freeze (const module_state *);
  void set_index (unsigned index);
  void set_name (tree name);
  void do_import (unsigned index, bool is_export);

 public:
  void dump (FILE *, bool);
};

/* Vector of module state.  */
static GTY(()) vec<module_state *, va_gc> *modules;

/* We need a module state, even if we're not a module.  We promote
   this to a real module upon meeting the module declaration.  */
static GTY(()) module_state *this_module;

/* Map from identifier to module index. */
static GTY(()) hash_map<lang_identifier *, unsigned> *module_map;

module_state::module_state ()
  : imports (BITMAP_GGC_ALLOC ()), exports (BITMAP_GGC_ALLOC ()),
    name (NULL_TREE), name_parts (NULL), direct_import (0),
    crc (0), stamp (0)
{
}

void module_state::freeze (const module_state *other)
{
  gcc_assert (!other->name);
  bitmap_copy (imports, other->imports);
}

/* We've been assigned INDEX.  Mark the self-import-export bits.  */

void
module_state::set_index (unsigned index)
{
  bitmap_set_bit (imports, index);
  bitmap_set_bit (exports, index);
}

/* Set NAME and PARTS fields from incoming NAME.  The name must have
   already been checked for well-formedness.  */

void
module_state::set_name (tree name_)
{
  name = name_;
  size_t len = IDENTIFIER_LENGTH (name);
  const char *ptr = IDENTIFIER_POINTER (name);
  const char *dot;

  do
    {
      dot = (const char *)memchr (ptr, '.', len);
      size_t l = dot ? dot - ptr : len;
      vec_safe_reserve (name_parts,
			vec_safe_length (name_parts) + 1,
			!name_parts && !dot);
      name_parts->quick_push (get_identifier_with_length (ptr, l));
      if (dot)
	l++;
      ptr += l;
      len -= l;
    }
  while (dot);
}

/* Return the IDENTIFIER_NODE naming module IX.  This is the name
   including dots.  */

tree
module_name (unsigned ix)
{
  return (*modules)[ix]->name;
}

/* Return the vector of IDENTIFIER_NODES naming module IX.  These are
   individual identifers per sub-module component.  */

vec<tree, va_gc> *
module_name_parts (unsigned ix)
{
  return (*modules)[ix]->name_parts;
}

/* Return the bitmap describing what modules are imported into
   MODULE.  Remember, we always import ourselves.  */

bitmap
module_import_bitmap (unsigned ix)
{
  const module_state *state = (*modules)[ix];

  return state ? state->imports : NULL;
}

/* We've just directly imported INDEX.  Update our import/export
   bitmaps.  TOP is true, if we're the main module.  IS_EXPORT is true
   if we're reexporting the module.  */

void
module_state::do_import (unsigned index, bool is_export)
{
  module_state *other = (*modules)[index];

  if (this == this_module)
    other->direct_import = 1 + is_export;
  bitmap_ior_into (imports, other->exports);
  if (is_export)
    bitmap_ior_into (exports, other->exports);
}

enum import_kind 
{
  ik_indirect,
  ik_direct,
  ik_interface,
  ik_implementation
};

/* Byte serializer base.  */
class cpm_serial {
protected:
  FILE *stream;
public:
  const char *name;
protected:
  char *buffer;
  size_t pos;
  size_t len;
  size_t alloc;
  int err;
  unsigned bit_val;
  unsigned bit_pos;
  unsigned crc;

public:
  cpm_serial (FILE *, const char *);
  ~cpm_serial ();

public:
  int error ()
  {
    return err;
  }

public:
  /* Set an error.  We store the first errno.  */
  void bad (int e = -1)
  {
    if (!err)
      err = e;
  }

protected:
  /* Finish bit packet.  Compute crc of bits used, rewind the bytes
     not used.  */
  unsigned bit_flush ()
  {
    gcc_assert (bit_pos);
    unsigned bytes = (bit_pos + 7) / 8;
    pos -= 4 - bytes;
    crc_unsigned_n (bit_val, bytes);
    bit_pos = 0;
    bit_val = 0;
    return bytes;
  }

protected:
  void crc_unsigned_n (unsigned v, unsigned n)
  {
    crc = crc32_unsigned_n (crc, v, n);
  }
  void crc_buffer (const char *ptr, size_t l);
  template<typename T> void crc_unsigned (T v)
  {
    unsigned bytes = sizeof (T);
    while (bytes > 4)
      {
	bytes -= 4;
	crc_unsigned_n (unsigned (v >> (bytes * 8)), 4);
      }
    crc_unsigned_n (unsigned (v), bytes);
  }
};

cpm_serial::cpm_serial (FILE *s, const char *n)
  :stream (s), name (n), pos (0), len (0),
   /* Force testing of buffer extension. */
   alloc (MODULE_STAMP ? 1 : 32768),
   err (0), bit_val (0), bit_pos (0), crc (0)
{
  buffer = XNEWVEC (char, alloc);
}

cpm_serial::~cpm_serial ()
{
  gcc_assert (pos == len || err);
  XDELETEVEC (buffer);
}

void
cpm_serial::crc_buffer (const char *ptr, size_t l)
{
  unsigned c = crc;
  for (size_t ix = 0; ix != l; ix++)
    c = crc32_byte (c, ptr[ix]);
  crc = c;
}

class cpm_stream;

/* Byte stream writer.  */
class cpm_writer : public cpm_serial {
  /* Bit instrumentation.  */
  unsigned spans[3];
  unsigned lengths[3];
  int is_set;
  unsigned checksums;

public:
  cpm_writer (FILE *s, const char *n)
    : cpm_serial (s, n)
  {
    spans[0] = spans[1] = spans[2] = 0;
    lengths[0] = lengths[1] = lengths[2] = 0;
    is_set = -1;
    checksums = 0;
  }
  ~cpm_writer ()
  {
  }

private:
  size_t reserve (size_t);
  void flush ();
  void bytes4 (unsigned);

public:
  int done ()
  {
    flush ();
    if (fflush (stream))
      bad (errno);
    return error ();
  }

public:
  void checkpoint ();
  void instrument (cpm_stream *d);

public:
  void b (bool);
  void bflush ();

public:
  void c (unsigned char);
  void i (int);
  void u (unsigned);
  void s (size_t s);
  void wi (HOST_WIDE_INT);
  void wu (unsigned HOST_WIDE_INT);
  void str (const char *, size_t);
  void buf (const char *, size_t);
};

/* Byte stream reader.  */
class cpm_reader : public cpm_serial {
public:
  cpm_reader (FILE *s, const char *n)
    : cpm_serial (s, n)
  {
  }
  ~cpm_reader ()
  {
  }

private:
  size_t fill (size_t);
  unsigned bytes4 ();

public:
  int done (bool atend = true)
  {
    if (atend && fill (1))
      bad ();
    return error ();
  }

public:
  bool checkpoint ();
  unsigned get_crc () const
  {
    return crc;
  }

public:
  bool b ();
  void bflush ();
private:
  void bfill ();

public:
  int c ();
  int i ();
  unsigned u ();
  size_t s ();
  HOST_WIDE_INT wi ();
  unsigned HOST_WIDE_INT wu ();
  const char *str (size_t * = NULL);
  const char *buf (size_t);
};

/* Checkpoint a crc.  */

inline void
cpm_writer::checkpoint ()
{
  checksums++;
  bytes4 (crc);
}

bool
cpm_reader::checkpoint ()
{
  if (bytes4 () != crc)
    {
      /* Map checksum error onto a reasonably specific errno.  */
#if defined (EPROTO)
      bad (EPROTO);
#elif defined (EBADMSG)
      bad (EBADMSG);
#elif defined (EIO)
      bad (EIO);
#else
      bad ();
#endif
    }
  return !error ();
}

/* Finish a set of bools.  */

void
cpm_writer::bflush ()
{
  if (bit_pos)
    {
      bytes4 (bit_val);
      lengths[2] += bit_flush ();
    }
  spans[2]++;
  is_set = -1;
}

void
cpm_reader::bflush ()
{
  if (bit_pos)
    bit_flush ();
}

/* When reading, we don't know how many bools we'll read in.  So read
   4 bytes-worth, and then rewind when flushing if we didn't need them
   all.  */

void
cpm_reader::bfill ()
{
  bit_val = bytes4 ();
}

/* Low level cpm_readers and cpm_writers.  I did think about making these
   templatized, but that started to look error prone, so went with
   type-specific names.
   b - bools,
   i, u - ints/unsigned
   wi/wu - wide ints/unsigned
   s - size_t
   buf - fixed size buffer
   str - variable length string  */

/* Bools are packed into bytes.  You cannot mix bools and non-bools.
   You must call bflush before emitting another type.  So batch your
   bools.

   It may be worth optimizing for most bools being zero.  some kind of
   run-length encoding?  */

void
cpm_writer::b (bool x)
{
  if (is_set != x)
    {
      is_set = x;
      spans[x]++;
    }
  lengths[x]++;
  bit_val |= unsigned (x) << bit_pos++;
  if (bit_pos == 32)
    {
      bytes4 (bit_val);
      lengths[2] += bit_flush ();
    }
}

bool
cpm_reader::b ()
{
  if (!bit_pos)
    bfill ();
  bool v = (bit_val >> bit_pos++) & 1;
  if (bit_pos == 32)
    bit_flush ();
  return v;
}

/* Exactly 4 bytes.  Used internally for bool packing and crc
   transfer -- hence no crc here.  */

void
cpm_writer::bytes4 (unsigned val)
{
  reserve (4);
  buffer[pos++] = val;
  buffer[pos++] = val >> 8;
  buffer[pos++] = val >> 16;
  buffer[pos++] = val >> 24;
}

unsigned
cpm_reader::bytes4 ()
{
  unsigned val = 0;
  if (fill (4) != 4)
    bad ();
  else
    {
      val |= (unsigned char)buffer[pos++];
      val |= (unsigned char)buffer[pos++] << 8;
      val |= (unsigned char)buffer[pos++] << 16;
      val |= (unsigned char)buffer[pos++] << 24;
    }

  return val;
}

/* Chars are unsigned and written as single bytes. */

void
cpm_writer::c (unsigned char v)
{
  reserve (1);
  buffer[pos++] = v;
  crc_unsigned (v);
}

int
cpm_reader::c ()
{
  int v = 0;
  if (fill (1))
    v = (unsigned char)buffer[pos++];
  else
    bad ();
  crc_unsigned (v);
  return v;
}

/* Ints are written as sleb128.  I suppose we could pack the first
   few bits into any partially-filled bool buffer.  */

void
cpm_writer::i (int v)
{
  crc_unsigned (v);
  reserve ((sizeof (v) * 8 + 6) / 7);

  int end = v < 0 ? -1 : 0;
  bool more;

  do
    {
      unsigned byte = v & 127;
      v >>= 6; /* Signed shift.  */
      more = v != end;
      buffer[pos++] = byte | (more << 7);
      v >>= 1; /* Signed shift.  */
    }
  while (more);
}

int
cpm_reader::i ()
{
  int v = 0;
  unsigned bit = 0;
  size_t bytes = fill ((sizeof (v) * 8 + 6) / 7);
  unsigned byte;

  do
    {
      if (!bytes--)
	{
	  bad ();
	  return v;
	}
      byte = buffer[pos++];
      v |= (byte & 127) << bit;
      bit += 7;
    }
  while (byte & 128);

  if (byte & 0x40 && bit < sizeof (v) * 8)
    v |= ~(unsigned)0 << bit;
  crc_unsigned (v);
  return v;
}

/* Unsigned are written as uleb128.  */

void
cpm_writer::u (unsigned v)
{
  crc_unsigned (v);
  reserve ((sizeof (v) * 8 + 6) / 7);

  bool more;
  do
    {
      unsigned byte = v & 127;
      v >>= 7;
      more = v != 0;
      buffer[pos++] = byte | (more << 7);
    }
  while (more);
}

unsigned
cpm_reader::u ()
{
  unsigned v = 0;
  unsigned bit = 0;
  size_t bytes = fill ((sizeof (v) * 8 + 6) / 7);
  unsigned byte;

  do
    {
      if (!bytes--)
	{
	  bad ();
	  return v;
	}
      byte = buffer[pos++];
      v |= (byte & 127) << bit;
      bit += 7;
    }
  while (byte & 128);
  crc_unsigned (v);

  return v;
}

void
cpm_writer::wi (HOST_WIDE_INT v)
{
  crc_unsigned (v);
  reserve ((sizeof (v) * 8 + 6) / 7);

  int end = v < 0 ? -1 : 0;
  bool more;

  do
    {
      unsigned byte = v & 127;
      v >>= 6; /* Signed shift.  */
      more = v != end;
      buffer[pos++] = byte | (more << 7);
      v >>= 1; /* Signed shift.  */
    }
  while (more);
}

HOST_WIDE_INT
cpm_reader::wi ()
{
  HOST_WIDE_INT v = 0;
  unsigned bit = 0;
  size_t bytes = fill ((sizeof (v) * 8 + 6) / 7);
  unsigned byte;

  do
    {
      if (!bytes--)
	{
	  bad ();
	  return v;
	}
      byte = buffer[pos++];
      v |= (byte & 127) << bit;
      bit += 7;
    }
  while (byte & 128);

  if (byte & 0x40 && bit < sizeof (v) * 8)
    v |= ~(unsigned HOST_WIDE_INT)0 << bit;
  crc_unsigned (v);
  return v;
}

inline void
cpm_writer::wu (unsigned HOST_WIDE_INT v)
{
  wi ((HOST_WIDE_INT) v);
}

inline unsigned HOST_WIDE_INT
cpm_reader::wu ()
{
  return (unsigned HOST_WIDE_INT) wi ();
}

inline void
cpm_writer::s (size_t s)
{
  if (sizeof (s) == sizeof (unsigned))
    u (s);
  else
    wu (s);
}

inline size_t
cpm_reader::s ()
{
  if (sizeof (size_t) == sizeof (unsigned))
    return u ();
  else
    return wu ();
}

void
cpm_writer::buf (const char *buf, size_t len)
{
  crc_buffer (buf, len);
  reserve (len);
  memcpy (buffer + pos, buf, len);
  pos += len;
}

const char *
cpm_reader::buf (size_t len)
{
  size_t have = fill (len);
  char *buf = &buffer[pos];
  if (have < len)
    {
      memset (buf + have, 0, len - have);
      bad ();
    }
  pos += have;
  crc_buffer (buf, len);
  return buf;
}

/* Strings:
   u:length
   checkpoint
   buf:bytes
*/

void
cpm_writer::str (const char *string, size_t len)
{
  s (len);
  checkpoint ();
  buf (string, len + 1);
}

const char *
cpm_reader::str (size_t *len_p)
{
  size_t len = s ();

  /* We're about to trust some user data.  */
  if (!checkpoint ())
    len = 0;
  *len_p = len;
  const char *str = buf (len + 1);
  if (str[len])
    {
      /* Force read string to be not totally broken.  */
      buffer[pos-1] = 0;
      bad ();
    }
  return str;
}

void
cpm_writer::flush ()
{
  size_t bytes = fwrite (buffer, 1, pos, stream);

  if (bytes != pos)
    bad (errno);
  pos = 0;
}

size_t
cpm_writer::reserve (size_t want)
{
  size_t have = alloc - pos;
  if (have < want)
    {
      flush ();
      if (alloc < want)
	{
	  alloc = want + (want / 8); /* Some hysteresis.  */
	  buffer = XRESIZEVEC (char, buffer, alloc);
	}
      have = alloc;
    }
  return have;
}

size_t
cpm_reader::fill (size_t want)
{
  size_t have = len - pos;
  if (have < want)
    {
      memmove (buffer, buffer + pos, len - pos);
      len -= pos;
      pos = 0;
      if (alloc < want)
	{
	  alloc = want + (want / 8); /* Some hysteresis.  */
	  buffer = XRESIZEVEC (char, buffer, alloc);
	}
      if (size_t bytes = fread (buffer + len, 1, alloc - len, stream))
	{
	  len += bytes;
	  have = len;
	}
      else if (ferror (stream))
	{
	  clearerr (stream);
	  bad (errno);
	}
    }
  return have < want ? have : want;
}

/* Module cpm_stream base.  */
class cpm_stream {
public:
  /* Record tags.  */
  enum record_tag
  {
    /* Module-specific records.  */
    rt_eof,		/* End Of File.  duh! */
    rt_conf,		/* Config info (baked in stuff like target-triplet) */
    rt_stamp,		/* Date stamp etc.  */
    rt_flags,		/* Flags that affect AST compatibility.  */
    rt_import,		/* An import. */
    rt_binding,		/* A name-binding.  */
    rt_definition,	/* A definition. */
    rt_trees,		/* Global trees.  */
    rt_type_name,	/* A type name.  */
    rt_typeinfo_var,	/* A typeinfo object.  */
    rt_typeinfo_pseudo, /* A typeinfo pseudo type.  */
    rt_tree_base = 0x100,	/* Tree codes.  */
    rt_ref_base = 0x1000	/* Back-reference indices.  */
  };
  struct gtp
  {
    const tree *ptr;
    unsigned num;
  };

public:
  static const gtp global_tree_arys[];

private:
  unsigned tag;

private:
  FILE *d;
  unsigned depth;
  unsigned nesting;
  bool bol;

public:
  cpm_stream (cpm_stream *chain = NULL);
  ~cpm_stream ();

protected:
  /* Allocate a new reference index.  */
  unsigned next ()
  {
    return tag++;
  }

public:
  static const char *ident ();
  static int version ();

  /* Version to date. */
  static unsigned v2d (int v)
  {
    if (MODULE_STAMP && v < 0)
      return -v / 10000 + 20000000;
    else
      return v;
  }

  /* Version to time. */
  static unsigned v2t (int v)
  {
    if (MODULE_STAMP && v < 0)
      return -v % 10000;
    else
      return 0;
  }
  typedef char tbuf[8];
  typedef char dbuf[16];
  void t2s (unsigned, tbuf &);
  void d2s (unsigned, dbuf &);

public:
  FILE *dumps () const
  {
    return d;
  }
  bool dump () const 
  {
    return d != NULL;
  }
  bool dump (const char *, ...);
  void nest ()
  {
    nesting++;
  }
  void unnest ()
  {
    nesting--;
  }
};

cpm_stream::cpm_stream (cpm_stream *chain)
  : tag (rt_ref_base), d (chain ? chain->d : NULL),
    depth (chain ? chain->depth + 1 : 0), nesting (0), bol (true)
{
  gcc_assert (MAX_TREE_CODES <= rt_ref_base - rt_tree_base);
  if (!chain)
    d = dump_begin (module_dump_id, NULL);
}

cpm_stream::~cpm_stream ()
{
  if (!depth && d)
    dump_end (module_dump_id, d);
}

void
cpm_stream::t2s (unsigned t, tbuf &buf)
{
  sprintf (buf, "%02u:%02u", t / 100, t % 100);
}

void
cpm_stream::d2s (unsigned d, dbuf &buf)
{
  sprintf (buf, "%04u/%02u/%02u", d / 10000, (d / 100) % 100, (d % 100));
}

static bool
dump_nested_name (tree t, FILE *d)
{
  if (t && TYPE_P (t))
    t = TYPE_NAME (t);

  if (t && DECL_P (t))
    {
      if (tree ctx = CP_DECL_CONTEXT (t))
	if (ctx == global_namespace
	    ? ctx != t : dump_nested_name (ctx, d))
	  fputs ("::", d);
      t = DECL_NAME (t);
    }

  if (t && TREE_CODE (t) == IDENTIFIER_NODE)
    {
      fwrite (IDENTIFIER_POINTER (t), 1, IDENTIFIER_LENGTH (t), d);
      return true;
    }
  return false;
}

/* Specialized printfy thing.  */

bool
cpm_stream::dump (const char *format, ...)
{
  va_list args;
  bool no_nl = format[0] == '+';
  format += no_nl;

  if (bol)
    {
      if (depth)
	{
	  /* Module import indenting.  */
	  const char *indent = ">>>>";
	  const char *dots   = ">...>";
	  if (depth > strlen (indent))
	    indent = dots;
	  else
	    indent += strlen (indent) - depth;
	  fputs (indent, d);
	}
      if (nesting)
	{
	  /* Tree indenting.  */
	  const char *indent = "      ";
	  const char *dots  =  "   ... ";
	  if (nesting > strlen (indent))
	    indent = dots;
	  else
	    indent += strlen (indent) - nesting;
	  fputs (indent, d);
	}
    }

  va_start (args, format);
  while (const char *esc = strchr (format, '%'))
    {
      fwrite (format, 1, (size_t)(esc - format), d);
      format = ++esc;
      switch (*format++)
	{
	case 'C': /* Code */
	  {
	    tree_code code = (tree_code)va_arg (args, unsigned);
	    fputs (get_tree_code_name (code), d);
	    break;
	  }
	case 'I': /* Identifier.  */
	  {
	    tree t = va_arg (args, tree);
	    dump_nested_name (t, d);
	    break;
	  }
	case 'N': /* Name.  */
	  {
	    tree t = va_arg (args, tree);
	    fputc ('\'', d);
	    dump_nested_name (t, d);
	    fputc ('\'', d);
	    break;
	  }
	case 'M': /* Mangled name */
	  {
	    tree t = va_arg (args, tree);
	    if (t && TYPE_P (t))
	      t = TYPE_NAME (t);
	    if (t && DECL_ASSEMBLER_NAME_SET_P (t))
	      {
		fputc ('(', d);
		fputs (IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (t)), d);
		fputc (')', d);
	      }
	    break;
	  }
	case 'P': /* Pair.  */
	  {
	    tree ctx = va_arg (args, tree);
	    tree name = va_arg (args, tree);
	    fputc ('\'', d);
	    dump_nested_name (ctx, d);
	    if (ctx && ctx != global_namespace)
	      fputs ("::", d);
	    dump_nested_name (name, d);
	    fputc ('\'', d);
	    break;
	  }
	case 'R': /* Ratio */
	  {
	    unsigned a = va_arg (args, unsigned);
	    unsigned b = va_arg (args, unsigned);
	    fprintf (d, "%.1f", (float) a / (b + !b));
	    break;
	  }
	case 'U': /* long unsigned.  */
	  {
	    unsigned long u = va_arg (args, unsigned long);
	    fprintf (d, "%lu", u);
	    break;
	  }
	case 'V': /* Verson.  */
	  {
	    unsigned v = va_arg (args, unsigned);
	    tbuf time;
	    dbuf date;
	    t2s (v2t (v), time);
	    d2s (v2d (v), date);
	    fprintf (d, "%s-%s", date, time);
	    break;
	  }
	case 'p': /* Pointer. */
	  {
	    void *p = va_arg (args, void *);
	    fprintf (d, "%p", p);
	    break;
	  }
	case 's': /* String. */
	  {
	    const char *s = va_arg (args, char *);
	    fputs (s, d);
	    break;
	  }
	case 'u': /* Unsigned.  */
	  {
	    unsigned u = va_arg (args, unsigned);
	    fprintf (d, "%u", u);
	    break;
	  }
	case 'x': /* Hex. */
	  {
	    unsigned x = va_arg (args, unsigned);
	    fprintf (d, "%x", x);
	    break;
	  }
	default:
	  gcc_unreachable ();
	}
    }
  fputs (format, d);
  va_end (args);
  if (!no_nl)
    {
      bol = true;
      fputc ('\n', d);
    }
  return true;
}

const char *
cpm_stream::ident ()
{
  return "g++m";
}

int
cpm_stream::version ()
{
  /* If the on-disk format changes, update the version number.  */
  int version = 20170210;

#if defined (MODULE_STAMP)
  /* MODULE_STAMP is a decimal encoding YYMMDDhhmm in local timezone.
     Using __TIME__ doesnt work very well with boostrapping!  */
  version = -MODULE_STAMP;
#endif
  return version;
}

/* cpm_stream cpms_out.  */
class cpms_out : public cpm_stream {
  cpm_writer w;

  struct non_null : pointer_hash <void>
  {
    static bool is_deleted (value_type) {return false;}
    static void remove (value_type) {}
  };
  typedef simple_hashmap_traits<non_null, unsigned> traits;
  hash_map<void *,unsigned,traits> tree_map; /* trees to ids  */

  /* Tree instrumentation. */
  unsigned unique;
  unsigned refs;
  unsigned nulls;
  unsigned records;

public:
  cpms_out (FILE *, const char *, tree);
  ~cpms_out ();

  void instrument ();

public:
  void header (tree);
  void tag_eof ();
  void tag_conf ();
  void tag_import (unsigned ix, const module_state *);
  void tag_trees ();
  tree tag_binding (tree ns, bool, tree name, tree ovl);
  void maybe_tag_definition (tree decl);
  void tag_definition (tree node);
  int done ()
  {
    return w.done ();
  }

private:
  void tag (record_tag rt)
  {
    records++;
    w.u (rt);
  }
  unsigned insert (tree);
  void start (tree_code, tree);
  void loc (location_t);
  bool mark_present (tree);
  void globals (unsigned, const gtp *);
  void core_bools (tree);
  void core_vals (tree);
  void lang_type_bools (tree);
  void lang_type_vals (tree);
  void lang_decl_bools (tree);
  void lang_decl_vals (tree);
  void chained_decls (tree);
  void tree_vec (vec<tree, va_gc> *);
  void define_function (tree);
  void define_class (tree);
  void ident_imported_decl (tree ctx, unsigned mod, tree decl);

public:
  void tree_node (tree);
  void bindings (tree ns);
};

cpms_out::cpms_out (FILE *s, const char *n, tree name)
  :cpm_stream (), w (s, n)
{
  unique = refs = nulls = 0;
  records = 0;
  dump () && dump ("Writing module %I", name);
}

cpms_out::~cpms_out ()
{
}

void
cpm_writer::instrument (cpm_stream *d)
{
  d->dump ("Wrote %U bytes", ftell (stream));
  d->dump ("Wrote %u bits in %u bytes", lengths[0] + lengths[1],
	   lengths[2]);
  for (unsigned ix = 0; ix < 2; ix++)
    d->dump ("  %u %s spans of %R bits", spans[ix],
	     ix ? "one" : "zero", lengths[ix], spans[ix]);
  d->dump ("  %u blocks with %R bits padding", spans[2],
	   lengths[2] * 8 - (lengths[0] + lengths[1]), spans[2]);
  d->dump ("Wrote %u checksums", checksums);
}

void
cpms_out::instrument ()
{
  if (dump ())
    {
      dump ("");
      w.instrument (this);
      dump ("Wrote %u trees", unique + refs + nulls);
      dump ("  %u unique", unique);
      dump ("  %u references", refs);
      dump ("  %u nulls", nulls);
      dump ("Wrote %u records", records);
    }
}

/* Cpm_Stream in.  */
class cpms_in : public cpm_stream {
  cpm_reader r;

  /* Module state being initialized.  */
  module_state *state;

  typedef simple_hashmap_traits<int_hash<unsigned,0>,void *> traits;
  hash_map<unsigned,void *,traits> tree_map; /* ids to trees  */

  unsigned mod_ix; /* Module index.  */
  unsigned HOST_WIDE_INT stamp;  /* Expected time stamp.  */

  /* Remapping from incoming module indices to current TU. */
  unsigned remap_num;
  unsigned *remap_vec;

public:
  cpms_in (FILE *, const char *, module_state *, unsigned HOST_WIDE_INT stamp,
	   cpms_in *);
  ~cpms_in ();

public:
  bool header ();
  int tag_eof ();
  bool tag_conf ();
  bool tag_import ();
  bool tag_binding ();
  tree tag_definition ();
  bool tag_trees ();
  int read_item ();
  int done ()
  {
    return r.done ();
  }
  unsigned get_mod () const
  {
    return mod_ix;
  }

private:
  unsigned insert (tree);
  tree finish_type (tree);

private:
  bool alloc_remap_vec (unsigned limit);
  tree start (tree_code);
  tree finish (tree);
  location_t loc ();
  bool mark_present (tree);
  bool globals (unsigned, const gtp *);
  bool core_bools (tree);
  bool core_vals (tree);
  bool lang_type_bools (tree);
  bool lang_type_vals (tree);
  bool lang_decl_bools (tree);
  bool lang_decl_vals (tree);
  tree chained_decls ();
  vec<tree, va_gc> *tree_vec ();
  tree define_function (tree);
  tree define_class (tree);
  tree ident_imported_decl (tree ctx, unsigned mod, tree name);

public:
  tree tree_node ();
};

cpms_in::cpms_in (FILE *s, const char *n,
		  module_state *state_, unsigned HOST_WIDE_INT stmp,
		  cpms_in *from)
  :cpm_stream (from), r (s, n), state (state_), mod_ix (GLOBAL_MODULE_INDEX),
   stamp (stmp), remap_num (0), remap_vec (NULL)
{
  dump () && dump ("Importing %I", state->name);
}

cpms_in::~cpms_in ()
{
  free (remap_vec);
}

unsigned
cpms_out::insert (tree t)
{
  unsigned tag = next ();
  bool existed = tree_map.put (t, tag);
  gcc_assert (!existed);
  return tag;
}

unsigned
cpms_in::insert (tree t)
{
  unsigned tag = next ();
  bool existed = tree_map.put (tag, t);
  gcc_assert (!existed);
  return tag;
}

static unsigned do_module_import (location_t, tree, import_kind,
				  unsigned HOST_WIDE_INT stamp,
				  unsigned crc, cpms_in * = NULL);

/* File header
   buf:ident
   u:version
   wu:timestamp
   str:module
*/

void
cpms_out::header (tree name)
{
  char const *id = ident ();
  w.buf (id, strlen (id));

  int v = version ();
  gcc_assert (v < 0); /* Not ready for prime-time.  */
  w.i (v);

  /* Although -1 is a legitimate time, it's not going to be a very
     common problem.  */
  time_t now = time (NULL);
  if ((time_t)-1 == now)
    now = 0;
  w.wu ((unsigned HOST_WIDE_INT)now);

  dump () && dump ("Writing \"%s\" version=%V stamp=%U",
		   id, v, (unsigned long)now);

  w.str (IDENTIFIER_POINTER (name), IDENTIFIER_LENGTH (name));
}

static char *
time2str (unsigned HOST_WIDE_INT t)
{
  const char *str = "<unknown>";
  char buf[24];
  if (t)
    {
      time_t e_time = (time_t)t;
      struct tm *l_time = localtime (&e_time);
      sprintf (buf, "%04d/%02d/%02d %02d:%02d:%02d",
	       l_time->tm_year + 1900, l_time->tm_mon + 1, l_time->tm_mday,
	       l_time->tm_hour, l_time->tm_min, l_time->tm_sec);
      str = buf;
    }
  return xstrdup (str);
}

/* Diagnose mismatched timestamps.  */

static void
timestamp_mismatch (tree name, unsigned HOST_WIDE_INT expected,
		    unsigned HOST_WIDE_INT actual)
{
  char *e_str = time2str (expected);
  char *a_str = time2str (actual);
  
  error ("module %qE time stamp expected %qs, discovered %qs",
	 name, e_str, a_str);


  free (e_str);
  free (a_str);
}

bool
cpms_in::header ()
{
  const char *id = ident ();
  const char *i = r.buf (strlen (id));
  if (memcmp (id, i, strlen (id)))
    {
      error ("%qs is not a module file", r.name);
      return false;
    }

  /* Check version.  */
  int ver = version ();
  int v = r.i ();
  if (v != ver)
    {
      int ver_date = v2d (ver);
      int v_date = v2d (v);
      dbuf v_dform, ver_dform;
      d2s (v_date, v_dform);
      d2s (ver_date, ver_dform);

      if (ver_date != v_date)
	{
	  /* Dates differ, decline.  */
	  error ("%qs built by version %s, this is version %s",
		 r.name, v_dform, ver_dform);
	  return false;
	}
      else
	{
	  /* Times differ, give it a go.  */
	  int ver_time = v2t (ver);
	  int v_time = v2t (v);
	  tbuf v_tform, ver_tform;
	  t2s (v_time, v_tform);
	  t2s (ver_time, ver_tform);
	  warning (0, "%qs is version %s, but build time was %s, not %s",
		   r.name, v_dform, v_tform, ver_tform);
	}
    }
  dump () && dump  ("Expecting %V found %V", ver, v);

  /* Check timestamp.  */
  state->stamp = r.wu ();
  if (stamp && stamp != state->stamp)
    {
      timestamp_mismatch (state->name, stamp, state->stamp);
      return false;
    }

  /* Check module name.  */
  size_t l;
  const char *n = r.str (&l);
  if (l != IDENTIFIER_LENGTH (state->name)
      || memcmp (n, IDENTIFIER_POINTER (state->name), l))
    {
      error ("%qs is module %qs, expected module %qE", r.name, n, state->name);
      return false;
    }

  return true;
}

void
cpms_out::tag_eof ()
{
  tag (rt_eof);
  w.checkpoint ();
}

int
cpms_in::tag_eof ()
{
  dump () && dump ("Read eof");
  if (!r.checkpoint ())
    return false;
  /* Record the crc.  */
  state->crc = r.get_crc ();
  return -1; /* Denote EOF.  */
}

/* Record config info
   str:<target-triplet>
   str:<host-triplet>  ; lock this for now.
*/

void
cpms_out::tag_conf ()
{
  dump () && dump ("Writing target='%s', host='%s'",
		   TARGET_MACHINE, HOST_MACHINE);
  tag (rt_conf);
  w.str (TARGET_MACHINE, strlen (TARGET_MACHINE));
  w.str (HOST_MACHINE, strlen (HOST_MACHINE));
  w.checkpoint ();
}

bool
cpms_in::tag_conf ()
{
  size_t l;
  const char *targ = r.str (&l);
  if (strcmp (targ, TARGET_MACHINE))
    {
      error ("%qs is target %qs, expected %qs", r.name, targ, TARGET_MACHINE);
      return false;
    }
  const char *host = r.str (&l);
  if (strcmp (host, HOST_MACHINE))
    {
      error ("%qs is host %qs, expected %qs", r.name, host, HOST_MACHINE);
      return false;
    }

  if (!r.checkpoint ())
    return false;

  dump () && dump ("Read target='%s', host='%s'",
		   TARGET_MACHINE, HOST_MACHINE);

  return true;
}

/* Dump the global trees directly to save encoding them for no reason.
   Further types such as sizetype and global_namespace are oddly
   recursive, and this avoids having to deal with that in the
   cpm_reader.

   <ary>*
   <u:0>
*/

const cpm_stream::gtp cpm_stream::global_tree_arys[] =
  {
    {sizetype_tab, stk_type_kind_last},
    {global_trees, TI_MAX},
    {cp_global_trees, CPTI_MAX},
    {NULL, 0}
  };

void
cpms_out::tag_trees ()
{
  tag (rt_trees);
  for (unsigned ix = 0; global_tree_arys[ix].ptr; ix++)
    globals (ix, &global_tree_arys[ix]);
  w.u (0);
  w.checkpoint ();
}

bool
cpms_in::tag_trees ()
{
  for (unsigned ix = 0; global_tree_arys[ix].ptr; ix++)
    if (!globals (ix, &global_tree_arys[ix]))
      return false;
  return !r.u () && r.checkpoint ();
}

bool
cpms_out::mark_present (tree t)
{
  bool existed = true;

  if (t)
    {
      unsigned *val = &tree_map.get_or_insert (t, &existed);
      if (!existed)
	*val = next ();
    }
  w.b (existed);

  return existed;
}

bool
cpms_in::mark_present (tree t)
{
  bool existed = r.b ();

  if (!existed)
    {
      unsigned tag = next ();

      gcc_assert (t);
      tree_map.put (tag, t);
    }
  return existed;
}

/* Global tree array
   u:count
   b[]:insert_p  */

void
cpms_out::globals (unsigned ary_num, const gtp *ary_p)
{
  const tree *ary = ary_p->ptr;
  unsigned num = ary_p->num;

  w.u (ary_num);
  w.u (num);

  dump () && dump ("Writing globals %u[%u]", ary_num, num);

  unsigned outer = 0, inner = 0;
  for (unsigned ix = 0; ix != num; ix++)
    {
      dump () && !(ix & 31) && dump ("") && dump ("+\t%u:", ix);

      tree t = ary[ix];
      unsigned first = !mark_present (t);
      if (first)
	{
	  outer++;
	  if (CODE_CONTAINS_STRUCT (TREE_CODE (t), TS_TYPED)
	      && !mark_present (TREE_TYPE (t)))
	    {
	      first++;
	      inner++;
	    }
	}
      dump () && dump ("+%u", first);
    }
  w.bflush ();
  dump () && dump ("") && dump ("Wrote %u unique %u inner", outer, inner);

  w.checkpoint ();
}

bool
cpms_in::globals (unsigned ary_num, const gtp *ary_p)
{
  const tree *ary = ary_p->ptr;
  unsigned num = ary_p->num;

  if (r.u () != ary_num || r.u () != num)
    return false;

  dump () && dump ("Reading globals %u[%u]", ary_num, num);

  unsigned outer = 0, inner = 0;
  for (unsigned ix = 0; ix != num; ix++)
    {
      dump () && !(ix & 31) && dump ("") && dump ("+\t%u:", ix);

      tree t = ary[ix];
      unsigned first = !mark_present (t);
      if (first)
	{
	  outer++;
	  if (CODE_CONTAINS_STRUCT (TREE_CODE (t), TS_TYPED)
	      && !mark_present (TREE_TYPE (t)))
	    {
	      first++;
	      inner++;
	    }
	}
      dump () && dump ("+%u", first);
    }
  r.bflush ();
  dump () && dump ("") && dump ("Read %u unique %u inner", outer, inner);

  return r.checkpoint ();
}

/* Item import
   u:index
   u:direct
   u:crc
   wu:stamp
   str:module_name  */

void
cpms_out::tag_import (unsigned ix, const module_state *state)
{
  dump () && dump ("Writing %simport %I (crc=%x)",
		   state->direct_import == 2 ? "export " :
		   state->direct_import ? "" : "indirect ",
		   state->name, state->crc);
  tag (rt_import);
  w.u (ix);
  w.u (state->direct_import);
  w.u (state->crc);
  w.wu (state->stamp);
  w.str (IDENTIFIER_POINTER (state->name), IDENTIFIER_LENGTH (state->name));
  w.checkpoint ();
}

bool
cpms_in::alloc_remap_vec (unsigned limit)
{
  if (!remap_num && limit < MODULE_INDEX_LIMIT)
    {
      if (limit < THIS_MODULE_INDEX)
	limit = THIS_MODULE_INDEX;

      remap_num = limit + 1;
      remap_vec = XNEWVEC (unsigned, remap_num);
      memset (remap_vec, 0, sizeof (unsigned) * remap_num);
    }
  return limit < remap_num;
}

bool
cpms_in::tag_import ()
{
  unsigned ix = r.u ();
  unsigned direct = r.u ();
  unsigned crc = r.u ();
  unsigned HOST_WIDE_INT stamp = r.wu ();
  size_t l;
  const char *mod = r.str (&l);

  /* Validate name.  Dotted sequence of identifiers.  */
  size_t dot = 0;
  for (size_t ix = 0; ix != l; ix++)
    if (ISALPHA (mod[ix]) || mod[ix] == '_')
      continue;
    else if (dot == ix)
      goto bad;
    else if (mod[ix] == '.')
      dot = ix + 1;
    else if (!ISDIGIT (mod[ix]))
      goto bad;
  if (!l || dot == l)
    {
      bad:
      error ("module name %qs is malformed", mod);
      return false;
    }

  tree imp = get_identifier_with_length (mod, l);
  if (!r.checkpoint ())
    return false;

  /* Not designed to import after having assigned our number. */
  if (mod_ix)
    {
      error ("misordered import %qs", mod);
      return false;
    }
  if (!alloc_remap_vec (ix))
    {
      error ("import %u is out of range", ix);
      return false;
    }

  dump () && dump ("Begin nested %simport %I",
		   direct == 2 ? "export " : direct ? "" : "indirect ", imp);
  int imp_ix = do_module_import (UNKNOWN_LOCATION, imp,
				 direct ? ik_direct : ik_indirect,
				 stamp, crc, this);
  if (imp_ix != GLOBAL_MODULE_INDEX)
    {
      remap_vec[ix] = imp_ix;
      if (direct)
	{
	  bool is_export = direct == 2;
	  dump () && dump ("Direct %simport %I %u",
			   is_export ? "export " : "", imp, imp_ix);
	  state->do_import (imp_ix, direct == 2);
	}
    }

  dump () && dump ("Completed nested import %I #%u %s", imp,
		   imp_ix, imp_ix != GLOBAL_MODULE_INDEX ? "ok" : "failed");
  return imp_ix != GLOBAL_MODULE_INDEX;
}

/* NAME is bound to OVL in namespace NS.  Write out the binding.
   We also write out the definitions of things in the binding list.
   NS must have already have a binding somewhere.

   tree:ns
   tree:name
   b:main_p
   b:stat_p
   [tree:stat_type]
   tree:binding
*/

/* Return a child namespace to walk.  */

tree
cpms_out::tag_binding (tree ns, bool main_p, tree name, tree binding)
{
  tree type = NULL_TREE;
  tree value = ovl_skip_hidden (decapsulate_binding (binding, &type));

  if (type && DECL_IS_BUILTIN (type))
    type = NULL_TREE;

  if (value)
    {
      if (TREE_CODE (value) == NAMESPACE_DECL && !DECL_NAMESPACE_ALIAS (value))
	{
	  gcc_assert (!type);
	  return value;
	}

      /* Don't write builtins (mostly).  */
      while (TREE_CODE (value) == OVERLOAD
	     && DECL_IS_BUILTIN (OVL_FUNCTION (value)))
	value = OVL_CHAIN (value);

      if (TREE_CODE (value) != OVERLOAD
	  && (DECL_IS_BUILTIN (value) || CP_DECL_CONTEXT (value) != ns))
	value = NULL_TREE;
      else if (TREE_CODE (value) == VAR_DECL && DECL_TINFO_P (value))
	value = NULL_TREE;
    }

  if (!value && !type)
    return NULL_TREE;

  dump () && dump ("Writing %N %s bindings for %N", ns,
		   main_p ? "main" : "global", name);

  tag (rt_binding);
  tree_node (ns);
  tree_node (name);
  w.b (main_p);
  w.b (type != NULL_TREE);
  w.bflush ();
  if (type)
    tree_node (type);
  tree_node (value);
  w.checkpoint ();

  if (type)
    maybe_tag_definition (type);
  for (ovl_iterator iter (value); iter; ++iter)
    /* We could still meet a builtin, if the user did something funky
       with a using declaration.  */
    if (!DECL_IS_BUILTIN (*iter))
      maybe_tag_definition (*iter);

  return NULL_TREE;
}

bool
cpms_in::tag_binding ()
{
  tree ns = tree_node ();
  tree name = tree_node ();
  unsigned main_p = r.b ();
  unsigned stat_p = r.b ();
  r.bflush ();
  dump () && dump ("Reading %N %s binding for %N", ns,
		   main_p ? "main" : "global", name);

  tree type = stat_p ? tree_node () : NULL_TREE;
  tree value = tree_node ();

  if (!r.checkpoint ())
    return false;

  return push_module_binding (ns, main_p ? mod_ix : GLOBAL_MODULE_INDEX,
			      name, value, type);
}

/* Stream a function definition.  */

void
cpms_out::define_function (tree decl)
{
  tree_node (DECL_RESULT (decl));
  tree_node (DECL_INITIAL (decl));
  tree_node (DECL_SAVED_TREE (decl));
}

tree
cpms_in::define_function (tree decl)
{
  tree result = tree_node ();
  tree initial = tree_node ();
  tree saved = tree_node ();

  if (r.error ())
    return NULL_TREE;

  if (TREE_CODE (CP_DECL_CONTEXT (decl)) == NAMESPACE_DECL)
    {
      unsigned mod = MAYBE_DECL_MODULE_INDEX (decl);
      if (mod == GLOBAL_MODULE_INDEX
	  && DECL_SAVED_TREE (decl))
	return decl; // FIXME check same
      else if (mod != mod_ix)
	{
	  error ("unexpected definition of %q#D", decl);
	  r.bad ();
	  return NULL_TREE;
	}
    }

  DECL_RESULT (decl) = result;
  DECL_INITIAL (decl) = initial;
  DECL_SAVED_TREE (decl) = saved;

  comdat_linkage (decl);
  note_vague_linkage_fn (decl);
  current_function_decl = decl;
  allocate_struct_function (decl, false);
  cfun->language = ggc_cleared_alloc<language_function> ();
  cfun->language->base.x_stmt_tree.stmts_are_full_exprs_p = 1;
  set_cfun (NULL);
  current_function_decl = NULL_TREE;
  cgraph_node::finalize_function (decl, false);

  return decl;
}

/* A chained set of decls.  */

void
cpms_out::chained_decls (tree decls)
{
  for (; decls; decls = DECL_CHAIN (decls))
    tree_node (decls);
  tree_node (NULL_TREE);
}

tree
cpms_in::chained_decls ()
{
  tree decls = NULL_TREE;
  for (tree *chain = &decls; chain && !r.error ();)
    if (tree decl = tree_node ())
      {
	if (!DECL_P (decl))
	  r.bad ();
	else
	  {
	    gcc_assert (!DECL_CHAIN (decl));
	    *chain = decl;
	    chain = &DECL_CHAIN (decl);
	  }
      }
    else
      chain = NULL;
  return decls;
}

/* A vector of trees.  */

void
cpms_out::tree_vec (vec<tree, va_gc> *v)
{
  unsigned len = vec_safe_length (v);
  w.u (len);
  if (len)
    for (unsigned ix = 0; ix != len; ix++)
      tree_node ((*v)[ix]);
}

vec<tree, va_gc> *
cpms_in::tree_vec ()
{
  vec<tree, va_gc> *v = NULL;
  if (unsigned len = r.u ())
    {
      vec_alloc (v, len);
      for (unsigned ix = 0; ix != len; ix++)
	v->quick_push (tree_node ());
    }
  return v;
}
  
/* Stream a class definition.  */

void
cpms_out::define_class (tree type)
{
  chained_decls (TYPE_FIELDS (type));
  chained_decls (TYPE_METHODS (type));
  tree_node (TYPE_VFIELD (type));
  tree_node (TYPE_BINFO (type));
  if (TYPE_LANG_SPECIFIC (type))
    {
      tree base = CLASSTYPE_AS_BASE (type);
      if (base == type)
	tree_node (base);
      else
	tag_definition (base);
      tree_vec (CLASSTYPE_METHOD_VEC (type));
      tree_node (CLASSTYPE_PRIMARY_BINFO (type));
      tree_vec (CLASSTYPE_VBASECLASSES (type));
      if (TYPE_CONTAINS_VPTR_P (type))
	tree_node (CLASSTYPE_KEY_METHOD (type));

      tree vtables = CLASSTYPE_VTABLES (type);
      chained_decls (vtables);
      /* Write the vtable initializers.  */
      for (; vtables; vtables = TREE_CHAIN (vtables))
	tree_node (DECL_INITIAL (vtables));
    }

#if 0
  gcc_assert (!lang->vcall_indices);
  // lang->nested_udts
  gcc_assert (!lang->pure_virtuals);
  WT (lang->friend_classes);
  // lang->decl_list
  // lang->template_info
  // lang->lambda_expr
#endif

  /* Now define all the members.  */
  for (tree method = TYPE_METHODS (type); method; method = TREE_CHAIN (method))
    maybe_tag_definition (method);
  tree_node (NULL_TREE);
}

/* Nop sorted needed for resorting the method vec.  */

static void
nop (void *, void *)
{
}

tree
cpms_in::define_class (tree type)
{
  gcc_assert (TYPE_MAIN_VARIANT (type) == type);

  tree fields = chained_decls ();
  tree methods = chained_decls ();
  tree vfield = tree_node ();
  tree binfo = tree_node ();
  tree base = NULL_TREE;
  vec<tree, va_gc> *method_vec = NULL;
  tree primary = NULL_TREE;
  vec<tree, va_gc> *vbases = NULL;
  tree key_method = NULL_TREE;
  tree vtables = NULL_TREE;
  if (TYPE_LANG_SPECIFIC (type))
    {
      base = tree_node ();
      method_vec = tree_vec ();
      primary = tree_node ();
      vbases = tree_vec ();
      /* TYPE_VBASECLASSES is not set yet.  */
      if (TYPE_POLYMORPHIC_P (type) || vbases)
	key_method = tree_node ();
      vtables = chained_decls ();
    }

#if 0
  gcc_assert (!lang->vcall_indices);
  // lang->nested_udts
  gcc_assert (!lang->pure_virtuals);
  RT (lang->friend_classes);
  // lang->decl_list
  // lang->template_info
  // lang->lambda_expr
#endif
  // FIXME: Sanity check stuff

  if (r.error ())
    return NULL_TREE;

  TYPE_FIELDS (type) = fields;
  TYPE_METHODS (type) = methods;
  TYPE_VFIELD (type) = vfield;
  TYPE_BINFO (type) = binfo;

  if (TYPE_LANG_SPECIFIC (type))
    {
      CLASSTYPE_AS_BASE (type) = base;
      CLASSTYPE_METHOD_VEC (type) = method_vec;
      CLASSTYPE_PRIMARY_BINFO (type) = primary;
      CLASSTYPE_VBASECLASSES (type) = vbases;

      CLASSTYPE_KEY_METHOD (type) = key_method;
      if (!key_method && TYPE_CONTAINS_VPTR_P (type))
	vec_safe_push (keyed_classes, type);

      CLASSTYPE_VTABLES (type) = vtables;
      /* Read the vtable initializers.  */
      for (; vtables; vtables = TREE_CHAIN (vtables))
	DECL_INITIAL (vtables) = tree_node ();

      /* Resort things that need to be sorted.  */
      resort_type_method_vec (method_vec, NULL, nop, NULL);
      create_classtype_sorted_fields (fields, type);
    }

  /* Propagate to all variants.  */
  fixup_type_variants (type);

  /* Now define all the members.  */
  while (tree_node ())
    if (r.error ())
      break;

  return type;
}

/* Write out DECL's definition, if importers need it.  */

void
cpms_out::maybe_tag_definition (tree t)
{
  if (TREE_CODE (t) == TYPE_DECL)
    t = TREE_TYPE (t);

  switch (TREE_CODE (t))
    {
    default:
      return;

    case RECORD_TYPE:
    case UNION_TYPE:
      if (!COMPLETE_TYPE_P (t))
	return;
      break;

    case FUNCTION_DECL:
      if (!DECL_SAVED_TREE (t))
	return;
      if (!DECL_DECLARED_INLINE_P (t))
	return;
      if (DECL_CLONED_FUNCTION_P (t))
	return;
    }

  tag_definition (t);
}

/* Write out T's definition  */

void
cpms_out::tag_definition (tree t)
{
  dump () && dump ("Writing definition for %C:%N%M", TREE_CODE (t), t, t);

  tag (rt_definition);
  tree_node (t);

  switch (TREE_CODE (t))
    {
    default:
      gcc_unreachable ();
    case FUNCTION_DECL:
      define_function (t);
      break;
    case RECORD_TYPE:
    case UNION_TYPE:
      define_class (t);
      break;
    }
}

tree
cpms_in::tag_definition ()
{
  tree t = tree_node ();
  dump () && dump ("Reading definition for %C:%N%M", TREE_CODE (t), t, t);

  if (r.error ())
    return NULL_TREE;

  switch (TREE_CODE (t))
    {
    default:
      // FIXME: read other things
      t = NULL_TREE;
      break;

    case FUNCTION_DECL:
      t = define_function (t);
      if (t && maybe_clone_body (t) && !DECL_DECLARED_CONSTEXPR_P (t))
	DECL_SAVED_TREE (t) = NULL_TREE;
      break;

    case RECORD_TYPE:
    case UNION_TYPE:
      t = define_class (t);
      break;
    }

  return t;
}

int
cpms_in::read_item ()
{
  if (r.error ())
    return false;

  unsigned rt = r.u ();

  switch (rt)
    {
    case rt_conf:
      return tag_conf ();
    case rt_import:
      return tag_import ();

    default:
      break;
    }

  if (mod_ix == GLOBAL_MODULE_INDEX)
    {
      if (state == this_module)
	{
	  mod_ix = THIS_MODULE_INDEX;
	  (*modules)[mod_ix] = state;
	}
      else
	{
	  mod_ix = modules->length ();
	  if (mod_ix == MODULE_INDEX_LIMIT)
	    {
	      sorry ("too many modules loaded (limit is %u)", mod_ix);
	      r.bad ();
	      return -1;
	    }
	  vec_safe_push (modules, state);
	  state->set_index (mod_ix);
	}
      alloc_remap_vec (THIS_MODULE_INDEX);
      remap_vec[THIS_MODULE_INDEX] = mod_ix;

      dump () && dump ("Assigning %N module index %u", state->name, mod_ix);
    }

  switch (rt)
    {
    case rt_eof:
      return tag_eof ();
    case rt_binding:
      return tag_binding ();
    case rt_definition:
      return tag_definition () != NULL_TREE;
    case rt_trees:
      return tag_trees ();

    default:
      error (rt < rt_tree_base ? "unknown key %qd"
	     : rt < rt_ref_base ? "unexpected tree code %qd"
	     : "unexpected tree reference %qd",rt);
      r.bad ();
      return false;
    }
}

/* Read & write locations.  */

void
cpms_out::loc (location_t)
{
  // FIXME:Do something
}

location_t
cpms_in::loc ()
{
  // FIXME:Do something^-1
  return UNKNOWN_LOCATION;
}

/* Start tree write.  Write information to allocate the receiving
   node.  */

void
cpms_out::start (tree_code code, tree t)
{
  switch (code)
    {
    default:
      if (TREE_CODE_CLASS (code) == tcc_vl_exp)
	w.u (VL_EXP_OPERAND_LENGTH (t));
      break;
    case IDENTIFIER_NODE:
      w.str (IDENTIFIER_POINTER (t), IDENTIFIER_LENGTH (t));
      break;
    case TREE_BINFO:
      w.u (BINFO_N_BASE_BINFOS (t));
      break;
    case TREE_VEC:
      w.u (TREE_VEC_LENGTH (t));
      break;
    case STRING_CST:
      w.str (TREE_STRING_POINTER (t), TREE_STRING_LENGTH (t));
      break;
    case VECTOR_CST:
      w.u (VECTOR_CST_NELTS (t));
      break;
    case INTEGER_CST:
      w.u (TREE_INT_CST_NUNITS (t));
      w.u (TREE_INT_CST_EXT_NUNITS (t));
      w.u (TREE_INT_CST_OFFSET_NUNITS (t));
      break;
    case OMP_CLAUSE:
      gcc_unreachable (); // FIXME:
    }
}

/* Start tree read.  Allocate the receiving node.  */

tree
cpms_in::start (tree_code code)
{
  tree t = NULL_TREE;
  
  switch (code)
    {
    default:
      if (TREE_CODE_CLASS (code) == tcc_vl_exp)
	{
	  unsigned ops = r.u ();
	  t = build_vl_exp (code, ops);
	}
      else
	t = make_node (code);
      break;
    case IDENTIFIER_NODE:
    case STRING_CST:
      {
	size_t l;
	const char *str = r.str (&l);
	if (code == IDENTIFIER_NODE)
	  t = get_identifier_with_length (str, l);
	else
	  t = build_string (l, str);
      }
      break;
    case TREE_BINFO:
      t = make_tree_binfo (r.u ());
      break;
    case TREE_VEC:
      t = make_tree_vec (r.u ());
      break;
    case VECTOR_CST:
      t = make_vector (r.u ());
      break;
    case INTEGER_CST:
      {
	unsigned n = r.u ();
	unsigned e = r.u ();
	t = make_int_cst (n, e);
	TREE_INT_CST_OFFSET_NUNITS(t) = r.u ();
      }
      break;
    case OMP_CLAUSE:
      gcc_unreachable (); // FIXME:
    }

  return t;
}

/* Semantic processing.  Add to symbol table etc.  Return
   possibly-remapped tree.  */

tree
cpms_in::finish (tree t)
{
  if (TYPE_P (t))
    {
      bool on_pr_list = false;
      if (POINTER_TYPE_P (t))
	{
	  on_pr_list = t->type_non_common.minval != NULL;

	  t->type_non_common.minval = NULL;

	  tree probe = TREE_TYPE (t);
	  for (probe = (TREE_CODE (t) == POINTER_TYPE
			? TYPE_POINTER_TO (probe)
			: TYPE_REFERENCE_TO (probe));
	       probe;
	       probe = (TREE_CODE (t) == POINTER_TYPE
			? TYPE_NEXT_PTR_TO (probe)
			: TYPE_NEXT_REF_TO (probe)))
	    if (TYPE_MODE_RAW (probe) == TYPE_MODE_RAW (t)
		&& (TYPE_REF_CAN_ALIAS_ALL (probe)
		    == TYPE_REF_CAN_ALIAS_ALL (t)))
	      return probe;
	}

      tree remap = finish_type (t);
      if (remap == t && on_pr_list)
	{
	  tree to_type = TREE_TYPE (remap);
	  gcc_assert ((TREE_CODE (remap) == POINTER_TYPE
		       ? TYPE_POINTER_TO (to_type)
		       : TYPE_REFERENCE_TO (to_type)) != remap);
	  if (TREE_CODE (remap) == POINTER_TYPE)
	    {
	      TYPE_NEXT_PTR_TO (remap) = TYPE_POINTER_TO (to_type);
	      TYPE_POINTER_TO (to_type) = remap;
	    }
	  else
	    {
	      TYPE_NEXT_REF_TO (remap) = TYPE_REFERENCE_TO (to_type);
	      TYPE_REFERENCE_TO (to_type) = remap;
	    }
	}
      return remap;
    }

  if (DECL_P (t) && MAYBE_DECL_MODULE_INDEX (t) == GLOBAL_MODULE_INDEX)
    {
      tree ctx = CP_DECL_CONTEXT (t);

      if (TREE_CODE (ctx) == NAMESPACE_DECL)
	{
	  /* A global-module decl.  See if there's already a duplicate.  */
	  tree old = merge_global_decl (CP_DECL_CONTEXT (t), t);

	  if (!old)
	    error ("failed to merge %#qD", t);
	  else
	    dump () && dump ("%s decl %N%M, (%p)",
			     old == t ? "New" : "Existing",
			     old, old, (void *)old);

	  return old;
	}
    }

  if (TREE_CODE (t) == INTEGER_CST)
    {
      // FIXME:Remap small ints?
    }

  return t;
}

/* The structure streamers access the raw fields, because the
   alternative, of using the accessor macros can require using
   different accessors for the same underlying field, depending on the
   tree code.  That's both confusing and annoying.  */

/* Read & write the core boolean flags.  */

void
cpms_out::core_bools (tree t)
{
#define WB(X) (w.b (X))
  tree_code code = TREE_CODE (t);

  WB (t->base.side_effects_flag);
  WB (t->base.constant_flag);
  WB (t->base.addressable_flag);
  WB (t->base.volatile_flag);
  WB (t->base.readonly_flag);
  WB (t->base.asm_written_flag);
  WB (t->base.nowarning_flag);
  // visited is zero
  WB (t->base.used_flag);
  WB (t->base.nothrow_flag);
  WB (t->base.static_flag);
  WB (t->base.public_flag);
  WB (t->base.private_flag);
  WB (t->base.protected_flag);
  WB (t->base.deprecated_flag);
  WB (t->base.default_def_flag);

  switch (code)
    {
    case TREE_VEC:
    case INTEGER_CST:
    case CALL_EXPR:
    case SSA_NAME:
    case MEM_REF:
    case TARGET_MEM_REF:
      /* These use different base.u fields.  */
      break;

    case BLOCK:
      WB (t->block.abstract_flag);
      /* FALLTHROUGH  */

    default:
      WB (t->base.u.bits.lang_flag_0);
      WB (t->base.u.bits.lang_flag_1);
      WB (t->base.u.bits.lang_flag_2);
      WB (t->base.u.bits.lang_flag_3);
      WB (t->base.u.bits.lang_flag_4);
      WB (t->base.u.bits.lang_flag_5);
      WB (t->base.u.bits.lang_flag_6);
      WB (t->base.u.bits.saturating_flag);
      WB (t->base.u.bits.unsigned_flag);
      WB (t->base.u.bits.packed_flag);
      WB (t->base.u.bits.user_align);
      WB (t->base.u.bits.nameless_flag);
      WB (t->base.u.bits.atomic_flag);
      break;
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPE_COMMON))
    {
      WB (t->type_common.no_force_blk_flag);
      WB (t->type_common.needs_constructing_flag);
      WB (t->type_common.transparent_aggr_flag);
      WB (t->type_common.restrict_flag);
      WB (t->type_common.string_flag);
      WB (t->type_common.lang_flag_0);
      WB (t->type_common.lang_flag_1);
      WB (t->type_common.lang_flag_2);
      WB (t->type_common.lang_flag_3);
      WB (t->type_common.lang_flag_4);
      WB (t->type_common.lang_flag_5);
      WB (t->type_common.lang_flag_6);
      WB (t->type_common.typeless_storage);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_COMMON))
    {
      WB (t->decl_common.nonlocal_flag);
      WB (t->decl_common.virtual_flag);
      WB (t->decl_common.ignored_flag);
      WB (t->decl_common.abstract_flag);
      WB (t->decl_common.artificial_flag);
      WB (t->decl_common.preserve_flag);
      WB (t->decl_common.debug_expr_is_from);
      WB (t->decl_common.lang_flag_0);
      WB (t->decl_common.lang_flag_1);
      WB (t->decl_common.lang_flag_2);
      WB (t->decl_common.lang_flag_3);
      WB (t->decl_common.lang_flag_4);
      WB (t->decl_common.lang_flag_5);
      WB (t->decl_common.lang_flag_6);
      WB (t->decl_common.lang_flag_7);
      WB (t->decl_common.lang_flag_8);
      WB (t->decl_common.decl_flag_0);
      WB (t->decl_common.decl_flag_1);
      WB (t->decl_common.decl_flag_2);
      WB (t->decl_common.decl_flag_3);
      WB (t->decl_common.gimple_reg_flag);
      WB (t->decl_common.decl_by_reference_flag);
      WB (t->decl_common.decl_read_flag);
      WB (t->decl_common.decl_nonshareable_flag);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_WITH_VIS))
    {
      WB (t->decl_with_vis.defer_output);
      WB (t->decl_with_vis.hard_register);
      WB (t->decl_with_vis.common_flag);
      WB (t->decl_with_vis.in_text_section);
      WB (t->decl_with_vis.in_constant_pool);
      WB (t->decl_with_vis.dllimport_flag);
      WB (t->decl_with_vis.weak_flag);
      WB (t->decl_with_vis.seen_in_bind_expr);
      WB (t->decl_with_vis.comdat_flag);
      WB (t->decl_with_vis.visibility_specified);
      WB (t->decl_with_vis.comdat_flag);
      WB (t->decl_with_vis.init_priority_p);
      WB (t->decl_with_vis.shadowed_for_var_p);
      WB (t->decl_with_vis.cxx_constructor);
      WB (t->decl_with_vis.cxx_destructor);
      WB (t->decl_with_vis.final);
      WB (t->decl_with_vis.regdecl_flag);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_FUNCTION_DECL))
    {
      WB (t->function_decl.static_ctor_flag);
      WB (t->function_decl.static_dtor_flag);
      WB (t->function_decl.uninlinable);
      WB (t->function_decl.possibly_inlined);
      WB (t->function_decl.novops_flag);
      WB (t->function_decl.returns_twice_flag);
      WB (t->function_decl.malloc_flag);
      WB (t->function_decl.operator_new_flag);
      WB (t->function_decl.declared_inline_flag);
      WB (t->function_decl.no_inline_warning_flag);
      WB (t->function_decl.no_instrument_function_entry_exit);
      WB (t->function_decl.no_limit_stack);
      WB (t->function_decl.disregard_inline_limits);
      WB (t->function_decl.pure_flag);
      WB (t->function_decl.looping_const_or_pure_flag);
      WB (t->function_decl.has_debug_args_flag);
      WB (t->function_decl.tm_clone_flag);
      WB (t->function_decl.versioned_function);
    }
#undef WB
}

bool
cpms_in::core_bools (tree t)
{
#define RB(X) ((X) = r.b ())
  tree_code code = TREE_CODE (t);

  RB (t->base.side_effects_flag);
  RB (t->base.constant_flag);
  RB (t->base.addressable_flag);
  RB (t->base.volatile_flag);
  RB (t->base.readonly_flag);
  RB (t->base.asm_written_flag);
  RB (t->base.nowarning_flag);
  // visited is zero
  RB (t->base.used_flag);
  RB (t->base.nothrow_flag);
  RB (t->base.static_flag);
  RB (t->base.public_flag);
  RB (t->base.private_flag);
  RB (t->base.protected_flag);
  RB (t->base.deprecated_flag);
  RB (t->base.default_def_flag);

  switch (code)
    {
    case TREE_VEC:
    case INTEGER_CST:
    case CALL_EXPR:
    case SSA_NAME:
    case MEM_REF:
    case TARGET_MEM_REF:
      /* These use different base.u fields.  */
      break;

    case BLOCK:
      RB (t->block.abstract_flag);
      /* FALLTHROUGH.  */

    default:
      RB (t->base.u.bits.lang_flag_0);
      RB (t->base.u.bits.lang_flag_1);
      RB (t->base.u.bits.lang_flag_2);
      RB (t->base.u.bits.lang_flag_3);
      RB (t->base.u.bits.lang_flag_4);
      RB (t->base.u.bits.lang_flag_5);
      RB (t->base.u.bits.lang_flag_6);
      RB (t->base.u.bits.saturating_flag);
      RB (t->base.u.bits.unsigned_flag);
      RB (t->base.u.bits.packed_flag);
      RB (t->base.u.bits.user_align);
      RB (t->base.u.bits.nameless_flag);
      RB (t->base.u.bits.atomic_flag);
      break;
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPE_COMMON))
    {
      RB (t->type_common.no_force_blk_flag);
      RB (t->type_common.needs_constructing_flag);
      RB (t->type_common.transparent_aggr_flag);
      RB (t->type_common.restrict_flag);
      RB (t->type_common.string_flag);
      RB (t->type_common.lang_flag_0);
      RB (t->type_common.lang_flag_1);
      RB (t->type_common.lang_flag_2);
      RB (t->type_common.lang_flag_3);
      RB (t->type_common.lang_flag_4);
      RB (t->type_common.lang_flag_5);
      RB (t->type_common.lang_flag_6);
      RB (t->type_common.typeless_storage);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_COMMON))
    {
      RB (t->decl_common.nonlocal_flag);
      RB (t->decl_common.virtual_flag);
      RB (t->decl_common.ignored_flag);
      RB (t->decl_common.abstract_flag);
      RB (t->decl_common.artificial_flag);
      RB (t->decl_common.preserve_flag);
      RB (t->decl_common.debug_expr_is_from);
      RB (t->decl_common.lang_flag_0);
      RB (t->decl_common.lang_flag_1);
      RB (t->decl_common.lang_flag_2);
      RB (t->decl_common.lang_flag_3);
      RB (t->decl_common.lang_flag_4);
      RB (t->decl_common.lang_flag_5);
      RB (t->decl_common.lang_flag_6);
      RB (t->decl_common.lang_flag_7);
      RB (t->decl_common.lang_flag_8);
      RB (t->decl_common.decl_flag_0);
      RB (t->decl_common.decl_flag_1);
      RB (t->decl_common.decl_flag_2);
      RB (t->decl_common.decl_flag_3);
      RB (t->decl_common.gimple_reg_flag);
      RB (t->decl_common.decl_by_reference_flag);
      RB (t->decl_common.decl_read_flag);
      RB (t->decl_common.decl_nonshareable_flag);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_WITH_VIS))
    {
      RB (t->decl_with_vis.defer_output);
      RB (t->decl_with_vis.hard_register);
      RB (t->decl_with_vis.common_flag);
      RB (t->decl_with_vis.in_text_section);
      RB (t->decl_with_vis.in_constant_pool);
      RB (t->decl_with_vis.dllimport_flag);
      RB (t->decl_with_vis.weak_flag);
      RB (t->decl_with_vis.seen_in_bind_expr);
      RB (t->decl_with_vis.comdat_flag);
      RB (t->decl_with_vis.visibility_specified);
      RB (t->decl_with_vis.comdat_flag);
      RB (t->decl_with_vis.init_priority_p);
      RB (t->decl_with_vis.shadowed_for_var_p);
      RB (t->decl_with_vis.cxx_constructor);
      RB (t->decl_with_vis.cxx_destructor);
      RB (t->decl_with_vis.final);
      RB (t->decl_with_vis.regdecl_flag);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_FUNCTION_DECL))
    {
      RB (t->function_decl.static_ctor_flag);
      RB (t->function_decl.static_dtor_flag);
      RB (t->function_decl.uninlinable);
      RB (t->function_decl.possibly_inlined);
      RB (t->function_decl.novops_flag);
      RB (t->function_decl.returns_twice_flag);
      RB (t->function_decl.malloc_flag);
      RB (t->function_decl.operator_new_flag);
      RB (t->function_decl.declared_inline_flag);
      RB (t->function_decl.no_inline_warning_flag);
      RB (t->function_decl.no_instrument_function_entry_exit);
      RB (t->function_decl.no_limit_stack);
      RB (t->function_decl.disregard_inline_limits);
      RB (t->function_decl.pure_flag);
      RB (t->function_decl.looping_const_or_pure_flag);
      RB (t->function_decl.has_debug_args_flag);
      RB (t->function_decl.tm_clone_flag);
      RB (t->function_decl.versioned_function);
    }
#undef RB
  return !r.error ();
}

void
cpms_out::lang_decl_bools (tree t)
{
#define WB(X) (w.b (X))
  const struct lang_decl *lang = DECL_LANG_SPECIFIC (t);

  WB (lang->u.base.language == lang_cplusplus);
  WB ((lang->u.base.use_template >> 0) & 1);
  WB ((lang->u.base.use_template >> 1) & 1);
  WB (lang->u.base.not_really_extern);
  WB (lang->u.base.initialized_in_class);
  WB (lang->u.base.repo_available_p);
  WB (lang->u.base.threadprivate_or_deleted_p);
  WB (lang->u.base.anticipated_p);
  WB (lang->u.base.friend_or_tls);
  WB (lang->u.base.template_conv_p);
  WB (lang->u.base.odr_used);
  WB (lang->u.base.u2sel);
  WB (lang->u.base.concept_p);
  WB (lang->u.base.var_declared_inline_p);
  switch (lang->u.base.selector)
    {
    case lds_fn:  /* lang_decl_fn.  */
      WB (lang->u.fn.global_ctor_p);
      WB (lang->u.fn.global_dtor_p);
      WB (lang->u.fn.assignment_operator_p);
      WB (lang->u.fn.static_function);
      WB (lang->u.fn.pure_virtual);
      WB (lang->u.fn.defaulted_p);
      WB (lang->u.fn.has_in_charge_parm_p);
      WB (lang->u.fn.has_vtt_parm_p);
      /* There shouldn't be a pending inline at this point.  */
      gcc_assert (!lang->u.fn.pending_inline_p);
      WB (lang->u.fn.nonconverting);
      WB (lang->u.fn.thunk_p);
      WB (lang->u.fn.this_thunk_p);
      WB (lang->u.fn.hidden_friend_p);
      WB (lang->u.fn.omp_declare_reduction_p);
      /* FALLTHROUGH.  */
    case lds_min:  /* lang_decl_min.  */
      /* No bools.  */
      break;
    case lds_ns:  /* lang_decl_ns.  */
      /* No bools.  */
      break;
    case lds_parm:  /* lang_decl_parm.  */
      /* No bools.  */
      break;
    default:
      gcc_unreachable ();
    }
#undef WB
}

bool
cpms_in::lang_decl_bools (tree t)
{
#define RB(X) ((X) = r.b ())
  struct lang_decl *lang = DECL_LANG_SPECIFIC (t);

  lang->u.base.language = r.b () ? lang_cplusplus : lang_c;
  unsigned v;
  v = r.b () << 0;
  v |= r.b () << 1;
  lang->u.base.use_template = v;
  RB (lang->u.base.not_really_extern);
  RB (lang->u.base.initialized_in_class);
  RB (lang->u.base.repo_available_p);
  RB (lang->u.base.threadprivate_or_deleted_p);
  RB (lang->u.base.anticipated_p);
  RB (lang->u.base.friend_or_tls);
  RB (lang->u.base.template_conv_p);
  RB (lang->u.base.odr_used);
  RB (lang->u.base.u2sel);
  RB (lang->u.base.concept_p);
  RB (lang->u.base.var_declared_inline_p);
  switch (lang->u.base.selector)
    {
    case lds_fn:  /* lang_decl_fn.  */
      RB (lang->u.fn.global_ctor_p);
      RB (lang->u.fn.global_dtor_p);
      RB (lang->u.fn.assignment_operator_p);
      RB (lang->u.fn.static_function);
      RB (lang->u.fn.pure_virtual);
      RB (lang->u.fn.defaulted_p);
      RB (lang->u.fn.has_in_charge_parm_p);
      RB (lang->u.fn.has_vtt_parm_p);
      RB (lang->u.fn.nonconverting);
      RB (lang->u.fn.thunk_p);
      RB (lang->u.fn.this_thunk_p);
      RB (lang->u.fn.hidden_friend_p);
      RB (lang->u.fn.omp_declare_reduction_p);
      /* FALLTHROUGH.  */
    case lds_min:  /* lang_decl_min.  */
      /* No bools.  */
      break;
    case lds_ns:  /* lang_decl_ns.  */
      /* No bools.  */
      break;
    case lds_parm:  /* lang_decl_parm.  */
      /* No bools.  */
      break;
    default:
      gcc_unreachable ();
    }
#undef RB
  return !r.error ();
}

void
cpms_out::lang_type_bools (tree t)
{
#define WB(X) (w.b (X))
  const struct lang_type *lang = TYPE_LANG_SPECIFIC (t);

  WB (lang->has_type_conversion);
  WB (lang->has_copy_ctor);
  WB (lang->has_default_ctor);
  WB (lang->const_needs_init);
  WB (lang->ref_needs_init);
  WB (lang->has_const_copy_assign);
  WB ((lang->use_template >> 0) & 1);
  WB ((lang->use_template >> 1) & 1);

  WB (lang->has_mutable);
  WB (lang->com_interface);
  WB (lang->non_pod_class);
  WB (lang->nearly_empty_p);
  WB (lang->user_align);
  WB (lang->has_copy_assign);
  WB (lang->has_new);
  WB (lang->has_array_new);
  WB ((lang->gets_delete >> 0) & 1);
  WB ((lang->gets_delete >> 1) & 1);
  // Interfaceness is recalculated upon reading.  May have to revisit?
  // lang->interface_only
  // lang->interface_unknown
  WB (lang->contains_empty_class_p);
  WB (lang->anon_aggr);
  WB (lang->non_zero_init);
  WB (lang->empty_p);
  WB (lang->vec_new_uses_cookie);
  WB (lang->declared_class);
  WB (lang->diamond_shaped);
  WB (lang->repeated_base);
  gcc_assert (!lang->being_defined);
  WB (lang->debug_requested);
  WB (lang->fields_readonly);
  WB (lang->ptrmemfunc_flag);
  WB (lang->was_anonymous);
  WB (lang->lazy_default_ctor);
  WB (lang->lazy_copy_ctor);
  WB (lang->lazy_copy_assign);
  WB (lang->lazy_destructor);
  WB (lang->has_const_copy_ctor);
  WB (lang->has_complex_copy_ctor);
  WB (lang->has_complex_copy_assign);
  WB (lang->non_aggregate);
  WB (lang->has_complex_dflt);
  WB (lang->has_list_ctor);
  WB (lang->non_std_layout);
  WB (lang->is_literal);
  WB (lang->lazy_move_ctor);
  WB (lang->lazy_move_assign);
  WB (lang->has_complex_move_ctor);
  WB (lang->has_complex_move_assign);
  WB (lang->has_constexpr_ctor);
  WB (lang->unique_obj_representations);
  WB (lang->unique_obj_representations_set);
#undef WB
}

bool
cpms_in::lang_type_bools (tree t)
{
#define RB(X) ((X) = r.b ())
  struct lang_type *lang = TYPE_LANG_SPECIFIC (t);

  RB (lang->has_type_conversion);
  RB (lang->has_copy_ctor);
  RB (lang->has_default_ctor);
  RB (lang->const_needs_init);
  RB (lang->ref_needs_init);
  RB (lang->has_const_copy_assign);
  unsigned v;
  v = r.b () << 0;
  v |= r.b () << 1;
  lang->use_template = v;

  RB (lang->has_mutable);
  RB (lang->com_interface);
  RB (lang->non_pod_class);
  RB (lang->nearly_empty_p);
  RB (lang->user_align);
  RB (lang->has_copy_assign);
  RB (lang->has_new);
  RB (lang->has_array_new);
  v = r.b () << 0;
  v |= r.b () << 1;
  lang->gets_delete = v;
  // lang->interface_only
  // lang->interface_unknown
  lang->interface_unknown = true; // Redetermine interface
  RB (lang->contains_empty_class_p);
  RB (lang->anon_aggr);
  RB (lang->non_zero_init);
  RB (lang->empty_p);
  RB (lang->vec_new_uses_cookie);
  RB (lang->declared_class);
  RB (lang->diamond_shaped);
  RB (lang->repeated_base);
  gcc_assert (!lang->being_defined);
  RB (lang->debug_requested);
  RB (lang->fields_readonly);
  RB (lang->ptrmemfunc_flag);
  RB (lang->was_anonymous);
  RB (lang->lazy_default_ctor);
  RB (lang->lazy_copy_ctor);
  RB (lang->lazy_copy_assign);
  RB (lang->lazy_destructor);
  RB (lang->has_const_copy_ctor);
  RB (lang->has_complex_copy_ctor);
  RB (lang->has_complex_copy_assign);
  RB (lang->non_aggregate);
  RB (lang->has_complex_dflt);
  RB (lang->has_list_ctor);
  RB (lang->non_std_layout);
  RB (lang->is_literal);
  RB (lang->lazy_move_ctor);
  RB (lang->lazy_move_assign);
  RB (lang->has_complex_move_ctor);
  RB (lang->has_complex_move_assign);
  RB (lang->has_constexpr_ctor);
  RB (lang->unique_obj_representations);
  RB (lang->unique_obj_representations_set);
#undef RB
  return !r.error ();
}

/* Read & write the core values and pointers.  */

void
cpms_out::core_vals (tree t)
{
#define WU(X) (w.u (X))
#define WT(X) (tree_node (X))
  tree_code code = TREE_CODE (t);

  switch (code)
    {
    case TREE_VEC:
    case INTEGER_CST:
      /* Length written earlier.  */
      break;
    case CALL_EXPR:
      WU (t->base.u.ifn);
      break;
    case SSA_NAME:
    case MEM_REF:
    case TARGET_MEM_REF:
      /* We shouldn't meet these.  */
      gcc_unreachable ();

    default:
      break;
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPED))
    WT (t->typed.type);

  /* Whether TREE_CHAIN is dumped depends on who's containing it.  */

  if (CODE_CONTAINS_STRUCT (code, TS_LIST))
    {
      WT (t->list.purpose);
      WT (t->list.value);
      WT (t->list.common.chain);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_VECTOR))
    {
      gcc_unreachable (); // FIXME
    }

  if (CODE_CONTAINS_STRUCT (code, TS_INT_CST))
    {
      unsigned num = TREE_INT_CST_EXT_NUNITS (t);
      for (unsigned ix = 0; ix != num; ix++)
	w.wu (TREE_INT_CST_ELT (t, ix));
    }

  if (CODE_CONTAINS_STRUCT (code, TS_REAL_CST))
    {
      gcc_unreachable (); // FIXME
    }
  
  if (CODE_CONTAINS_STRUCT (code, TS_FIXED_CST))
    {
      gcc_unreachable (); // FIXME
    }

  if (CODE_CONTAINS_STRUCT (code, TS_BINFO))
    {
      WT (t->binfo.offset);
      WT (t->binfo.vtable);
      WT (t->binfo.virtuals);
      WT (t->binfo.vptr_field);
      WT (t->binfo.inheritance);
      WT (t->binfo.vtt_subvtt);
      WT (t->binfo.vtt_vptr);
      gcc_assert (BINFO_N_BASE_BINFOS (t)
		  == vec_safe_length (BINFO_BASE_ACCESSES (t)));
      tree_vec (BINFO_BASE_ACCESSES (t));
      if (unsigned num = BINFO_N_BASE_BINFOS (t))
	for (unsigned ix = 0; ix != num; ix++)
	  WT (BINFO_BASE_BINFO (t, ix));
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPE_COMMON))
    {
      /* By construction we want to make sure we have the canonical
	 and main variants already in the type table, so emit them
	 now.  */
      WT (t->type_common.main_variant);
      WT (t->type_common.canonical);

      /* type_common.next_variant is internally manipulated.  */
      /* type_common.pointer_to, type_common.reference_to.  */

      WU (t->type_common.precision);
      WU (t->type_common.contains_placeholder_bits);
      WU (t->type_common.mode);
      WU (t->type_common.align);

      WT (t->type_common.size);
      WT (t->type_common.size_unit);
      WT (t->type_common.attributes);
      WT (t->type_common.name);
      WT (CP_TYPE_CONTEXT (t)); /* Frobbed type_common.context  */

      WT (t->type_common.common.chain); /* TYPE_STUB_DECL.  */
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPE_NON_COMMON))
    {
      /* Records and unions hold FIELDS, METHODS, VFIELD & BINFO
	 on these things.  */
      if (!RECORD_OR_UNION_CODE_P (code))
	{
	  WT (t->type_non_common.values);
	  /* POINTER and REFERENCE types hold NEXT_{PTR,REF}_TO */
	  if (POINTER_TYPE_P (t))
	    {
	      /* We need to record whether we're on the
		 TYPE_{POINTER,REFERENCE}_TO list of the type we refer
		 to.  Do that by recording NULL or self reference
		 here.  */
	      tree probe = TREE_TYPE (t);
	      for (probe = (TREE_CODE (t) == POINTER_TYPE
			    ? TYPE_POINTER_TO (probe)
			    : TYPE_REFERENCE_TO (probe));
		   probe && probe != t;
		   probe = (TREE_CODE (t) == POINTER_TYPE
			    ? TYPE_NEXT_PTR_TO (probe)
			    : TYPE_NEXT_REF_TO (probe)))
		continue;
	      WT (probe);
	    }
	  else
	    WT (t->type_non_common.minval);
	  WT (t->type_non_common.maxval);
	  WT (t->type_non_common.binfo);
	}
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_MINIMAL))
    {
      /* decl_minimal.name & decl_minimal.context already read in.  */
      loc (t->decl_minimal.locus);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_COMMON))
    {
      WU (t->decl_common.mode);
      WU (t->decl_common.off_align);
      WU (t->decl_common.align);

      WT (t->decl_common.size_unit);
      WT (t->decl_common.attributes);
      switch (code)
	{
	default:
	  break;
	case PARM_DECL:
	  WT (t->decl_common.initial);
	  break;
	}
      /* decl_common.initial, decl_common.abstract_origin.  */
    }

  if (CODE_CONTAINS_STRUCT (code, TS_LABEL_DECL))
    {
      WU (t->label_decl.label_decl_uid);
      WU (t->label_decl.eh_landing_pad_nr);
    }

  /* TS_DECL_WITH_RTL.  */

  if (CODE_CONTAINS_STRUCT (code, TS_FIELD_DECL))
    {
      WT (t->field_decl.offset);
      WT (t->field_decl.bit_field_type);
      WT (t->field_decl.qualifier);
      WT (t->field_decl.bit_offset);
      WT (t->field_decl.fcontext);
    }

  /* TS_RESULT_DECL. */
  /* TS_CONST_DECL.  */
  /* TS_PARM_DECL.  */

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_WITH_VIS))
    {
      WT (t->decl_with_vis.assembler_name);
      WU (t->decl_with_vis.visibility);
    }
  /* TS_VAR_DECL. */

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_NON_COMMON))
    {
      /* decl_non_common.result. */
    }

  if (CODE_CONTAINS_STRUCT (code, TS_FUNCTION_DECL))
    {
      chained_decls (t->function_decl.arguments);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_BLOCK))
    {
      WT (t->block.supercontext);
      chained_decls (t->block.vars);
      WT (t->block.abstract_origin);
      // FIXME nonlocalized_vars, fragment_origin, fragment_chain
      WT (t->block.subblocks);
      WT (t->block.chain);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_CONSTRUCTOR))
    {
      unsigned len = vec_safe_length (t->constructor.elts);
      WU (len);
      if (len)
	for (unsigned ix = 0; ix != len; ix++)
	  {
	    const constructor_elt &elt = (*t->constructor.elts)[ix];

	    WT (elt.index);
	    WT (elt.value);
	  }
    }

  if (TREE_CODE_CLASS (code) == tcc_vl_exp)
    for (unsigned ix = VL_EXP_OPERAND_LENGTH (t); --ix;)
      WT (TREE_OPERAND (t, ix));
  else if (CODE_CONTAINS_STRUCT (code, TS_EXP)
	   /* For some reason, some tcc_expression nodes do not claim
	      to contain TS_EXP.  */
	   || TREE_CODE_CLASS (code) == tcc_expression)
    for (unsigned ix = TREE_OPERAND_LENGTH (t); ix--;)
      WT (TREE_OPERAND (t, ix));

  if (CODE_CONTAINS_STRUCT (code, TS_STATEMENT_LIST))
    {
      for (tree_stmt_iterator iter = tsi_start (t);
	   !tsi_end_p (iter); tsi_next (&iter))
	if (tree stmt = tsi_stmt (iter))
	  WT (stmt);
      WT (NULL_TREE);
    }

  switch (code)
    {
    case OVERLOAD:
      WT (((lang_tree_node *)t)->overload.function);
      WT (t->common.chain);
      break;

    case DEFERRED_NOEXCEPT:
      WT (((lang_tree_node *)t)->deferred_noexcept.pattern);
      WT (((lang_tree_node *)t)->deferred_noexcept.args);
      break;

    default:
      break;
    }

#undef WT
#undef WU
}

bool
cpms_in::core_vals (tree t)
{
#define RU(X) ((X) = r.u ())
#define RUC(T,X) ((X) = T (r.u ()))
#define RT(X) ((X) = tree_node ())
  tree_code code = TREE_CODE (t);

  switch (code)
    {
    case TREE_VEC:
    case INTEGER_CST:
      /* Length read earlier.  */
      break;
    case CALL_EXPR:
      RUC (internal_fn, t->base.u.ifn);
      break;
    case SSA_NAME:
    case MEM_REF:
    case TARGET_MEM_REF:
      /* We shouldn't meet these.  */
      return false;

    default:
      break;
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPED))
    RT (t->typed.type);

  /* Whether TREE_CHAIN is dumped depends on who's containing it.  */

  if (CODE_CONTAINS_STRUCT (code, TS_LIST))
    {
      RT (t->list.purpose);
      RT (t->list.value);
      RT (t->list.common.chain);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_VECTOR))
    {
      gcc_unreachable (); // FIXME
    }

  if (CODE_CONTAINS_STRUCT (code, TS_INT_CST))
    {
      unsigned num = TREE_INT_CST_EXT_NUNITS (t);
      for (unsigned ix = 0; ix != num; ix++)
	TREE_INT_CST_ELT (t, ix) = r.wu ();
    }

  if (CODE_CONTAINS_STRUCT (code, TS_REAL_CST))
    {
      gcc_unreachable (); // FIXME
    }
  
  if (CODE_CONTAINS_STRUCT (code, TS_FIXED_CST))
    {
      gcc_unreachable (); // FIXME
    }

  if (CODE_CONTAINS_STRUCT (code, TS_BINFO))
    {
      RT (t->binfo.offset);
      RT (t->binfo.vtable);
      RT (t->binfo.virtuals);
      RT (t->binfo.vptr_field);
      RT (t->binfo.inheritance);
      RT (t->binfo.vtt_subvtt);
      RT (t->binfo.vtt_vptr);
      BINFO_BASE_ACCESSES (t) = tree_vec ();
      if (BINFO_BASE_ACCESSES (t))
	{
	  unsigned num = BINFO_BASE_ACCESSES (t)->length ();
	  for (unsigned ix = 0; ix != num; ix++)
	    BINFO_BASE_APPEND (t, tree_node ());
	}
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPE_COMMON))
    {
      RT (t->type_common.main_variant);
      RT (t->type_common.canonical);

      /* type_common.next_variant is internally manipulated.  */
      /* type_common.pointer_to, type_common.reference_to.  */

      RU (t->type_common.precision);
      RU (t->type_common.contains_placeholder_bits);
      RUC (machine_mode, t->type_common.mode);
      RU (t->type_common.align);

      RT (t->type_common.size);
      RT (t->type_common.size_unit);
      RT (t->type_common.attributes);
      RT (t->type_common.name);
      RT (t->type_common.context); /* Frobbed  */

      RT (t->type_common.common.chain); /* TYPE_STUB_DECL.  */
    }

  if (CODE_CONTAINS_STRUCT (code, TS_TYPE_NON_COMMON))
    {
      /* Records and unions hold FIELDS, METHODS, VFIELD & BINFO
	 on these things.  */
      if (!RECORD_OR_UNION_CODE_P (code))
	{
	  RT (t->type_non_common.values);
	  /* POINTER and REFERENCE types hold NEXT_{PTR,REF}_TO.  We
	     store a marker there to indicate whether we're on the
	     referred to type's pointer/reference to list.  */
	  RT (t->type_non_common.minval);
	  if (POINTER_TYPE_P (t) && t->type_non_common.minval
	      && t->type_non_common.minval != t)
	    {
	      t->type_non_common.minval = NULL_TREE;
	      r.bad ();
	    }
	  RT (t->type_non_common.maxval);
	  RT (t->type_non_common.binfo);
	}
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_MINIMAL))
    {
      /* decl_minimal.name & decl_minimal.context already read in.  */
      /* Don't zap the locus just yet, we don't record it correctly
	 and thus lose all location information.  */
      /* t->decl_minimal.locus = */
      loc ();
    }

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_COMMON))
    {
      RUC (machine_mode, t->decl_common.mode);
      RU (t->decl_common.off_align);
      RU (t->decl_common.align);

      RT (t->decl_common.size_unit);
      RT (t->decl_common.attributes);
      switch (code)
	{
	default:
	  break;
	case PARM_DECL:
	  RT (t->decl_common.initial);
	  break;
	}
      /* decl_common.initial, decl_common.abstract_origin.  */
    }

  if (CODE_CONTAINS_STRUCT (code, TS_LABEL_DECL))
    {
      RU (t->label_decl.label_decl_uid);
      RU (t->label_decl.eh_landing_pad_nr);
    }

  /* TS_DECL_WITH_RTL.  */

  if (CODE_CONTAINS_STRUCT (code, TS_FIELD_DECL))
    {
      RT (t->field_decl.offset);
      RT (t->field_decl.bit_field_type);
      RT (t->field_decl.qualifier);
      RT (t->field_decl.bit_offset);
      RT (t->field_decl.fcontext);
    }

  /* TS_RESULT_DECL. */
  /* TS_CONST_DECL.  */
  /* TS_PARM_DECL.  */

  if (CODE_CONTAINS_STRUCT (code, TS_DECL_WITH_VIS))
    {
      RT (t->decl_with_vis.assembler_name);
      RUC (symbol_visibility, t->decl_with_vis.visibility);
    }

  if (CODE_CONTAINS_STRUCT (TREE_CODE (t), TS_DECL_NON_COMMON))
    {
      /* decl_non_common.result. */
    }

  if (CODE_CONTAINS_STRUCT (code, TS_FUNCTION_DECL))
    {
      t->function_decl.arguments = chained_decls ();
    }

  if (CODE_CONTAINS_STRUCT (code, TS_BLOCK))
    {
      RT (t->block.supercontext);
      t->block.vars = chained_decls ();
      RT (t->block.abstract_origin);
      // FIXME nonlocalized_vars, fragment_origin, fragment_chain
      RT (t->block.subblocks);
      RT (t->block.chain);
    }

  if (CODE_CONTAINS_STRUCT (code, TS_CONSTRUCTOR))
    {
      if (unsigned len = r.u ())
	{
	  vec_alloc (t->constructor.elts, len);
	  for (unsigned ix = 0; ix != len; ix++)
	    {
	      constructor_elt elt;

	      RT (elt.index);
	      RT (elt.value);
	      t->constructor.elts->quick_push (elt);
	    }
	}
    }

  if (TREE_CODE_CLASS (code) == tcc_vl_exp)
    for (unsigned ix = VL_EXP_OPERAND_LENGTH (t); --ix;)
      RT (TREE_OPERAND (t, ix));
  else if (CODE_CONTAINS_STRUCT (code, TS_EXP)
	   || TREE_CODE_CLASS (code) == tcc_expression)
    for (unsigned ix = TREE_OPERAND_LENGTH (t); ix--;)
      RT (TREE_OPERAND (t, ix));

  if (CODE_CONTAINS_STRUCT (code, TS_STATEMENT_LIST))
    {
      tree_stmt_iterator iter = tsi_start (t);
      for (tree stmt; RT (stmt);)
	tsi_link_after (&iter, stmt, TSI_CONTINUE_LINKING);
    }
      
  switch (code)
    {
    case OVERLOAD:
      RT (((lang_tree_node *)t)->overload.function);
      RT (t->common.chain);
      break;

    case DEFERRED_NOEXCEPT:
      RT (((lang_tree_node *)t)->deferred_noexcept.pattern);
      RT (((lang_tree_node *)t)->deferred_noexcept.args);
      break;

    default:
      break;
    }
#undef RT
#undef RM
#undef RU
  return !r.error ();
}

void
cpms_out::lang_decl_vals (tree t)
{
  const struct lang_decl *lang = DECL_LANG_SPECIFIC (t);
#define WU(X) (w.u (X))
#define WT(X) (tree_node (X))
  /* Module index already written.  */
  switch (lang->u.base.selector)
    {
    case lds_fn:  /* lang_decl_fn.  */
      WU (lang->u.fn.operator_code);
      if (lang->u.fn.thunk_p)
	w.wi (lang->u.fn.u5.fixed_offset);
      else
	WT (lang->u.fn.u5.cloned_function);
      /* FALLTHROUGH.  */
    case lds_min:  /* lang_decl_min.  */
      // FIXME: no templates yet
      gcc_assert (!lang->u.min.template_info);
      if (lang->u.base.u2sel)
	WU (lang->u.min.u2.discriminator);
      else
	WT (lang->u.min.u2.access);
      break;
    case lds_ns:  /* lang_decl_ns.  */
      break;
    case lds_parm:  /* lang_decl_parm.  */
      break;
    default:
      gcc_unreachable ();
    }
#undef WU
#undef WT
}

bool
cpms_in::lang_decl_vals (tree t)
{
  struct lang_decl *lang = DECL_LANG_SPECIFIC (t);
#define RU(X) ((X) = r.u ())
#define RT(X) ((X) = tree_node ())

  /* Module index already read.  */

  switch (lang->u.base.selector)
    {
    case lds_fn:  /* lang_decl_fn.  */
      {
	unsigned code = r.u ();
	/* It seems to be hard to check this is in range.  */
	lang->u.fn.operator_code = (tree_code)code;
	if (lang->u.fn.thunk_p)
	  lang->u.fn.u5.fixed_offset = r.wi ();
	else
	  RT (lang->u.fn.u5.cloned_function);
      }
      /* FALLTHROUGH.  */
    case lds_min:  /* lang_decl_min.  */
      // FIXME: no templates yet
      gcc_assert (!lang->u.min.template_info);
      if (lang->u.base.u2sel)
	RU (lang->u.min.u2.discriminator);
      else
	RT (lang->u.min.u2.access);
      break;
    case lds_ns:  /* lang_decl_ns.  */
      break;
    case lds_parm:  /* lang_decl_parm.  */
      break;
    default:
      gcc_unreachable ();
    }
#undef RU
#undef RT
  return !r.error ();
}

/* Most of the value contents of lang_type is streamed in
   define_class.  */

void
cpms_out::lang_type_vals (tree t)
{
  const struct lang_type *lang = TYPE_LANG_SPECIFIC (t);
#define WU(X) (w.u (X))
#define WT(X) (tree_node (X))
  WU (lang->align);
  WT (lang->befriending_classes);
#undef WU
#undef WT
}

bool
cpms_in::lang_type_vals (tree t)
{
  struct lang_type *lang = TYPE_LANG_SPECIFIC (t);
#define RU(X) ((X) = r.u ())
#define RT(X) ((X) = tree_node ())
  RU (lang->align);
  RT (lang->befriending_classes);
#undef RU
#undef RT
  return !r.error ();
}

/* Refer to imported decls via reference information, not directly.  */

void
cpms_out::ident_imported_decl (tree ctx, unsigned mod, tree decl)
{
  if (TREE_CODE (ctx) == NAMESPACE_DECL)
    {
      unsigned key = get_ident_in_namespace (ctx, mod, DECL_NAME (decl), decl);
      w.u (key);
    }
  else if (TYPE_P (ctx))
    {
      /* Until class scopes look like namespace scopes, we'll have to
	 search the TYPE_FIELDS and TYPE_METHODS array.  Ew.  */
      int key = 0, inc = +1;
      tree probe = TYPE_FIELDS (ctx);

      if (DECL_DECLARES_FUNCTION_P (decl))
	{
	  inc = -1;
	  key = -1;
	  probe = TYPE_METHODS (ctx);
	}
      for (; probe != decl; probe = TREE_CHAIN (probe))
	key += inc;
      w.s (key);
    }
  else
    gcc_unreachable ();
}

tree
cpms_in::ident_imported_decl (tree ctx, unsigned mod, tree name)
{
  tree res;

  if (TREE_CODE (ctx) == NAMESPACE_DECL)
    {
      unsigned key = r.u ();
      res = find_by_ident_in_namespace (ctx, mod, name, key);
    }
  else if (TYPE_P (ctx))
    {
      /* Until class scopes look like namespace scopes, we'll have to
	 search the TYPE_FIELDS and TYPE_METHODS array.  Ew.  */
      int key = r.s (), inc = 1;
      tree probe = TYPE_FIELDS (ctx);

      if (key < 0)
	{
	  inc = -1;
	  key -= inc;
	  probe = TYPE_METHODS (ctx);
	}
      for (; key; probe = TREE_CHAIN (probe))
	{
	  if (!probe)
	    break;
	  key -= inc;
	}
      res = probe;
    }
  else
    gcc_unreachable ();

  return res;
}

/* Write either the decl (as a declaration) itself (and create a
   mapping for it), or write the existing mapping or write null.  This
   is essentially the lisp self-referential structure pretty-printer,
   except that we implicitly number every node, so need neither two
   passes, nor explicit labelling.

   We emit in the following order:
     <tag>
     IF DECL_P
       <context>
       <name>
       <module-no>
       IF imported
         <tag>
	 goto done
     <length_info>
     IF IDENTIFIER_P
       goto done

     <core bools>
     <lang-specific-p>
     <bflush & checkpoint>
     if lang-specific-p
       <lang-specific bools>
       <bflush & checkpoint>
     <core vals & trees>
     if lang-specific-p
       <lang-specific vals & trees>

   done:
     <checkpoint>
*/

void
cpms_out::tree_node (tree t)
{
  if (!t)
    {
      nulls++;
      w.u (0); /* This also matches t_eof, but we cannot be confused. */
      return;
    }

  nest ();
  if (unsigned *val = tree_map.get (t))
    {
      refs++;
      w.u (*val);
      dump () && dump ("Wrote:%u referenced %C:%N%M", *val,
		       TREE_CODE (t), t, t);
      unnest ();
      return;
    }

  if (TREE_CODE_CLASS (TREE_CODE (t)) == tcc_type && TYPE_NAME (t))
    {
      tree name = TYPE_NAME (t);

      gcc_assert (TREE_CODE (name) == TYPE_DECL);
      if (DECL_TINFO_P (name))
	{
	  unsigned ix = get_pseudo_tinfo_index (t);

	  /* Make sure we're identifying this exact variant.  */
	  gcc_assert (get_pseudo_tinfo_type (ix) == t);
	  w.u (rt_typeinfo_pseudo);
	  w.u (ix);
	  unsigned tag = insert (t);
	  dump () && dump ("Wrote:%u typeinfo pseudo %u %N", tag, ix, t);
	  unnest ();
	  return;
	}
      else if (!tree_map.get (TYPE_NAME (t)))
	{
	  /* T is a named type whose name we have not met yet.  Write the
	     type name as an interstitial, and then start over.  */
	  dump () && dump ("Writing interstitial type name %C:%N%M",
			   TREE_CODE (name), name, name);
	  w.u (rt_type_name);
	  tree_node (name);
	  dump () && dump ("Wrote interstitial type name %C:%N%M",
			   TREE_CODE (name), name, name);
	  unnest ();
	  /* The type could be a variant of TREE_TYPE (name).  */
	  tree_node (t);
	  return;
	}
    }

  if (TREE_CODE (t) == VAR_DECL && DECL_TINFO_P (t))
    {
      /* T is a typeinfo object.  These need recreating by the loader.
	 The type it is for is stashed on the name's TREE_TYPE.  */
      tree type = TREE_TYPE (DECL_NAME (t));
      dump () && dump ("Writing typeinfo %M for %N", t, type);
      w.u (rt_typeinfo_var);
      tree_node (type);
      unsigned tag = insert (t);
      dump () && dump ("Wrote:%u typeinfo %M for %N", tag, t, type);
      unnest ();
      return;
    }

  tree_code code = TREE_CODE (t);
  tree_code_class klass = TREE_CODE_CLASS (code);
  gcc_assert (rt_tree_base + code < rt_ref_base);

  unique++;
  w.u (rt_tree_base + code);

  int body = 1;
  if (code == IDENTIFIER_NODE)
    body = 0;
  else if (klass == tcc_declaration)
    {
      /* Write out ctx, name & maybe import reference info.  */
      tree ctx = CP_DECL_CONTEXT (t);
      tree_node (ctx);
      tree_node (DECL_NAME (t));
      unsigned mod = GLOBAL_MODULE_INDEX;

      tree outer = ctx, probe = t;
      while (TYPE_P (outer))
	{
	  probe = TYPE_NAME (outer);
	  outer = CP_TYPE_CONTEXT (outer);
	}
      if (TREE_CODE (outer) == NAMESPACE_DECL)
	{
	  mod = MAYBE_DECL_MODULE_INDEX (probe);
	  w.u (mod);
	}

      if (mod >= IMPORTED_MODULE_BASE)
	{
	  ident_imported_decl (ctx, mod, t);
	  dump () && dump ("Writing imported %N@%I", t, module_name (mod));
	  body = -1;
	}
    }

  if (body >= 0)
    start (code, t);

  unsigned tag = insert (t);
  dump () && dump ("Writing:%u %C:%N%M%s", tag, TREE_CODE (t), t, t,
		   klass == tcc_declaration && DECL_MODULE_EXPORT_P (t)
		   ? " (exported)": "");

  if (body > 0)
    {
      bool specific = false;

      if (klass == tcc_type || klass == tcc_declaration)
	{
	  if (klass == tcc_declaration)
	    specific = DECL_LANG_SPECIFIC (t) != NULL;
	  else if (TYPE_MAIN_VARIANT (t) == t)
	    specific = TYPE_LANG_SPECIFIC (t) != NULL;
	  else
	    gcc_assert (TYPE_LANG_SPECIFIC (t)
			== TYPE_LANG_SPECIFIC (TYPE_MAIN_VARIANT (t)));
	  w.b (specific);
	  if (specific && code == VAR_DECL)
	    w.b (DECL_DECOMPOSITION_P (t));

	  if (specific)
	    dump () && dump ("%u %C:%N%M has lang_specific",
			     tag, TREE_CODE (t), t, t);
	}

      core_bools (t);
      if (specific)
	{
	  if (klass == tcc_type)
	    lang_type_bools (t);
	  else
	    {
	      lang_decl_bools (t);
	    }
	}
      w.bflush ();
      w.checkpoint ();

      core_vals (t);
      if (specific)
	{
	  if (klass == tcc_type)
	    lang_type_vals (t);
	  else
	    lang_decl_vals (t);
	}
    }
  else if (body < 0 && TREE_TYPE (t))
    {
      tree type = TREE_TYPE (t);
      bool existed;
      unsigned *val = &tree_map.get_or_insert (type, &existed);
      if (!existed)
	{
	  tag = next ();
	  *val = tag;
	  dump () && dump ("Writing:%u %C:%N%M imported type", tag,
			   TREE_CODE (type), type, type);
	}
      w.u (existed);
    }

  w.checkpoint ();

  unnest ();
}

/* Read in a tree using TAG.  TAG is either a back reference, or a
   TREE_CODE for a new TREE.  For any tree that is a DECL, this does
   not read in a definition (initial value, class defn, function body,
   instantiations, whatever).  Return true on success.  Sets *TP to
   error_mark_node if TAG is totally bogus.  */

tree
cpms_in::tree_node ()
{
  unsigned tag = r.u ();

  if (!tag)
    return NULL_TREE;

  nest ();
  if (tag >= rt_ref_base)
    {
      tree *val = (tree *)tree_map.get (tag);
      if (!val || !*val)
	{
	  error ("unknown tree reference %qd", tag);
	  r.bad ();
	  return NULL_TREE;
	}

      tree res = *val;
      dump () && dump ("Read:%u found %C:%N%M", tag,
		       TREE_CODE (res), res, res);
      unnest ();
      return res;
    }

  if (tag == rt_type_name)
    {
      /* An interstitial type name.  Read the name and then start
	 over.  */
      tree name = tree_node ();
      if (!name || TREE_CODE (name) != TYPE_DECL)
	r.bad ();
      else
	dump () && dump ("Read interstitial type name %C:%N%M",
			 TREE_CODE (name), name, name);
      unnest ();
      return tree_node ();
    }
  else if (tag == rt_typeinfo_var)
    {
      /* A typeinfo.  Get the type and recreate the var decl.  */
      tree var = NULL_TREE, type = tree_node ();
      if (!type || !TYPE_P (type))
	r.bad ();
      else
	{
	  var = get_tinfo_decl (type);
	  unsigned tag = insert (var);
	  dump () && dump ("Created:%u typeinfo var %M for %N",
			   tag, var, type);
	}
      unnest ();
      return var;
    }
  else if (tag == rt_typeinfo_pseudo)
    {
      /* A pseuto typeinfo.  Get the index and recreate the pseudo.  */
      unsigned ix = r.u ();
      tree type = NULL_TREE;

      if (ix >= 1000)
	r.bad ();
      else
	type = get_pseudo_tinfo_type (ix);

      unsigned tag = insert (type);
      dump () && dump ("Created:%u typeinfo pseudo %u %N", tag, ix, type);
      unnest ();
      return type;
    }
  else if (tag == rt_definition)
    {
      /* An immediate definition.  */
      tree res = tag_definition ();
      if (res)
	dump () && dump ("Read immediate definition %C:%N%M",
			 TREE_CODE (res), res, res);
      unnest ();
      return res;
    }
  else if (tag < rt_tree_base || tag >= rt_tree_base + MAX_TREE_CODES)
    {
      error (tag < rt_tree_base ? "unexpected key %qd"
	     : "unknown tree code %qd" , tag);
      r.bad ();
      unnest ();
      return NULL_TREE;
    }

  tree_code code = tree_code (tag - rt_tree_base);
  tree_code_class klass = TREE_CODE_CLASS (code);
  tree t = NULL_TREE;

  int body = 1;
  tree name = NULL_TREE;
  tree ctx = NULL_TREE;
  unsigned mod = GLOBAL_MODULE_INDEX;

  if (code == IDENTIFIER_NODE)
    body = 0;
  else if (klass == tcc_declaration)
    {
      ctx = tree_node ();
      name = tree_node ();
      if (r.error ())
	{
	  unnest ();
	  return NULL_TREE;
	}

      bool is_imported = false;
      tree outer = ctx;
      while (TYPE_P (outer))
	outer = CP_TYPE_CONTEXT (outer);
      if (TREE_CODE (outer) == NAMESPACE_DECL)
	{
	  mod = r.u ();

	  is_imported = mod >= IMPORTED_MODULE_BASE;
	  if (mod < remap_num)
	    mod = remap_vec[mod];
	  else
	    r.bad ();
	}

      if (is_imported)
	{
	  t = ident_imported_decl (ctx, mod, name);
	  if (!t || TREE_CODE (t) != code)
	    {
	      if (ctx != global_namespace)
		error ("failed to find %<%E::%E@%E%>",
		       ctx, name, module_name (mod));
	      else
		error ("failed to find %<%E@%E%>",
		       name, module_name (mod));
	      r.bad ();
	      t = NULL_TREE;
	    }
	  dump () && dump ("Importing %P@%I",
			   ctx, name, module_name (mod));
	  body = -1;
	}
    }

  if (body >= 0)
    t = start (code);

  /* Insert into map.  */
  tag = insert (t);
  dump () && dump ("%s:%u %C:%N", body < 0 ? "Imported" : "Reading", tag,
		   code, name);

  if (body > 0)
    {
      bool specific = false;
      bool lied = false;

      if (klass == tcc_type || klass == tcc_declaration)
	{
	  specific = r.b ();
	  if (specific)
	    {
	      dump () && dump ("%u %C:%N has lang_specific", tag, code, name);

	      if (klass == tcc_type
		  ? !maybe_add_lang_type_raw (t)
		  : !maybe_add_lang_decl_raw (t, code == VAR_DECL && r.b ()))
		lied = true;
	    }
	}

      if (!core_bools (t))
	lied = true;
      else if (specific)
	{
	  if (klass == tcc_type
	      ? !lang_type_bools (t)
	      : !lang_decl_bools (t))
	    lied = true;
	}
      r.bflush ();
      if (lied || !r.checkpoint ())
	goto barf;

      if (klass == tcc_declaration)
	{
	  DECL_CONTEXT (t) = ctx;
	  DECL_NAME (t) = name;
	}

      if (!core_vals (t))
	goto barf;

      if (specific)
	{
	  if (klass == tcc_type)
	    {
	      gcc_assert (TYPE_MAIN_VARIANT (t) == t);
	      if (!lang_type_vals (t))
		goto barf;
	    }
	  else
	    {
	      DECL_MODULE_INDEX (t) = mod;
	      if (!lang_decl_vals (t))
		goto barf;
	    }
	}
      else if (klass == tcc_type)
	TYPE_LANG_SPECIFIC (t) = TYPE_LANG_SPECIFIC (TYPE_MAIN_VARIANT (t));
    }
  else if (body < 0 && TREE_TYPE (t) && !r.u ())
    {
      tree type = TREE_TYPE (t);
      tag = insert (type);
      dump () && dump ("Read:%u %C:%N%M imported type", tag,
		       TREE_CODE (type), type, type);
    }

  if (!r.checkpoint ())
    {
    barf:
      tree_map.put (tag, NULL_TREE);
      r.bad ();
      unnest ();
      return NULL_TREE;
    }

  if (body > 0)
    {
      tree found = finish (t);

      if (found != t)
	{
	  /* Update the mapping.  */
	  t = found;
	  tree_map.put (tag, t);
	  dump () && dump ("Index %u remapping %C:%N%M", tag,
			   TREE_CODE (t), t, t);
	}
    }

  unnest ();
  return t;
}

/* Walk the bindings of NS, writing out the bindings for the global
   module and the main module.  */

void
cpms_out::bindings (tree ns)
{
  dump () && dump ("Walking namespace %N", ns);

  hash_map<lang_identifier *, tree>::iterator end
    (DECL_NAMESPACE_BINDINGS (ns)->end ());
  for (hash_map<lang_identifier *, tree>::iterator iter
	 (DECL_NAMESPACE_BINDINGS (ns)->begin ()); iter != end; ++iter)
    {
      std::pair<tree, tree> binding (*iter);

      tree name = binding.first;
      tree global = binding.second;
      tree inner = NULL_TREE;

      if (TREE_CODE (global) == MODULE_VECTOR)
	{
	  const module_cluster *cluster = &MODULE_VECTOR_CLUSTER (global, 0);
	  global = cluster->slots[GLOBAL_MODULE_INDEX];

	  if (tree main = cluster->slots[THIS_MODULE_INDEX])
	    inner = tag_binding (ns, true, name, main);
	}

      if (global)
	if (tree ginner = tag_binding (ns, false, name, global))
	  {
	    gcc_assert (!inner || inner == ginner);
	    inner = ginner;
	  }

      if (inner)
	bindings (inner);
    }

  dump () && dump ("Walked namespace %N", ns);
}

/* Mangling for module files.  */
#define MOD_FNAME_PFX "g++-"
#define MOD_FNAME_SFX ".nms" /* New Module System.  Honest.  */
#define MOD_FNAME_DOT '-'

static location_t module_loc;	 /* Location of the module decl.  */
static GTY(()) tree proclaimer;
static int export_depth; /* -1 for singleton export.  */

/* Rebuild a streamed in type.  */
// FIXME: c++-specific types are not in the canonical type hash.
// Perhaps that should be changed?

tree
cpms_in::finish_type (tree type)
{
  tree main = TYPE_MAIN_VARIANT (type);

  if (main != type)
    {
      /* See if we have this type already on the variant
	 list.  This could only happen if the originally read in main
	 variant was remapped, but we don't have that knowledge.
	 FIXME: Determine if this is a problem, and then maybe fix
	 it?  That would avoid a fruitless search along the variant
	 chain.  */
      for (tree probe = main; probe; probe = TYPE_NEXT_VARIANT (probe))
	if (check_base_type (type, probe)
	    && TYPE_QUALS (type) == TYPE_QUALS (probe)
	    && comp_except_specs (TYPE_RAISES_EXCEPTIONS (type),
				  TYPE_RAISES_EXCEPTIONS (probe), ce_exact)
	    && type_memfn_rqual (type) == type_memfn_rqual (probe))
	  {
	    dump () && dump ("Type %p already found as %p variant of %p",
			     (void *)type, (void *)probe, (void *)main);
	    free_node (type);
	    type = probe;
	    goto found_variant;
	  }

      /* Splice it into the variant list.  */
      dump () && dump ("Type %p added as variant of %p",
		       (void *)type, (void *)main);
      TYPE_NEXT_VARIANT (type) = TYPE_NEXT_VARIANT (main);
      TYPE_NEXT_VARIANT (main) = type;
      if (RECORD_OR_UNION_CODE_P (TREE_CODE (type)))
	{
	  /* The main variant might already have been defined, copy
	     the bits of its definition that we need.  */
	  TYPE_BINFO (type) = TYPE_BINFO (main);
	  TYPE_VFIELD (type) = TYPE_VFIELD (main);
	  TYPE_FIELDS (type) = TYPE_FIELDS (main);
	}

      /* CANONICAL_TYPE is either already correctly remapped.  Or
         correctly already us.  FIXME:Are we sure about this?  */
    found_variant:;
    }
  else if (!TYPE_STRUCTURAL_EQUALITY_P (type)
	   && !TYPE_NAME (type))
    {
      gcc_assert (TYPE_ALIGN (type));
      hashval_t hash = type_hash_canon_hash (type);
      /* type_hash_canon frees type, if we find it already.  */
      type = type_hash_canon (hash, type);
      // FIXME: This is where it'd be nice to determine if type
      // was already found.  See above.
      dump () && dump ("Adding type %p with canonical %p",
		       (void *)main, (void *)type);
    }

  return type;
}

/* Nest a module export level.  Return true if we were already in a
   level.  */

int
push_module_export (bool singleton, tree proclaiming)
{
  int previous = export_depth;

  if (proclaiming)
    {
      proclaimer = proclaimer;
      export_depth = -2;
    }
  else if (singleton)
    export_depth = -1;
  else
    export_depth = +1;
  return previous;
}

/* Unnest a module export level.  */

void
pop_module_export (int previous)
{
  proclaimer = NULL;
  export_depth = previous;
}

int
module_exporting_level ()
{
  return export_depth;
}

/* Set the module EXPORT and INDEX fields on DECL.  */

void
decl_set_module (tree decl)
{
  if (export_depth)
    DECL_MODULE_EXPORT_P (decl) = true;
  if (this_module && this_module->name)
    {
      retrofit_lang_decl (decl);
      DECL_MODULE_INDEX (decl) = THIS_MODULE_INDEX;
    }
}

/* Return true iff we're in the purview of a named module.  */

bool
module_purview_p ()
{
  return this_module && this_module->name;
}

/* Return true iff we're the interface TU (this also means we're in a
   module perview.  */

bool
module_interface_p ()
{
  return this_module && this_module->direct_import;
}

/* Convert a module name into a file name.  The name is malloced.

   (for the moment) this replaces '.' with '-' adds a prefix and
   suffix.

   FIXME: Add host-applicable hooks.  */

static char *
module_to_filename (tree id)
{
  char *name = concat (MOD_FNAME_PFX, IDENTIFIER_POINTER (id),
		       MOD_FNAME_SFX, NULL);
  char *ptr = name + strlen (MOD_FNAME_PFX);
  size_t len = IDENTIFIER_LENGTH (id);

  if (MOD_FNAME_DOT != '.')
    for (; len--; ptr++)
      if (*ptr == '.')
	*ptr = MOD_FNAME_DOT;

  return name;
}

/* Read a module NAME file name FNAME on STREAM.  Returns its module
   index, or 0 */

static unsigned
read_module (FILE *stream, const char *fname, module_state *state,
	     unsigned HOST_WIDE_INT stamp, cpms_in *from = NULL)
{
  cpms_in in (stream, fname, state, stamp, from);

  int ok = in.header ();
  if (ok)
    do
      ok = in.read_item ();
    while (ok > 0);

  if (int e = in.done ())
    {
      /* strerror and friends returns capitalized strings.  */
      char const *err = e >= 0 ? xstrerror (e) : "Bad file data";
      char c = TOLOWER (err[0]);
      error ("%c%s", c, err + 1);
      ok = false;
    }

  if (ok)
    {
      ok = in.get_mod ();
      gcc_assert (ok >= THIS_MODULE_INDEX);
    }
  else
    /* Failure to read a module is going to cause big problems, so
       bail out now.  */
    fatal_error (input_location,
		 "failed to read module %qE (%qs)", state->name, fname);

  return ok;
}

/* Import the module NAME into the current TU.  This includes the
   main module's interface and as implementation.  */

unsigned
do_module_import (location_t loc, tree name, import_kind kind,
		  unsigned HOST_WIDE_INT stamp, unsigned crc, cpms_in *from)
{
  if (!module_map)
    {
      module_map = hash_map<lang_identifier *, unsigned>::create_ggc (31);
      vec_safe_reserve (modules, IMPORTED_MODULE_BASE);
      this_module = new (ggc_alloc <module_state> ()) module_state ();
      for (unsigned ix = IMPORTED_MODULE_BASE; ix--;)
	modules->quick_push (NULL);
      /* Insert map as unless/until we declare a module, we're the
	 global modle.  */
      if (kind <= ik_interface)
	(*modules)[GLOBAL_MODULE_INDEX] = this_module;
    }

  bool existed;
  unsigned *val = &module_map->get_or_insert (name, &existed);
  unsigned index = GLOBAL_MODULE_INDEX;
  module_state *state = NULL;

  if (existed)
    switch (*val)
      {
      case ~0U:
	error_at (loc, "circular dependency of module %qE", name);
	return GLOBAL_MODULE_INDEX;
      case GLOBAL_MODULE_INDEX:
	error_at (loc, "already failed to read module %qE", name);
	return GLOBAL_MODULE_INDEX;
      case THIS_MODULE_INDEX:
	error_at (loc, "already declared as module %qE", name);
	return GLOBAL_MODULE_INDEX;
      default:
	if (kind >= ik_interface)
	  {
	    error_at (loc, "module %qE already imported", name);
	    return GLOBAL_MODULE_INDEX;
	  }
	index = *val;
	state = (*modules)[index];
	if (stamp && stamp != state->stamp)
	  {
	    timestamp_mismatch (name, stamp, state->stamp);
	    index = GLOBAL_MODULE_INDEX;
	  }
      }
  else if (kind == ik_indirect)
    {
      /* The ordering of the import table implies that indirect
	 imports should have already been loaded.  */
      error ("indirect import %qE not present", name);
      index = GLOBAL_MODULE_INDEX;
    }
  else
    {
      *val = ~0U;

      if (kind >= ik_interface)
	{
	  state = this_module;
	  state->set_index (THIS_MODULE_INDEX);
	  (*modules)[THIS_MODULE_INDEX] = state;
	}
      else
	state = new (ggc_alloc<module_state> ()) module_state ();

      state->set_name (name);
      if (kind == ik_interface)
	index = THIS_MODULE_INDEX;
      else
	{
	  // FIXME:Path search along the -I path
	  // FIXME: Think about make dependency generation
	  char *fname = module_to_filename (name);
	  FILE *stream = fopen (fname, "rb");

	  if (!stream)
	    error_at (loc, "cannot find module %qE (%qs): %m", name, fname);
	  else
	    {
	      gcc_assert (global_namespace == current_scope ());
	      index = read_module (stream, fname, state, stamp, from);
	      fclose (stream);
	      gcc_assert (*val == ~0U);
	    }
	  free (fname);
	}
    }

  if (index != GLOBAL_MODULE_INDEX && stamp && crc != state->crc)
    {
      error ("module %qE crc mismatch", name);
      index = GLOBAL_MODULE_INDEX;
    }

  *val = index;
  return index;
}

/* Import the module NAME into the current TU and maybe re-export it.  */

void
import_module (location_t loc, tree name, tree)
{
  gcc_assert (global_namespace == current_scope ());
  unsigned index = do_module_import (loc, name, ik_direct, 0, 0);
  if (index != GLOBAL_MODULE_INDEX)
    this_module->do_import (index, export_depth != 0);
  gcc_assert (global_namespace == current_scope ());
}

/* Declare the name of the current module to be NAME. ATTRS is used to
   determine if this is the interface or not.  */

void
declare_module (location_t loc, tree name, bool inter, tree)
{
  if (this_module && this_module->name)
    {
      error_at (loc, "module %qE already declared", name);
      inform (module_loc, "existing declaration");
      return;
    }

  gcc_assert (global_namespace == current_scope ());

  module_loc = loc;

  module_state *frozen = NULL;
  if (modules)
    {
      /* If we did any importing already, freeze it.  */
      gcc_assert ((*modules)[GLOBAL_MODULE_INDEX] == this_module);
      frozen = new (ggc_alloc <module_state> ()) module_state (*this_module);
      frozen->freeze (this_module);
    }

  unsigned index = do_module_import
    (loc, name, inter ? ik_interface : ik_implementation, 0, 0);
  if (index != GLOBAL_MODULE_INDEX)
    {
      gcc_assert (index == THIS_MODULE_INDEX);

      (*modules)[GLOBAL_MODULE_INDEX] = frozen;
      current_module = index;
      this_module->direct_import = inter;
    }
}

static void
write_module (FILE *stream, const char *fname, tree name)
{
  cpms_out out (stream, fname, name);

  out.header (name);
  out.tag_conf ();
  // FIXME:Write 'important' flags etc

  /* Write the direct imports.  Write in reverse order, so that when
     checking an indirect import we should have already read it.  */
  for (unsigned ix = modules->length (); --ix > THIS_MODULE_INDEX;)
    if (module_state *state = (*modules)[ix])
      out.tag_import (ix, state);

  out.tag_trees ();

  /* Write decls.  */
  out.bindings (global_namespace);

  out.tag_eof ();
  if (int e = out.done ())
    error ("failed to write module %qE (%qs): %s", name, fname,
	   e >= 0 ? xstrerror (errno) : "Bad file data");
  out.instrument ();
}

/* Finalize the module at end of parsing.  */

void
finish_module ()
{
  if (this_module && this_module->direct_import)
    {
      // FIXME:option to specify location? take dirname from output file?
      char *fname = module_to_filename (this_module->name);

      if (!errorcount)
	{
	  FILE *stream = fopen (fname, "wb");

	  if (!stream)
	    error_at (module_loc, "cannot open module interface %qE (%qs): %m",
		      this_module->name, fname);
	  else
	    {
	      write_module (stream, fname, this_module->name);
	      fclose (stream);
	    }
	}
      if (errorcount)
	unlink (fname);
      free (fname);
    }

  /* GC can clean up the detritus.  */
  module_map = NULL;
  this_module = NULL;
}

#include "gt-cp-module.h"
