/* Header for code translation functions
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

#ifndef GFC_TRANS_H
#define GFC_TRANS_H

/* Mangled symbols take the form __module__name.  */
#define GFC_MAX_MANGLED_SYMBOL_LEN  (GFC_MAX_SYMBOL_LEN*2+4)

/* Struct for holding a block of statements.  It should be treated as an
   opaque entity and not modified directly.  This allows us to change the
   underlying representation of statement lists.  */
typedef struct
{
  tree head;
  int has_scope:1;
}
stmtblock_t;

/* a simplified expresson */
typedef struct gfc_se
{
  /* Code blocks to be executed before and after using the value.  */
  stmtblock_t pre;
  stmtblock_t post;

  /* the result of the expression */
  tree expr;

  /* The length of a character string value.  */
  tree string_length;

  /* If set gfc_conv_variable will return an expression for the array
     descriptor. When set, want_pointer should also be set.
     If not set scalarizing variables will be substituted.  */
  unsigned descriptor_only:1;

  /* When this is set gfc_conv_expr returns the address of a variable.  Only
     applies to EXPR_VARIABLE nodes.
     Also used by gfc_conv_array_parameter. When set this indicates a pointer
     to the descriptor should be returned, rather than the descriptor itself.
   */
  unsigned want_pointer:1;

  /* An array function call returning without a temporary.  Also used for array
     pointer assignments.  */
  unsigned direct_byref:1;

  /* Ignore absent optional arguments.  Used for some intrinsics.  */
  unsigned ignore_optional:1;

  /* Scalarization parameters.  */
  struct gfc_se *parent;
  struct gfc_ss *ss;
  struct gfc_loopinfo *loop;
}
gfc_se;


/* Scalarisation State chain.  Created by walking an expression tree before
   creating the scalarization loops. Then passed as part of a gfc_se structure
   to translate the expression inside the loop.  Note that these chains are
   terminated by gfc_se_terminator, not NULL.  A NULL pointer in a gfc_se
   indicates to gfc_conv_* that this is a scalar expression.
   Note that some member arrays correspond to scalarizer rank and others
   are the variable rank.  */

typedef struct gfc_ss_info
{
  int dimen;
  /* The ref that holds information on this section.  */
  gfc_ref *ref;
  /* The descriptor of this array.  */
  tree descriptor;
  /* holds the pointer to the data array.  */
  tree data;
  tree pdata;
  /* To move some of the array index calculation out of the innermost loop.  */
  tree offset;
  tree saved_offset;
  tree stride0;
  /* Holds the SS for a subscript.  Indexed by actual dimension.  */
  struct gfc_ss *subscript[GFC_MAX_DIMENSIONS];

  /* stride and delta are used to access this inside a scalarization loop.
     start is used in the calculation of these.  Indexed by scalarizer
     dimension.  */
  tree start[GFC_MAX_DIMENSIONS];
  tree stride[GFC_MAX_DIMENSIONS];
  tree delta[GFC_MAX_DIMENSIONS];

  /* Translation from scalariser dimensions to actual dimensions.
     actual = dim[scalarizer]  */
  int dim[GFC_MAX_DIMENSIONS];
}
gfc_ss_info;

typedef enum
{
  /* A scalar value.  This will be evaluated before entering the
     scalarization loop.  */
  GFC_SS_SCALAR,

  /* Like GFC_SS_SCALAR except it evaluates a pointer the the expression.
     Used for elemental function parameters.  */
  GFC_SS_REFERENCE,

  /* An array section.  Scalarization indices will be substituted during
     expression translation.  */
  GFC_SS_SECTION,

  /* A non-elemental function call returning an array.  The call is executed
     before entering the scalarization loop, storing the result in a
     temporary.  This temporary is then used inside the scalarization loop.
     Simple assignments, eg. a(:) = fn() are handles without a temporary
     as a special case.  */
  GFC_SS_FUNCTION,

  /* An array constructor.  The current implementation is sub-optimal in
     many cases.  It allocated a temporary, assigns the values to it, then
     uses this temporary inside the scalarization loop.  */
  GFC_SS_CONSTRUCTOR,

  /* A vector subscript.  Only used as the SS chain for a subscript.
     Similar int format to a GFC_SS_SECTION.  */
  GFC_SS_VECTOR,

  /* A temporary array allocated by the scalarizer.  Its rank can be less
     than that of the assignment expression.  */
  GFC_SS_TEMP,

  /* An intrinsic function call.  Many intrinsic functions which map directly
     to library calls are created as GFC_SS_FUNCTION nodes.  */
  GFC_SS_INTRINSIC
}
gfc_ss_type;

