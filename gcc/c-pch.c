/* Precompiled header implementation for the C languages.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.

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
#include "cpplib.h"
#include "tree.h"
#include "c-common.h"
#include "output.h"
#include "toplev.h"

struct c_pch_header 
{
  unsigned long asm_size;
};

static const char pch_ident[8] = "gpchC010";

static FILE *pch_outfile;

extern char *asm_file_name;
static off_t asm_file_startpos;
static void (*old_late_init_hook) PARAMS((void));

static void save_asm_offset PARAMS((void));

static void
save_asm_offset ()
{
  if (old_late_init_hook)
    old_late_init_hook ();
  asm_file_startpos = ftello (asm_out_file);
}

void
pch_init ()
{
  FILE *f;
  
  if (pch_file)
    {
      f = fopen (pch_file, "wb");
      if (f == NULL)
	fatal_io_error ("can't open %s", pch_file);
      pch_outfile = f;
      
      if (fwrite (pch_ident, sizeof (pch_ident), 1, f) != 1)
	fatal_io_error ("can't write to %s", pch_file);
#if 0
      cpp_save_state (&parse_in, f);
#endif

      /* We need to be able to re-read the output.  */
      /* The driver always provides a valid -o option.  */
      if (asm_file_name == NULL
	  || strcmp (asm_file_name, "-") == 0)
	fatal_error ("`%s' is not a valid output file", asm_file_name);

      old_late_init_hook = late_init_hook;
      late_init_hook = save_asm_offset;
    }
}

void
c_common_write_pch ()
{
  char *buf;
  off_t asm_file_end;
  off_t written;
  struct c_pch_header h;

#if 0
  cpp_write_pch (&parse_in, pch_outfile);
#endif

  asm_file_end = ftello (asm_out_file);
  h.asm_size = asm_file_end - asm_file_startpos;
  
  if (fwrite (&h, sizeof (h), 1, pch_outfile) != 1)
    fatal_io_error ("can't write %s", pch_file);
  
  buf = xmalloc (16384);
  fflush (asm_out_file);

  if (fseeko (asm_out_file, asm_file_startpos, SEEK_SET) != 0)
    fatal_io_error ("can't seek in %s", asm_file_name);

  for (written = asm_file_startpos; written < asm_file_end; )
    {
      off_t size = asm_file_end - written;
      if (size > 16384)
	size = 16384;
      if (fread (buf, size, 1, asm_out_file) != 1)
	fatal_io_error ("can't read %s", asm_file_name);
      if (fwrite (buf, size, 1, pch_outfile) != 1)
	fatal_io_error ("can't write %s", pch_file);
      written += size;
    }
  free (buf);

  fclose (pch_outfile);
}

int
c_common_valid_pch (pfile, name, fd)
     cpp_reader *pfile;
     const char *name;
     int fd;
{
  int sizeread;
  int result;
  char ident[sizeof (pch_ident)];

  if (! allow_pch)
    return 2;

  /* Perform a quick test of whether this is a valid
     precompiled header for C.  */

  sizeread = read (fd, ident, sizeof (pch_ident));
  if (sizeread == -1)
    {
      fatal_io_error ("can't read %s", name);
      return 2;
    }
  else if (sizeread != sizeof (pch_ident))
    return 2;
  
  if (memcmp (ident, pch_ident, sizeof (pch_ident)) != 0)
    {
      if (memcmp (ident, pch_ident, 5) == 0)
	/* It's a PCH, for the right language, but has the wrong version.  */
	cpp_error (pfile, DL_WARNING, "not compatible with this GCC version");
      else if (memcmp (ident, pch_ident, 4) == 0)
	/* It's a PCH for the wrong language.  */
	cpp_error (pfile, DL_WARNING, "not for C language");
      return 2;
    }

#if 0
  /* Check the preprocessor macros are the same as when the PCH was
     generated.  */
  
  result = cpp_valid_state (pfile, fd);
  if (result == -1)
    return 2;
  else
    return result == 0;
#else
  return 1;
#endif
}

void
c_common_read_pch (pfile, fd)
     cpp_reader *pfile;
     int fd;
{
  FILE *f;
  struct c_pch_header h;
  char *buf;
  unsigned long written;
  
  f = fdopen (fd, "rb");
  if (f == NULL)
    {
      cpp_errno (pfile, DL_ERROR, "calling fdopen");
      return;
    }

  allow_pch = 0;

#if 0
  if (cpp_read_state (pfile, f) != 0)
    return;
#endif

  if (fread (&h, sizeof (h), 1, f) != 1)
    {
      cpp_errno (pfile, DL_ERROR, "reading");
      return;
    }

  buf = xmalloc (16384);
  for (written = 0; written < h.asm_size; )
    {
      off_t size = h.asm_size - written;
      if (size > 16384)
	size = 16384;
      if (fread (buf, size, 1, f) != 1
	  || fwrite (buf, size, 1, asm_out_file) != 1)
	cpp_errno (pfile, DL_ERROR, "reading");
      written += size;
    }
  free (buf);

  fclose (f);
}

