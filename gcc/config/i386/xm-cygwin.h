/* Configuration for GNU C-compiler for hosting on Windows NT.
   using a unix style C library.
   Copyright (C) 1995, 1996, 1997, 1998, 1999 Free Software Foundation, Inc.

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
Boston, MA 02111-1307, USA. */

#define EXECUTABLE_SUFFIX ".exe"
#define NO_SYS_SIGLIST 1
#define HAVE_BCOPY 1
#define HAVE_BZERO 1
#define HAVE_BCMP 1
#define HAVE_RINDEX 1
#define HAVE_INDEX 1

/* We support both "/" and "\" since everybody tests both but we
   default to "/".  This is important because if gcc produces Win32
   paths containing backslashes, make and configure may treat the
   backslashes as escape characters.  Many Win32 programs use forward
   slashes so using a forward slash shouldn't be problematic from the
   perspective of wanting gcc to produce native Win32 paths. */
#define DIR_SEPARATOR '/'

/* If we allow both '/' and '\' as dir separators, then
   allow both unix and win32 PATH syntax. */
#undef GET_ENV_PATH_LIST
#define GET_ENV_PATH_LIST(VAR,NAME)					\
do {									\
  char *_epath;								\
  char *_win32epath;							\
  _epath = _win32epath = getenv (NAME);					\
  /* if we have a posix path list, convert to win32 path list */	\
  if (_epath != NULL && *_epath != 0					\
      && cygwin32_posix_path_list_p (_epath))				\
    {									\
      char *p;								\
      _win32epath = (char *) xmalloc					\
	(cygwin32_posix_to_win32_path_list_buf_size (_epath));		\
      cygwin32_posix_to_win32_path_list (_epath, _win32epath);		\
      for (p = _win32epath; p && *p; ++p)				\
        {								\
	  if (*p == '\\')						\
	    *p = '/';							\
	}								\
    }									\
  (VAR) = _win32epath;							\
} while (0)

#define PATH_SEPARATOR ';'

/* This is needed so that protoize will compile.  */
#ifndef POSIX
#define POSIX
#endif
