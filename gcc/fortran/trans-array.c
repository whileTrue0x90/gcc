/* Array translation routines
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

/* trans-array.c-- Various array related code, including scalarization,
                   allocation, initialization and other support routines.  */

/* How the scalarizer works.
   In gfortran, array expressions use the same core routines as scalar
   expressions.
   First, a Scalarization State (SS) chain is built.  This is done by walking
   the expression tree, and building a linear list of the terms in the
   expression.  As the tree is walked, scalar subexpressions are translated.

   The scalarization parameters are stored in a gfc_loopinfo structure.
   First the start and stride of each term is calculated by
   gfc_conv_ss_startstride.  During this process the expressions for the array
   descriptors and data pointers are also translated.

   If the expression is an assignment, we must then resolve and dependencies.
   In fortran all the rhs values of an assignment must be evaluated before
   any assignments take place.  This can require a temporary array to store the
   values.  We also require a temporary when we are passing array expressions
   or vector subecripts as procedure parameters.

   Array sections are passed without copying to a temporary.  These use the
   scalarizer to determine the shape of the section.  The flag
   loop->array_parameter tells the scalarizer that the actual values and loop
   variables will not be required.

   The function gfc_conv_loop_setup generates the scalarization setup code.
   It determines the range of the scalarizing loop variables.  If a temporary
   is required, this is created and initialized.  Code for scalar expressions
   taken outside the loop is also generated at this time.  Next the offset and
   scaling required to translate from loop variables to array indices for each
   term is calculated.

   A call to gfc_start_scalarized_body marks the start of the scalarized
   expression.  This creates a scope and declares the loop variables.  Before
   calling this gfc_make_ss_chain_used must be used to indicate which terms
   will be used inside this loop.

   The scalar gfc_conv_* functions are then used to build the main body of the
   scalarization loop.  Scalarization loop variables and precalculated scalar
   values are automaticaly substituted.  Note that gfc_advance_se_ss_chain
   must be used, rather than changing the se->ss directly.

   For assignment expressions requiring a temporary two sub loops are
   generated.  The first stores the result of the expression in the temporary,
   the second copies it to the result.  A Call to
   gfc_trans_scalarized_loop_boundary marks the end of the main loop code and
   the start of the copying loop.  The temporary may be less than full rank.

   Finally gfc_trans_scalarizing_loops is called to generate the impicit do
   loops.  The loops are added to the pre chain of the loopinfo.  The post
   chain may still contain cleanup code.

   After the loop code has been added into its parent scope gfc_cleanup_loop
   is called to free all the SS allocated by the scalarizer.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-simple.h"
#include <stdio.h>
#include "ggc.h"
#include "toplev.h"
#include "real.h"
#include "flags.h"
#include <assert.h>
#include <gmp.h>
#include "gfortran.h"
#include "trans.h"
#include "trans-stmt.h"
#include "trans-types.h"
#include "trans-array.h"
#include "trans-const.h"
#include "dependency.h"

static gfc_ss *gfc_walk_subexpr (gfc_ss *, gfc_expr *);

/* The contents of this structure aren't actualy used, just the address.  */
static gfc_ss gfc_ss_terminator_var;
gfc_ss * const gfc_ss_terminator = &gfc_ss_terminator_var;

unsigned HOST_WIDE_INT gfc_stack_space_left;


/* Returns true if a variable of specified size should go on the stack.  */

int
gfc_can_put_var_on_stack (tree size)
{
  unsigned HOST_WIDE_INT low;

  if (!INTEGER_CST_P (size))
    return 0;

  if (gfc_option.flag_max_stack_var_size < 0)
    return 1;

  if (TREE_INT_CST_HIGH (size) != 0)
    return 0;

  low = TREE_INT_CST_LOW (size);
  if (low > (unsigned HOST_WIDE_INT) gfc_option.flag_max_stack_var_size)
    return 0;

/* TODO: Set a per-function stack size limit.  */
#if 0
  /* We should be a bit more clever with array temps.  */
  if (gfc_option.flag_max_function_vars_size >= 0)
    {
      if (low > gfc_stack_space_left)
	return 0;

      gfc_stack_space_left -= low;
    }
#endif

  return 1;
}

static tree
gfc_array_dataptr_type (tree desc)
{
  return (GFC_TYPE_ARRAY_DATAPTR_TYPE (TREE_TYPE (desc)));
}


/* Build expressions to access the members of an array descriptor.
   It's surprisingly easy to mess up here, so never access
   an array descriptor by "brute force", always use these
   functions.  This also avoids problems if we change the format
   of an array descriptor.

   To understand these magic numbers, look at the comments
   before gfc_build_array_type() in trans-types.c.

   The code within these defines should be the only code which knows the format
   of an array descriptor.

   Any code just needing to read obtain the bounds of an array should use
   gfc_conv_array_* rather than the following functions as these will return
   know constant values, and work with arrays which do not have descriptors.

   Don't forget to #undef these!  */

#define DATA_FIELD 0
#define BASE_FIELD 1
#define DTYPE_FIELD 2
#define DIMENSION_FIELD 3

#define STRIDE_SUBFIELD 0
#define LBOUND_SUBFIELD 1
#define UBOUND_SUBFIELD 2

tree
gfc_conv_descriptor_data (tree desc)
{
  tree field;
  tree type;

  type = TREE_TYPE (desc);
  assert (GFC_DESCRIPTOR_TYPE_P (type));

  field = TYPE_FIELDS (type);
  assert (DATA_FIELD == 0);
  assert (field != NULL_TREE
	  && TREE_CODE (TREE_TYPE (field)) == POINTER_TYPE
	  && TREE_CODE (TREE_TYPE (TREE_TYPE (field))) == ARRAY_TYPE);

  return build (COMPONENT_REF, TREE_TYPE (field), desc, field);
}

tree
gfc_conv_descriptor_base (tree desc)
{
  tree field;
  tree type;

  type = TREE_TYPE (desc);
  assert (GFC_DESCRIPTOR_TYPE_P (type));

  field = gfc_advance_chain (TYPE_FIELDS (type), BASE_FIELD);
  assert (field != NULL_TREE
	  && TREE_CODE (TREE_TYPE (field)) == POINTER_TYPE
	  && TREE_CODE (TREE_TYPE (TREE_TYPE (field))) == ARRAY_TYPE);

  return build (COMPONENT_REF, TREE_TYPE (field), desc, field);
}

tree
gfc_conv_descriptor_dtype (tree desc)
{
  tree field;
  tree type;

  type = TREE_TYPE (desc);
  assert (GFC_DESCRIPTOR_TYPE_P (type));

  field = gfc_advance_chain (TYPE_FIELDS (type), DTYPE_FIELD);
  assert (field != NULL_TREE && TREE_TYPE (field) == gfc_array_index_type);

  return build (COMPONENT_REF, TREE_TYPE (field), desc, field);
}

static tree
gfc_conv_descriptor_dimension (tree desc, tree dim)
{
  tree field;
  tree type;
  tree tmp;

  type = TREE_TYPE (desc);
  assert (GFC_DESCRIPTOR_TYPE_P (type));

  field = gfc_advance_chain (TYPE_FIELDS (type), DIMENSION_FIELD);
  assert (field != NULL_TREE
	  && TREE_CODE (TREE_TYPE (field)) == ARRAY_TYPE
	  && TREE_CODE (TREE_TYPE (TREE_TYPE (field))) == RECORD_TYPE);

  tmp = build (COMPONENT_REF, TREE_TYPE (field), desc, field);
  tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, dim);
  return tmp;
}

tree
gfc_conv_descriptor_stride (tree desc, tree dim)
{
  tree tmp;
  tree field;

  tmp = gfc_conv_descriptor_dimension (desc, dim);
  field = TYPE_FIELDS (TREE_TYPE (tmp));
  field = gfc_advance_chain (field, STRIDE_SUBFIELD);
  assert (field != NULL_TREE && TREE_TYPE (field) == gfc_array_index_type);

  tmp = build (COMPONENT_REF, TREE_TYPE (field), tmp, field);
  return tmp;
}

tree
gfc_conv_descriptor_lbound (tree desc, tree dim)
{
  tree tmp;
  tree field;

  tmp = gfc_conv_descriptor_dimension (desc, dim);
  field = TYPE_FIELDS (TREE_TYPE (tmp));
  field = gfc_advance_chain (field, LBOUND_SUBFIELD);
  assert (field != NULL_TREE && TREE_TYPE (field) == gfc_array_index_type);

  tmp = build (COMPONENT_REF, TREE_TYPE (field), tmp, field);
  return tmp;
}

tree
gfc_conv_descriptor_ubound (tree desc, tree dim)
{
  tree tmp;
  tree field;

  tmp = gfc_conv_descriptor_dimension (desc, dim);
  field = TYPE_FIELDS (TREE_TYPE (tmp));
  field = gfc_advance_chain (field, UBOUND_SUBFIELD);
  assert (field != NULL_TREE && TREE_TYPE (field) == gfc_array_index_type);

  tmp = build (COMPONENT_REF, TREE_TYPE (field), tmp, field);
  return tmp;
}


/* Generate an initializer for a static pointer or allocatable array.  */

void
gfc_trans_static_array_pointer (gfc_symbol * sym)
{
  tree tmp;
  tree field;
  tree type;

  assert (TREE_STATIC (sym->backend_decl));
  /* Just zero the data member.  */
  type = TREE_TYPE (sym->backend_decl);
  assert (GFC_DESCRIPTOR_TYPE_P (type));
  assert (DATA_FIELD == 0);
  field = TYPE_FIELDS (type);

  tmp = tree_cons (field, null_pointer_node, NULL_TREE);
  tmp = build1 (CONSTRUCTOR, type, tmp);
  TREE_CONSTANT (tmp) = 1;
  DECL_INITIAL (sym->backend_decl) = tmp;
}


/* Cleanup those #defines.  */

#undef DATA_FIELD
#undef BASE_FIELD
#undef DTYPE_FIELD
#undef DIMENSION_FIELD
#undef STRIDE_SUBFIELD
#undef LBOUND_SUBFIELD
#undef UBOUND_SUBFIELD


/* Mark a SS chain as used.  Flags specifies in which loops the SS is used.
   flags & 1 = Main loop body.
   flags & 2 = temp copy loop.  */

void
gfc_mark_ss_chain_used (gfc_ss * ss, unsigned flags)
{
  for (; ss != gfc_ss_terminator; ss = ss->next)
    ss->useflags = flags;
}

static void gfc_free_ss (gfc_ss *);


/* Free a gfc_ss chain.  */

static void
gfc_free_ss_chain (gfc_ss * ss)
{
  gfc_ss *next;

  while (ss != gfc_ss_terminator)
    {
      assert (ss != NULL);
      next = ss->next;
      gfc_free_ss (ss);
      ss = next;
    }
}


/* Free a SS.  */

static void
gfc_free_ss (gfc_ss * ss)
{
  int n;

  switch (ss->type)
    {
    case GFC_SS_SECTION:
    case GFC_SS_VECTOR:
      for (n = 0; n < GFC_MAX_DIMENSIONS; n++)
	{
	  if (ss->data.info.subscript[n])
	    gfc_free_ss_chain (ss->data.info.subscript[n]);
	}
      break;

    default:
      break;
    }

  gfc_free (ss);
}


/* Free all the SS associated with a loop.  */

void
gfc_cleanup_loop (gfc_loopinfo * loop)
{
  gfc_ss *ss;
  gfc_ss *next;

  ss = loop->ss;
  while (ss != gfc_ss_terminator)
    {
      assert (ss != NULL);
      next = ss->loop_chain;
      gfc_free_ss (ss);
      ss = next;
    }
}


/* Associate a SS chain with a loop.  */

void
gfc_add_ss_to_loop (gfc_loopinfo * loop, gfc_ss * head)
{
  gfc_ss *ss;

  if (head == gfc_ss_terminator)
    return;

  ss = head;
  for (; ss && ss != gfc_ss_terminator; ss = ss->next)
    {
      if (ss->next == gfc_ss_terminator)
	ss->loop_chain = loop->ss;
      else
	ss->loop_chain = ss->next;
    }
  assert (ss == gfc_ss_terminator);
  loop->ss = head;
}


/* Generate code to allocate an array temporary, or create a variable to
   hold the data.  */

static void
gfc_trans_allocate_array_storage (gfc_loopinfo * loop, gfc_ss_info * info,
				  tree size, tree nelem)
{
  tree tmp;
  tree args;
  tree desc;
  tree data;

  desc = info->descriptor;
  data = gfc_conv_descriptor_data (desc);
  if (gfc_can_put_var_on_stack (size))
    {
      /* Make a temporary variable to hold the data.  */
      tmp = fold (build (MINUS_EXPR, TREE_TYPE (nelem), nelem,
			 integer_one_node));
      tmp = build_range_type (gfc_array_index_type, integer_zero_node, tmp);
      tmp = build_array_type (gfc_get_element_type (TREE_TYPE (desc)), tmp);
      tmp = gfc_create_var (tmp, "A");
      TREE_ADDRESSABLE (tmp) = 1;
      tmp = build1 (ADDR_EXPR, TREE_TYPE (data), tmp);
      info->data = gfc_evaluate_now (tmp, &loop->pre);

      gfc_add_modify_expr (&loop->pre, data, info->data);
      info->pdata = NULL_TREE;
    }
  else
    {
      /* Allocate memory to hold the data.  */
      info->pdata = build1 (ADDR_EXPR, ppvoid_type_node, data);

      /* Build a call to allocate storage.  */
      args = gfc_chainon_list (NULL_TREE, info->pdata);
      args = gfc_chainon_list (args, size);

      if (gfc_index_integer_kind == 4)
	tmp = gfor_fndecl_internal_malloc;
      else if (gfc_index_integer_kind == 8)
	tmp = gfor_fndecl_internal_malloc64;
      else
	abort ();
      tmp = gfc_build_function_call (tmp, args);
      gfc_add_expr_to_block (&loop->pre, tmp);

      info->data = gfc_evaluate_now (data, &loop->pre);
    }

  /* The offset is zero because we create temporaries with a zero
     lower bound.  */
  tmp = gfc_conv_descriptor_base (desc);
  gfc_add_modify_expr (&loop->pre, tmp, info->data);

  if (info->pdata != NULL_TREE)
    {
      /* Free the temporary.  */
      tmp = gfc_chainon_list (NULL_TREE, info->pdata);
      tmp = gfc_build_function_call (gfor_fndecl_internal_free, tmp);
      gfc_add_expr_to_block (&loop->post, tmp);
    }
}


/* Generate code to allocate and initialize the descriptor for a temporary
   array.  Fills in info->descriptor and info->data.  Also adjusts the loop
   variables to be zero-based.  Returns the size of the array.  */

tree
gfc_trans_allocate_temp_array (gfc_loopinfo * loop, gfc_ss_info * info,
			       tree eltype, tree string_length)
{
  tree type;
  tree desc;
  tree tmp;
  tree size;
  tree nelem;
  int n;
  int dim;

  assert (info->dimen > 0);
  /* Set the lower bound to zero.  */
  for (dim = 0; dim < info->dimen; dim++)
    {
      n = loop->order[dim];
      if (n < loop->temp_dim)
	assert (integer_zerop (loop->from[n]));
      else
	{
	  loop->to[n] = fold (build (MINUS_EXPR, gfc_array_index_type,
				     loop->to[n], loop->from[n]));
	  loop->from[n] = integer_zero_node;
	}

      info->delta[dim] = integer_zero_node;
      info->start[dim] = integer_zero_node;
      info->stride[dim] = integer_one_node;
      info->dim[dim] = dim;
    }

  /* Initialise the descriptor.  */
  type =
    gfc_get_array_type_bounds (eltype, info->dimen, loop->from, loop->to, 1);
  desc = gfc_create_var (type, "atmp");
  GFC_DECL_PACKED_ARRAY (desc) = 1;
  if (string_length)
    {
      gfc_allocate_lang_decl (desc);
      GFC_DECL_STRING (desc) = 1;
      GFC_DECL_STRING_LENGTH (desc) = string_length;
    }

  info->descriptor = desc;
  size = integer_one_node;

  /* Fill in the array dtype.  */
  tmp = gfc_conv_descriptor_dtype (desc);
  gfc_add_modify_expr (&loop->pre, tmp,
		       GFC_TYPE_ARRAY_DTYPE (TREE_TYPE (desc)));

  /* Fill in the bounds and stride.  This is a packed array, so:
     size = 1;
     for (n = 0; n < rank; n++)
     {
     stride[n] = size
     delta = ubound[n] + 1 - lbound[n];
     size = size * delta;
     }
     size = size * sizeof(element);  */
  for (n = 0; n < info->dimen; n++)
    {
      /* Store the stride and bound components in the descriptor.  */
      tmp = gfc_conv_descriptor_stride (desc, gfc_rank_cst[n]);
      gfc_add_modify_expr (&loop->pre, tmp, size);

      tmp = gfc_conv_descriptor_lbound (desc, gfc_rank_cst[n]);
      gfc_add_modify_expr (&loop->pre, tmp, integer_zero_node);

      tmp = gfc_conv_descriptor_ubound (desc, gfc_rank_cst[n]);
      gfc_add_modify_expr (&loop->pre, tmp, loop->to[n]);

      tmp = fold (build (PLUS_EXPR, gfc_array_index_type,
			 loop->to[n], integer_one_node));

      size = fold (build (MULT_EXPR, gfc_array_index_type, size, tmp));
      size = gfc_evaluate_now (size, &loop->pre);
    }

  if (string_length)
    gfc_todo_error ("Arrays of strings");

  /* Get the size of the array.  */
  nelem = size;
  size = fold (build (MULT_EXPR, gfc_array_index_type, size,
		      TYPE_SIZE_UNIT (gfc_get_element_type (type))));

  gfc_trans_allocate_array_storage (loop, info, size, nelem);

  if (info->dimen > loop->temp_dim)
    loop->temp_dim = info->dimen;

  return size;
}


