/* Mudflap: narrow-pointer bounds-checking by tree rewriting.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
   Contributed by Frank Ch. Eigler <fche@redhat.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#ifndef TREE_MUDFLAP_H
#define TREE_MUDFLAP_H

extern void mudflap_c_function PARAMS ((tree));
extern void mudflap_enqueue_decl PARAMS ((tree, const char *));
extern void mudflap_enqueue_constant PARAMS ((tree, const char *));
extern void mudflap_finish_file PARAMS ((void));

extern tree mf_static_data;


#endif /* TREE_MUDFLAP_H */
