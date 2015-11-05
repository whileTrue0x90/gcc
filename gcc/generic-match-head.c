/* Preamble and helpers for the autogenerated generic-match.c file.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.

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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "backend.h"
#include "target.h"
#include "rtl.h"
#include "tree.h"
#include "gimple.h"
#include "ssa.h"
#include "cgraph.h"
#include "fold-const.h"
#include "stor-layout.h"
#include "tree-dfa.h"
#include "builtins.h"
#include "dumpfile.h"


/* Routine to determine if the types T1 and T2 are effectively
   the same for GENERIC.  If T1 or T2 is not a type, the test
   applies to their TREE_TYPE.  */

static inline bool
types_match (tree t1, tree t2)
{
  if (!TYPE_P (t1))
    t1 = TREE_TYPE (t1);
  if (!TYPE_P (t2))
    t2 = TREE_TYPE (t2);

  return TYPE_MAIN_VARIANT (t1) == TYPE_MAIN_VARIANT (t2);
}

/* Return if T has a single use.  For GENERIC, we assume this is
   always true.  */

static inline bool
single_use (tree t ATTRIBUTE_UNUSED)
{
  return true;
}

/* Return true if math operations should be canonicalized,
   e.g. sqrt(sqrt(x)) -> pow(x, 0.25).  */

static inline bool
canonicalize_math_p ()
{
  return true;
}