/* Make sure offset is a variable.  */

static void
gfc_put_offset_into_var (stmtblock_t * pblock, tree * poffset,
			 tree * offsetvar)
{
  /* We should have already created the offset variable.  We cannot
     create it here because we may be in an inner scopde.  */
  assert (*offsetvar != NULL_TREE);
  gfc_add_modify_expr (pblock, *offsetvar, *poffset);
  *poffset = *offsetvar;
  TREE_USED (*offsetvar) = 1;
}


/* Add the contents of an array to the constructor.  */

static void
gfc_trans_array_constructor_subarray (stmtblock_t * pblock, tree type,
				      tree pointer, gfc_expr * expr,
				      tree * poffset, tree * offsetvar)
{
  gfc_se se;
  gfc_ss *ss;
  gfc_loopinfo loop;
  stmtblock_t body;
  tree tmp;

  /* We need this to be a variable so we can increment it.  */
  gfc_put_offset_into_var (pblock, poffset, offsetvar);

  gfc_init_se (&se, NULL);

  /* Walk the array expression.  */
  ss = gfc_walk_expr (expr);
  assert (ss != gfc_ss_terminator);

  /* Initialize the scalarizer.  */
  gfc_init_loopinfo (&loop);
  gfc_add_ss_to_loop (&loop, ss);

  /* Initialize the loop.  */
  gfc_conv_ss_startstride (&loop);
  gfc_conv_loop_setup (&loop);

  /* Make the loop body.  */
  gfc_mark_ss_chain_used (ss, 1);
  gfc_start_scalarized_body (&loop, &body);
  gfc_copy_loopinfo_to_se (&se, &loop);
  se.ss = ss;

  gfc_conv_expr (&se, expr);
  gfc_add_block_to_block (&body, &se.pre);

  /* Store the value.  */
  tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (pointer)), pointer);
  tmp = build (ARRAY_REF, type, tmp, *poffset);
  gfc_add_modify_expr (&body, tmp, se.expr);

  /* Increment the offset.  */
  tmp = build (PLUS_EXPR, gfc_array_index_type, *poffset, integer_one_node);
  gfc_add_modify_expr (&body, *poffset, tmp);

  /* Finish the loop.  */
  gfc_add_block_to_block (&body, &se.post);
  assert (se.ss == gfc_ss_terminator);
  gfc_trans_scalarizing_loops (&loop, &body);
  gfc_add_block_to_block (&loop.pre, &loop.post);
  tmp = gfc_finish_block (&loop.pre);
  gfc_add_expr_to_block (pblock, tmp);

  gfc_cleanup_loop (&loop);
}


/* Assign the values to the elements of an array constructor.  */

static void
gfc_trans_array_constructor_value (stmtblock_t * pblock, tree type,
				   tree pointer, gfc_constructor * c,
				   tree * poffset, tree * offsetvar)
{
  tree tmp;
  tree ref;
  stmtblock_t body;
  tree loopbody;
  gfc_se se;

  for (; c; c = c->next)
    {
      /* If this is an iterator or an array, the offset must be a variable.  */
      if ((c->iterator || c->expr->rank > 0) && INTEGER_CST_P (*poffset))
	gfc_put_offset_into_var (pblock, poffset, offsetvar);

      gfc_start_block (&body);

      if (c->expr->expr_type == EXPR_ARRAY)
	{
	  /* Array constructors can be nested.  */
	  gfc_trans_array_constructor_value (&body, type, pointer,
					     c->expr->value.constructor,
					     poffset, offsetvar);
	}
      else if (c->expr->rank > 0)
	{
	  gfc_trans_array_constructor_subarray (&body, type, pointer,
						c->expr, poffset, offsetvar);
	}
      else
	{
	  /* This code really upsets the gimplifier so don't bother for now.  */
	  gfc_constructor *p;
	  HOST_WIDE_INT n;
	  HOST_WIDE_INT size;

	  p = c;
	  n = 0;
	  while (p && !(p->iterator || p->expr->expr_type != EXPR_CONSTANT))
	    {
	      p = p->next;
	      n++;
	    }
	  if (n < 4)
	    {
	      /* Scalar values.  */
	      gfc_init_se (&se, NULL);
	      gfc_conv_expr (&se, c->expr);
	      gfc_add_block_to_block (&body, &se.pre);

	      ref = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (pointer)),
			    pointer);
	      ref = build (ARRAY_REF, type, ref, *poffset);
	      gfc_add_modify_expr (&body, ref, se.expr);
	      gfc_add_block_to_block (&body, &se.post);

	      *poffset = fold (build (PLUS_EXPR, gfc_array_index_type,
				      *poffset, integer_one_node));
	    }
	  else
	    {
	      /* Collect multiple scalar constants into a constructor.  */
	      tree list;
	      tree init;
	      tree bound;
	      tree tmptype;

	      p = c;
	      list = NULL_TREE;
              /* Count the number of consecutive scalar constants.  */
	      while (p && !(p->iterator
			    || p->expr->expr_type != EXPR_CONSTANT))
		{
		  gfc_init_se (&se, NULL);
		  gfc_conv_constant (&se, p->expr);
		  list = tree_cons (NULL_TREE, se.expr, list);
		  c = p;
		  p = p->next;
		}

	      bound = build_int_2 (n - 1, 0);
              /* Create an array type to hold them.  */
	      tmptype = build_range_type (gfc_array_index_type,
					  integer_zero_node, bound);
	      tmptype = build_array_type (type, tmptype);

	      init = build1 (CONSTRUCTOR, tmptype, nreverse (list));
	      TREE_CONSTANT (init) = 1;
	      TREE_STATIC (init) = 1;
	      /* Create a static variable to hold the data.  */
	      tmp = gfc_create_var (tmptype, "data");
	      TREE_STATIC (tmp) = 1;
	      TREE_CONSTANT (tmp) = 1;
	      TREE_ADDRESSABLE (tmp) = 1;
	      DECL_INITIAL (tmp) = init;
	      init = tmp;

	      /* Use BUILTIN_MEMCPY to assign the values.  */
	      if (TREE_CODE (pointer) == ADDR_EXPR)
		tmp = TREE_OPERAND (pointer, 0);
	      else
	        tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (pointer)),
			      pointer);
	      tmp = build (ARRAY_REF, type, tmp, *poffset);
	      tmp = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (tmp)),
			    tmp);
	      init = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (init)),
			     init);

	      size = TREE_INT_CST_LOW (TYPE_SIZE_UNIT (type));
	      bound = build_int_2 (n * size, 0);
	      tmp = gfc_chainon_list (NULL_TREE, tmp);
	      tmp = gfc_chainon_list (tmp, init);
	      tmp = gfc_chainon_list (tmp, bound);
	      tmp = gfc_build_function_call (built_in_decls[BUILT_IN_MEMCPY],
					     tmp);
	      gfc_add_expr_to_block (&body, tmp);

	      *poffset = fold (build (PLUS_EXPR, gfc_array_index_type,
				      *poffset, bound));
	    }
	  if (!INTEGER_CST_P (*poffset))
            {
              gfc_add_modify_expr (&body, *offsetvar, *poffset);
              *poffset = *offsetvar;
            }
	}

      /* The frontend should already have done any expansions.  */
      if (c->iterator)
	{
	  tree end;
	  tree step;
	  tree loopvar;
	  tree exit_label;

	  loopbody = gfc_finish_block (&body);

	  gfc_init_se (&se, NULL);
	  gfc_conv_expr (&se, c->iterator->var);
	  gfc_add_block_to_block (pblock, &se.pre);
	  loopvar = se.expr;

	  /* Initialize thie loop.  */
	  gfc_init_se (&se, NULL);
	  gfc_conv_expr_val (&se, c->iterator->start);
	  gfc_add_block_to_block (pblock, &se.pre);
	  gfc_add_modify_expr (pblock, loopvar, se.expr);

	  gfc_init_se (&se, NULL);
	  gfc_conv_expr_val (&se, c->iterator->end);
	  gfc_add_block_to_block (pblock, &se.pre);
	  end = gfc_evaluate_now (se.expr, pblock);

	  gfc_init_se (&se, NULL);
	  gfc_conv_expr_val (&se, c->iterator->step);
	  gfc_add_block_to_block (pblock, &se.pre);
	  step = gfc_evaluate_now (se.expr, pblock);

	  /* Generate the loop body.  */
	  exit_label = gfc_build_label_decl (NULL_TREE);
	  gfc_start_block (&body);

	  /* Generate the exit condition.  */
	  end = build (GT_EXPR, boolean_type_node, loopvar, end);
	  tmp = build1_v (GOTO_EXPR, exit_label);
	  TREE_USED (exit_label) = 1;
	  tmp = build_v (COND_EXPR, end, tmp, build_empty_stmt ());
	  gfc_add_expr_to_block (&body, tmp);

	  /* The main loop body.  */
	  gfc_add_expr_to_block (&body, loopbody);

	  /* Increment the loop variable.  */
	  tmp = build (PLUS_EXPR, TREE_TYPE (loopvar), loopvar, step);
	  gfc_add_modify_expr (&body, loopvar, tmp);

	  /* Finish the loop.  */
	  tmp = gfc_finish_block (&body);
	  tmp = build_v (LOOP_EXPR, tmp);
	  gfc_add_expr_to_block (pblock, tmp);

	  /* Add the exit label.  */
	  tmp = build1_v (LABEL_EXPR, exit_label);
	  gfc_add_expr_to_block (pblock, tmp);
	}
      else
	{
	  /* Pass the code as is.  */
	  tmp = gfc_finish_block (&body);
	  gfc_add_expr_to_block (pblock, tmp);
	}
    }
}


/* Get the size of an expression.  Returns -1 if the size isn't constant.
   Implied do loops with non-constant bounds are tricky because we must only
   evaluate the bounds once.  */

static void
gfc_get_array_cons_size (mpz_t * size, gfc_constructor * c)
{
  gfc_iterator *i;
  mpz_t val;
  mpz_t len;

  mpz_set_ui (*size, 0);
  mpz_init (len);
  mpz_init (val);

  for (; c; c = c->next)
    {
      if (c->expr->expr_type == EXPR_ARRAY)
	{
          /* A nested array constructor.  */
	  gfc_get_array_cons_size (&len, c->expr->value.constructor);
	  if (mpz_sgn (len) < 0)
	    {
	      mpz_set (*size, len);
	      mpz_clear (len);
	      mpz_clear (val);
	      return;
	    }
	}
      else
	{
	  if (c->expr->rank > 0)
	    {
	      mpz_set_si (*size, -1);
	      mpz_clear (len);
	      mpz_clear (val);
	      return;
	    }
	  mpz_set_ui (len, 1);
	}

      if (c->iterator)
	{
	  i = c->iterator;

	  if (i->start->expr_type != EXPR_CONSTANT
	      || i->end->expr_type != EXPR_CONSTANT
	      || i->step->expr_type != EXPR_CONSTANT)
	    {
	      mpz_set_si (*size, -1);
	      mpz_clear (len);
	      mpz_clear (val);
	      return;
	    }

	  mpz_add (val, i->end->value.integer, i->start->value.integer);
	  mpz_tdiv_q (val, val, i->step->value.integer);
	  mpz_add_ui (val, val, 1);
	  mpz_mul (len, len, val);
	}
      mpz_add (*size, *size, len);
    }
  mpz_clear (len);
  mpz_clear (val);
}


/* Array constructors are handled by constructing a temporary, then using that
   within the scalarization loop.  This is not optimal, but seems by far the
   simplest method.  */

static void
gfc_trans_array_constructor (gfc_loopinfo * loop, gfc_ss * ss)
{
  tree offset;
  tree offsetvar;
  tree desc;
  tree size;
  tree type;

  if (ss->expr->ts.type == BT_CHARACTER)
    gfc_todo_error ("Character string array constructors");
  type = gfc_typenode_for_spec (&ss->expr->ts);
  ss->data.info.dimen = loop->dimen;
  size =
    gfc_trans_allocate_temp_array (loop, &ss->data.info, type, NULL_TREE);

  desc = ss->data.info.descriptor;
  offset = integer_zero_node;
  offsetvar = gfc_create_var_np (gfc_array_index_type, "offset");
  TREE_USED (offsetvar) = 0;
  gfc_trans_array_constructor_value (&loop->pre, type,
				     ss->data.info.data,
				     ss->expr->value.constructor, &offset,
				     &offsetvar);

  if (TREE_USED (offsetvar))
    pushdecl (offsetvar);
  else
    assert (INTEGER_CST_P (offset));
#if 0
  /* Disable bound checking for now cos it's probably broken.  */
  if (flag_bounds_check)
    {
      abort ();
    }
#endif
}


/* Add the pre and post chains for all the scalar expressions in a SS chain
   to loop.  This is called after the loop parameters have been calculated,
   but before the actual scalarizing loops.  */
/*GCC ARRAYS*/

static void
gfc_add_loop_ss_code (gfc_loopinfo * loop, gfc_ss * ss, bool subscript)
{
  gfc_se se;
  int n;

  assert (ss != NULL);

  for (; ss != gfc_ss_terminator; ss = ss->loop_chain)
    {
      assert (ss);

      switch (ss->type)
	{
	case GFC_SS_SCALAR:
	  /* Scalar expression.  Evaluate this now.  This includes elemental
	     dimension indices, but not array section bounds.  */
	  gfc_init_se (&se, NULL);
          gfc_conv_expr (&se, ss->expr);
          gfc_add_block_to_block (&loop->pre, &se.pre);

          if (ss->expr->ts.type != BT_CHARACTER)
            {
              /* Move the evaluation of scalar expressions outside the
                 scalarization loop.  */
              if (subscript)
                se.expr = convert(gfc_array_index_type, se.expr);
              se.expr = gfc_evaluate_now (se.expr, &loop->pre);
              gfc_add_block_to_block (&loop->pre, &se.post);
            }
          else
            gfc_add_block_to_block (&loop->post, &se.post);

	  ss->data.scalar.expr = se.expr;
	  ss->data.scalar.string_length = se.string_length;
	  break;

	case GFC_SS_REFERENCE:
	  /* Scalar reference.  Evaluate this now.  */
	  gfc_init_se (&se, NULL);
	  gfc_conv_expr_reference (&se, ss->expr);
	  gfc_add_block_to_block (&loop->pre, &se.pre);
	  gfc_add_block_to_block (&loop->post, &se.post);

	  ss->data.scalar.expr = gfc_evaluate_now (se.expr, &loop->pre);
	  ss->data.scalar.string_length = se.string_length;
	  break;

	case GFC_SS_SECTION:
	case GFC_SS_VECTOR:
	  /* Scalarized expression.  Evaluate any scalar subscripts.  */
	  for (n = 0; n < GFC_MAX_DIMENSIONS; n++)
	    {
	      /* Add the expressions for scalar subscripts.  */
	      if (ss->data.info.subscript[n])
		gfc_add_loop_ss_code (loop, ss->data.info.subscript[n], true);
	    }
	  break;

	case GFC_SS_INTRINSIC:
	  gfc_add_intrinsic_ss_code (loop, ss);
	  break;

	case GFC_SS_FUNCTION:
	  /* Array function return value.  We call the function and save its
	     result in a temporary for use inside the loop.  */
	  gfc_init_se (&se, NULL);
	  se.loop = loop;
	  se.ss = ss;
	  gfc_conv_expr (&se, ss->expr);
	  gfc_add_block_to_block (&loop->pre, &se.pre);
	  gfc_add_block_to_block (&loop->post, &se.post);
	  break;

	case GFC_SS_CONSTRUCTOR:
	  gfc_trans_array_constructor (loop, ss);
	  break;

	default:
	  abort ();
	}
    }
}


