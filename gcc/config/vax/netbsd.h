/* Definitions of target machine for GNU compiler.
   NetBSD/vax a.out version.
   Copyright (C) 1997, 1998, 1999, 2000, 2001, 2002, 2007, 2010
   Free Software Foundation, Inc.

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


#define TARGET_OS_CPP_BUILTINS()		\
  do						\
    {						\
      NETBSD_OS_CPP_BUILTINS_AOUT();		\
    }						\
  while (0)

#undef CPP_SPEC
#define CPP_SPEC NETBSD_CPP_SPEC

/* Make gcc agree with <machine/ansi.h> */

#undef SIZE_TYPE
#define SIZE_TYPE itk_unsigned_int

#undef PTRDIFF_TYPE
#define PTRDIFF_TYPE itk_int

/* Until they use ELF or something that handles dwarf2 unwinds
   and initialization stuff better.  Use sjlj exceptions.  */
#undef DWARF2_UNWIND_INFO

/* We use gas, not the UNIX assembler.  */
#undef TARGET_DEFAULT
#define TARGET_DEFAULT 0
