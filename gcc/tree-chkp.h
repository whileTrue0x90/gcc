/* Declaration of interface functions of Pointer Bounds Checker.
   Copyright (C) 2013 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef GCC_TREE_CHKP_H
#define GCC_TREE_CHKP_H

#include "tree.h"

#define DECL_BOUNDS(NODE) (chkp_get_bounds (DECL_WRTL_CHECK (NODE)))

#define SET_DECL_BOUNDS(NODE, VAL) \
  (chkp_set_bounds (DECL_WRTL_CHECK (NODE), VAL))

extern tree chkp_get_bounds (tree node);
extern void chkp_set_bounds (tree node, tree val);
extern bool chkp_register_var_initializer (tree var);
extern void chkp_finish_file (void);
extern bool chkp_type_has_pointer (const_tree type);
extern unsigned chkp_type_bounds_count (const_tree type);
extern tree chkp_make_bounds_for_struct_addr (tree ptr);
extern tree chkp_get_zero_bounds_var (void);
extern bool chkp_variable_size_type (tree type);
extern tree chkp_build_make_bounds_call (tree lb, tree size);
extern tree chkp_build_bndstx_call (tree addr, tree ptr, tree bounds);
extern vec<bool> chkp_find_bound_slots (const_tree type);
extern void chkp_build_bndstx (tree addr, tree ptr, tree bounds,
			       gimple_stmt_iterator *gsi);
extern tree chkp_parm_for_arg_bnd_arg (tree arg);
extern gimple chkp_retbnd_call_by_val (tree val);
extern bool chkp_function_instrumented_p (tree fndecl);
extern tree chkp_get_call_arg_bounds (tree arg);
extern void chkp_copy_bounds_for_assign (gimple assign,
					 struct cgraph_edge *edge);

#endif /* GCC_TREE_CHKP_H */