/* Translate expressions for the descriptor and data pointer of a SS.  */
/*GCC ARRAYS*/

static void
gfc_conv_ss_descriptor (stmtblock_t * block, gfc_ss * ss, int base)
{
  gfc_se se;

  /* Get the descriptor for the array to be scalarized.  */
  assert (ss->expr->expr_type == EXPR_VARIABLE);
  gfc_init_se (&se, NULL);
  se.descriptor_only = 1;
  gfc_conv_expr_lhs (&se, ss->expr);
  gfc_add_block_to_block (block, &se.pre);
  ss->data.info.descriptor = se.expr;

  if (base)
    {
      /* Also the data pointer.  */
      se.expr = gfc_conv_array_base (se.expr);
      ss->data.info.data = gfc_evaluate_now (se.expr, block);
    }
}


/* Initialise a gfc_loopinfo structure.  */

void
gfc_init_loopinfo (gfc_loopinfo * loop)
{
  int n;

  memset (loop, 0, sizeof (gfc_loopinfo));
  gfc_init_block (&loop->pre);
  gfc_init_block (&loop->post);

  /* Initialy scalarize in order.  */
  for (n = 0; n < GFC_MAX_DIMENSIONS; n++)
    loop->order[n] = n;

  loop->ss = gfc_ss_terminator;
}


/* Copies the loop variable info to a gfc_se sructure. Does not copy the SS
   chain.  */

void
gfc_copy_loopinfo_to_se (gfc_se * se, gfc_loopinfo * loop)
{
  se->loop = loop;
}


/* Return an expression for the data pointer of an array.  */

tree
gfc_conv_array_data (tree descriptor)
{
  tree type;

  type = TREE_TYPE (descriptor);
  if (GFC_ARRAY_TYPE_P (type))
    {
      if (TREE_CODE (type) == POINTER_TYPE)
        return descriptor;
      else
        {
          /* Descritporless arrays.  */
          TREE_ADDRESSABLE (descriptor) = 1;
          return build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (descriptor)),
                         descriptor);
        }
    }
  else
    return gfc_conv_descriptor_data (descriptor);
}


/* Return an expression for the base pointer of an array.  */

tree
gfc_conv_array_base (tree descriptor)
{
  tree type;
  tree base;

  type = TREE_TYPE (descriptor);
  if (GFC_ARRAY_TYPE_P (type))
    {
      /* Descriptorless arrays.  */
      base = GFC_TYPE_ARRAY_OFFSET (type);
      if (INTEGER_CST_P (base))
        {
          if (POINTER_TYPE_P (type))
            {
              type = TREE_TYPE (type);
              descriptor = build1 (INDIRECT_REF, type, descriptor);
            }
          else
            TREE_ADDRESSABLE (descriptor) = 1;

          base = build (ARRAY_REF, gfc_get_element_type (type), descriptor,
                        base);
          base = build1 (ADDR_EXPR, gfc_array_dataptr_type (descriptor), base);
        }
      return base;
    }
  else
    return gfc_conv_descriptor_base (descriptor);
}


/* Get an expression for the array stride.  */

tree
gfc_conv_array_stride (tree descriptor, int dim)
{
  tree tmp;
  tree type;

  type = TREE_TYPE (descriptor);

  /* For descriptorless arrays use the array size.  */
  tmp = GFC_TYPE_ARRAY_STRIDE (type, dim);
  if (tmp != NULL_TREE)
    return tmp;

  tmp = gfc_conv_descriptor_stride (descriptor, gfc_rank_cst[dim]);
  return tmp;
}


/* Like gfc_conv_array_stride, but for the lower bound.  */

tree
gfc_conv_array_lbound (tree descriptor, int dim)
{
  tree tmp;
  tree type;

  type = TREE_TYPE (descriptor);

  tmp = GFC_TYPE_ARRAY_LBOUND (type, dim);
  if (tmp != NULL_TREE)
    return tmp;

  tmp = gfc_conv_descriptor_lbound (descriptor, gfc_rank_cst[dim]);
  return tmp;
}


/* Like gfc_conv_array_stride, but for the upper bound.  */

tree
gfc_conv_array_ubound (tree descriptor, int dim)
{
  tree tmp;
  tree type;

  type = TREE_TYPE (descriptor);

  tmp = GFC_TYPE_ARRAY_UBOUND (type, dim);
  if (tmp != NULL_TREE)
    return tmp;

  /* This should only ever happen when passing an assumed shape array
     as an actual parameter.  The value will never be used.  */
  if (GFC_ARRAY_TYPE_P (TREE_TYPE (descriptor)))
    return integer_zero_node;

  tmp = gfc_conv_descriptor_ubound (descriptor, gfc_rank_cst[dim]);
  return tmp;
}


/* Translate an array reference.  The descriptor should be in se->expr.
   Do not use this function, it wil be removed soon.  */
/*GCC ARRAYS*/

static void
gfc_conv_array_index_ref (gfc_se * se, tree pointer, tree * indices,
			  int dimen)
{
  tree array;
  tree tmp;
  tree index;
  int n;

  array = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (pointer)), pointer);

  index = integer_zero_node;
  for (n = 0; n < dimen; n++)
    {
      /* index = index + stride[n]*indices[n] */
      tmp = gfc_conv_array_stride (se->expr, n);
      tmp = fold (build (MULT_EXPR, gfc_array_index_type, indices[n], tmp));

      index = fold (build (PLUS_EXPR, gfc_array_index_type, index, tmp));
    }

  /* Result = data[index].  */
  assert (TREE_CODE (TREE_TYPE (array)) == ARRAY_TYPE);
  tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (array)), array, index);

  /* Check we've used the correct number of dimensions.  */
  assert (TREE_CODE (TREE_TYPE (tmp)) != ARRAY_TYPE);

  se->expr = tmp;
}


/* Generate code to perform an array index bound check.  */

static tree
gfc_trans_array_bound_check (gfc_se * se, tree descriptor, tree index, int n)
{
  tree cond;
  tree fault;
  tree tmp;

  if (!flag_bounds_check)
    return index;

  index = gfc_evaluate_now (index, &se->pre);
  /* Check lower bound.  */
  tmp = gfc_conv_array_lbound (descriptor, n);
  fault = fold (build (LT_EXPR, boolean_type_node, index, tmp));
  /* Check upper bound.  */
  tmp = gfc_conv_array_ubound (descriptor, n);
  cond = fold (build (GT_EXPR, boolean_type_node, index, tmp));
  fault = fold (build (TRUTH_OR_EXPR, boolean_type_node, fault, cond));

  gfc_trans_runtime_check (fault, gfc_strconst_fault, &se->pre);

  return index;
}


/* A reference to an array vector subscript.  Uses recursion to handle nested
   vector subscripts.  */

static tree
gfc_conv_vector_array_index (gfc_se * se, tree index, gfc_ss * ss)
{
  tree descsave;
  tree indices[GFC_MAX_DIMENSIONS];
  gfc_array_ref *ar;
  gfc_ss_info *info;
  int n;

  assert (ss && ss->type == GFC_SS_VECTOR);

  /* Save the descriptor.  */
  descsave = se->expr;
  info = &ss->data.info;
  se->expr = info->descriptor;

  ar = &info->ref->u.ar;
  for (n = 0; n < ar->dimen; n++)
    {
      switch (ar->dimen_type[n])
	{
	case DIMEN_ELEMENT:
	  assert (info->subscript[n] != gfc_ss_terminator
		  && info->subscript[n]->type == GFC_SS_SCALAR);
	  indices[n] = info->subscript[n]->data.scalar.expr;
	  break;

	case DIMEN_RANGE:
	  indices[n] = index;
	  break;

	case DIMEN_VECTOR:
	  index = gfc_conv_vector_array_index (se, index, info->subscript[n]);

	  indices[n] =
	    gfc_trans_array_bound_check (se, info->descriptor, index, n);
	  break;

	default:
	  abort ();
	}
    }
  /* Get the index from the vector.  */
  gfc_conv_array_index_ref (se, info->data, indices, ar->dimen);
  index = se->expr;
  /* Put the descriptor back.  */
  se->expr = descsave;

  return index;
}


/* Return the offset for an index.  Performs bound checking for elemental
   dimensions.  Single element references are processed seperately.  */

static tree
gfc_conv_array_index_offset (gfc_se * se, gfc_ss_info * info, int dim, int i,
			     gfc_array_ref * ar, tree stride)
{
  tree index;

  /* Get the index into the array for this dimension.  */
  if (ar)
    {
      assert (ar->type != AR_ELEMENT);
      if (ar->dimen_type[dim] == DIMEN_ELEMENT)
	{
	  assert (i == -1);
	  /* Elemental dimension.  */
	  assert (info->subscript[dim]
		  && info->subscript[dim]->type == GFC_SS_SCALAR);
	  /* We've already translated this value outside the loop.  */
	  index = info->subscript[dim]->data.scalar.expr;

	  index =
	    gfc_trans_array_bound_check (se, info->descriptor, index, dim);
	}
      else
	{
	  /* Scalarized dimension.  */
	  assert (info && se->loop);

          /* Multiply the loop variable by the stride and dela.  */
	  index = se->loop->loopvar[i];
	  index = fold (build (MULT_EXPR, gfc_array_index_type, index,
			       info->stride[i]));
	  index = fold (build (PLUS_EXPR, gfc_array_index_type, index,
			       info->delta[i]));

	  if (ar->dimen_type[dim] == DIMEN_VECTOR)
	    {
              /* Handle vector subscripts.  */
	      index = gfc_conv_vector_array_index (se, index,
						   info->subscript[dim]);
	      index =
		gfc_trans_array_bound_check (se, info->descriptor, index,
					     dim);
	    }
	  else
	    assert (ar->dimen_type[dim] == DIMEN_RANGE);
	}
    }
  else
    {
      /* Temporary array.  */
      assert (se->loop);
      index = se->loop->loopvar[se->loop->order[i]];
    }

  /* Multiply by the stride.  */
  index = fold (build (MULT_EXPR, gfc_array_index_type, index, stride));

  return index;
}


/* Build a scalarized reference to an array.  */

static void
gfc_conv_scalarized_array_ref (gfc_se * se, gfc_array_ref * ar)
{
  gfc_ss_info *info;
  tree index;
  tree tmp;
  int n;

  info = &se->ss->data.info;
  if (ar)
    n = se->loop->order[0];
  else
    n = 0;

  index = gfc_conv_array_index_offset (se, info, info->dim[n], n, ar,
				       info->stride0);
  /* Add the offset for this dimension to the stored offset for all other
     dimensions.  */
  index = fold (build (PLUS_EXPR, gfc_array_index_type, index, info->offset));

  tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (info->data)), info->data);
  se->expr = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, index);
}


/* Translate access of temporary array.  */

void
gfc_conv_tmp_array_ref (gfc_se * se)
{
  tree desc;

  desc = se->ss->data.info.descriptor;
  if (GFC_DECL_STRING (desc))
    se->string_length = GFC_DECL_STRING_LENGTH (desc);

  gfc_conv_scalarized_array_ref (se, NULL);
}


/* Build an array reference.  se->expr already holds the array descriptor.
   This should be either a variable, indirect variable reference or component
   reference.  For arrays which do not have a descriptor, se->expr will be
   the data pointer.
   a(i, j, k) = base[i * stride[0] + j * stride[1] + k * stride[2]]*/

void
gfc_conv_array_ref (gfc_se * se, gfc_array_ref * ar)
{
  int n;
  tree index;
  tree tmp;
  tree stride;
  tree fault;
  gfc_se indexse;

  /* Handle scalarized references seperately.  */
  if (ar->type != AR_ELEMENT)
    {
      gfc_conv_scalarized_array_ref (se, ar);
      return;
    }

  index = integer_zero_node;

  fault = integer_zero_node;

  /* Calculate the offsets from all the dimensions.  */
  for (n = 0; n < ar->dimen; n++)
    {
      /* Calculate the index for this demension.  */
      gfc_init_se (&indexse, NULL);
      gfc_conv_expr_type (&indexse, ar->start[n], gfc_array_index_type);
      gfc_add_block_to_block (&se->pre, &indexse.pre);

      if (flag_bounds_check)
	{
	  /* Check array bounds.  */
	  tree cond;

	  indexse.expr = gfc_evaluate_now (indexse.expr, &se->pre);

	  tmp = gfc_conv_array_lbound (se->expr, n);
	  cond = fold (build (LT_EXPR, boolean_type_node, indexse.expr, tmp));
	  fault =
	    fold (build (TRUTH_OR_EXPR, boolean_type_node, fault, cond));

	  tmp = gfc_conv_array_ubound (se->expr, n);
	  cond = fold (build (GT_EXPR, boolean_type_node, indexse.expr, tmp));
	  fault =
	    fold (build (TRUTH_OR_EXPR, boolean_type_node, fault, cond));
	}

      /* Multiply the index by the stride.  */
      stride = gfc_conv_array_stride (se->expr, n);
      tmp = fold (build (MULT_EXPR, gfc_array_index_type, indexse.expr,
			 stride));

      /* And add it to the total.  */
      index = fold (build (PLUS_EXPR, gfc_array_index_type, index, tmp));
    }

  if (flag_bounds_check)
    gfc_trans_runtime_check (fault, gfc_strconst_fault, &se->pre);

  /* Access the calculated element.  */
  tmp = gfc_conv_array_base (se->expr);
  tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp);
  se->expr = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, index);
}


/* Generate the code to be executed immediately before entering a
   scalarization loop.  */

static void
gfc_trans_preloop_setup (gfc_loopinfo * loop, int dim, int flag,
			 stmtblock_t * pblock)
{
  tree index;
  tree stride;
  gfc_ss_info *info;
  gfc_ss *ss;
  gfc_se se;
  int i;

  /* This code will be executed before entering the scalarization loop
     for this dimension.  */
  for (ss = loop->ss; ss != gfc_ss_terminator; ss = ss->loop_chain)
    {
      if ((ss->useflags & flag) == 0)
	continue;

      if (ss->type != GFC_SS_SECTION
	  && ss->type != GFC_SS_FUNCTION && ss->type != GFC_SS_CONSTRUCTOR)
	continue;

      info = &ss->data.info;

      if (dim >= info->dimen)
	continue;

      if (dim == info->dimen - 1)
	{
	  /* For the outermost loop calculate the offset due to any
	     elemental dimensions.  */
	  info->offset = integer_zero_node;
	  if (info->ref)
	    {
	      for (i = 0; i < info->ref->u.ar.dimen; i++)
		{
		  if (info->ref->u.ar.dimen_type[i] != DIMEN_ELEMENT)
		    continue;

		  gfc_init_se (&se, NULL);
		  se.loop = loop;
		  se.expr = info->descriptor;
		  stride = gfc_conv_array_stride (info->descriptor, i);
		  index = gfc_conv_array_index_offset (&se, info, i, -1,
						       &info->ref->u.ar,
						       stride);
		  gfc_add_block_to_block (pblock, &se.pre);

		  info->offset = fold (build (PLUS_EXPR, gfc_array_index_type,
					      info->offset, index));
		  info->offset = gfc_evaluate_now (info->offset, pblock);
		}

	      i = loop->order[0];
	      stride = gfc_conv_array_stride (info->descriptor, info->dim[i]);
	    }
	  else
	    stride = gfc_conv_array_stride (info->descriptor, 0);

	  /* Calculate the stride of the innermost loop.  Hopefully this will
             allow the backend optimizers to do their stuff more effectively.
           */
	  info->stride0 = gfc_evaluate_now (stride, pblock);
	}
      else
	{
	  /* Add the offset for the previous loop dimension.  */
	  gfc_array_ref *ar;

	  if (info->ref)
	    {
	      ar = &info->ref->u.ar;
	      i = loop->order[dim + 1];
	    }
	  else
	    {
	      ar = NULL;
	      i = dim + 1;
	    }

	  gfc_init_se (&se, NULL);
	  se.loop = loop;
	  se.expr = info->descriptor;
	  stride = gfc_conv_array_stride (info->descriptor, i);
	  index = gfc_conv_array_index_offset (&se, info, info->dim[i], i,
					       ar, stride);
	  gfc_add_block_to_block (pblock, &se.pre);
	  info->offset = fold (build (PLUS_EXPR, gfc_array_index_type,
				      info->offset, index));
	  info->offset = gfc_evaluate_now (info->offset, pblock);
	}

      if (dim == loop->temp_dim - 1)
        info->saved_offset = info->offset;
    }
}


