/* Generic helper function for repacking arrays.
   Copyright 2003 Free Software Foundation, Inc.
   Contributed by Paul Brook <paul@nowt.org>

This file is part of the GNU Fortran 95 runtime library (libgfor).

Libgfor is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

Ligbfor is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with libgfor; see the file COPYING.LIB.  If not,
write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "config.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "libgfortran.h"

extern void *internal_pack (gfc_array_char *);
export_proto(internal_pack);

void *
internal_pack (gfc_array_char * source)
{
  index_type count[GFC_MAX_DIMENSIONS - 1];
  index_type extent[GFC_MAX_DIMENSIONS - 1];
  index_type stride[GFC_MAX_DIMENSIONS - 1];
  index_type stride0;
  index_type dim;
  index_type ssize;
  const char *src;
  char *dest;
  void *destptr;
  int n;
  int packed;
  index_type size;

  if (source->dim[0].stride == 0)
    {
      source->dim[0].stride = 1;
      return source->data;
    }

  size = GFC_DESCRIPTOR_SIZE (source);
  switch (size)
    {
    case 4:
      return internal_pack_4 ((gfc_array_i4 *)source);

    case 8:
      return internal_pack_8 ((gfc_array_i8 *)source);
    }

  dim = GFC_DESCRIPTOR_RANK (source);
  ssize = 1;
  packed = 1;
  for (n = 0; n < dim; n++)
    {
      count[n] = 0;
      stride[n] = source->dim[n].stride;
      extent[n] = source->dim[n].ubound + 1 - source->dim[n].lbound;
      if (extent[n] <= 0)
        {
          /* Do nothing.  */
          packed = 1;
          break;
        }

      if (ssize != stride[n])
        packed = 0;

      ssize *= extent[n];
    }

  if (packed)
    return source->data;

   /* Allocate storage for the destination.  */
  destptr = internal_malloc_size (ssize * size);
  dest = (char *)destptr;
  src = source->data;
  stride0 = stride[0] * size;

  while (src)
    {
      /* Copy the data.  */
      memcpy(dest, src, size);
      /* Advance to the next element.  */
      dest += size;
      src += stride0;
      count[0]++;
      /* Advance to the next source element.  */
      n = 0;
      while (count[n] == extent[n])
        {
          /* When we get to the end of a dimension, reset it and increment
             the next dimension.  */
          count[n] = 0;
          /* We could precalculate these products, but this is a less
             frequently used path so proabably not worth it.  */
          src -= stride[n] * extent[n] * size;
          n++;
          if (n == dim)
            {
              src = NULL;
              break;
            }
          else
            {
              count[n]++;
              src += stride[n] * size;
            }
        }
    }
  return destptr;
}
