/* Null garbage collection for the GNU compiler.
   Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place - Suite 330, Boston, MA
   02111-1307, USA.  */

/* This version is used by the gen* programs, where we don't really
   need GC at all.  This prevents problems with pulling in all the
   tree stuff.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "ggc.h"

void *
ggc_alloc (size)
     size_t size;
{
  return xmalloc (size);
}

void *
ggc_alloc_cleared (size)
     size_t size;
{
  return xcalloc (size, 1);
}

void *
ggc_realloc (x, size)
     void *x;
     size_t size;
{
  return xrealloc (x, size);
}
