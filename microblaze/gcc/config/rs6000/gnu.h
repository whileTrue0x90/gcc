/* Definitions of target machine for GNU compiler,
   for PowerPC machines running GNU.
   Copyright (C) 2001, 2003, 2007 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#undef	CPP_OS_DEFAULT_SPEC
#define CPP_OS_DEFAULT_SPEC "%(cpp_os_gnu)"

#undef	STARTFILE_DEFAULT_SPEC
#define STARTFILE_DEFAULT_SPEC "%(startfile_gnu)"

#undef	ENDFILE_DEFAULT_SPEC
#define ENDFILE_DEFAULT_SPEC "%(endfile_gnu)"

#undef	LINK_START_DEFAULT_SPEC
#define LINK_START_DEFAULT_SPEC "%(link_start_gnu)"

#undef	LINK_OS_DEFAULT_SPEC
#define LINK_OS_DEFAULT_SPEC "%(link_os_gnu)"

#undef  TARGET_VERSION
#define TARGET_VERSION fprintf (stderr, " (PowerPC GNU)");
