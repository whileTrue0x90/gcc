/* Definitions for VAX running Linux-based GNU systems with ELF format.
   Copyright (C) 2007 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.  */


#undef TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (VAX GNU/Linux with ELF)");

#define TARGET_OS_CPP_BUILTINS()		\
  do						\
    {						\
	LINUX_TARGET_OS_CPP_BUILTINS();		\
	if (flag_pic)				\
	  {					\
	    builtin_define ("__PIC__");		\
	    builtin_define ("__pic__");		\
	  }					\
    }						\
  while (0)

/* We use GAS, G-float double and want new DI patterns.  */
#undef TARGET_DEFAULT
#define TARGET_DEFAULT (MASK_QMATH | MASK_G_FLOAT)

#undef CPP_SPEC
#define CPP_SPEC "%{posix:-D_POSIX_SOURCE} %{pthread:-D_REENTRANT}"

#undef ASM_SPEC
#define ASM_SPEC "%{fpic|fPIC:-k}"

#undef LINK_SPEC
#define LINK_SPEC \
 "%(endian_spec) \
  %{shared:-shared} \
  %{!shared: \
    %{!static: \
      %{rdynamic:-export-dynamic} \
      %{!dynamic-linker:-dynamic-linker /lib/ld.so.1}} \
    %{static:-static}}"