/* SS structures can only belong to a single loopinfo.  They must be added
   otherwise they will not get freed.  */
typedef struct gfc_ss
{
  gfc_ss_type type;
  gfc_expr *expr;
  union
  {
    /* If type is GFC_SS_SCALAR or GFC_SS_REFERENCE.  */
    struct
    {
      tree expr;
      tree string_length;
    }
    scalar;

    /* GFC_SS_TEMP.  */
    struct
    {
      /* The rank of the temporary.  May be less than the rank of the
         assigned expression.  */
      int dimen;
      tree type;
      tree string_length;
    }
    temp;
    /* All other types.  */
    gfc_ss_info info;
  }
  data;

  /* All the SS in a loop and linked through loop_chain.  The SS for an
     expression are linked by the next pointer.  */
  struct gfc_ss *loop_chain;
  struct gfc_ss *next;

  /* This is used by assignments requiring teporaries. The bits specify which
     loops the terms appear in.  This will be 1 for the RHS expressions,
     2 for the LHS expressions, and 3(=1|2) for the temporary.  */
  unsigned useflags:2;
}
gfc_ss;
#define gfc_get_ss() gfc_getmem(sizeof(gfc_ss))

/* The contents of this aren't actualy used.  A NULL SS chain indicates a
   scalar expression, so this pointer is used to terminate SS chains.  */
extern gfc_ss * const gfc_ss_terminator;

/* Holds information about an expression while it is being scalarized.  */
typedef struct gfc_loopinfo
{
  stmtblock_t pre;
  stmtblock_t post;

  int dimen;

  /* All the SS involved with this loop.  */
  gfc_ss *ss;
  /* The SS describing the teporary used in an assignment.  */
  gfc_ss *temp_ss;

  /* The scalarization loop index variables.  */
  tree loopvar[GFC_MAX_DIMENSIONS];

  /* The bounds of the scalarization loops.  */
  tree from[GFC_MAX_DIMENSIONS];
  tree to[GFC_MAX_DIMENSIONS];
  gfc_ss *specloop[GFC_MAX_DIMENSIONS];

  /* The code member contains the code for the body of the next outer loop.  */
  stmtblock_t code[GFC_MAX_DIMENSIONS];

  /* Order in which the dimensions should be looped, innermost first.  */
  int order[GFC_MAX_DIMENSIONS];

  /* The number of dimensions for which a temporary is used.  */
  int temp_dim;

  /* If set we don't need the loop variables.  */
  unsigned array_parameter:1;
}
gfc_loopinfo;

/* Advance the SS chain to the next term.  */
void gfc_advance_se_ss_chain (gfc_se *);

/* Call this to initialise a gfc_se structure before use
   first parameter is structure to initialise, second is
   parent to get scalarization data from, or NULL.  */
void gfc_init_se (gfc_se *, gfc_se *);

/* Create an artificial variable decl and add it to the current scope.  */
tree gfc_create_var (tree, const char *);
/* Like above but doesn't add it to the current scope.  */
tree gfc_create_var_np (tree, const char *);

/* Store the result of an expression in a temp variable so it can be used
   repeatedly even if the original changes */
void gfc_make_safe_expr (gfc_se * se);

/* Makes sure se is suitable for passing as a function string parameter.  */
void gfc_conv_string_parameter (gfc_se * se);

