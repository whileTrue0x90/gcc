/* Header for code constant translation functions
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   Contributed by Paul Brook

This file is part of GNU G95.

GNU G95 is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU G95 is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU G95; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Returns an INT_CST.  */
tree gfc_conv_mpz_to_tree (mpz_t, int);

/* Returns a REAL_CST.  */
tree gfc_conv_mpf_to_tree (mpf_t, int);

/* Build a tree for a constant.  Must be an EXPR_CONSTANT gfc_expr.
   For CHARACTER literal constants, the caller still has to set the
   string length as a separate operation.  */
tree gfc_conv_constant_to_tree (gfc_expr *);

/* Like gfc_conv_noncharacter_constant, but works on simplified expression
   structures.  Also sets the length of CHARACTER strings in the gfc_se.  */
void gfc_conv_constant (gfc_se *, gfc_expr *);

tree gfc_build_string_const (int, const char *);

/* Translate a string constant for a static initializer.  */
tree gfc_conv_string_init (tree, gfc_expr *);

/* Initialise the nodes for constants.  */
void gfc_init_constants (void);

/* Build a constant with given type from an int_cst.  */
tree gfc_build_const (tree, tree);

/* String constants.  */
extern GTY(()) tree gfc_strconst_current_filename;
extern GTY(()) tree gfc_strconst_bounds;
extern GTY(()) tree gfc_strconst_fault;
extern GTY(()) tree gfc_strconst_wrong_return;

/* Integer constants 0..GFC_MAX_DIMENSIONS.  */
extern GTY(()) tree gfc_rank_cst[GFC_MAX_DIMENSIONS + 1];
