/* ACLE support for AArch64 SVE (function_base classes)
   Copyright (C) 2018-2020 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#ifndef GCC_AARCH64_SVE_BUILTINS_FUNCTIONS_H
#define GCC_AARCH64_SVE_BUILTINS_FUNCTIONS_H

namespace aarch64_sve {

/* Wrap T, which is derived from function_base, and indicate that the
   function never has side effects.  It is only necessary to use this
   wrapper on functions that might have floating-point suffixes, since
   otherwise we assume by default that the function has no side effects.  */
template<typename T>
class quiet : public T
{
public:
  CONSTEXPR quiet () : T () {}

  /* Unfortunately we can't use parameter packs yet.  */
  template<typename T1>
  CONSTEXPR quiet (const T1 &t1) : T (t1) {}

  template<typename T1, typename T2>
  CONSTEXPR quiet (const T1 &t1, const T2 &t2) : T (t1, t2) {}

  template<typename T1, typename T2, typename T3>
  CONSTEXPR quiet (const T1 &t1, const T2 &t2, const T3 &t3)
    : T (t1, t2, t3) {}

  unsigned int
  call_properties (const function_instance &) const OVERRIDE
  {
    return 0;
  }
};

/* A function_base that sometimes or always operates on tuples of
   vectors.  */
class multi_vector_function : public function_base
{
public:
  CONSTEXPR multi_vector_function (unsigned int vectors_per_tuple)
    : m_vectors_per_tuple (vectors_per_tuple) {}

  unsigned int
  vectors_per_tuple () const OVERRIDE
  {
    return m_vectors_per_tuple;
  }

  /* The number of vectors in a tuple, or 1 if the function only operates
     on single vectors.  */
  unsigned int m_vectors_per_tuple;
};

/* A function_base that loads or stores contiguous memory elements
   without extending or truncating them.  */
class full_width_access : public multi_vector_function
{
public:
  CONSTEXPR full_width_access (unsigned int vectors_per_tuple = 1)
    : multi_vector_function (vectors_per_tuple) {}

  tree
  memory_scalar_type (const function_instance &fi) const OVERRIDE
  {
    return fi.scalar_type (0);
  }

  machine_mode
  memory_vector_mode (const function_instance &fi) const OVERRIDE
  {
    machine_mode mode = fi.vector_mode (0);
    if (m_vectors_per_tuple != 1)
      mode = targetm.array_mode (mode, m_vectors_per_tuple).require ();
    return mode;
  }
};

/* A function_base that loads elements from memory and extends them
   to a wider element.  The memory element type is a fixed part of
   the function base name.  */
class extending_load : public function_base
{
public:
  CONSTEXPR extending_load (type_suffix_index memory_type)
    : m_memory_type (memory_type) {}

  unsigned int
  call_properties (const function_instance &) const OVERRIDE
  {
    return CP_READ_MEMORY;
  }

  tree
  memory_scalar_type (const function_instance &) const OVERRIDE
  {
    return scalar_types[type_suffixes[m_memory_type].vector_type];
  }

  machine_mode
  memory_vector_mode (const function_instance &fi) const OVERRIDE
  {
    machine_mode mem_mode = type_suffixes[m_memory_type].vector_mode;
    machine_mode reg_mode = fi.vector_mode (0);
    return aarch64_sve_data_mode (GET_MODE_INNER (mem_mode),
				  GET_MODE_NUNITS (reg_mode)).require ();
  }

  /* Return the rtx code associated with the kind of extension that
     the load performs.  */
  rtx_code
  extend_rtx_code () const
  {
    return (type_suffixes[m_memory_type].unsigned_p
	    ? ZERO_EXTEND : SIGN_EXTEND);
  }

  /* The type of the memory elements.  This is part of the function base
     name rather than a true type suffix.  */
  type_suffix_index m_memory_type;
};

/* A function_base that truncates vector elements and stores them to memory.
   The memory element width is a fixed part of the function base name.  */
class truncating_store : public function_base
{
public:
  CONSTEXPR truncating_store (scalar_int_mode to_mode) : m_to_mode (to_mode) {}

  unsigned int
  call_properties (const function_instance &) const OVERRIDE
  {
    return CP_WRITE_MEMORY;
  }

  tree
  memory_scalar_type (const function_instance &fi) const OVERRIDE
  {
    /* In truncating stores, the signedness of the memory element is defined
       to be the same as the signedness of the vector element.  The signedness
       doesn't make any difference to the behavior of the function.  */
    type_class_index tclass = fi.type_suffix (0).tclass;
    unsigned int element_bits = GET_MODE_BITSIZE (m_to_mode);
    type_suffix_index suffix = find_type_suffix (tclass, element_bits);
    return scalar_types[type_suffixes[suffix].vector_type];
  }

