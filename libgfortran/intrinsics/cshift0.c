/* Generic implementation of the CSHIFT intrinsic
   Copyright 2003 Free Software Foundation, Inc.
   Contributed by Feng Wang <wf_cs@yahoo.com>

This file is part of the GNU Fortran 95 runtime library (libgfortran).

Libgfortran is free software; you can redistribute it and/or
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

/* TODO: make this work for large shifts when
   sizeof(int) < sizeof (index_type).  */

static void
__cshift0 (const gfc_array_char * ret, const gfc_array_char * array,
    int shift, int which)
{
  /* r.* indicates the return array.  */
  index_type rstride[GFC_MAX_DIMENSIONS - 1];
  index_type rstride0;
  index_type roffset;
  char *rptr;
  char *dest;
  /* s.* indicates the source array.  */
  index_type sstride[GFC_MAX_DIMENSIONS - 1];
  index_type sstride0;
  index_type soffset;
  const char *sptr;
  const char *src;

  index_type count[GFC_MAX_DIMENSIONS - 1];
  index_type extent[GFC_MAX_DIMENSIONS - 1];
  index_type dim;
  index_type size;
  index_type len;
  index_type n;

  if (which < 1 || which > GFC_DESCRIPTOR_RANK (array))
    runtime_error ("Argument 'DIM' is out of range in call to 'CSHIFT'");

  size = GFC_DESCRIPTOR_SIZE (ret);

  which = which - 1;

  extent[0] = 1;
  count[0] = 0;
  size = GFC_DESCRIPTOR_SIZE (array);
  n = 0;

/* Initialized for avoiding compiler warnings.  */
  roffset = size;
  soffset = size;
  len = 0;

  for (dim = 0; dim < GFC_DESCRIPTOR_RANK (array); dim++)
    {
      if (dim == which)
        {
          roffset = ret->dim[dim].stride * size;
          if (roffset == 0)
            roffset = size;
          soffset = array->dim[dim].stride * size;
          if (soffset == 0)
            soffset = size;
          len = array->dim[dim].ubound + 1 - array->dim[dim].lbound;
        }
      else
        {
          count[n] = 0;
          extent[n] = array->dim[dim].ubound + 1 - array->dim[dim].lbound;
          rstride[n] = ret->dim[dim].stride * size;
          sstride[n] = array->dim[dim].stride * size;
          n++;
        }
    }
  if (sstride[0] == 0)
    sstride[0] = size;
  if (rstride[0] == 0)
    rstride[0] = size;

  dim = GFC_DESCRIPTOR_RANK (array);
  rstride0 = rstride[0];
  sstride0 = sstride[0];
  rptr = ret->data;
  sptr = array->data;

  shift = (div (shift, len)).rem;
  if (shift < 0)
    shift += len;

  while (rptr)
    {
      /* Do the shift for this dimension.  */
      src = &sptr[shift * soffset];
      dest = rptr;
      for (n = 0; n < len; n++)
        {
          memcpy (dest, src, size);
          dest += roffset;
          if (n == len - shift - 1)
            src = sptr;
          else
            src += soffset;
        }

      /* Advance to the next section.  */
      rptr += rstride0;
      sptr += sstride0;
      count[0]++;
      n = 0;
      while (count[n] == extent[n])
        {
          /* When we get to the end of a dimension, reset it and increment
             the next dimension.  */
          count[n] = 0;
          /* We could precalculate these products, but this is a less
             frequently used path so proabably not worth it.  */
          rptr -= rstride[n] * extent[n];
          sptr -= sstride[n] * extent[n];
          n++;
          if (n >= dim - 1)
            {
              /* Break out of the loop.  */
              rptr = NULL;
              break;
            }
          else
            {
              count[n]++;
              rptr += rstride[n];
              sptr += sstride[n];
            }
        }
    }
}


void
__cshift0_4 (const gfc_array_char * ret, const gfc_array_char * array,
    const GFC_INTEGER_4 * pshift, const GFC_INTEGER_4 * pdim)
{
  __cshift0 (ret, array, *pshift, pdim ? *pdim : 1);
}


void
__cshift0_8 (const gfc_array_char * ret, const gfc_array_char * array,
    const GFC_INTEGER_8 * pshift, const GFC_INTEGER_8 * pdim)
{
  __cshift0 (ret, array, *pshift, pdim ? *pdim : 1);
}

