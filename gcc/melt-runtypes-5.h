/** -*- C++ -*-
   MELT header melt-runtypes-6.h
   [[middle end lisp translator, see http://gcc-melt.org/ for more.]]
   Copyright (C)  20016 Free Software Foundation, Inc.
   Contributed by Basile Starynkevitch <basile@starynkevitch.net>
   The typedefs for MELT, with GCC 6.0

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.   If not see
<http://www.gnu.org/licenses/>.
**/

#ifndef MELTRUNTYPES_INCLUDED_
#define MELTRUNTYPES_INCLUDED_

// all these are GTY-ed pointers
typedef gimple melt_gimpleptr_t;
typedef tree melt_treeptr_t;
typedef gimple_seq melt_gimpleseqptr_t;
typedef basic_block melt_basicblockptr_t;
typedef edge melt_edgeptr_t;
typedef loop_p melt_loopptr_t;
typedef rtx melt_rtxptr_t;
typedef bitmap melt_bitmapptr_t;
typedef rtvec melt_rtvecptr_t;
#endif /*MELTRUNTYPES_INCLUDED_*/
