/* Control and data flow functions for trees.
   Copyright (C) 2001, 2002 Free Software Foundation, Inc.
   Contributed by Diego Novillo <dnovillo@redhat.com>

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tree.h"
#include "rtl.h"
#include "tm_p.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "output.h"
#include "expr.h"
#include "diagnostic.h"
#include "basic-block.h"
#include "flags.h"
#include "tree-flow.h"
#include "tree-dump.h"
#include "timevar.h"
#include "cfgloop.h"

/* Rewrite a function tree to the SSA form and perform the SSA-based
   optimizations on it.  */

/* Main entry point to the tree SSA transformation routines.  FNDECL is the
   FUNCTION_DECL node for the function to optimize.  */

void
optimize_function_tree (fndecl)
     tree fndecl;
{
  tree fnbody;

  /* Don't bother doing anything if the program has errors.  */
  if (errorcount || sorrycount)
    return;
  
  fnbody = DECL_SAVED_TREE (fndecl);

  /* Build the flowgraph.  */
  init_flow ();
  build_tree_cfg (fnbody);

  /* Begin analysis and optimization passes.  */
  if (n_basic_blocks > 0 && ! (errorcount || sorrycount))
    {
#if 0
      struct loops *loops;
      
      /* The loop analyzer should be initialized after the CFG construction.  */
      loops = loop_optimizer_init (NULL);
      
      /* For the moment the loops structure is not used.  If you use this 
	 structure you'll have to ensure that optimizers don't invalidate the
	 information gathered in the loops structure via modifications to the
	 underlying structure: the CFG.  */
      loop_optimizer_finalize (loops, NULL);
#endif 
      /* Rewrite the function into SSA form.  */
      rewrite_into_ssa (fndecl);
      
      if (flag_tree_pre)
	tree_perform_ssapre (fndecl);

      if (flag_tree_ccp)
	tree_ssa_ccp (fndecl);
      
      if (flag_tree_copyprop)
	tree_ssa_copyprop (fndecl);

      if (flag_tree_dce)
	tree_ssa_dce (fndecl);

      /* Rewrite the function out of SSA form.  */
      rewrite_out_of_ssa (fndecl);
    }

  /* Debugging dump after optimization.  */
  dump_function (TDI_optimized, fndecl);
}
