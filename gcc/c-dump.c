/* Tree-dumping functionality for C-family languages.
   Copyright (C) 2002, 2004, 2005 Free Software Foundation, Inc.
   Written by Mark Mitchell <mark@codesourcery.com>

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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "c-tree.h"
#include "tree-dump.h"

/* Dump information common to statements from STMT.  */

void
dump_stmt (dump_info_p di, tree t)
{
  if (EXPR_HAS_LOCATION (t))
    dump_int (di, "line", EXPR_LINENO (t));
}

/* Dump any C-specific tree codes and attributes of common codes.  */

bool
c_dump_tree (void *dump_info, tree t)
{
  enum tree_code code;
  dump_info_p di = (dump_info_p) dump_info;

  /* Figure out what kind of node this is.  */
  code = TREE_CODE (t);

  switch (code)
    {
    case FIELD_DECL:
      if (DECL_C_BIT_FIELD (t))
	dump_string (di, "bitfield");
      break;

    case EXPR_STMT:
      dump_stmt (di, t);
      dump_child ("expr", EXPR_STMT_EXPR (t));
      break;

    default:
      break;
    }

  return false;
}