/* Start a scalarized expression.  Creates a scope and declares loop
   variables.  */

void
gfc_start_scalarized_body (gfc_loopinfo * loop, stmtblock_t * pbody)
{
  int dim;
  int n;
  int flags;

  assert (!loop->array_parameter);

  for (dim = loop->dimen - 1; dim >= 0; dim--)
    {
      n = loop->order[dim];

      gfc_start_block (&loop->code[n]);

      /* Create the loop variable.  */
      loop->loopvar[n] = gfc_create_var (gfc_array_index_type, "S");

      if (dim < loop->temp_dim)
	flags = 3;
      else
	flags = 1;
      /* Calculate values that will be constant within this loop.  */
      gfc_trans_preloop_setup (loop, dim, flags, &loop->code[n]);
    }
  gfc_start_block (pbody);
}


/* Generates the actual loop code for a scalarization loop.  */

static void
gfc_trans_scalarized_loop_end (gfc_loopinfo * loop, int n,
			       stmtblock_t * pbody)
{
  stmtblock_t block;
  tree cond;
  tree tmp;
  tree loopbody;
  tree exit_label;

  loopbody = gfc_finish_block (pbody);

  /* Initialize the loopvar.  */
  gfc_add_modify_expr (&loop->code[n], loop->loopvar[n], loop->from[n]);

  exit_label = gfc_build_label_decl (NULL_TREE);

  /* Generate the loop body.  */
  gfc_init_block (&block);

  /* The exit condition.  */
  cond = build (GT_EXPR, boolean_type_node, loop->loopvar[n], loop->to[n]);
  tmp = build1_v (GOTO_EXPR, exit_label);
  TREE_USED (exit_label) = 1;
  tmp = build_v (COND_EXPR, cond, tmp, build_empty_stmt ());
  gfc_add_expr_to_block (&block, tmp);

  /* The main body.  */
  gfc_add_expr_to_block (&block, loopbody);

  /* Increment the loopvar.  */
  tmp = build (PLUS_EXPR, gfc_array_index_type,
	       loop->loopvar[n], integer_one_node);
  gfc_add_modify_expr (&block, loop->loopvar[n], tmp);

  /* Build the loop.  */
  tmp = gfc_finish_block (&block);
  tmp = build_v (LOOP_EXPR, tmp);
  gfc_add_expr_to_block (&loop->code[n], tmp);

  /* Add the exit label.  */
  tmp = build1_v (LABEL_EXPR, exit_label);
  gfc_add_expr_to_block (&loop->code[n], tmp);
}


/* Finishes and generates the loops for a scalarized expression.  */

void
gfc_trans_scalarizing_loops (gfc_loopinfo * loop, stmtblock_t * body)
{
  int dim;
  int n;
  gfc_ss *ss;
  stmtblock_t *pblock;
  tree tmp;

  pblock = body;
  /* Generate the loops.  */
  for (dim = 0; dim < loop->dimen; dim++)
    {
      n = loop->order[dim];
      gfc_trans_scalarized_loop_end (loop, n, pblock);
      loop->loopvar[n] = NULL_TREE;
      pblock = &loop->code[n];
    }

  tmp = gfc_finish_block (pblock);
  gfc_add_expr_to_block (&loop->pre, tmp);

  /* Clear all the used flags.  */
  for (ss = loop->ss; ss; ss = ss->loop_chain)
    ss->useflags = 0;
}


/* Finish the main body of a scalarized expression, and start the secondary
   copying body.  */

void
gfc_trans_scalarized_loop_boundary (gfc_loopinfo * loop, stmtblock_t * body)
{
  int dim;
  int n;
  stmtblock_t *pblock;
  gfc_ss *ss;

  pblock = body;
  /* We finish as many loops as are used by the temporary.  */
  for (dim = 0; dim < loop->temp_dim - 1; dim++)
    {
      n = loop->order[dim];
      gfc_trans_scalarized_loop_end (loop, n, pblock);
      loop->loopvar[n] = NULL_TREE;
      pblock = &loop->code[n];
    }

  /* We don't want to finish the outermost loop entirely.  */
  n = loop->order[loop->temp_dim - 1];
  gfc_trans_scalarized_loop_end (loop, n, pblock);

  /* Restore the initial offsets.  */
  for (ss = loop->ss; ss != gfc_ss_terminator; ss = ss->loop_chain)
    {
      if ((ss->useflags & 2) == 0)
	continue;

      if (ss->type != GFC_SS_SECTION
	  && ss->type != GFC_SS_FUNCTION && ss->type != GFC_SS_CONSTRUCTOR)
	continue;

      ss->data.info.offset = ss->data.info.saved_offset;
    }

  /* Restart all the inner loops we just finished.  */
  for (dim = loop->temp_dim - 2; dim >= 0; dim--)
    {
      n = loop->order[dim];

      gfc_start_block (&loop->code[n]);

      loop->loopvar[n] = gfc_create_var (gfc_array_index_type, "Q");

      gfc_trans_preloop_setup (loop, dim, 2, &loop->code[n]);
    }

  /* Start a block for the secondary copying code.  */
  gfc_start_block (body);
}


/* Calculate the upper bound of an array section.  */

static tree
gfc_conv_section_upper_bound (gfc_ss * ss, int n, stmtblock_t * pblock)
{
  int dim;
  gfc_ss *vecss;
  gfc_expr *end;
  tree desc;
  tree bound;
  gfc_se se;

  assert (ss->type == GFC_SS_SECTION);

  /* For vector array subscripts we want the size of the vector.  */
  dim = ss->data.info.dim[n];
  vecss = ss;
  while (vecss->data.info.ref->u.ar.dimen_type[dim] == DIMEN_VECTOR)
    {
      vecss = vecss->data.info.subscript[dim];
      assert (vecss && vecss->type == GFC_SS_VECTOR);
      dim = vecss->data.info.dim[0];
    }

  assert (vecss->data.info.ref->u.ar.dimen_type[dim] == DIMEN_RANGE);
  end = vecss->data.info.ref->u.ar.end[dim];
  desc = vecss->data.info.descriptor;

  if (end)
    {
      /* The upper bound was specified.  */
      gfc_init_se (&se, NULL);
      gfc_conv_expr_type (&se, end, gfc_array_index_type);
      gfc_add_block_to_block (pblock, &se.pre);
      bound = se.expr;
    }
  else
    {
      /* No upper bound was specified, so use the bound of the array. */
      bound = gfc_conv_array_ubound (desc, dim);
    }

  return bound;
}


/* Calculate the lower bound of an array section.  */

static void
gfc_conv_section_startstride (gfc_loopinfo * loop, gfc_ss * ss, int n)
{
  gfc_expr *start;
  gfc_expr *stride;
  gfc_ss *vecss;
  tree desc;
  gfc_se se;
  gfc_ss_info *info;
  int dim;

  info = &ss->data.info;

  dim = info->dim[n];

  /* For vector array subscripts we want the size of the vector.  */
  vecss = ss;
  while (vecss->data.info.ref->u.ar.dimen_type[dim] == DIMEN_VECTOR)
    {
      vecss = vecss->data.info.subscript[dim];
      assert (vecss && vecss->type == GFC_SS_VECTOR);
      /* Get the descriptors for the vector subscripts as well.  */
      if (!vecss->data.info.descriptor)
	gfc_conv_ss_descriptor (&loop->pre, vecss, !loop->array_parameter);
      dim = vecss->data.info.dim[0];
    }

  assert (vecss->data.info.ref->u.ar.dimen_type[dim] == DIMEN_RANGE);
  start = vecss->data.info.ref->u.ar.start[dim];
  stride = vecss->data.info.ref->u.ar.stride[dim];
  desc = vecss->data.info.descriptor;

  /* Calculate the start of the range.  For vector subscripts this will
     be the range of the vector.  */
  if (start)
    {
      /* Specified section start.  */
      gfc_init_se (&se, NULL);
      gfc_conv_expr_type (&se, start, gfc_array_index_type);
      gfc_add_block_to_block (&loop->pre, &se.pre);
      info->start[n] = se.expr;
    }
  else
    {
      /* No lower bound specified so use the bound of the array.  */
      info->start[n] = gfc_conv_array_lbound (desc, dim);
    }
  info->start[n] = gfc_evaluate_now (info->start[n], &loop->pre);

  /* Calculate the stride.  */
  if (stride == NULL)
    info->stride[n] = integer_one_node;
  else
    {
      gfc_init_se (&se, NULL);
      gfc_conv_expr_type (&se, stride, gfc_array_index_type);
      gfc_add_block_to_block (&loop->pre, &se.pre);
      info->stride[n] = gfc_evaluate_now (se.expr, &loop->pre);
    }
}


/* Calculates the range start and stride for a SS chain.  Also gets the
   descriptor and data pointer.  The range of vector subscripts is the size
   of the vector.  Array bounds are also checked.  */

void
gfc_conv_ss_startstride (gfc_loopinfo * loop)
{
  int n;
  tree tmp;
  gfc_ss *ss;
  gfc_ss *vecss;
  tree desc;

  loop->dimen = 0;
  /* Determine the rank of the loop.  */
  for (ss = loop->ss;
       ss != gfc_ss_terminator && loop->dimen == 0; ss = ss->loop_chain)
    {
      switch (ss->type)
	{
	case GFC_SS_SECTION:
	case GFC_SS_CONSTRUCTOR:
	case GFC_SS_FUNCTION:
	  loop->dimen = ss->data.info.dimen;
	  break;

	default:
	  break;
	}
    }

  if (loop->dimen == 0)
    gfc_todo_error ("Unable to determine rank of expression");


  /* loop over all the SS in the chain.  */
  for (ss = loop->ss; ss != gfc_ss_terminator; ss = ss->loop_chain)
    {
      switch (ss->type)
	{
	case GFC_SS_SECTION:
	  /* Get the descriptor for the array.  */
	  gfc_conv_ss_descriptor (&loop->pre, ss, !loop->array_parameter);

	  for (n = 0; n < ss->data.info.dimen; n++)
	    gfc_conv_section_startstride (loop, ss, n);
	  break;

	case GFC_SS_CONSTRUCTOR:
	case GFC_SS_FUNCTION:
	  for (n = 0; n < ss->data.info.dimen; n++)
	    {
	      ss->data.info.start[n] = integer_zero_node;
	      ss->data.info.stride[n] = integer_one_node;
	    }
	  break;

	default:
	  break;
	}
    }

  /* The rest is just runtime bound checking.  */
  if (flag_bounds_check)
    {
      stmtblock_t block;
      tree fault;
      tree bound;
      tree end;
      tree size[GFC_MAX_DIMENSIONS];
      gfc_ss_info *info;
      int dim;

      gfc_start_block (&block);

      fault = integer_zero_node;
      for (n = 0; n < loop->dimen; n++)
	size[n] = NULL_TREE;

      for (ss = loop->ss; ss != gfc_ss_terminator; ss = ss->loop_chain)
	{
	  if (ss->type != GFC_SS_SECTION)
	    continue;

	  /* TODO: range checking for mapped dimensions.  */
	  info = &ss->data.info;

	  /* This only checks scalarized dimensions, elemental dimensions are
	     checked later.  */
	  for (n = 0; n < loop->dimen; n++)
	    {
	      dim = info->dim[n];
	      vecss = ss;
	      while (vecss->data.info.ref->u.ar.dimen_type[dim]
		     == DIMEN_VECTOR)
		{
		  vecss = vecss->data.info.subscript[dim];
		  assert (vecss && vecss->type == GFC_SS_VECTOR);
		  dim = vecss->data.info.dim[0];
		}
	      assert (vecss->data.info.ref->u.ar.dimen_type[dim]
		      == DIMEN_RANGE);
	      desc = vecss->data.info.descriptor;

	      /* Check lower bound.  */
	      bound = gfc_conv_array_lbound (desc, dim);
	      tmp = info->start[n];
	      tmp = fold (build (LT_EXPR, boolean_type_node, tmp, bound));
	      fault = fold (build (TRUTH_OR_EXPR, boolean_type_node, fault,
				   tmp));

	      /* Check the upper bound.  */
	      bound = gfc_conv_array_ubound (desc, dim);
	      end = gfc_conv_section_upper_bound (ss, n, &block);
	      tmp = fold (build (GT_EXPR, boolean_type_node, end, bound));
	      fault = fold (build (TRUTH_OR_EXPR, boolean_type_node, fault,
				   tmp));

	      /* Check the section sizes match.  */
	      tmp = fold (build (MINUS_EXPR, gfc_array_index_type, end,
				 info->start[n]));
	      tmp = fold (build (FLOOR_DIV_EXPR, gfc_array_index_type, tmp,
				 info->stride[n]));
	      /* We remember the size of the first section, and check all the
	         others against this.  */
	      if (size[n])
		{
		  tmp =
		    fold (build (NE_EXPR, boolean_type_node, tmp, size[n]));
		  fault =
		    build (TRUTH_OR_EXPR, boolean_type_node, fault, tmp);
		}
	      else
		size[n] = gfc_evaluate_now (tmp, &block);
	    }
	}
      gfc_trans_runtime_check (fault, gfc_strconst_bounds, &block);

      tmp = gfc_finish_block (&block);
      gfc_add_expr_to_block (&loop->pre, tmp);
    }
}


/* Return true if the two SS could be aliased, ie. both point to the same data
   object.  */
/* TODO: resolve aliases based on frontend expressions.  */

static int
gfc_could_be_alias (gfc_ss * lss, gfc_ss * rss)
{
  gfc_ref *lref;
  gfc_ref *rref;
  gfc_symbol *lsym;
  gfc_symbol *rsym;

  lsym = lss->expr->symtree->n.sym;
  rsym = rss->expr->symtree->n.sym;
  if (gfc_symbols_could_alias (lsym, rsym))
    return 1;

  if (rsym->ts.type != BT_DERIVED
      && lsym->ts.type != BT_DERIVED)
    return 0;

  /* For Derived types we must check all the component types.  We can ignore
     array references as these will have the same base type as the previous
     component ref.  */
  for (lref = lss->expr->ref; lref != lss->data.info.ref; lref = lref->next)
    {
      if (lref->type != REF_COMPONENT)
	continue;

      if (gfc_symbols_could_alias (lref->u.c.sym, rsym))
	return 1;

      for (rref = rss->expr->ref; rref != rss->data.info.ref;
	   rref = rref->next)
	{
	  if (rref->type != REF_COMPONENT)
	    continue;

	  if (gfc_symbols_could_alias (lref->u.c.sym, rref->u.c.sym))
	    return 1;
	}
    }

  for (rref = rss->expr->ref; rref != rss->data.info.ref; rref = rref->next)
    {
      if (rref->type != REF_COMPONENT)
	break;

      if (gfc_symbols_could_alias (rref->u.c.sym, lsym))
	return 1;
    }

  return 0;
}


/* Resolve array data dependencies.  Creates a temporary if required.  */
/* TODO: Calc dependencies with gfc_expr rather than gfc_ss, and move to
   dependency.c.  */

void
gfc_conv_resolve_dependencies (gfc_loopinfo * loop, gfc_ss * dest,
			       gfc_ss * rss)
{
  gfc_ss *ss;
  gfc_ref *lref;
  gfc_ref *rref;
  gfc_ref *aref;
  int nDepend = 0;
  int temp_dim = 0;

  loop->temp_ss = NULL;
  aref = dest->data.info.ref;
  temp_dim = 0;

  for (ss = rss; ss != gfc_ss_terminator; ss = ss->next)
    {
      if (ss->type != GFC_SS_SECTION)
	continue;

      if (gfc_could_be_alias (dest, ss))
	{
	  nDepend = 1;
	  break;
	}

      if (dest->expr->symtree->n.sym == ss->expr->symtree->n.sym)
	{
	  lref = dest->expr->ref;
	  rref = ss->expr->ref;

	  nDepend = gfc_dep_resolver (lref, rref);
#if 0
	  /* TODO : loop shifting.  */
	  if (nDepend == 1)
	    {
	      /* Mark the dimensions for LOOP SHIFTING */
	      for (n = 0; n < loop->dimen; n++)
	        {
	          int dim = dest->data.info.dim[n];

		  if (lref->u.ar.dimen_type[dim] == DIMEN_VECTOR)
		    depends[n] = 2;
		  else if (! gfc_is_same_range (&lref->u.ar,
						&rref->u.ar, dim, 0))
		    depends[n] = 1;
	         }

	      /* Put all the dimensions with dependancies in the
		 innermost loops.  */
	      dim = 0;
	      for (n = 0; n < loop->dimen; n++)
		{
		  assert (loop->order[n] == n);
		  if (depends[n])
		  loop->order[dim++] = n;
		}
	      temp_dim = dim;
	      for (n = 0; n < loop->dimen; n++)
	        {
		  if (! depends[n])
		  loop->order[dim++] = n;
		}

	      assert (dim == loop->dimen);
	      break;
	    }
#endif
	}
    }

  if (nDepend == 1)
    {
      loop->temp_ss = gfc_get_ss ();
      loop->temp_ss->type = GFC_SS_TEMP;
      loop->temp_ss->data.temp.type =
	gfc_get_element_type (TREE_TYPE (dest->data.info.descriptor));
      loop->temp_ss->data.temp.string_length =
	gfc_conv_string_length (dest->data.info.descriptor);
      loop->temp_ss->data.temp.dimen = loop->dimen;
      loop->temp_ss->next = gfc_ss_terminator;
      gfc_add_ss_to_loop (loop, loop->temp_ss);
    }
  else
    loop->temp_ss = NULL;
}


