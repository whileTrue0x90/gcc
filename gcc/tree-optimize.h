/* Tree transformation functions.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Diego Novillo <dnovillo@redhat.com>

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
Boston, MA 02111-1307, USA.  */

#ifndef _TREE_OPTIMIZE_H
#define _TREE_OPTIMIZE_H 1

/* Function prototypes.  */

void optimize_function_tree PARAMS ((tree));
void build_tree_ssa PARAMS ((tree));

/* Functions in tree-ssa-pre.c  */
extern void tree_perform_ssapre PARAMS ((void));

/* Functions in tree-ssa-ccp.c  */
void tree_ssa_ccp PARAMS ((tree));

#endif /* _TREE_OPTIMIZE_H */
