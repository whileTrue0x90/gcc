/* Header for Fortran 95 types backend support.
   Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   Contributed by Paul Brook <paul@nowt.org>
   and Steven Bosscher <s.bosscher@student.tudelft.nl>

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


#ifndef GFC_BACKEND_H
#define GFC_BACKEND_H

enum
{
  F95_INT1_TYPE,
  F95_INT2_TYPE,
  F95_INT4_TYPE,
  F95_INT8_TYPE,
  F95_INT16_TYPE,
  F95_REAL4_TYPE,
  F95_REAL8_TYPE,
  F95_REAl16_TYPE,
  F95_COMPLEX4_TYPE,
  F95_COMPLEX8_TYPE,
  F95_COMPLEX16_TYPE,
  F95_LOGICAL1_TYPE,
  F95_LOGICAL2_TYPE,
  F95_LOGICAL4_TYPE,
  F95_LOGICAL8_TYPE,
  F95_LOGICAL16_TYPE,
  F95_CHARACTER1_TYPE,
  NUM_F95_TYPES
};

#define GFC_DTYPE_RANK_MASK 0x07
#define GFC_DTYPE_TYPE_SHIFT 3
#define GFC_DTYPE_TYPE_MASK 0x38
#define GFC_DTYPE_SIZE_SHIFT 6

enum
{
  GFC_DTYPE_UNKNOWN = 0,
  GFC_DTYPE_INTEGER,
  GFC_DTYPE_LOGICAL,
  GFC_DTYPE_REAL,
  GFC_DTYPE_COMPLEX,
  GFC_DTYPE_DERIVED,
  GFC_DTYPE_CHARACTER
};

extern GTY(()) tree gfc_type_nodes[NUM_F95_TYPES];

extern GTY(()) tree gfc_array_index_type;
extern GTY(()) tree ppvoid_type_node;
extern GTY(()) tree pvoid_type_node;
extern GTY(()) tree pchar_type_node;

#define gfc_int1_type_node  gfc_type_nodes[F95_INT1_TYPE]
#define gfc_int2_type_node  gfc_type_nodes[F95_INT2_TYPE]
#define gfc_int4_type_node  gfc_type_nodes[F95_INT4_TYPE]
#define gfc_int8_type_node  gfc_type_nodes[F95_INT8_TYPE]
#define gfc_int16_type_node gfc_type_nodes[F95_INT16_TYPE]

#define gfc_real4_type_node  gfc_type_nodes[F95_REAL4_TYPE]
#define gfc_real8_type_node  gfc_type_nodes[F95_REAL8_TYPE]
#define gfc_real16_type_node gfc_type_nodes[F95_REAL16_TYPE]

#define gfc_complex4_type_node  gfc_type_nodes[F95_COMPLEX4_TYPE]
#define gfc_complex8_type_node  gfc_type_nodes[F95_COMPLEX8_TYPE]
#define gfc_complex16_type_node gfc_type_nodes[F95_COMPLEX16_TYPE]

#define gfc_logical1_type_node  gfc_type_nodes[F95_LOGICAL1_TYPE]
#define gfc_logical2_type_node  gfc_type_nodes[F95_LOGICAL2_TYPE]
#define gfc_logical4_type_node  gfc_type_nodes[F95_LOGICAL4_TYPE]
#define gfc_logical8_type_node  gfc_type_nodes[F95_LOGICAL8_TYPE]
#define gfc_logical16_type_node gfc_type_nodes[F95_LOGICAL16_TYPE]

#define gfc_character1_type_node gfc_type_nodes[F95_CHARACTER1_TYPE]

#define gfc_strlen_type_node gfc_int4_type_node

/* These C-specific types are used while building builtin function decls.
   For now it doesn't really matter what these are defined to as we don't
   need any of the builtins that use them.  */
#define intmax_type_node gfc_int8_type_node
#define string_type_node pchar_type_node
#define const_string_type_node pchar_type_node

/* be-function.c */
void gfc_convert_function_code (gfc_namespace *);

/* trans-types.c */
void gfc_init_types (void);

tree gfc_get_int_type (int);
tree gfc_get_real_type (int);
tree gfc_get_complex_type (int);
tree gfc_get_logical_type (int);
tree gfc_get_character_type (int, gfc_charlen *);

tree gfc_sym_type (gfc_symbol *);
tree gfc_typenode_for_spec (gfc_typespec *);

tree gfc_get_function_type (gfc_symbol *);

tree gfc_type_for_size (unsigned, int);
tree gfc_type_for_mode (enum machine_mode, int);
tree gfc_unsigned_type (tree);
tree gfc_signed_type (tree);
tree gfc_signed_or_unsigned_type (int, tree);

tree gfc_get_element_type (tree);
tree gfc_get_array_type_bounds (tree, int, tree *, tree *, int);
tree gfc_get_nodesc_array_type (tree, gfc_array_spec *, int);

/* Add a field of given name and type to a UNION_TYPE or RECORD_TYPE.  */
tree gfc_add_field_to_struct (tree *, tree, tree, tree);

/* Layout and output debugging info for a type.  */
void gfc_finish_type (tree);

/* Some functions have an extra parameter for the return value.  */
int gfc_return_by_reference (gfc_symbol *);

#endif