/* Initialise the scalarization loop.  Creates the loop variables.  Determines
   the range of the loop variables.  Creates a temporary if required.
   Calculates how to transform from loop variables to array indices for each
   expression.  Also generates code for scalar expressions which have been
   moved outside the loop. */

void
gfc_conv_loop_setup (gfc_loopinfo * loop)
{
  int n;
  int dim;
  gfc_ss_info *info;
  gfc_ss_info *specinfo;
  gfc_ss *ss;
  tree tmp;
  tree len;
  gfc_ss *loopspec[GFC_MAX_DIMENSIONS];
  mpz_t *cshape;
  mpz_t i;

  mpz_init (i);
  for (n = 0; n < loop->dimen; n++)
    {
      loopspec[n] = NULL;
      /* We use one SS term, and use that to determine the bounds of the
         loop for this dimension.  We try to pick the simplest term.  */
      for (ss = loop->ss; ss != gfc_ss_terminator; ss = ss->loop_chain)
	{
	  if (ss->expr && ss->expr->shape)
	    {
	      /* The frontend has worked out the size for us.  */
	      loopspec[n] = ss;
	      continue;
	    }

	  if (ss->type == GFC_SS_CONSTRUCTOR)
	    {
	      /* Try to figure out the size of the constructior.  */
	      /* TODO: avoid this by making the prontend set the shape.  */
	      gfc_get_array_cons_size (&i, ss->expr->value.constructor);
	      /* A negative value meens we failed. */
	      if (mpz_sgn (i) > 0)
		{
		  mpz_sub_ui (i, i, 1);
		  loop->to[n] =
		    gfc_conv_mpz_to_tree (i, gfc_index_integer_kind);
		  loopspec[n] = ss;
		}
	      continue;
	    }

	  /* We don't know how to handle functions yet.
	     This may not be possible in all cases.  */
	  if (ss->type != GFC_SS_SECTION)
	    continue;

	  info = &ss->data.info;

	  if (loopspec[n])
	    specinfo = &loopspec[n]->data.info;
	  else
	    specinfo = NULL;
	  info = &ss->data.info;

	  /* Criteria for choosing a loop specifier (most important first):
	     stride of one
	     known stride
	     known lower bound
	     known upper bound
	   */
	  if (!specinfo)
	    loopspec[n] = ss;
	  else if (loopspec[n]->type != GFC_SS_CONSTRUCTOR)
	    {
	      if (integer_onep (info->stride[n])
		  && !integer_onep (specinfo->stride[n]))
		loopspec[n] = ss;
	      else if (INTEGER_CST_P (info->stride[n])
		       && !INTEGER_CST_P (specinfo->stride[n]))
		loopspec[n] = ss;
	      else if (INTEGER_CST_P (info->start[n])
		       && !INTEGER_CST_P (specinfo->start[n]))
		loopspec[n] = ss;
	      /* We don't work out the upper bound.
	         else if (INTEGER_CST_P (info->finish[n])
	         && ! INTEGER_CST_P (specinfo->finish[n]))
	         loopspec[n] = ss; */
	    }
	}

      if (!loopspec[n])
	gfc_todo_error ("Unable to find scalarization loop specifier");

      info = &loopspec[n]->data.info;

      /* Set the extents of this range.  */
      cshape = loopspec[n]->expr->shape;
      if (cshape && INTEGER_CST_P (info->start[n])
	  && INTEGER_CST_P (info->stride[n]))
	{
	  loop->from[n] = info->start[n];
	  mpz_set (i, cshape[n]);
	  mpz_sub_ui (i, i, 1);
	  /* To = from + (size - 1) * stride.  */
	  tmp = gfc_conv_mpz_to_tree (i, gfc_index_integer_kind);
	  if (!integer_onep (info->stride[n]))
	    {
	      tmp = fold (build (MULT_EXPR, gfc_array_index_type,
				 tmp, info->stride[n]));
	    }
	  loop->to[n] = fold (build (PLUS_EXPR, gfc_array_index_type,
			      loop->from[n], tmp));
	}
      else
	{
	  loop->from[n] = info->start[n];
	  switch (loopspec[n]->type)
	    {
	    case GFC_SS_CONSTRUCTOR:
	      assert (info->dimen == 1);
	      assert (loop->to[n]);
	      break;

	    case GFC_SS_SECTION:
	      loop->to[n] = gfc_conv_section_upper_bound (loopspec[n], n,
							  &loop->pre);
	      break;

	    default:
	      abort ();
	    }
	}

      /* Transform everything so we have a simple incrementing variable.  */
      if (integer_onep (info->stride[n]))
	info->delta[n] = integer_zero_node;
      else
	{
	  /* Set the delta for this section.  */
	  info->delta[n] = gfc_evaluate_now (loop->from[n], &loop->pre);
	  /* Number of iterations is (end - start + step) / step.
	     with start = 0, this simplifies to
	     last = end / step;
	     for (i = 0; i<=last; i++){...};  */
	  tmp = fold (build (MINUS_EXPR, gfc_array_index_type, loop->to[n],
			     loop->from[n]));
	  tmp = fold (build (TRUNC_DIV_EXPR, gfc_array_index_type, tmp,
			     info->stride[n]));
	  loop->to[n] = gfc_evaluate_now (tmp, &loop->pre);
	  /* Make the loop variable start at 0.  */
	  loop->from[n] = integer_zero_node;
	}
    }

  /* If we want a temporary then create it.  */
  if (loop->temp_ss != NULL)
    {
      assert (loop->temp_ss->type == GFC_SS_TEMP);
      tmp = loop->temp_ss->data.temp.type;
      len = loop->temp_ss->data.temp.string_length;
      n = loop->temp_ss->data.temp.dimen;
      memset (&loop->temp_ss->data.info, 0, sizeof (gfc_ss_info));
      loop->temp_ss->type = GFC_SS_SECTION;
      loop->temp_ss->data.info.dimen = n;
      gfc_trans_allocate_temp_array (loop, &loop->temp_ss->data.info,
				     tmp, len);
    }

  /* Add all the scalar code that can be taken out of the loops.  */
  gfc_add_loop_ss_code (loop, loop->ss, false);

  for (n = 0; n < loop->temp_dim; n++)
    loopspec[loop->order[n]] = NULL;

  mpz_clear (i);

  /* For array parameters we don't have loop variables, so don't calculate the
     translations.  */
  if (loop->array_parameter)
    return;

  /* Calculate the translation from loop variables to array indices.  */
  for (ss = loop->ss; ss != gfc_ss_terminator; ss = ss->loop_chain)
    {
      if (ss->type != GFC_SS_SECTION)
	continue;

      info = &ss->data.info;

      for (n = 0; n < info->dimen; n++)
	{
	  dim = info->dim[n];

	  /* If we are specifying the range the delta may already be set.  */
	  if (loopspec[n] != ss)
	    {
	      /* Calculate the offset relative to the loop variable.
	         First multiply by the stride.  */
	      tmp = fold (build (MULT_EXPR, gfc_array_index_type,
				 loop->from[n], info->stride[n]));

	      /* Then subtract this from our starting value.  */
	      tmp = fold (build (MINUS_EXPR, gfc_array_index_type,
				 info->start[n], tmp));

	      info->delta[n] = gfc_evaluate_now (tmp, &loop->pre);
	    }
	}
    }
}


/* Fills in an array descriptor, and returns the size of the array.  The size
   will be a simple_val, ie a variable or a constant.  Also calculates the
   offset of the base.  Returns the number of elements in the arrary.
   {
    stride = 1;
    offset = 0;
    for (n = 0; n < rank; n++)
      {
        a.lbound[n] = specified_lower_bound;
        offset = offset + a.lbond[n] * stride;
        size = 1 - lbound;
        a.ubound[n] = specified_upper_bound;
        a.stride[n] = stride;
        size = ubound + size; //size = ubound + 1 - lbound
        stride = stride * size;
      }
    return (stride);
   }  */
/*GCC ARRAYS*/

static tree
gfc_array_init_size (tree descriptor, int rank, tree * poffset,
		     gfc_expr ** lower, gfc_expr ** upper,
		     stmtblock_t * pblock, tree * pstring)
{
  tree type;
  tree tmp;
  tree size;
  tree offset;
  tree stride;
  tree string_len;
  gfc_expr *ubound;
  gfc_se se;
  int n;

  type = TREE_TYPE (descriptor);

  stride = integer_one_node;
  offset = integer_zero_node;

  /* Set the dtype.  */
  tmp = gfc_conv_descriptor_dtype (descriptor);
  gfc_add_modify_expr (pblock, tmp,
                       GFC_TYPE_ARRAY_DTYPE (TREE_TYPE (descriptor)));

  for (n = 0; n < rank; n++)
    {
      /* We have 3 possibilities for determining the size of the array:
         lower == NULL    => lbound = 1, ubound = upper[n]
         upper[n] = NULL  => lbound = 1, ubound = lower[n]
         upper[n] != NULL => lbound = lower[n], ubound = upper[n]  */
      ubound = upper[n];

      /* Set lower bound.  */
      gfc_init_se (&se, NULL);
      if (lower == NULL)
	se.expr = integer_one_node;
      else
	{
	  assert (lower[n]);
          if (ubound)
            {
	      gfc_conv_expr_type (&se, lower[n], gfc_array_index_type);
	      gfc_add_block_to_block (pblock, &se.pre);
            }
          else
            {
              se.expr = integer_one_node;
              ubound = lower[n];
            }
	}
      tmp = gfc_conv_descriptor_lbound (descriptor, gfc_rank_cst[n]);
      gfc_add_modify_expr (pblock, tmp, se.expr);

      /* Work out the offset for this component.  */
      tmp = fold (build (MULT_EXPR, gfc_array_index_type, se.expr, stride));
      offset = fold (build (MINUS_EXPR, gfc_array_index_type, offset, tmp));

      /* Start the calculation for the size of this dimension.  */
      size = build (MINUS_EXPR, gfc_array_index_type,
		    integer_one_node, se.expr);

      /* Set upper bound.  */
      gfc_init_se (&se, NULL);
      assert (ubound);
      gfc_conv_expr_type (&se, ubound, gfc_array_index_type);
      gfc_add_block_to_block (pblock, &se.pre);

      tmp = gfc_conv_descriptor_ubound (descriptor, gfc_rank_cst[n]);
      gfc_add_modify_expr (pblock, tmp, se.expr);

      /* Store the stride.  */
      tmp = gfc_conv_descriptor_stride (descriptor, gfc_rank_cst[n]);
      gfc_add_modify_expr (pblock, tmp, stride);

      /* Calculate the size of this dimension.  */
      size = fold (build (PLUS_EXPR, gfc_array_index_type, se.expr, size));

      /* Multiply the stride by the number of elements in this dimension.  */
      stride = fold (build (MULT_EXPR, gfc_array_index_type, stride, size));
      stride = gfc_evaluate_now (stride, pblock);
    }

  if (pstring && *pstring)
    {
      string_len = *pstring;
      string_len = fold (build (MULT_EXPR, gfc_array_index_type, stride,
				string_len));
    }
  else
    string_len = NULL_TREE;

  /* The stride is the number of elements in the array, so multiply by the
     size of an element to get the total size.  */
  tmp = TYPE_SIZE_UNIT (gfc_get_element_type (type));
  size = fold (build (MULT_EXPR, gfc_array_index_type, stride, tmp));

  if (string_len)
    size = fold (build (PLUS_EXPR, gfc_array_index_type, size, string_len));

  if (poffset != NULL)
    {
      offset = gfc_evaluate_now (offset, pblock);
      *poffset = offset;
    }

  size = gfc_evaluate_now (size, pblock);
  return size;
}


/* Initialises the descriptor and generates a call to _gfor_allocate.  Does
   the work for an ALLOCATE statement.  */
/*GCC ARRAYS*/

void
gfc_array_allocate (gfc_se * se, gfc_ref * ref, tree pstat)
{
  tree tmp;
  tree base;
  tree pointer;
  tree allocate;
  tree offset;
  tree size;
  tree len;
  gfc_expr **lower;
  gfc_expr **upper;

  /* Figure out the size of the array.  */
  switch (ref->u.ar.type)
    {
    case AR_ELEMENT:
      lower = NULL;
      upper = ref->u.ar.start;
      break;

    case AR_FULL:
      assert (ref->u.ar.as->type == AS_EXPLICIT);

      lower = ref->u.ar.as->lower;
      upper = ref->u.ar.as->upper;
      break;

    case AR_SECTION:
      lower = ref->u.ar.start;
      upper = ref->u.ar.end;
      break;

    default:
      abort ();
      break;
    }

  len = se->string_length;
  size = gfc_array_init_size (se->expr, ref->u.ar.as->rank, &offset,
			      lower, upper, &se->pre, &len);

  /* The size is the number of elements in the array, so multiply by the
     size of an element to get the total size.  */
  tmp = TYPE_SIZE_UNIT (gfc_get_element_type (TREE_TYPE (se->expr)));
  size = fold (build (MULT_EXPR, gfc_array_index_type, size, tmp));

  /* Allocate memory to store the data.  */
  tmp = gfc_conv_descriptor_data (se->expr);
  pointer = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (tmp)), tmp);
  pointer = gfc_evaluate_now (pointer, &se->pre);

  if (gfc_array_index_type == gfc_int4_type_node)
    allocate = gfor_fndecl_allocate;
  else if (gfc_array_index_type == gfc_int8_type_node)
    allocate = gfor_fndecl_allocate64;
  else
    abort ();

  tmp = gfc_chainon_list (NULL_TREE, pointer);
  tmp = gfc_chainon_list (tmp, size);
  tmp = gfc_chainon_list (tmp, pstat);
  tmp = gfc_build_function_call (allocate, tmp);
  gfc_add_expr_to_block (&se->pre, tmp);

  pointer = gfc_conv_descriptor_data (se->expr);
  /* Set base = &data[offset].  */
  if (!integer_zerop (offset))
    {
      tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (pointer)), pointer);
      tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, offset);
      pointer = build1 (ADDR_EXPR, TREE_TYPE (pointer), tmp);
    }
  base = gfc_conv_descriptor_base (se->expr);
  gfc_add_modify_expr (&se->pre, base, pointer);

  /* Initialize the pointers for a character array.  */
  if (len)
    {
      gfc_todo_error ("arrays of strings");
    }
}


/* Deallocate an array variable.  Also used when an allocated variable goes
   out of scope.  */
/*GCC ARRAYS*/

tree
gfc_array_deallocate (tree descriptor)
{
  tree var;
  tree tmp;
  stmtblock_t block;

  gfc_start_block (&block);
  /* Get a pointer to the data.  */
  tmp = gfc_conv_descriptor_data (descriptor);
  tmp = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (tmp)), tmp);
  var = gfc_create_var (TREE_TYPE (tmp), "ptr");
  gfc_add_modify_expr (&block, var, tmp);

  /* Parameter is the address of the data component.  */
  tmp = gfc_chainon_list (NULL_TREE, var);
  tmp = gfc_chainon_list (tmp, integer_zero_node);
  tmp = gfc_build_function_call (gfor_fndecl_deallocate, tmp);
  gfc_add_expr_to_block (&block, tmp);

  return gfc_finish_block (&block);
}


/* Create an array constructor from an initialization expression.
   We assume the frontend already did any expansions and conversions.  */

