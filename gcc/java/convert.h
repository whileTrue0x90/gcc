/* Definition of conversion functions.
   Copyright (C) 1993, 1998, 2000, 2003, 2007
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

/* Written by Jeffrey Hsu <hsu@cygnus.com> */

extern tree convert_to_integer (tree type, tree expr);
extern tree convert_to_real (tree type, tree expr);
extern tree convert_to_pointer (tree type, tree expr);