/* Add an item to the end of TREE_LIST.  */
tree gfc_chainon_list (tree, tree);

/* When using the gfc_conv_* make sure you understand what they do, ie.
   when a POST chain may be created, and what the retured expression may be
   used for.  Note that character strings have special handling.  This
   should not be a problem as most statements/operations only deal with
   numeric/logical types.  */

/* Entry point for expression translation.  */
void gfc_conv_expr (gfc_se * se, gfc_expr * expr);
/* Like gfc_conv_expr, but the POST block is guaranteed to be empty for
   numeric expressions.  */
void gfc_conv_expr_val (gfc_se * se, gfc_expr * expr);
/* Like gfc_conv_expr_val, but the value is also suitable for use in the lhs of
   an assignment.  */
void gfc_conv_expr_lhs (gfc_se * se, gfc_expr * expr);
/* Converts an expression so that it can be passed be reference.  */
void gfc_conv_expr_reference (gfc_se * se, gfc_expr *);
/* Equivalent to convert(type, gfc_conv_expr_val(se, expr)).  */
void gfc_conv_expr_type (gfc_se * se, gfc_expr *, tree);
/* If the value is not constant, Create a temporary and copy the value.  */
tree gfc_evaluate_now (tree, stmtblock_t *);

/* Intrinsic function handling.  */
void gfc_conv_intrinsic_function (gfc_se *, gfc_expr *);

/* Does an intrinsic map directly to an external library call.  */
int gfc_is_intrinsic_libcall (gfc_expr *);

/* Also used to CALL subroutines.  */
void gfc_conv_function_call (gfc_se *, gfc_symbol *, gfc_actual_arglist *);
/* gfc_trans_* shouldn't call push/poplevel, use gfc_push/pop_scope */

/* Generate code for a scalar assignment.  */
tree gfc_trans_scalar_assign (gfc_se *, gfc_se *, bt);

/* Translate EQUIVALENCE lists.  */
void gfc_trans_equivalence (gfc_namespace *);

/* Translate COMMON blocks.  */
void gfc_trans_common (gfc_namespace *);

/* Return an expression which determines if a dummy parameter is present.  */
tree gfc_conv_expr_present (gfc_symbol *);

/* Generate code to allocate a string temporary.  */
tree gfc_conv_string_tmp (gfc_se *, tree, tree);
/* Get the length of a string.  */
tree gfc_conv_string_length (tree);
/* Initialize a string length variable.  */
tree gfc_conv_init_string_length (gfc_symbol *, stmtblock_t *);

/* Add an expression to the end of a block.  */
void gfc_add_expr_to_block (stmtblock_t *, tree);
/* Add a block to the end of a block.  */
void gfc_add_block_to_block (stmtblock_t *, stmtblock_t *);
/* Add a MODIFY_EXPR to a block.  */
void gfc_add_modify_expr (stmtblock_t *, tree, tree);

/* Initialize a statement block.  */
void gfc_init_block (stmtblock_t *);
/* Start a new satement block.  Like gfc_init_block but also starts a new
   variable scope.  */
void gfc_start_block (stmtblock_t *);
/* Finish a statement block.  Also closes the scope if the block was created
   with gfc_start_block.  */
tree gfc_finish_block (stmtblock_t *);
/* Merge the scope of a block with its parent.  */
void gfc_merge_block_scope (stmtblock_t * block);

/* Return the backend label decl.  */
tree gfc_get_label_decl (gfc_st_label *);

/* Return the decl for an external function.  */
tree gfc_get_extern_function_decl (gfc_symbol *);

/* Return the decl for a function.  */
tree gfc_get_function_decl (gfc_symbol *);

/* Build a CALL_EXPR.  */
tree gfc_build_function_call (tree, tree);

/* Creates an label.  Decl is artificial if label_id == NULL_TREE.  */
tree gfc_build_label_decl (tree);

/* Return the decl used to hold the function return value.
   Do not use if the function has an explicit result variable.  */
tree gfc_get_fake_result_decl (gfc_symbol *);