static tree
gfc_conv_array_initializer (tree type, gfc_expr * expr)
{
  gfc_constructor *c;
  tree list;
  tree tmp;
  gfc_se se;
  HOST_WIDE_INT hi;
  unsigned HOST_WIDE_INT lo;

  list = NULL_TREE;
  switch (expr->expr_type)
    {
    case EXPR_CONSTANT:
    case EXPR_STRUCTURE:
      /* A single scalar or derived type value.  Create an array with all
         elements equal to that value.  */
      gfc_init_se (&se, NULL);
      gfc_conv_expr (&se, expr);

      tmp = TYPE_MAX_VALUE (TYPE_DOMAIN (type));
      assert (tmp && INTEGER_CST_P (tmp));
      hi = TREE_INT_CST_HIGH (tmp);
      lo = TREE_INT_CST_LOW (tmp);
      lo++;
      if (lo == 0)
	hi++;
      /* This will probably eat buckets of memory for large arrays.  */
      while (hi != 0 || lo != 0)
        {
          list = tree_cons (NULL_TREE, se.expr, list);
          if (lo == 0)
            hi--;
          lo--;
        }
      break;

    case EXPR_ARRAY:
      /* Create a list of all the elements.  */
      for (c = expr->value.constructor; c; c = c->next)
        {
          if (c->iterator)
            {
              /* Problems occur when we get something like
                 integer :: a(lots) = (/(i, i=1,lots)/)  */
              /* TODO: Unexpanded array initializers.  */
              internal_error
                ("Possible frontend bug: array constructor not expanded");
            }

          gfc_init_se (&se, NULL);
	  switch (c->expr->expr_type)
	    {
	    case EXPR_CONSTANT:
	      gfc_conv_constant (&se, c->expr);
	      break;

	    case EXPR_STRUCTURE:
	      gfc_conv_expr (&se, c->expr);
	      break;

	    default:
	      abort();
	    }
          list = tree_cons (NULL_TREE, se.expr, list);
        }
      /* We created the list in reverse order.  */
      list = nreverse (list);
      break;

    default:
      abort();
    }

  /* Create a constructor from the list of elements.  */
  tmp = build1 (CONSTRUCTOR, type, list);
  TREE_CONSTANT (tmp) = 1;
  return tmp;
}


/* Generate code to evaluate non-constant array bounds.  Sets *poffset and
   returns the size (in elements) of the array.  */

static tree
gfc_trans_array_bounds (tree type, gfc_symbol * sym, tree * poffset,
                        stmtblock_t * pblock)
{
  gfc_array_spec *as;
  tree size;
  tree stride;
  tree offset;
  tree ubound;
  tree lbound;
  tree tmp;
  gfc_se se;

  int dim;

  as = sym->as;

  size = integer_one_node;
  offset = integer_zero_node;
  for (dim = 0; dim < as->rank; dim++)
    {
      /* Evaluate non-constant array bound expressions.  */
      lbound = GFC_TYPE_ARRAY_LBOUND (type, dim);
      if (as->lower[dim] && !INTEGER_CST_P (lbound))
        {
          gfc_init_se (&se, NULL);
          gfc_conv_expr_type (&se, as->lower[dim], gfc_array_index_type);
          gfc_add_block_to_block (pblock, &se.pre);
          gfc_add_modify_expr (pblock, lbound, se.expr);
        }
      ubound = GFC_TYPE_ARRAY_UBOUND (type, dim);
      if (as->upper[dim] && !INTEGER_CST_P (ubound))
        {
          gfc_init_se (&se, NULL);
          gfc_conv_expr_type (&se, as->upper[dim], gfc_array_index_type);
          gfc_add_block_to_block (pblock, &se.pre);
          gfc_add_modify_expr (pblock, ubound, se.expr);
        }
      /* The offset of this dimension.  offset = offset - lbound * stride. */
      tmp = fold (build (MULT_EXPR, gfc_array_index_type, lbound, size));
      offset = fold (build (MINUS_EXPR, gfc_array_index_type, offset, tmp));

      /* The size of this dimension, and the stride of the next.  */
      if (dim + 1 < as->rank)
        stride = GFC_TYPE_ARRAY_STRIDE (type, dim + 1);
      else
        stride = NULL_TREE;

      if (ubound != NULL_TREE && !(stride && INTEGER_CST_P (stride)))
        {
          /* Calculate stride = size * (ubound + 1 - lbound).  */
          tmp = fold (build (MINUS_EXPR, gfc_array_index_type,
                             integer_one_node, lbound));
          tmp = fold (build (PLUS_EXPR, gfc_array_index_type, ubound, tmp));
          tmp = fold (build (MULT_EXPR, gfc_array_index_type, size, tmp));
          if (stride)
            gfc_add_modify_expr (pblock, stride, tmp);
          else
            stride = gfc_evaluate_now (tmp, pblock);
        }

      size = stride;
    }

  *poffset = offset;
  return size;
}


/* Generate code to initialize the descriptor for an array variable.  */

tree
gfc_trans_auto_array_allocation (tree descriptor, gfc_symbol * sym)
{
  stmtblock_t block;
  tree type;
  tree tmp;
  tree size;
  tree pointer;
  tree offset;

  assert (!(sym->attr.pointer || sym->attr.allocatable));

  if (sym->ts.type == BT_CHARACTER)
    gfc_todo_error ("arrays of strings");

  type = TREE_TYPE (descriptor);
  assert (GFC_ARRAY_TYPE_P (type));
  if (TREE_CODE (type) != POINTER_TYPE)
    {
      /* TODO: Put large arrays on the heap.  */
      if (sym->value && !sym->attr.use_assoc)
	{
	  DECL_INITIAL (descriptor) =
	    gfc_conv_array_initializer (TREE_TYPE (descriptor), sym->value);
	}

      return NULL_TREE;
    }

  type = TREE_TYPE (type);

  assert (!sym->attr.use_assoc);
  assert (!TREE_STATIC (descriptor));
  assert (!sym->module[0]);

  gfc_start_block (&block);

  size = gfc_trans_array_bounds (type, sym, &offset, &block);

  /* The size is the number of elements in the array, so multiply by the
     size of an element to get the total size.  */
  tmp = TYPE_SIZE_UNIT (gfc_get_element_type (type));
  size = fold (build (MULT_EXPR, gfc_array_index_type, size, tmp));

  /* Allocate memory to hold the data.  */
  /* Get the address of the data component.  */
  TREE_ADDRESSABLE (descriptor) = 1;
  tmp = descriptor;
  tmp = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (tmp)), tmp);
  pointer = gfc_create_var (TREE_TYPE (tmp), "ptr");
  gfc_add_modify_expr (&block, pointer, tmp);

  /* Now allocate the memory.  */
  tmp = gfc_chainon_list (NULL_TREE, pointer);
  tmp = gfc_chainon_list (tmp, size);

  if (gfc_index_integer_kind == 4)
    pointer = gfor_fndecl_internal_malloc;
  else if (gfc_index_integer_kind == 8)
    pointer = gfor_fndecl_internal_malloc64;
  else
    abort ();
  tmp = gfc_build_function_call (pointer, tmp);
  gfc_add_expr_to_block (&block, tmp);

  tmp = descriptor;
  /* Set the base of the array.  */
  if (!integer_zerop (offset))
    {
      tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp);
      tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, offset);
      tmp = build1 (ADDR_EXPR, TREE_TYPE (descriptor), tmp);
    }
  gfc_add_modify_expr (&block, GFC_TYPE_ARRAY_OFFSET (type), tmp);

  /* Automatic arrays should not have initializers.  */
  assert (!sym->value);

  return gfc_finish_block (&block);
}


/* Generate entry and exit code for g77 calling convention arrays.  */

static tree
gfc_trans_g77_array (gfc_symbol * sym, tree body)
{
  tree parm;
  tree type;
  tree size;
  locus loc;
  tree offset;
  tree tmp;
  stmtblock_t block;

  gfc_get_backend_locus (&loc);
  gfc_set_backend_locus (&sym->declared_at);

  /* Descriptor type.  */
  parm = sym->backend_decl;
  type = TREE_TYPE (parm);
  assert (GFC_ARRAY_TYPE_P (type));

  gfc_start_block (&block);

  /* Evaluate the bounds of the array.  */
  size = gfc_trans_array_bounds (type, sym, &offset, &block);

  /* Set the base pointer.  */
  tmp = parm;
  if (!integer_zerop (offset))
    {
      tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp);
      tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, offset);
      tmp = build1 (ADDR_EXPR, TREE_TYPE (parm), tmp);
    }
  gfc_add_modify_expr (&block, GFC_TYPE_ARRAY_OFFSET (type), tmp);

  tmp = gfc_finish_block (&block);

  gfc_set_backend_locus (&loc);

  gfc_start_block (&block);
  /* Add the initialization code to the start of the function.  */
  gfc_add_expr_to_block (&block, tmp);
  gfc_add_expr_to_block (&block, body);

  return gfc_finish_block (&block);
}


/* Generate entry and exit code for assumed size arrays.  */

tree
gfc_trans_assumed_size (gfc_symbol * sym, tree body)
{
  tree parm;
  tree tmpdesc;
  tree type;
  tree size;
  locus loc;
  tree offset;
  tree args;
  tree tmp;
  tree stmt;
  stmtblock_t block;

  tmpdesc = sym->backend_decl;
  if (TREE_CODE (tmpdesc) == PARM_DECL)
    return gfc_trans_g77_array (sym, body);

  gfc_get_backend_locus (&loc);
  gfc_set_backend_locus (&sym->declared_at);

  /* Descriptor type.  */
  type = TREE_TYPE (tmpdesc);
  assert (GFC_ARRAY_TYPE_P (type));
  parm = GFC_DECL_SAVED_DESCRIPTOR (tmpdesc);

  gfc_start_block (&block);

  /* Evaluate the bounds of the array.  */
  size = gfc_trans_array_bounds (type, sym, &offset, &block);

  /* The actual argument descriptor.  */
  args = gfc_chainon_list (NULL_TREE, parm);

  /* Library call to pack the array.  */
  tmp = gfor_fndecl_in_pack;
  tmp = gfc_build_function_call (tmp, args);
  gfc_add_modify_expr (&block, tmpdesc, tmp);

  /* Set the base pointer.  */
  tmp = tmpdesc;
  if (!integer_zerop (offset))
    {
      tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp);
      tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, offset);
      tmp = build1 (ADDR_EXPR, TREE_TYPE (tmpdesc), tmp);
    }
  gfc_add_modify_expr (&block, GFC_TYPE_ARRAY_OFFSET (type), tmp);

  tmp = gfc_finish_block (&block);

  gfc_set_backend_locus (&loc);

  gfc_start_block (&block);
  /* Add the initialization code to the start of the function.  */
  gfc_add_expr_to_block (&block, tmp);
  gfc_add_expr_to_block (&block, body);

  if (sym->attr.intent != INTENT_IN)
    {
      /* Copy the data back if it was repacked.  */
      args = gfc_chainon_list (NULL_TREE, parm);
      args = gfc_chainon_list (args, tmpdesc);
      tmp = gfor_fndecl_in_unpack;
      stmt = gfc_build_function_call (tmp, args);

      tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (parm)), parm);
      tmp = gfc_conv_descriptor_data (tmp);
      tmp = build (NE_EXPR, boolean_type_node, tmp, tmpdesc);
      tmp = build_v (COND_EXPR, tmp, stmt, build_empty_stmt ());
      gfc_add_expr_to_block (&block, tmp);
    }
  /* We don't need to free any memory allocated by internal_pack as it will
     be freed at the end of the function by pop_context.  */
  return gfc_finish_block (&block);
}


/* Modify the descriptor of an array parameter so that it has the
   correct lower bound.  Also move the upper bound accordingly.
   If the array is not packed, it will be copied into a temporary.
   For each dimension we set the new lower and upper bounds.  Then we copy the
   stride and calculate the offset for this dimension.  We also work out
   what the stride of a packed array would be, and see it the two match.
   If the array need repacking, we set the stride to the values we just
   calculated, recalculate the offset and copy the array data.
   Code is also added to copy the data back at the end of the function.
   */

