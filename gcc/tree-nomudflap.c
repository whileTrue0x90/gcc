/* Mudflap: narrow-pointer bounds-checking by tree rewriting.
   Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
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


#include "config.h"
#include "errors.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "tree-inline.h"
#include "c-tree.h"
#include "c-common.h"
#include "tree-simple.h"
#include "diagnostic.h"
#include "hashtab.h"
#include "output.h"
#include "varray.h"
#include "langhooks.h"
#include "tree-mudflap.h"
#include "ggc.h"



/* This file contains placeholder functions, to be used only for
   language processors that cannot handle tree-mudflap.c directly.
   (e.g. Fortran).  */


static void
nogo (void)
{
  internal_error ("mudflap: this language is not supported");
}


void
mudflap_c_function_decls (tree t ATTRIBUTE_UNUSED)
{
  nogo ();
}


void
mudflap_c_function_ops (tree t ATTRIBUTE_UNUSED)
{
  nogo ();
}


void
mudflap_enqueue_decl (tree obj ATTRIBUTE_UNUSED)
{
  nogo ();
}


void
mudflap_enqueue_constant (tree obj ATTRIBUTE_UNUSED)
{
  nogo ();
}


/* Emit file-wide instrumentation.  */

void
mudflap_finish_file (void)
{
  nogo ();
}


int
mf_marked_p (tree t ATTRIBUTE_UNUSED)
{
  nogo ();
  return 0;
}


tree
mf_mark (tree t ATTRIBUTE_UNUSED)
{
  nogo ();
  return NULL;
}


/* Instead of:
#include "gt-tree-mudflap.h"
We prepare a little dummy struct here.
*/

const struct ggc_root_tab gt_ggc_r_gt_tree_mudflap_h[] = {
  LAST_GGC_ROOT_TAB
};