/* Get the return label for the current function.  */
tree gfc_get_return_label (void);

/* Add a decl to the binding level for the current function.  */
void gfc_add_decl_to_function (tree);

/* Make prototypes for runtime library functions.  */
void gfc_build_builtin_function_decls (void);

/* Return the variable decl for a symbol.  */
tree gfc_get_symbol_decl (gfc_symbol *);

/* Allocate the lang-spcific part of a decl node.  */
void gfc_allocate_lang_decl (tree);

/* Advance along a TREE_CHAIN.  */
tree gfc_advance_chain (tree, int);

/* Create a decl for a function.  */
void gfc_build_function_decl (gfc_symbol *);
/* Generate the code for a function.  */
void gfc_generate_function_code (gfc_namespace *);
/* Output a decl for a module variable.  */
void gfc_generate_module_vars (gfc_namespace *);

/* Get and set the current location.  */
void gfc_set_backend_locus (locus *);
void gfc_get_backend_locus (locus *);

/* Handle static constructor functions.  */
extern GTY(()) tree gfc_static_ctors;
void gfc_generate_constructors (void);

/* Generate a runtime error check.  */
void gfc_trans_runtime_check (tree, tree, stmtblock_t *);

/* Generate code for an assigment, includes scalarization.  */
tree gfc_trans_assignment (gfc_expr *, gfc_expr *);

/* Generate code for an pointer assignment.  */
tree gfc_trans_pointer_assignment (gfc_expr *, gfc_expr *);

/* Initialize function decls for library functions.  */
void gfc_build_intrinsic_lib_fndecls (void);
/* Create function decls for IO library functions.  */
void gfc_build_io_library_fndecls (void);
/* Build a function decl for a library function.  */
tree gfc_build_library_function_decl (tree, tree, int, ...);

/* somewhere! */
tree pushdecl (tree);
tree pushdecl_top_level (tree);
void pushlevel (int);
tree poplevel (int, int, int);
void expand_function_body (tree, int);
tree getdecls (void);
tree gfc_truthvalue_conversion (tree);

/* Runtime library function decls.  */
extern GTY(()) tree gfor_fndecl_push_context;
extern GTY(()) tree gfor_fndecl_pop_context;
extern GTY(()) tree gfor_fndecl_internal_malloc;
extern GTY(()) tree gfor_fndecl_internal_malloc64;
extern GTY(()) tree gfor_fndecl_internal_free;
extern GTY(()) tree gfor_fndecl_allocate;
extern GTY(()) tree gfor_fndecl_allocate64;
extern GTY(()) tree gfor_fndecl_deallocate;
extern GTY(()) tree gfor_fndecl_pause_numeric;
extern GTY(()) tree gfor_fndecl_pause_string;
extern GTY(()) tree gfor_fndecl_stop_numeric;
extern GTY(()) tree gfor_fndecl_stop_string;
extern GTY(()) tree gfor_fndecl_select_string;
extern GTY(()) tree gfor_fndecl_runtime_error;
extern GTY(()) tree gfor_fndecl_in_pack;
extern GTY(()) tree gfor_fndecl_in_unpack;
extern GTY(()) tree gfor_fndecl_associated;

/* Math functions.  Many other math functions are handled in
   trans-intrinsic.c.  */
extern GTY(()) tree gfor_fndecl_math_powf;
extern GTY(()) tree gfor_fndecl_math_pow;
extern GTY(()) tree gfor_fndecl_math_cpowf;
extern GTY(()) tree gfor_fndecl_math_cpow;
extern GTY(()) tree gfor_fndecl_math_cabsf;
extern GTY(()) tree gfor_fndecl_math_cabs;
extern GTY(()) tree gfor_fndecl_math_sign4;
extern GTY(()) tree gfor_fndecl_math_sign8;
extern GTY(()) tree gfor_fndecl_math_ishftc4;
extern GTY(()) tree gfor_fndecl_math_ishftc8;