tree
gfc_trans_dummy_array_bias (gfc_symbol * sym, tree tmpdesc, tree body)
{
  tree size;
  tree type;
  tree offset;
  locus loc;
  stmtblock_t block;
  tree lbound;
  tree ubound;
  tree dubound;
  tree dlbound;
  tree dumdesc;
  tree tmp;
  tree stmt;
  tree stride;
  tree stmt_packed;
  tree stmt_unpacked;
  tree partial;
  gfc_se se;
  int n;
  int checkparm;
  int no_repack;

  if (sym->attr.dummy && TREE_CODE (tmpdesc) == PARM_DECL)
    return gfc_trans_g77_array (sym, body);

  gfc_get_backend_locus (&loc);
  gfc_set_backend_locus (&sym->declared_at);

  /* Descriptor type.  */
  type = TREE_TYPE (tmpdesc);
  assert (GFC_ARRAY_TYPE_P (type));
  dumdesc = GFC_DECL_SAVED_DESCRIPTOR (tmpdesc);
  dumdesc = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (dumdesc)), dumdesc);
  gfc_start_block (&block);

  checkparm = (sym->as->type == AS_EXPLICIT && flag_bounds_check);

  no_repack = !(GFC_DECL_PACKED_ARRAY (tmpdesc)
                || GFC_DECL_PARTIAL_PACKED_ARRAY (tmpdesc));

  if (GFC_DECL_PARTIAL_PACKED_ARRAY (tmpdesc))
    {
      /* For non-constant shape arrays we only check if the first dimension
         is contiguous.  Repacking higher dimensions wouldn't gain us
         anything as we still don't know the array stride.  */
      partial = gfc_create_var (boolean_type_node, "partial");
      TREE_USED (partial) = 1;
      tmp = gfc_conv_descriptor_stride (dumdesc, gfc_rank_cst[0]);
      tmp = fold (build (EQ_EXPR, boolean_type_node, tmp, integer_one_node));
      gfc_add_modify_expr (&block, partial, tmp);
    }
  else
    {
      partial = NULL_TREE;
    }

  /* The naming of stmt_unpacked and stmt_packed may be counter-intuitive
     here, however I think it does the right thing.  */
  if (no_repack)
    {
      /* Set the first stride.  */
      stride = gfc_conv_descriptor_stride (dumdesc, gfc_rank_cst[0]);
      stride = gfc_evaluate_now (stride, &block);

      tmp = build (EQ_EXPR, boolean_type_node, stride, integer_zero_node);
      tmp = build (COND_EXPR, gfc_array_index_type, tmp,
                   integer_one_node, stride);
      stride = GFC_TYPE_ARRAY_STRIDE (type, 0);
      gfc_add_modify_expr (&block, stride, tmp);

      /* Allow the user to disable array repacking.  */
      stmt_unpacked = NULL_TREE;
    }
  else
    {
      assert (integer_onep (GFC_TYPE_ARRAY_STRIDE (type, 0)));
      /* A library call to repack the array if neccessary.  */
      tmp = GFC_DECL_SAVED_DESCRIPTOR (tmpdesc);
      tmp = gfc_chainon_list (NULL_TREE, tmp);
      stmt_unpacked = gfc_build_function_call (gfor_fndecl_in_pack, tmp);

      stride = integer_one_node;
    }

  /* This is for the case where the array data is used directly without
     calling the repack function.  */
  if (no_repack || partial != NULL_TREE)
    stmt_packed = gfc_conv_descriptor_data (dumdesc);
  else
    stmt_packed = NULL_TREE;

  /* Assign the data pointer.  */
  if (stmt_packed != NULL_TREE && stmt_unpacked != NULL_TREE)
    {
      /* Don't repack unknown shape arrays when the first stride is 1.  */
      tmp = build (COND_EXPR, TREE_TYPE (stmt_packed), partial,
                   stmt_packed, stmt_unpacked);
    }
  else
    tmp = stmt_packed != NULL_TREE ? stmt_packed : stmt_unpacked;
  gfc_add_modify_expr (&block, tmpdesc, tmp);

  offset = integer_zero_node;
  size = integer_one_node;

  /* Evaluate the bounds of the array.  */
  for (n = 0; n < sym->as->rank; n++)
    {
      if (checkparm || !sym->as->upper[n])
	{
	  /* Get the bounds of the actual parameter.  */
	  dubound = gfc_conv_descriptor_ubound (dumdesc, gfc_rank_cst[n]);
	  dlbound = gfc_conv_descriptor_lbound (dumdesc, gfc_rank_cst[n]);
	}
      else
        {
	  dubound = NULL_TREE;
	  dlbound = NULL_TREE;
        }

      lbound = GFC_TYPE_ARRAY_LBOUND (type, n);
      if (!INTEGER_CST_P (lbound))
        {
          gfc_init_se (&se, NULL);
          gfc_conv_expr_type (&se, sym->as->upper[n],
                              gfc_array_index_type);
          gfc_add_block_to_block (&block, &se.pre);
          gfc_add_modify_expr (&block, lbound, se.expr);
        }

      ubound = GFC_TYPE_ARRAY_UBOUND (type, n);
      /* Set the desired upper bound.  */
      if (sym->as->upper[n])
	{
	  /* We know what we want the upper bound to be.  */
          if (!INTEGER_CST_P (ubound))
            {
	      gfc_init_se (&se, NULL);
	      gfc_conv_expr_type (&se, sym->as->upper[n],
                                  gfc_array_index_type);
	      gfc_add_block_to_block (&block, &se.pre);
              gfc_add_modify_expr (&block, ubound, se.expr);
            }

	  /* Check the sizes match.  */
	  if (checkparm)
	    {
	      /* Check (ubound(a) - lbound(a) == ubound(b) - lbound(b)).  */

	      tmp = fold (build (MINUS_EXPR, gfc_array_index_type, ubound,
				 lbound[n]));
              stride = build (MINUS_EXPR, gfc_array_index_type, dubound,
                            dlbound);
              tmp = fold (build (NE_EXPR, gfc_array_index_type, tmp, stride));
	      gfc_trans_runtime_check (tmp, gfc_strconst_bounds, &block);
	    }
	}
      else
	{
	  /* For assumed shape arrays move the upper bound by the same amount
	     as the lower bound.  */
          tmp = build (MINUS_EXPR, gfc_array_index_type, dubound, dlbound);
          tmp = fold (build (PLUS_EXPR, gfc_array_index_type, tmp, lbound));
          gfc_add_modify_expr (&block, ubound, tmp);
	}
      /* The offset of this dimension.  offset = offset - lbound * stride. */
      tmp = fold (build (MULT_EXPR, gfc_array_index_type, lbound, stride));
      offset = fold (build (MINUS_EXPR, gfc_array_index_type, offset, tmp));

      /* The size of this dimension, and the stride of the next.  */
      if (n + 1 < sym->as->rank)
        {
          stride = GFC_TYPE_ARRAY_STRIDE (type, n + 1);

          if (no_repack || partial != NULL_TREE)
            {
              stmt_unpacked =
                gfc_conv_descriptor_stride (dumdesc, gfc_rank_cst[n+1]);
            }

          /* Figure out the stride if not a known constant.  */
          if (!INTEGER_CST_P (stride))
            {
              if (no_repack)
                stmt_packed = NULL_TREE;
              else
                {
                  /* Calculate stride = size * (ubound + 1 - lbound).  */
                  tmp = fold (build (MINUS_EXPR, gfc_array_index_type,
                                     integer_one_node, lbound));
                  tmp = fold (build (PLUS_EXPR, gfc_array_index_type,
                                     ubound, tmp));
                  size = fold (build (MULT_EXPR, gfc_array_index_type,
                                      size, tmp));
                  stmt_packed = size;
                }

              /* Assign the stride.  */
              if (stmt_packed != NULL_TREE && stmt_unpacked != NULL_TREE)
                {
                  tmp = build (COND_EXPR, gfc_array_index_type, partial,
                               stmt_unpacked, stmt_packed);
                }
              else
                tmp = (stmt_packed != NULL_TREE) ? stmt_packed : stmt_unpacked;
              gfc_add_modify_expr (&block, stride, tmp);
            }
        }
    }

  /* Set the base pointer.  */
  tmp = tmpdesc;
  if (!integer_zerop (offset))
    {
      tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp);
      tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, offset);
      tmp = build1 (ADDR_EXPR, TREE_TYPE (tmpdesc), tmp);
    }
  gfc_add_modify_expr (&block, GFC_TYPE_ARRAY_OFFSET (type), tmp);

  stmt = gfc_finish_block (&block);

  gfc_start_block (&block);

  /* Only do the entry/initialization code if the arg is present.  */
  dumdesc = GFC_DECL_SAVED_DESCRIPTOR (tmpdesc);
  if (sym->attr.optional)
    {
      tmp = gfc_conv_expr_present (sym);
      stmt = build_v (COND_EXPR, tmp, stmt, build_empty_stmt ());
    }
  gfc_add_expr_to_block (&block, stmt);

  /* Add the main function body.  */
  gfc_add_expr_to_block (&block, body);

  /* Cleanup code.  */
  if (!(no_repack || sym->attr.intent == INTENT_IN))
    {
      /* Copy the data back if it was repacked.  */
      tmp = gfc_chainon_list (NULL_TREE, dumdesc);
      tmp = gfc_chainon_list (tmp, tmpdesc);
      stmt = gfc_build_function_call (gfor_fndecl_in_unpack, tmp);

      tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (dumdesc)), dumdesc);
      tmp = gfc_conv_descriptor_data (tmp);
      tmp = build (NE_EXPR, boolean_type_node, tmp, tmpdesc);
      stmt = build_v (COND_EXPR, tmp, stmt, build_empty_stmt ());

      if (sym->attr.optional)
        {
          tmp = gfc_conv_expr_present (sym);
          stmt = build_v (COND_EXPR, tmp, stmt, build_empty_stmt ());
        }
      gfc_add_expr_to_block (&block, stmt);
    }
  /* We don't need to free any memory allocated by internal_pack as it will
     be freed at the end of the function by pop_context.  */
  return gfc_finish_block (&block);
}


/* Convert an array for passing as an actual parameter.  Expressions
   and vector subscripts are evaluated and stored in a teporary, which is then
   passed.  For whole arrays the descriptor is passed.  For array sections
   a modified copy of the descriptor is passed, but using the original data.
   Also used for array pointer assignments by setting se->direct_byref.  */

void
gfc_conv_expr_descriptor (gfc_se * se, gfc_expr * expr, gfc_ss * ss)
{
  gfc_loopinfo loop;
  gfc_ss *secss;
  gfc_ss_info *info;
  int need_tmp;
  int n;
  tree tmp;
  tree desc;
  stmtblock_t block;
  tree start;
  tree offset;
  int full;

  assert (ss != gfc_ss_terminator);

  if (expr->ts.type == BT_CHARACTER)
    gfc_todo_error ("Character string array actual parameters");

  /* TODO: Pass constant array constructors without a temporary.  */
  /* If we have a linear array section, we can pass it directly.  Otherwise
     we need to copy it into a temporary.  */
  if (expr->expr_type == EXPR_VARIABLE)
    {
      gfc_ss *vss;

      /* Find the SS for the array section.  */
      secss = ss;
      while (secss != gfc_ss_terminator && secss->type != GFC_SS_SECTION)
	secss = secss->next;

      assert (secss != gfc_ss_terminator);

      need_tmp = 0;
      for (n = 0; n < secss->data.info.dimen; n++)
	{
	  vss = secss->data.info.subscript[secss->data.info.dim[n]];
	  if (vss && vss->type == GFC_SS_VECTOR)
	    need_tmp = 1;
	}

      info = &secss->data.info;

      /* Get the descriptor for the array.  */
      gfc_conv_ss_descriptor (&se->pre, secss, 0);
      desc = info->descriptor;
      if (GFC_ARRAY_TYPE_P (TREE_TYPE (desc)))
	{
	  /* Create a new descriptor if the array doesn't have one.  */
	  full = 0;
	}
      else if (info->ref->u.ar.type == AR_FULL)
	full = 1;
      else if (se->direct_byref)
	full = 0;
      else
	{
	  assert (info->ref->u.ar.type == AR_SECTION);

	  full = 1;
	  for (n = 0; n < info->ref->u.ar.dimen; n++)
	    {
	      /* Detect passing the full array as a section.  This could do
	         even more checking, but it doesn't seem worth it.  */
	      if (info->ref->u.ar.start[n]
		  || info->ref->u.ar.end[n]
		  || (info->ref->u.ar.stride[n]
		      && !gfc_expr_is_one (info->ref->u.ar.stride[n], 0)))
		{
		  full = 0;
		  break;
		}
	    }
	}
      if (full)
	{
	  if (se->direct_byref)
	    {
	      /* Copy the descriptor for pointer assignments.  */
	      gfc_add_modify_expr (&se->pre, se->expr, desc);
	    }
	  else if (se->want_pointer)
	    {
	      /* We pass full arrays directly.  This means that pointers and
	         allocatable arrays should also work.  */
	      if (TREE_CODE (desc) == INDIRECT_REF)
		se->expr = TREE_OPERAND (desc, 0);
	      else
		{
		  assert (is_gimple_lvalue (desc));
		  if (is_gimple_variable (desc))
		    TREE_ADDRESSABLE (desc) = 1;
		  se->expr = build1 (ADDR_EXPR,
				     build_pointer_type (TREE_TYPE (desc)),
				     desc);
		}
	    }
	  else
	    {
	      se->expr = desc;
	    }
	  return;
	}
    }
  else
    {
      need_tmp = 1;
      secss = NULL;
      info = NULL;
    }

  gfc_init_loopinfo (&loop);

  /* Associate the SS with the loop.  */
  gfc_add_ss_to_loop (&loop, ss);

  /* Tell the scalarizer not to bother creating loop varliables, etc.  */
  if (!need_tmp)
    loop.array_parameter = 1;
  else
    assert (se->want_pointer && !se->direct_byref);

  /* Setup the scalarizing loops and bounds.  */
  gfc_conv_ss_startstride (&loop);

  if (need_tmp)
    {
      /* Tell the scalarizer to make a temporary.  */
      if (expr->ts.type == BT_CHARACTER)
	gfc_todo_error ("Passing character string expressions");
      loop.temp_ss = gfc_get_ss ();
      loop.temp_ss->type = GFC_SS_TEMP;
      loop.temp_ss->next = gfc_ss_terminator;
      loop.temp_ss->data.temp.type = gfc_typenode_for_spec (&expr->ts);
      loop.temp_ss->data.temp.string_length = NULL;
      loop.temp_ss->data.temp.dimen = loop.dimen;
      gfc_add_ss_to_loop (&loop, loop.temp_ss);
    }

  gfc_conv_loop_setup (&loop);

  if (need_tmp)
    {
      /* Copy into a temporary and pass that.  We don't need to copy the data
         back because expressions and vector subscripts must be INTENT_IN.  */
      /* TODO: Optimize passing function return values.  */
      gfc_se lse;
      gfc_se rse;

      /* Start the copying loops.  */
      gfc_mark_ss_chain_used (loop.temp_ss, 1);
      gfc_mark_ss_chain_used (ss, 1);
      gfc_start_scalarized_body (&loop, &block);

      /* Copy each data element.  */
      gfc_init_se (&lse, NULL);
      gfc_copy_loopinfo_to_se (&lse, &loop);
      gfc_init_se (&rse, NULL);
      gfc_copy_loopinfo_to_se (&rse, &loop);

      lse.ss = loop.temp_ss;
      rse.ss = ss;

      gfc_conv_scalarized_array_ref (&lse, NULL);
      gfc_conv_expr_val (&rse, expr);

      gfc_add_block_to_block (&block, &rse.pre);
      gfc_add_block_to_block (&block, &lse.pre);

      gfc_add_modify_expr (&block, lse.expr, rse.expr);

      /* Finish the copying loops.  */
      gfc_trans_scalarizing_loops (&loop, &block);

      /* Set the first stride component to zero to indicate a temporary.  */
      desc = loop.temp_ss->data.info.descriptor;
      tmp = gfc_conv_descriptor_stride (desc, gfc_rank_cst[0]);
      gfc_add_modify_expr (&loop.pre, tmp, integer_zero_node);

      assert (is_gimple_lvalue (desc));
      TREE_ADDRESSABLE (desc) = 1;
      se->expr = build1 (ADDR_EXPR, build_pointer_type (TREE_TYPE (desc)),
			 desc);
    }
  else
    {
      /* We pass sections without copying to a temporary.  A function may
         decide to repack the array to speed up access, but we're not
         bothered about that here.  */
      int dim;
      tree parm;
      tree parmtype;
      tree stride;
      tree from;
      tree to;
      tree base;

      /* Otherwise make a new descriptor and point it at the section we
         want.  The loop variable limits will be the limits of the section.
       */
      desc = info->descriptor;
      assert (secss && secss != gfc_ss_terminator);
      if (se->direct_byref)
	{
	  /* For pointer assignments we fill in the destination.  */
	  parm = se->expr;
	  parmtype = TREE_TYPE (parm);
	}
      else
	{
	  /* Otherwise make a new one.  */
	  parmtype = gfc_get_element_type (TREE_TYPE (desc));
	  parmtype = gfc_get_array_type_bounds (parmtype, loop.dimen,
						loop.from, loop.to, 0);
	  parm = gfc_create_var (parmtype, "parm");
	}

      offset = integer_zero_node;
      dim = 0;

      /* The following can be somewhat confusing.  We have two
         descriptors, a new one and the original array.
         {parm, parmtype, dim} refer to the new one.
         {desc, type, n, secss, loop} refer to the original, which maybe
         a descriptorless array.
         The bounds of the scaralization are the bounds of the section.
         We don't have to worry about numeric overflows when calculating
         the offsets because all elements are within the array data.  */

      /* Set the dtype.  */
      tmp = gfc_conv_descriptor_dtype (parm);
      gfc_add_modify_expr (&loop.pre, tmp, GFC_TYPE_ARRAY_DTYPE (parmtype));

      if (se->direct_byref)
	base = integer_zero_node;
      else
	base = NULL_TREE;

      for (n = 0; n < info->ref->u.ar.dimen; n++)
	{
	  stride = gfc_conv_array_stride (desc, n);

	  /* Work out the offset.  */
	  if (info->ref->u.ar.dimen_type[n] == DIMEN_ELEMENT)
	    {
	      assert (info->subscript[n]
		      && info->subscript[n]->type == GFC_SS_SCALAR);
	      start = info->subscript[n]->data.scalar.expr;
	    }
	  else
	    {
	      /* Check we haven't somehow got out of sync.  */
	      assert (info->dim[dim] == n);

	      /* Evaluate and remember the start of the section.  */
	      start = info->start[dim];
	      stride = gfc_evaluate_now (stride, &loop.pre);
	    }

	  tmp = gfc_conv_array_lbound (desc, n);
	  tmp = fold (build (MINUS_EXPR, TREE_TYPE (tmp), start, tmp));

	  tmp = fold (build (MULT_EXPR, TREE_TYPE (tmp), tmp, stride));
	  offset = fold (build (PLUS_EXPR, TREE_TYPE (tmp), offset, tmp));

	  if (info->ref->u.ar.dimen_type[n] == DIMEN_ELEMENT)
	    {
	      /* For elemental dimensions, we only need the offset.  */
	      continue;
	    }

	  /* Vector subscripts need copying and are handled elsewhere.  */
	  assert (info->ref->u.ar.dimen_type[n] == DIMEN_RANGE);

	  /* Set the new lower bound.  */
	  from = loop.from[dim];
	  to = loop.to[dim];
          if (!integer_onep (from))
	    {
	      /* Make sure the new section starts at 1.  */
	      tmp = fold (build (MINUS_EXPR, TREE_TYPE (from),
				 integer_one_node, from));
	      to = fold (build (PLUS_EXPR, TREE_TYPE (to), to, tmp));
	      from = integer_one_node;
	    }
	  tmp = gfc_conv_descriptor_lbound (parm, gfc_rank_cst[dim]);
	  gfc_add_modify_expr (&loop.pre, tmp, from);

	  /* Set the new upper bound.  */
	  tmp = gfc_conv_descriptor_ubound (parm, gfc_rank_cst[dim]);
	  gfc_add_modify_expr (&loop.pre, tmp, to);

	  /* Multiply the stride by the section stride to get the
	     total stride.  */
	  stride = fold (build (MULT_EXPR, gfc_array_index_type, stride,
				info->stride[dim]));

	  if (se->direct_byref)
	    {
	      base = fold (build (MINUS_EXPR, TREE_TYPE (base),
				  base, stride));
	    }

	  /* Store the new stride.  */
	  tmp = gfc_conv_descriptor_stride (parm, gfc_rank_cst[dim]);
	  gfc_add_modify_expr (&loop.pre, tmp, stride);

	  dim++;
	}

      /* Point the data pointer at the first element in the section.  */
      tmp = gfc_conv_array_data (desc);
      if (TREE_CODE (tmp) == INDIRECT_REF)
	tmp = TREE_OPERAND (tmp, 0);
      else
	tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp);
      tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, offset);
      offset = build1 (ADDR_EXPR, gfc_array_dataptr_type (desc), tmp);

      tmp = gfc_conv_descriptor_data (parm);
      gfc_add_modify_expr (&loop.pre, tmp, offset);

      if (se->direct_byref)
	{
	  /* Set the base pointer according to POINTER itself.  */
	  tmp = gfc_conv_array_data (parm);
	  if (TREE_CODE (tmp) == INDIRECT_REF)
	    tmp = TREE_OPERAND (tmp, 0);
	  else
	    tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp);
	  tmp = build (ARRAY_REF, TREE_TYPE (TREE_TYPE (tmp)), tmp, base);
	  base = build1 (ADDR_EXPR, gfc_array_dataptr_type (desc), tmp);

	  tmp = gfc_conv_descriptor_base (parm);
	  gfc_add_modify_expr (&loop.pre, tmp, base);
	}
      else
	{
	  /* Invaidate the base pointer.  */
	  tmp = gfc_conv_descriptor_base (parm);
	  gfc_add_modify_expr (&loop.pre, tmp, integer_zero_node);
	}

      if (!se->direct_byref)
	{
	  /* Get a pointer to the new descriptor.  */
          if (se->want_pointer)
            {
              TREE_ADDRESSABLE (parm) = 1;
              se->expr = build1 (ADDR_EXPR, build_pointer_type (parmtype),
                                 parm);
            }
          else
            se->expr = parm;
	}
    }

  gfc_add_block_to_block (&se->pre, &loop.pre);
  gfc_add_block_to_block (&se->post, &loop.post);

  /* Cleanup the scalarizer.  */
  gfc_cleanup_loop (&loop);
}


