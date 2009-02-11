/* Copyright (C) 1999, 2000, 2001, 2007 Free Software Foundation, Inc.
   Contributed by Andrew MacLeod  <amacleod@cygnus.com>
                  Andrew Haley  <aph@cygnus.com>

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
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

struct unw_table_entry
{
  unsigned long start_offset;
  unsigned long end_offset;
  unsigned long info_offset;
};

extern struct unw_table_entry *
_Unwind_FindTableEntry (void *pc, unsigned long *segment_base,
			unsigned long *gp)
			__attribute__ ((__visibility__ ("hidden")));
