/* Pluggable tree transforms
   Copyright 2005 Free Software Foundation, Inc.
   Contributed by Sean Callanan <sean@fsl.cs.sunysb.edu>

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

#ifndef GCC_TREE_PLUGIN_H
#define GCC_TREE_PLUGIN_H

/* Pass declaration. */

extern struct tree_opt_pass pass_plugin_gimple;
extern struct tree_opt_pass pass_plugin_ipa;
extern struct tree_opt_pass pass_plugin_rtl;

/* Function prototypes.  */

void register_tree_plugin(const char* path);

void plugins_pre_translation_unit(void);
void plugins_transform_ctrees(tree fndecl);
unsigned int plugins_transform_gimple(void);
unsigned int plugins_transform_cgraph(void);
unsigned int plugins_transform_rtl(void);
void plugins_post_translation_unit(void);

unsigned int plugins_require_ipa(void);

/* Interface to retrieve plug-in arguments. */

struct plugin_argument {
  char* key;
  char* value;
};

#endif /* GCC_TREE_PLUGIN_H */
