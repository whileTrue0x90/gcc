/* Machine description for AArch64 architecture.
   Copyright (C) 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

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

#ifndef GCC_AARCH64_LINUX_H
#define GCC_AARCH64_LINUX_H

#define MULTIARCH_TUPLE "aarch64-linux-gnu"

#undef STANDARD_STARTFILE_PREFIX_1
#define STANDARD_STARTFILE_PREFIX_1 "/lib/" MULTIARCH_TUPLE "/"

#undef STANDARD_STARTFILE_PREFIX_2
#define STANDARD_STARTFILE_PREFIX_2 "/usr/lib/" MULTIARCH_TUPLE "/"

#define GLIBC_DYNAMIC_LINKER "/lib/ld-linux-aarch64.so.1"

#define LINUX_TARGET_LINK_SPEC  "%{h*}		\
   %{static:-Bstatic}				\
   %{shared:-shared}				\
   %{symbolic:-Bsymbolic}			\
   %{rdynamic:-export-dynamic}			\
   -dynamic-linker " GNU_USER_DYNAMIC_LINKER "	\
   -X						\
   %{mbig-endian:-EB} %{mlittle-endian:-EL}"

#define LINK_SPEC LINUX_TARGET_LINK_SPEC

#define TARGET_OS_CPP_BUILTINS()		\
  do						\
    {						\
	GNU_USER_TARGET_OS_CPP_BUILTINS();	\
    }						\
  while (0)

#endif  /* GCC_AARCH64_LINUX_H */