  machine_mode
  memory_vector_mode (const function_instance &fi) const OVERRIDE
  {
    poly_uint64 nunits = GET_MODE_NUNITS (fi.vector_mode (0));
    return aarch64_sve_data_mode (m_to_mode, nunits).require ();
  }

  /* The mode of a single memory element.  */
  scalar_int_mode m_to_mode;
};

/* A function_base for functions that have an associated rtx code.
   It supports all forms of predication except PRED_implicit.  */
class rtx_code_function : public function_base
{
public:
  CONSTEXPR rtx_code_function (rtx_code code_for_sint, rtx_code code_for_uint,
			       int unspec_for_fp = -1)
    : m_code_for_sint (code_for_sint), m_code_for_uint (code_for_uint),
      m_unspec_for_fp (unspec_for_fp) {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    return e.map_to_rtx_codes (m_code_for_sint, m_code_for_uint,
			       m_unspec_for_fp);
  }

  /* The rtx code to use for signed and unsigned integers respectively.
     Can be UNKNOWN for functions that don't have integer forms.  */
  rtx_code m_code_for_sint;
  rtx_code m_code_for_uint;

  /* The UNSPEC_COND_* to use for floating-point operations.  Can be -1
     for functions that only operate on integers.  */
  int m_unspec_for_fp;
};

/* Like rtx_code_function, but for functions that take what is normally
   the final argument first.  One use of this class is to handle binary
   reversed operations; another is to handle MLA-style operations that
   are normally expressed in GCC as MAD-style operations.  */
class rtx_code_function_rotated : public function_base
{
public:
  CONSTEXPR rtx_code_function_rotated (rtx_code code_for_sint,
				       rtx_code code_for_uint,
				       int unspec_for_fp = -1)
    : m_code_for_sint (code_for_sint), m_code_for_uint (code_for_uint),
      m_unspec_for_fp (unspec_for_fp) {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    /* Rotate the inputs into their normal order, but continue to make _m
       functions merge with what was originally the first vector argument.  */
    unsigned int nargs = e.args.length ();
    e.rotate_inputs_left (e.pred != PRED_none ? 1 : 0, nargs);
    return e.map_to_rtx_codes (m_code_for_sint, m_code_for_uint,
			       m_unspec_for_fp, nargs - 1);
  }

  /* The rtx code to use for signed and unsigned integers respectively.
     Can be UNKNOWN for functions that don't have integer forms.  */
  rtx_code m_code_for_sint;
  rtx_code m_code_for_uint;

  /* The UNSPEC_COND_* to use for floating-point operations.  Can be -1
     for functions that only operate on integers.  */
  int m_unspec_for_fp;
};

/* A function_base for functions that have an associated unspec code.
   It supports all forms of predication except PRED_implicit.  */
class unspec_based_function : public function_base
{
public:
  CONSTEXPR unspec_based_function (int unspec_for_sint, int unspec_for_uint,
				   int unspec_for_fp)
    : m_unspec_for_sint (unspec_for_sint),
      m_unspec_for_uint (unspec_for_uint),
      m_unspec_for_fp (unspec_for_fp)
  {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    return e.map_to_unspecs (m_unspec_for_sint, m_unspec_for_uint,
			     m_unspec_for_fp);
  }

  /* The unspec code associated with signed-integer, unsigned-integer
     and floating-point operations respectively.  */
  int m_unspec_for_sint;
  int m_unspec_for_uint;
  int m_unspec_for_fp;
};

/* Like unspec_based_function, but for functions that take what is normally
   the final argument first.  One use of this class is to handle binary
   reversed operations; another is to handle MLA-style operations that
   are normally expressed in GCC as MAD-style operations.  */
class unspec_based_function_rotated : public function_base
{
public:
  CONSTEXPR unspec_based_function_rotated (int unspec_for_sint,
					   int unspec_for_uint,
					   int unspec_for_fp)
    : m_unspec_for_sint (unspec_for_sint),
      m_unspec_for_uint (unspec_for_uint),
      m_unspec_for_fp (unspec_for_fp)
  {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    /* Rotate the inputs into their normal order, but continue to make _m
       functions merge with what was originally the first vector argument.  */
    unsigned int nargs = e.args.length ();
    e.rotate_inputs_left (e.pred != PRED_none ? 1 : 0, nargs);
    return e.map_to_unspecs (m_unspec_for_sint, m_unspec_for_uint,
			     m_unspec_for_fp, nargs - 1);
  }

  /* The unspec code associated with signed-integer, unsigned-integer
     and floating-point operations respectively.  */
  int m_unspec_for_sint;
  int m_unspec_for_uint;
  int m_unspec_for_fp;
};