/* Convert an array for passing as an actual parameter.  */
/* TODO: Optimize passing g77 arrays.  */

void
gfc_conv_array_parameter (gfc_se * se, gfc_expr * expr, gfc_ss * ss, int g77)
{
  tree ptr;
  tree desc;
  tree tmp;
  tree stmt;
  stmtblock_t block;

  se->want_pointer = 1;
  gfc_conv_expr_descriptor (se, expr, ss);

  if (g77)
    {
      desc = se->expr;
      /* Repack the array.  */
      tmp = gfc_chainon_list (NULL_TREE, desc);
      ptr = gfc_build_function_call (gfor_fndecl_in_pack, tmp);
      ptr = gfc_evaluate_now (ptr, &se->pre);
      se->expr = ptr;

      gfc_start_block (&block);

      /* Copy the data back.  */
      tmp = gfc_chainon_list (NULL_TREE, desc);
      tmp = gfc_chainon_list (tmp, ptr);
      tmp = gfc_build_function_call (gfor_fndecl_in_unpack, tmp);
      gfc_add_expr_to_block (&block, tmp);

      /* Free the temporary.  */
      TREE_ADDRESSABLE (ptr) = 1;
      tmp = build1 (ADDR_EXPR, ppvoid_type_node, ptr);
      tmp = gfc_chainon_list (NULL_TREE, tmp);
      tmp = gfc_build_function_call (gfor_fndecl_internal_free, tmp);
      gfc_add_expr_to_block (&block, tmp);

      stmt = gfc_finish_block (&block);

      gfc_init_block (&block);
      /* Only if it was repacked.  This code needs to be executed before the
         loop cleanup code.  */
      if (TREE_CODE (desc) == ADDR_EXPR)
        tmp = TREE_OPERAND (desc, 0);
      else
        tmp = build1 (INDIRECT_REF, TREE_TYPE (TREE_TYPE (desc)), desc);
      tmp = gfc_conv_array_data (tmp);
      tmp = build (NE_EXPR, boolean_type_node, ptr, tmp);
      tmp = build_v (COND_EXPR, tmp, stmt, build_empty_stmt ());

      gfc_add_expr_to_block (&block, tmp);
      gfc_add_block_to_block (&block, &se->post);

      gfc_init_block (&se->post);
      gfc_add_block_to_block (&se->post, &block);
    }
}


/* NULLIFY an allocated/pointer array on function entry, free it on exit.  */

tree
gfc_trans_deferred_array (gfc_symbol * sym, tree body)
{
  tree type;
  tree tmp;
  tree descriptor;
  tree deallocate;
  stmtblock_t block;
  stmtblock_t fnblock;
  locus loc;

  /* Make sure the frontend gets these right.  */
  if (!(sym->attr.pointer || sym->attr.allocatable))
    fatal_error
      ("Possible frontend bug: Deferred array size without pointer or allocatable attribute.");

  gfc_init_block (&fnblock);

  assert (TREE_CODE (sym->backend_decl) == VAR_DECL);
  if (GFC_DECL_STRING (sym->backend_decl))
    {
      tmp = GFC_DECL_STRING_LENGTH (sym->backend_decl);
      if (!INTEGER_CST_P (tmp))
	gfc_conv_init_string_length (sym, &fnblock);
    }


  /* Parameter variables don't need anything special.  */
  if (sym->attr.dummy)
    {
      gfc_add_expr_to_block (&fnblock, body);

      return gfc_finish_block (&fnblock);
    }

  gfc_get_backend_locus (&loc);
  gfc_set_backend_locus (&sym->declared_at);
  descriptor = sym->backend_decl;

  if (TREE_STATIC (descriptor))
    {
      /* SAVEd variables are not freed on exit.  */
      gfc_trans_static_array_pointer (sym);
      return body;
    }

  /* Get the descriptor type.  */
  type = TREE_TYPE (sym->backend_decl);
  assert (GFC_DESCRIPTOR_TYPE_P (type));

  /* NULLIFY the data pointer.  */
  tmp = gfc_conv_descriptor_data (descriptor);
  gfc_add_modify_expr (&fnblock, tmp, integer_zero_node);

  gfc_add_expr_to_block (&fnblock, body);

  gfc_set_backend_locus (&loc);
  /* Allocatable arrays need to be freed when they go out of scope.  */
  if (sym->attr.allocatable)
    {
      gfc_start_block (&block);

      /* Deallocate if still allocated at the end of the procedure.  */
      deallocate = gfc_array_deallocate (descriptor);

      tmp = gfc_conv_descriptor_data (descriptor);
      tmp = build (NE_EXPR, boolean_type_node, tmp, integer_zero_node);
      tmp = build_v (COND_EXPR, tmp, deallocate, build_empty_stmt ());
      gfc_add_expr_to_block (&block, tmp);

      tmp = gfc_finish_block (&block);
      gfc_add_expr_to_block (&fnblock, tmp);
    }

  return gfc_finish_block (&fnblock);
}

/************ Expression Walking Functions ******************/

/* Walk a variable reference.

   Possible extension - multiple component subscripts.
    x(:,:) = foo%a(:)%b(:)
   Transforms to
    forall (i=..., j=...)
      x(i,j) = foo%a(j)%b(i)
    end forall
   This adds a fair amout of complexity because you need to deal with more
   than one ref.  Maybe handle in a similar manner to vector subscripts.
   Maybe not worth the effort.  */


static gfc_ss *
gfc_walk_variable_expr (gfc_ss * ss, gfc_expr * expr)
{
  gfc_ref *ref;
  gfc_array_ref *ar;
  gfc_ss *newss;
  gfc_ss *head;
  int n;

  for (ref = expr->ref; ref; ref = ref->next)
    {
      /* We're only interested in array sections.  */
      if (ref->type != REF_ARRAY)
	continue;

      ar = &ref->u.ar;
      switch (ar->type)
	{
	case AR_ELEMENT:
          /* TODO: Take elemental array references out of scalarization
             loop.  */
	  break;

	case AR_FULL:
	  newss = gfc_get_ss ();
	  newss->type = GFC_SS_SECTION;
	  newss->expr = expr;
	  newss->next = ss;
	  newss->data.info.dimen = ar->as->rank;
	  newss->data.info.ref = ref;

	  /* Make sure array is the same as array(:,:), this way
	     we don't need to special case all the time.  */
	  ar->dimen = ar->as->rank;
	  for (n = 0; n < ar->dimen; n++)
	    {
	      newss->data.info.dim[n] = n;
	      ar->dimen_type[n] = DIMEN_RANGE;

	      assert (ar->start[n] == NULL);
	      assert (ar->end[n] == NULL);
	      assert (ar->stride[n] == NULL);
	    }
	  return newss;

	case AR_SECTION:
	  newss = gfc_get_ss ();
	  newss->type = GFC_SS_SECTION;
	  newss->expr = expr;
	  newss->next = ss;
	  newss->data.info.dimen = 0;
	  newss->data.info.ref = ref;

	  head = newss;

          /* We add SS chains for all the subscripts in the section.  */
	  for (n = 0; n < ar->dimen; n++)
	    {
	      gfc_ss *indexss;

	      switch (ar->dimen_type[n])
		{
		case DIMEN_ELEMENT:
		  /* Add SS for elemental (scalar) subscripts.  */
		  assert (ar->start[n]);
		  indexss = gfc_get_ss ();
		  indexss->type = GFC_SS_SCALAR;
		  indexss->expr = ar->start[n];
		  indexss->next = gfc_ss_terminator;
		  indexss->loop_chain = gfc_ss_terminator;
		  newss->data.info.subscript[n] = indexss;
		  break;

		case DIMEN_RANGE:
                  /* We don't add anything for sections, just remember this
                     dimension for later.  */
		  newss->data.info.dim[newss->data.info.dimen] = n;
		  newss->data.info.dimen++;
		  break;

		case DIMEN_VECTOR:
		  /* Get a SS for the vector.  This will not be added to the
		     chain directly.  */
		  indexss = gfc_walk_expr (ar->start[n]);
		  if (indexss == gfc_ss_terminator)
		    internal_error ("scalar vector subscript???");

                  /* We currently only handle really simple vector
                     subscripts.  */
		  if (indexss->next != gfc_ss_terminator)
		    gfc_todo_error ("vector subscript expressions");
		  indexss->loop_chain = gfc_ss_terminator;

		  /* Mark this as a vector subscript.  We don't add this
                     directly into the chain, but as a subscript of the
                     existing SS for this term.  */
		  indexss->type = GFC_SS_VECTOR;
		  newss->data.info.subscript[n] = indexss;
                  /* Also remember this dimension.  */
		  newss->data.info.dim[newss->data.info.dimen] = n;
		  newss->data.info.dimen++;
		  break;

		default:
		  /* We should know what sort of section it is by now.  */
		  abort ();
		}
	    }
	  /* We should have at least one non-elemental dimension.  */
	  assert (newss->data.info.dimen > 0);
	  return head;
	  break;

	default:
	  /* We should know what sort of section it is by now.  */
	  abort ();
	}

    }
  return ss;
}


/* Walk an expression operator. If only one operand of a binary expression is
   scalar, we must also add the scalar term to the SS chain.  */

static gfc_ss *
gfc_walk_op_expr (gfc_ss * ss, gfc_expr * expr)
{
  gfc_ss *head;
  gfc_ss *head2;
  gfc_ss *newss;

  head = gfc_walk_subexpr (ss, expr->op1);
  if (expr->op2 == NULL)
    head2 = head;
  else
    head2 = gfc_walk_subexpr (head, expr->op2);

  /* All operands are scalar.  Pass back and let the caller deal with it.  */
  if (head2 == ss)
    return head2;

  /* All operands require scalarization. */
  if (head != ss && (expr->op2 == NULL || head2 != head))
    return head2;

  /* One of the operands needs scalarization, the other is scalar.
     Create a gfc_ss for the scalar expression.  */
  newss = gfc_get_ss ();
  newss->type = GFC_SS_SCALAR;
  if (head == ss)
    {
      /* First operand is scalar.  We build the chain in reverse order, so
         add the scarar SS after the second operand.  */
      head = head2;
      while (head && head->next != ss)
	head = head->next;
      /* Check we haven't somehow broken the chain.  */
      assert (head);
      newss->next = ss;
      head->next = newss;
      newss->expr = expr->op1;
    }
  else				/* head2 == head */
    {
      assert (head2 == head);
      /* Second operand is scalar.  */
      newss->next = head2;
      head2 = newss;
      newss->expr = expr->op2;
    }

  return head2;
}


/* Reverse a SS chain.  */

static gfc_ss *
gfc_reverse_ss (gfc_ss * ss)
{
  gfc_ss *next;
  gfc_ss *head;

  assert (ss != NULL);

  head = gfc_ss_terminator;
  while (ss != gfc_ss_terminator)
    {
      next = ss->next;
      assert (next != NULL);	/* Check we didn't somehow break the chain.  */
      ss->next = head;
      head = ss;
      ss = next;
    }

  return (head);
}


/* Walk the arguments of an elemental function.  */

gfc_ss *
gfc_walk_elemental_function_args (gfc_ss * ss, gfc_expr * expr,
				  gfc_ss_type type)
{
  gfc_actual_arglist *arg;
  int scalar;
  gfc_ss *head;
  gfc_ss *tail;
  gfc_ss *newss;

  head = gfc_ss_terminator;
  tail = NULL;
  scalar = 1;
  for (arg = expr->value.function.actual; arg; arg = arg->next)
    {
      if (!arg->expr)
	continue;

      newss = gfc_walk_subexpr (head, arg->expr);
      if (newss == head)
	{
	  /* Scalar argumet.  */
	  newss = gfc_get_ss ();
	  newss->type = type;
	  newss->expr = arg->expr;
	  newss->next = head;
	}
      else
	scalar = 0;

      head = newss;
      if (!tail)
        {
          tail = head;
          while (tail->next != gfc_ss_terminator)
            tail = tail->next;
        }
    }

  if (scalar)
    {
      /* If all the arguments are scalar we don't need the argument SS.  */
      gfc_free_ss_chain (head);
      /* Pass it back.  */
      return ss;
    }

  /* Add it onto the existing chain.  */
  tail->next = ss;
  return head;
}


/* Walk a function call.  Scalar functions are passed back, and taken out of
   scalarization loops.  For elemental functions we walk their arguments.
   The result of functions returning arrays is stored in a temporary outside
   the loop, so that the function is only called once.  Hence we do not need
   to walk their arguments.  */

static gfc_ss *
gfc_walk_function_expr (gfc_ss * ss, gfc_expr * expr)
{
  gfc_ss *newss;
  gfc_intrinsic_sym *isym;
  gfc_symbol *sym;

  isym = expr->value.function.isym;

  /* Handle intrinsic functions seperately.  */
  if (isym)
    return gfc_walk_intrinsic_function (ss, expr, isym);

  sym = expr->value.function.esym;
  if (!sym)
      sym = expr->symtree->n.sym;

  /* A function that returns arrays.  */
  if (gfc_return_by_reference (sym) && sym->result->attr.dimension)
    {
      newss = gfc_get_ss ();
      newss->type = GFC_SS_FUNCTION;
      newss->expr = expr;
      newss->next = ss;
      newss->data.info.dimen = expr->rank;
      return newss;
    }

  /* Walk the parameters of an elemental function.  For now we always pass
     by reference.  */
  if (sym->attr.elemental)
    return gfc_walk_elemental_function_args (ss, expr, GFC_SS_REFERENCE);

  /* Scalar functions are OK as these are evaluated outside the scalarisation
     loop.  Pass back and let the caller deal with it.  */
  return ss;
}


/* An array temporary is constructed for array constructors.  */

static gfc_ss *
gfc_walk_array_constructor (gfc_ss * ss, gfc_expr * expr)
{
  gfc_ss *newss;
  int n;

  newss = gfc_get_ss ();
  newss->type = GFC_SS_CONSTRUCTOR;
  newss->expr = expr;
  newss->next = ss;
  newss->data.info.dimen = expr->rank;
  for (n = 0; n < expr->rank; n++)
    newss->data.info.dim[n] = n;

  return newss;
}


/* Walk an expresson.  Add walked expressions to the head of the SS chain.
   A wholy scalar expression will not be added.  */

static gfc_ss *
gfc_walk_subexpr (gfc_ss * ss, gfc_expr * expr)
{
  gfc_ss *head;

  switch (expr->expr_type)
    {
    case EXPR_VARIABLE:
      head = gfc_walk_variable_expr (ss, expr);
      return head;

    case EXPR_OP:
      head = gfc_walk_op_expr (ss, expr);
      return head;

    case EXPR_FUNCTION:
      head = gfc_walk_function_expr (ss, expr);
      return head;

    case EXPR_CONSTANT:
    case EXPR_NULL:
    case EXPR_STRUCTURE:
      /* Pass back and let the caller deal with it.  */
      break;

    case EXPR_ARRAY:
      head = gfc_walk_array_constructor (ss, expr);
      return head;

    case EXPR_SUBSTRING:
      /* Pass back and let the caller deal with it.  */
      break;

    default:
      internal_error ("bad expression type during walk (%d)",
		      expr->expr_type);
    }
  return ss;
}


/* Entry point for expression walking.
   A return value equal to the passed chain means this is
   a scalar expression.  It is up to the caller to take whatever action is
   neccessary to translate these.  */

gfc_ss *
gfc_walk_expr (gfc_expr * expr)
{
  gfc_ss *res;

  res = gfc_walk_subexpr (gfc_ss_terminator, expr);
  return gfc_reverse_ss (res);
}