/* String functions.  */
extern GTY(()) tree gfor_fndecl_copy_string;
extern GTY(()) tree gfor_fndecl_compare_string;
extern GTY(()) tree gfor_fndecl_concat_string;
extern GTY(()) tree gfor_fndecl_string_len_trim;
extern GTY(()) tree gfor_fndecl_string_index;
extern GTY(()) tree gfor_fndecl_string_scan;
extern GTY(()) tree gfor_fndecl_string_verify;
extern GTY(()) tree gfor_fndecl_adjustl;
extern GTY(()) tree gfor_fndecl_adjustr;

/* Other misc. runtime library functions.  */
extern GTY(()) tree gfor_fndecl_size0;
extern GTY(()) tree gfor_fndecl_size1;

/* True if node is an integer constant.  */
#define INTEGER_CST_P(node) (TREE_CODE(node) == INTEGER_CST)

/* G95-specific declaration information.  */

/* Array types only.  */
struct lang_type		GTY(())
{
  int rank;
  tree lbound[GFC_MAX_DIMENSIONS];
  tree ubound[GFC_MAX_DIMENSIONS];
  tree stride[GFC_MAX_DIMENSIONS];
  tree size;
  tree offset;
  tree dtype;
  tree dataptr_type;
};

/* String nodes only.  */
struct lang_decl		GTY(())
{
  tree stringlength;
  tree saved_descriptor;
};

#define GFC_DECL_STRING_LENGTH(node) (DECL_LANG_SPECIFIC(node)->stringlength)
#define GFC_DECL_SAVED_DESCRIPTOR(node) \
  (DECL_LANG_SPECIFIC(node)->saved_descriptor)
#define GFC_DECL_STRING(node) DECL_LANG_FLAG_0(node)
#define GFC_DECL_PACKED_ARRAY(node) DECL_LANG_FLAG_1(node)
#define GFC_DECL_PARTIAL_PACKED_ARRAY(node) DECL_LANG_FLAG_2(node)

#define GFC_KNOWN_SIZE_STRING_TYPE(node) TYPE_LANG_FLAG_0(node)
/* An array descriptor.  */
#define GFC_DESCRIPTOR_TYPE_P(node) TYPE_LANG_FLAG_1(node)
/* An array without a descriptor.  */
#define GFC_ARRAY_TYPE_P(node) TYPE_LANG_FLAG_2(node)
/* The GFC_TYPE_ARRAY_* members are present in both descriptor and
   descriptorless array types.  */
#define GFC_TYPE_ARRAY_LBOUND(node, dim) \
  (TYPE_LANG_SPECIFIC(node)->lbound[dim])
#define GFC_TYPE_ARRAY_UBOUND(node, dim) \
  (TYPE_LANG_SPECIFIC(node)->ubound[dim])
#define GFC_TYPE_ARRAY_STRIDE(node, dim) \
  (TYPE_LANG_SPECIFIC(node)->stride[dim])
#define GFC_TYPE_ARRAY_RANK(node) (TYPE_LANG_SPECIFIC(node)->rank)
#define GFC_TYPE_ARRAY_SIZE(node) (TYPE_LANG_SPECIFIC(node)->size)
#define GFC_TYPE_ARRAY_OFFSET(node) (TYPE_LANG_SPECIFIC(node)->offset)
#define GFC_TYPE_ARRAY_DTYPE(node) (TYPE_LANG_SPECIFIC(node)->dtype)
#define GFC_TYPE_ARRAY_DATAPTR_TYPE(node) \
  (TYPE_LANG_SPECIFIC(node)->dataptr_type)

/* I changed this from sorry(...) because it should not return.  */
/* TODO: Remove gfc_todo_error before releasing version 1.0.  */
#define gfc_todo_error(args...) fatal_error("gfc_todo: Not Implemented: " args)

/* Build an expression with void type.  */
#define build1_v(code, arg) build(code, void_type_node, arg)
#define build_v(code, args...) build(code, void_type_node, args)

#endif /* GFC_TRANS_H */
