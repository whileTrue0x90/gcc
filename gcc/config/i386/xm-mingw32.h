/* Configuration for GNU C-compiler for hosting on Windows32.
   using GNU tools and the Windows32 API Library.
   Copyright (C) 1997 Free Software Foundation, Inc.

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

#define NO_STAB_H
#define HAVE_RUSAGE
#define HAVE_FILE_H

#ifndef USG
#define USG 1
#endif

#ifndef ONLY_INT_FIELD
#define ONLY_INT_FIELDS 1
#endif

#ifndef USE_PROTOTYPES
#define USE_PROTOTYPES 1
#endif

#define NO_SYS_SIGLIST 1
#define bcmp(a,b,c) memcmp (a,b,c)
#define bcopy(a,b,c) memcpy (b,a,c)
#define bzero(a,b) memset (a,0,b)
#define index  strchr
#define rindex strrchr
#define kill(a,b) raise(b)
#define link(a,b) -1
#define environ _environ

/* Even though we support "/", allow "\" since everybody tests both.  */
#define DIR_SEPARATOR '\\'
#define EXECUTABLE_SUFFIX ".exe"