/* A function_base for functions that permute their arguments.  */
class permute : public quiet<function_base>
{
public:
  /* Fold a unary or binary permute with the permute vector given by
     BUILDER.  */
  gimple *
  fold_permute (const gimple_folder &f, const vec_perm_builder &builder) const
  {
    /* Punt for now on _b16 and wider; we'd need more complex evpc logic
       to rerecognize the result.  */
    if (f.type_suffix (0).bool_p && f.type_suffix (0).element_bits > 8)
      return NULL;

    unsigned int nargs = gimple_call_num_args (f.call);
    poly_uint64 nelts = TYPE_VECTOR_SUBPARTS (TREE_TYPE (f.lhs));
    vec_perm_indices indices (builder, nargs, nelts);
    tree perm_type = build_vector_type (ssizetype, nelts);
    return gimple_build_assign (f.lhs, VEC_PERM_EXPR,
				gimple_call_arg (f.call, 0),
				gimple_call_arg (f.call, nargs - 1),
				vec_perm_indices_to_tree (perm_type, indices));
  }
};

/* A function_base for functions that permute two vectors using a fixed
   choice of indices.  */
class binary_permute : public permute
{
public:
  CONSTEXPR binary_permute (int unspec) : m_unspec (unspec) {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    insn_code icode = code_for_aarch64_sve (m_unspec, e.vector_mode (0));
    return e.use_exact_insn (icode);
  }

  /* The unspec code associated with the operation.  */
  int m_unspec;
};

/* A function_base for functions that reduce a vector to a scalar.  */
class reduction : public function_base
{
public:
  CONSTEXPR reduction (int unspec)
    : m_unspec_for_sint (unspec),
      m_unspec_for_uint (unspec),
      m_unspec_for_fp (unspec)
  {}

  CONSTEXPR reduction (int unspec_for_sint, int unspec_for_uint,
		       int unspec_for_fp)
    : m_unspec_for_sint (unspec_for_sint),
      m_unspec_for_uint (unspec_for_uint),
      m_unspec_for_fp (unspec_for_fp)
  {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    machine_mode mode = e.vector_mode (0);
    int unspec = (!e.type_suffix (0).integer_p ? m_unspec_for_fp
		  : e.type_suffix (0).unsigned_p ? m_unspec_for_uint
		  : m_unspec_for_sint);
    /* There's no distinction between SADDV and UADDV for 64-bit elements;
       the signed versions only exist for narrower elements.  */
    if (GET_MODE_UNIT_BITSIZE (mode) == 64 && unspec == UNSPEC_SADDV)
      unspec = UNSPEC_UADDV;
    return e.use_exact_insn (code_for_aarch64_pred_reduc (unspec, mode));
  }

  /* The unspec code associated with signed-integer, unsigned-integer
     and floating-point operations respectively.  */
  int m_unspec_for_sint;
  int m_unspec_for_uint;
  int m_unspec_for_fp;
};

/* A function_base for functions that shift narrower-than-64-bit values
   by 64-bit amounts.  */
class shift_wide : public function_base
{
public:
  CONSTEXPR shift_wide (rtx_code code, int wide_unspec)
    : m_code (code), m_wide_unspec (wide_unspec) {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    machine_mode mode = e.vector_mode (0);
    machine_mode elem_mode = GET_MODE_INNER (mode);

    /* If the argument is a constant that the normal shifts can handle
       directly, use them instead.  */
    rtx shift = unwrap_const_vec_duplicate (e.args.last ());
    if (aarch64_simd_shift_imm_p (shift, elem_mode, m_code == ASHIFT))
      {
	e.args.last () = shift;
	return e.map_to_rtx_codes (m_code, m_code, -1);
      }

    if (e.pred == PRED_x)
      return e.use_unpred_insn (code_for_aarch64_sve (m_wide_unspec, mode));

    return e.use_cond_insn (code_for_cond (m_wide_unspec, mode));
  }

  /* The rtx code associated with a "normal" shift.  */
  rtx_code m_code;

  /* The unspec code associated with the wide shift.  */
  int m_wide_unspec;
};

/* A function_base for unary functions that count bits.  */
class unary_count : public quiet<function_base>
{
public:
  CONSTEXPR unary_count (rtx_code code) : m_code (code) {}

  rtx
  expand (function_expander &e) const OVERRIDE
  {
    /* The md patterns treat the operand as an integer.  */
    machine_mode mode = aarch64_sve_int_mode (e.vector_mode (0));
    e.args.last () = gen_lowpart (mode, e.args.last ());

    if (e.pred == PRED_x)
      return e.use_pred_x_insn (code_for_aarch64_pred (m_code, mode));

    return e.use_cond_insn (code_for_cond (m_code, mode));
  }

  /* The rtx code associated with the operation.  */
  rtx_code m_code;
};

}

/* Declare the global function base NAME, creating it from an instance
   of class CLASS with constructor arguments ARGS.  */
#define FUNCTION(NAME, CLASS, ARGS) \
  namespace { static CONSTEXPR const CLASS NAME##_obj ARGS; } \
  namespace functions { const function_base *const NAME = &NAME##_obj; }

#endif
