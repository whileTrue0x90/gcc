/* Operations with very long integers.  -*- C++ -*-
   Copyright (C) 2012-2013 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

GCC is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#ifndef WIDE_INT_H
#define WIDE_INT_H

/* wide-int.[cc|h] implements a class that efficiently performs
   mathematical operations on finite precision integers.  wide_ints
   are designed to be transient - they are not for long term storage
   of values.  There is tight integration between wide_ints and the
   other longer storage GCC representations (rtl and tree).

   The actual precision of a wide_int depends on the flavor.  There
   are three predefined flavors:

     1) wide_int (the default).  This flavor does the math in the
     precision of its input arguments.  It is assumed (and checked)
     that the precisions of the operands and results are consistent.
     This is the most efficient flavor.  It is not possible to examine
     bits above the precision that has been specified.  Because of
     this, the default flavor has semantics that are simple to
     understand and in general model the underlying hardware that the
     compiler is targetted for.

     This flavor must be used at the RTL level of gcc because there
     is, in general, not enough information in the RTL representation
     to extend a value beyond the precision specified in the mode.

     This flavor should also be used at the TREE and GIMPLE levels of
     the compiler except for the circumstances described in the
     descriptions of the other two flavors.

     The default wide_int representation does not contain any
     information inherent about signedness of the represented value,
     so it can be used to represent both signed and unsigned numbers.
     For operations where the results depend on signedness (full width
     multiply, division, shifts, comparisons, and operations that need
     overflow detected), the signedness must be specified separately.

     2) addr_wide_int.  This is a fixed size representation that is
     guaranteed to be large enough to compute any bit or byte sized
     address calculation on the target.  Currently the value is 64 + 4
     bits rounded up to the next number even multiple of
     HOST_BITS_PER_WIDE_INT (but this can be changed when the first
     port needs more than 64 bits for the size of a pointer).

     This flavor can be used for all address math on the target.  In
     this representation, the values are sign or zero extended based
     on their input types to the internal precision.  All math is done
     in this precision and then the values are truncated to fit in the
     result type.  Unlike most gimple or rtl intermediate code, it is
     not useful to perform the address arithmetic at the same
     precision in which the operands are represented because there has
     been no effort by the front ends to convert most addressing
     arithmetic to canonical types.

     In the addr_wide_int, all numbers are represented as signed
     numbers.  There are enough bits in the internal representation so
     that no infomation is lost by representing them this way.

     3) max_wide_int.  This representation is an approximation of
     infinite precision math.  However, it is not really infinite
     precision math as in the GMP library.  It is really finite
     precision math where the precision is 4 times the size of the
     largest integer that the target port can represent.

     Like, the addr_wide_ints, all numbers are inherently signed.

     There are several places in the GCC where this should/must be used:

     * Code that does widening conversions.  The canonical way that
       this is performed is to sign or zero extend the input value to
       the max width based on the sign of the type of the source and
       then to truncate that value to the target type.  This is in
       preference to using the sign of the target type to extend the
       value directly (which gets the wrong value for the conversion
       of large unsigned numbers to larger signed types).

     * Code that does induction variable optimizations.  This code
       works with induction variables of many different types at the
       same time.  Because of this, it ends up doing many different
       calculations where the operands are not compatible types.  The
       max_wide_int makes this easy, because it provides a field where
       nothing is lost when converting from any variable,

     * There are a small number of passes that currently use the
       max_wide_int that should use the default.  These should be
       changed.

   There are surprising features of addr_wide_int and max_wide_int
   that the users should be careful about:

     1) Shifts and rotations are just weird.  You have to specify a
     precision in which the shift or rotate is to happen in.  The bits
     above this precision remain unchanged.  While this is what you
     want, it is clearly is non obvious.

     2) Larger precision math sometimes does not produce the same
     answer as would be expected for doing the math at the proper
     precision.  In particular, a multiply followed by a divide will
     produce a different answer if the first product is larger than
     what can be represented in the input precision.

   The addr_wide_int and the max_wide_int flavors are more expensive
   than the default wide int, so in addition to the caveats with these
   two, the default is the prefered representation.

   All three flavors of wide_int are represented as a vector of
   HOST_WIDE_INTs.  The vector contains enough elements to hold a
   value of MAX_BITSIZE_MODE_ANY_INT / HOST_BITS_PER_WIDE_INT which is
   a derived for each host/target combination.  The values are stored
   in the vector with the least significant HOST_BITS_PER_WIDE_INT
   bits of the value stored in element 0.

   A wide_int contains three fields: the vector (VAL), precision and a
   length (LEN).  The length is the number of HWIs needed to
   represent the value.  For the max_wide_int and the addr_wide_int,
   the precision is a constant that cannot be changed.  For the
   default wide_int, the precision is set from the constructor.

   Since most integers used in a compiler are small values, it is
   generally profitable to use a representation of the value that is
   as small as possible.  LEN is used to indicate the number of
   elements of the vector that are in use.  The numbers are stored as
   sign extended numbers as a means of compression.  Leading
   HOST_WIDE_INTs that contain strings of either -1 or 0 are removed
   as long as they can be reconstructed from the top bit that is being
   represented.

   There are constructors to create the various forms of wide_int from
   trees, rtl and constants.  For trees and constants, you can simply say:

             tree t = ...;
	     wide_int x = t;
	     wide_int y = 6;

   However, a little more syntax is required for rtl constants since
   they do have an explicit precision.  To make an rtl into a
   wide_int, you have to pair it with a mode.  The canonical way to do
   this is with std::make_pair as in:

             rtx r = ...
	     wide_int x = std::make_pair (r, mode);

   Wide ints sometimes have a value with the precision of 0.  These
   come from two separate sources:

   * The front ends do sometimes produce values that really have a
     precision of 0.  The only place where these seem to come in are
     the MIN and MAX value for types with a precision of 0.  Asside
     from the computation of these MIN and MAX values, there appears
     to be no other use of true precision 0 numbers so the overloading
     of precision 0 does not appear to be an issue.  These appear to
     be associated with 0 width bit fields.  They are harmless, but
     there are several paths through the wide int code to support this
     without having to special case the front ends.

   * When a constant that has an integer type is converted to a
     wide_int it comes in with precision 0.  For these constants the
     top bit does accurately reflect the sign of that constant; this
     is an exception to the normal rule that the signedness is not
     represented.  When used in a binary operation, the wide_int
     implementation properly extends these constants so that they
     properly match the other operand of the computation.  This allows
     you write:

                tree t = ...
                wide_int x = t + 6;

     assuming t is a int_cst.

   Note that the bits above the precision are not defined and the
   algorithms used here are careful not to depend on their value.  In
   particular, values that come in from rtx constants may have random
   bits.  When the precision is 0, all the bits in the LEN elements of
   VEC are significant with no undefined bits.  Precisionless
   constants are limited to being one or two HOST_WIDE_INTs.  When two
   are used the upper value is 0, and the high order bit of the first
   value is set.  (Note that this may need to be generalized if it is
   ever necessary to support 32bit HWIs again).

   Many binary operations require that the precisions of the two
   operands be the same.  However, the API tries to keep this relaxed
   as much as possible.  In particular:

   * shifts do not care about the precision of the second operand.

   * values that come in from gcc source constants or variables are
     not checked as long one of the two operands has a precision.
     This is allowed because it is always known whether to sign or zero
     extend these values.

   * The comparisons do not require that the operands be the same
     length.  This allows wide ints to be used in hash tables where
     all of the values may not be the same precision.  */


#include <utility>
#include "system.h"
#include "hwint.h"
#include "signop.h"
#include "insn-modes.h"

#if 0
#define DEBUG_WIDE_INT
#endif

/* The MAX_BITSIZE_MODE_ANY_INT is automatically generated by a very
   early examination of the target's mode file.  Thus it is safe that
   some small multiple of this number is easily larger than any number
   that that target could compute.  The place in the compiler that
   currently needs the widest ints is the code that determines the
   range of a multiply.  This code needs 2n + 2 bits.  */

#define WIDE_INT_MAX_ELTS \
  ((4 * MAX_BITSIZE_MODE_ANY_INT + HOST_BITS_PER_WIDE_INT - 1) \
   / HOST_BITS_PER_WIDE_INT)

/* This is the max size of any pointer on any machine.  It does not
   seem to be as easy to sniff this out of the machine description as
   it is for MAX_BITSIZE_MODE_ANY_INT since targets may support
   multiple address sizes and may have different address sizes for
   different address spaces.  However, currently the largest pointer
   on any platform is 64 bits.  When that changes, then it is likely
   that a target hook should be defined so that targets can make this
   value larger for those targets.  */
#define ADDR_MAX_BITSIZE 64

/* This is the internal precision used when doing any address
   arithmetic.  The '4' is really 3 + 1.  Three of the bits are for
   the number of extra bits needed to do bit addresses and single bit is
   allow everything to be signed without loosing any precision.  Then
   everything is rounded up to the next HWI for efficiency.  */
#define ADDR_MAX_PRECISION \
  ((ADDR_MAX_BITSIZE + 4 + HOST_BITS_PER_WIDE_INT - 1) & ~(HOST_BITS_PER_WIDE_INT - 1))

namespace wi
{
  /* Classifies an integer based on its precision.  */
  enum precision_type {
    /* The integer has both a precision and defined signedness.  This allows
       the integer to be converted to any width, since we know whether to fill
       any extra bits with zeros or signs.  */
    FLEXIBLE_PRECISION,

    /* The integer has a variable precision but no defined signedness.  */
    VAR_PRECISION,

    /* The integer has a constant precision (known at GCC compile time)
       but no defined signedness.  */
    CONST_PRECISION
  };

  /* This class, which has no default implementation, is expected to
     provide the following members:

     static const enum precision_type precision_type;
       Classifies the type of T.

     static const unsigned int precision;
       Only defined if precision_type == CONST_PRECISION.  Specifies the
       precision of all integers of type T.

     static const bool host_dependent_precision;
       True if the precision of T depends (or can depend) on the host.

     static unsigned int get_precision (const T &x)
       Return the number of bits in X.

     static wi::storage_ref *decompose (HOST_WIDE_INT *scratch,
					unsigned int precision, const T &x)
       Decompose X as a PRECISION-bit integer, returning the associated
       wi::storage_ref.  SCRATCH is available as scratch space if needed.
       The routine should assert that PRECISION is acceptable.  */
  template <typename T> struct int_traits;

  /* This class provides a single type, result_type, which specifies the
     type of integer produced by a binary operation whose inputs have
     types T1 and T2.  The definition should be symmetric.  */
  template <typename T1, typename T2,
	    enum precision_type P1 = int_traits <T1>::precision_type,
	    enum precision_type P2 = int_traits <T2>::precision_type>
  struct binary_traits;

  /* The result of a unary operation on T is the same as the result of
     a binary operation on two values of type T.  */
  template <typename T>
  struct unary_traits : public binary_traits <T, T> {};
}

/* The type of result produced by a binary operation on types T1 and T2.
   Defined purely for brevity.  */
#define WI_BINARY_RESULT(T1, T2) \
  typename wi::binary_traits <T1, T2>::result_type

/* The type of result produced by a unary operation on type T.  */
#define WI_UNARY_RESULT(T) \
  typename wi::unary_traits <T>::result_type

/* Define a variable RESULT to hold the result of a binary operation on
   X and Y, which have types T1 and T2 respectively.  Define VAR to
   point to the blocks of RESULT.  Once the user of the macro has
   filled in VAR, it should call RESULT.set_len to set the number
   of initialized blocks.  */
#define WI_BINARY_RESULT_VAR(RESULT, VAL, T1, X, T2, Y) \
  WI_BINARY_RESULT (T1, T2) RESULT = \
    wi::int_traits <WI_BINARY_RESULT (T1, T2)>::get_binary_result (X, Y); \
  HOST_WIDE_INT *VAL = RESULT.write_val ()

/* Similar for the result of a unary operation on X, which has type T.  */
#define WI_UNARY_RESULT_VAR(RESULT, VAL, T, X) \
  WI_UNARY_RESULT (T) RESULT = \
    wi::int_traits <WI_UNARY_RESULT (T)>::get_binary_result (X, X); \
  HOST_WIDE_INT *VAL = RESULT.write_val ()

template <typename T> struct generic_wide_int;

struct wide_int_storage;
typedef generic_wide_int <wide_int_storage> wide_int;

struct wide_int_ref_storage;
typedef generic_wide_int <wide_int_ref_storage> wide_int_ref;

/* Public functions for querying and operating on integers.  */
namespace wi
{
  template <typename T>
  unsigned int get_precision (const T &);

  template <typename T1, typename T2>
  unsigned int get_binary_precision (const T1 &, const T2 &);

  /* FIXME: should disappear.  */
  template <typename T>
  void clear_undef (T &, signop);

  bool fits_shwi_p (const wide_int_ref &);
  bool fits_uhwi_p (const wide_int_ref &);
  bool neg_p (const wide_int_ref &, signop = SIGNED);
  bool only_sign_bit_p (const wide_int_ref &, unsigned int);
  bool only_sign_bit_p (const wide_int_ref &);
  HOST_WIDE_INT sign_mask (const wide_int_ref &);

  template <typename T1, typename T2>
  bool eq_p (const T1 &, const T2 &);

  template <typename T1, typename T2>
  bool ne_p (const T1 &, const T2 &);

  bool lt_p (const wide_int_ref &, const wide_int_ref &, signop);
  bool lts_p (const wide_int_ref &, const wide_int_ref &);
  bool ltu_p (const wide_int_ref &, const wide_int_ref &);
  bool le_p (const wide_int_ref &, const wide_int_ref &, signop);
  bool les_p (const wide_int_ref &, const wide_int_ref &);
  bool leu_p (const wide_int_ref &, const wide_int_ref &);
  bool gt_p (const wide_int_ref &, const wide_int_ref &, signop);
  bool gts_p (const wide_int_ref &, const wide_int_ref &);
  bool gtu_p (const wide_int_ref &, const wide_int_ref &);
  bool ge_p (const wide_int_ref &, const wide_int_ref &, signop);
  bool ges_p (const wide_int_ref &, const wide_int_ref &);
  bool geu_p (const wide_int_ref &, const wide_int_ref &);
  int cmp (const wide_int_ref &, const wide_int_ref &, signop);
  int cmps (const wide_int_ref &, const wide_int_ref &);
  int cmpu (const wide_int_ref &, const wide_int_ref &);

#define UNARY_FUNCTION \
  template <typename T> WI_UNARY_RESULT (T)
#define BINARY_FUNCTION \
  template <typename T1, typename T2> WI_BINARY_RESULT (T1, T2)
#define SHIFT_FUNCTION \
  template <typename T> WI_UNARY_RESULT (T)

  UNARY_FUNCTION bit_not (const T &);
  UNARY_FUNCTION neg (const T &);
  UNARY_FUNCTION neg (const T &, bool *);
  UNARY_FUNCTION abs (const T &);
  UNARY_FUNCTION ext (const T &, unsigned int, signop);
  UNARY_FUNCTION sext (const T &, unsigned int);
  UNARY_FUNCTION zext (const T &, unsigned int);
  UNARY_FUNCTION set_bit (const T &, unsigned int);

  BINARY_FUNCTION min (const T1 &, const T2 &, signop);
  BINARY_FUNCTION smin (const T1 &, const T2 &);
  BINARY_FUNCTION umin (const T1 &, const T2 &);
  BINARY_FUNCTION max (const T1 &, const T2 &, signop);
  BINARY_FUNCTION smax (const T1 &, const T2 &);
  BINARY_FUNCTION umax (const T1 &, const T2 &);

  BINARY_FUNCTION bit_and (const T1 &, const T2 &);
  BINARY_FUNCTION bit_and_not (const T1 &, const T2 &);
  BINARY_FUNCTION bit_or (const T1 &, const T2 &);
  BINARY_FUNCTION bit_or_not (const T1 &, const T2 &);
  BINARY_FUNCTION bit_xor (const T1 &, const T2 &);
  BINARY_FUNCTION add (const T1 &, const T2 &);
  BINARY_FUNCTION add (const T1 &, const T2 &, signop, bool *);
  BINARY_FUNCTION sub (const T1 &, const T2 &);
  BINARY_FUNCTION sub (const T1 &, const T2 &, signop, bool *);
  BINARY_FUNCTION mul (const T1 &, const T2 &);
  BINARY_FUNCTION mul (const T1 &, const T2 &, signop, bool *);
  BINARY_FUNCTION smul (const T1 &, const T2 &, bool *);
  BINARY_FUNCTION umul (const T1 &, const T2 &, bool *);
  BINARY_FUNCTION mul_high (const T1 &, const T2 &, signop);
  BINARY_FUNCTION div_trunc (const T1 &, const T2 &, signop, bool * = 0);
  BINARY_FUNCTION sdiv_trunc (const T1 &, const T2 &);
  BINARY_FUNCTION udiv_trunc (const T1 &, const T2 &);
  BINARY_FUNCTION div_floor (const T1 &, const T2 &, signop, bool * = 0);
  BINARY_FUNCTION udiv_floor (const T1 &, const T2 &);
  BINARY_FUNCTION sdiv_floor (const T1 &, const T2 &);
  BINARY_FUNCTION div_ceil (const T1 &, const T2 &, signop, bool * = 0);
  BINARY_FUNCTION div_round (const T1 &, const T2 &, signop, bool * = 0);
  BINARY_FUNCTION divmod_trunc (const T1 &, const T2 &, signop,
				WI_BINARY_RESULT (T1, T2) *);
  BINARY_FUNCTION mod_trunc (const T1 &, const T2 &, signop, bool * = 0);
  BINARY_FUNCTION smod_trunc (const T1 &, const T2 &);
  BINARY_FUNCTION umod_trunc (const T1 &, const T2 &);
  BINARY_FUNCTION mod_floor (const T1 &, const T2 &, signop, bool * = 0);
  BINARY_FUNCTION umod_floor (const T1 &, const T2 &);
  BINARY_FUNCTION mod_ceil (const T1 &, const T2 &, signop, bool * = 0);
  BINARY_FUNCTION mod_round (const T1 &, const T2 &, signop, bool * = 0);

  template <typename T1, typename T2>
  bool multiple_of_p (const T1 &, const T2 &, signop,
		      WI_BINARY_RESULT (T1, T2) *);

  unsigned int trunc_shift (const wide_int_ref &, unsigned int, unsigned int);

  SHIFT_FUNCTION lshift (const T &, const wide_int_ref &, unsigned int = 0);
  SHIFT_FUNCTION lrshift (const T &, const wide_int_ref &, unsigned int = 0);
  SHIFT_FUNCTION arshift (const T &, const wide_int_ref &, unsigned int = 0);
  SHIFT_FUNCTION rshift (const T &, const wide_int_ref &, signop sgn,
			 unsigned int = 0);
  SHIFT_FUNCTION lrotate (const T &, const wide_int_ref &, unsigned int = 0);
  SHIFT_FUNCTION rrotate (const T &, const wide_int_ref &, unsigned int = 0);

#undef SHIFT_FUNCTION
#undef BINARY_FUNCTION
#undef UNARY_FUNCTION

  int clz (const wide_int_ref &);
  int clrsb (const wide_int_ref &);
  int ctz (const wide_int_ref &);
  int exact_log2 (const wide_int_ref &);
  int floor_log2 (const wide_int_ref &);
  int ffs (const wide_int_ref &);
  int popcount (const wide_int_ref &);
  int parity (const wide_int_ref &);

  template <typename T>
  unsigned HOST_WIDE_INT extract_uhwi (const T &, unsigned int, unsigned int);
}

namespace wi
{
  /* Contains the components of a decomposed integer for easy, direct
     access.  */
  struct storage_ref
  {
    storage_ref (const HOST_WIDE_INT *, unsigned int, unsigned int);

    const HOST_WIDE_INT *val;
    unsigned int len;
    unsigned int precision;

    /* Provide enough trappings for this class to act as storage for
       generic_wide_int.  */
    unsigned int get_len () const;
    unsigned int get_precision () const;
    const HOST_WIDE_INT *get_val () const;
  };
}

inline::wi::storage_ref::storage_ref (const HOST_WIDE_INT *val_in,
				      unsigned int len_in,
				      unsigned int precision_in)
  : val (val_in), len (len_in), precision (precision_in)
{
}

inline unsigned int
wi::storage_ref::get_len () const
{
  return len;
}

inline unsigned int
wi::storage_ref::get_precision () const
{
  return precision;
}

inline const HOST_WIDE_INT *
wi::storage_ref::get_val () const
{
  return val;
}

namespace wi
{
  template <>
  struct int_traits <wi::storage_ref>
  {
    static const enum precision_type precision_type = VAR_PRECISION;
    /* wi::storage_ref can be a reference to a primitive type,
       so this is the conservatively-correct setting.  */
    static const bool host_dependent_precision = true;
  };
}

/* This class defines an integer type using the storage provided by the
   template argument.  The storage class must provide the following
   functions:

   unsigned int get_precision () const
     Return the number of bits in the integer.

   HOST_WIDE_INT *get_val () const
     Return a pointer to the array of blocks that encodes the integer.

   unsigned int get_len () const
     Return the number of blocks in get_val ().  If this is smaller
     than the number of blocks implied by get_precision (), the
     remaining blocks are sign extensions of block get_len () - 1.

   Although not required by generic_wide_int itself, writable storage
   classes can also provide the following functions:

   HOST_WIDE_INT *write_val ()
     Get a modifiable version of get_val ()

   unsigned int set_len (unsigned int len)
     Set the value returned by get_len () to LEN.  */
template <typename storage>
class GTY(()) generic_wide_int : public storage
{
public:
  generic_wide_int ();

  template <typename T>
  generic_wide_int (const T &);

  template <typename T>
  generic_wide_int (const T &, unsigned int);

  /* Conversions.  */
  HOST_WIDE_INT to_shwi (unsigned int = 0) const;
  unsigned HOST_WIDE_INT to_uhwi (unsigned int = 0) const;
  HOST_WIDE_INT to_short_addr () const;

  /* Public accessors for the interior of a wide int.  */
  HOST_WIDE_INT sign_mask () const;
  HOST_WIDE_INT elt (unsigned int) const;
  unsigned HOST_WIDE_INT ulow () const;
  unsigned HOST_WIDE_INT uhigh () const;

#define BINARY_PREDICATE(OP, F) \
  template <typename T> \
  bool OP (const T &c) const { return wi::F (*this, c); }

#define UNARY_OPERATOR(OP, F) \
  generic_wide_int OP () const { return wi::F (*this); }

#define BINARY_OPERATOR(OP, F) \
  template <typename T> \
  generic_wide_int OP (const T &c) const { return wi::F (*this, c); }

#define ASSIGNMENT_OPERATOR(OP, F) \
  template <typename T> \
  generic_wide_int &OP (const T &c) { return (*this = wi::F (*this, c)); }

#define INCDEC_OPERATOR(OP, DELTA) \
  generic_wide_int &OP () { *this += DELTA; return *this; }

  UNARY_OPERATOR (operator ~, bit_not) \
  UNARY_OPERATOR (operator -, neg) \
  BINARY_PREDICATE (operator ==, eq_p) \
  BINARY_PREDICATE (operator !=, ne_p) \
  BINARY_OPERATOR (operator &, bit_and) \
  BINARY_OPERATOR (and_not, bit_and_not) \
  BINARY_OPERATOR (operator |, bit_or) \
  BINARY_OPERATOR (or_not, bit_or_not) \
  BINARY_OPERATOR (operator ^, bit_xor) \
  BINARY_OPERATOR (operator +, add) \
  BINARY_OPERATOR (operator -, sub) \
  BINARY_OPERATOR (operator *, mul) \
  ASSIGNMENT_OPERATOR (operator &=, bit_and) \
  ASSIGNMENT_OPERATOR (operator |=, bit_or) \
  ASSIGNMENT_OPERATOR (operator ^=, bit_xor) \
  ASSIGNMENT_OPERATOR (operator +=, add) \
  ASSIGNMENT_OPERATOR (operator -=, sub) \
  ASSIGNMENT_OPERATOR (operator *=, mul) \
  INCDEC_OPERATOR (operator ++, 1) \
  INCDEC_OPERATOR (operator --, -1)

#undef BINARY_PREDICATE
#undef UNARY_OPERATOR
#undef BINARY_OPERATOR
#undef ASSIGNMENT_OPERATOR
#undef INCDEC_OPERATOR

  char *dump (char *) const;
};

template <typename storage>
inline generic_wide_int <storage>::generic_wide_int () {}

template <typename storage>
template <typename T>
inline generic_wide_int <storage>::generic_wide_int (const T &x)
  : storage (x)
{
}

template <typename storage>
template <typename T>
inline generic_wide_int <storage>::generic_wide_int (const T &x,
						     unsigned int precision)
  : storage (x, precision)
{
}

/* Return THIS as a signed HOST_WIDE_INT, sign-extending from PRECISION.
   If THIS does not fit in PRECISION, the information is lost.  */
template <typename storage>
inline HOST_WIDE_INT
generic_wide_int <storage>::to_shwi (unsigned int precision) const
{
  if (precision == 0)
    precision = this->get_precision ();
  if (precision < HOST_BITS_PER_WIDE_INT)
    return sext_hwi (this->get_val ()[0], precision);
  else
    return this->get_val ()[0];
}

/* Return THIS as an unsigned HOST_WIDE_INT, zero-extending from
   PRECISION.  If THIS does not fit in PRECISION, the information
   is lost.  */
template <typename storage>
inline unsigned HOST_WIDE_INT
generic_wide_int <storage>::to_uhwi (unsigned int precision) const
{
  if (precision == 0)
    precision = this->get_precision ();
  if (precision < HOST_BITS_PER_WIDE_INT)
    return zext_hwi (this->get_val ()[0], precision);
  else
    return this->get_val ()[0];
}

/* TODO: The compiler is half converted from using HOST_WIDE_INT to
   represent addresses to using addr_wide_int to represent addresses.
   We use to_short_addr at the interface from new code to old,
   unconverted code.  */
template <typename storage>
inline HOST_WIDE_INT
generic_wide_int <storage>::to_short_addr () const
{
  return this->get_val ()[0];
}

/* Return the implicit value of blocks above get_len ().  */
template <typename storage>
inline HOST_WIDE_INT
generic_wide_int <storage>::sign_mask () const
{
  return this->get_val ()[this->get_len () - 1] < 0 ? -1 : 0;
}

/* Return the value of the least-significant explicitly-encoded block.  */
template <typename storage>
inline unsigned HOST_WIDE_INT
generic_wide_int <storage>::ulow () const
{
  return this->get_val ()[0];
}

/* Return the value of the most-significant explicitly-encoded block.  */
template <typename storage>
inline unsigned HOST_WIDE_INT
generic_wide_int <storage>::uhigh () const
{
  return this->get_val ()[this->get_len () - 1];
}

/* Return block I, which might be implicitly or explicit encoded.  */
template <typename storage>
inline HOST_WIDE_INT
generic_wide_int <storage>::elt (unsigned int i) const
{
  if (i >= this->get_len ())
    return sign_mask ();
  else
    return this->get_val ()[i];
}

namespace wi
{
  template <>
  template <typename storage>
  struct int_traits < generic_wide_int <storage> >
    : public wi::int_traits <storage>
  {
    static unsigned int get_precision (const generic_wide_int <storage> &);
    static wi::storage_ref decompose (HOST_WIDE_INT *, unsigned int,
				      const generic_wide_int <storage> &);
  };
}

template <typename storage>
inline unsigned int
wi::int_traits < generic_wide_int <storage> >::
get_precision (const generic_wide_int <storage> &x)
{
  return x.get_precision ();
}

template <typename storage>
inline wi::storage_ref
wi::int_traits < generic_wide_int <storage> >::
decompose (HOST_WIDE_INT *, unsigned int precision,
	   const generic_wide_int <storage> &x)
{
  gcc_checking_assert (precision == x.get_precision ());
  return wi::storage_ref (x.get_val (), x.get_len (), precision);
}

/* Provide the storage for a wide_int_ref.  This acts like a read-only
   wide_int, with the optimization that VAL is normally a pointer to another
   integer's storage, so that no array copy is needed.  */
struct wide_int_ref_storage : public wi::storage_ref
{
private:
  /* Scratch space that can be used when decomposing the original integer.
     It must live as long as this object.  */
  HOST_WIDE_INT scratch[WIDE_INT_MAX_ELTS];

public:
  template <typename T>
  wide_int_ref_storage (const T &);

  template <typename T>
  wide_int_ref_storage (const T &, unsigned int);
};

/* Create a reference to integer X in its natural precision.  Note that
   the natural precision is host-dependent for primitive types.  */
template <typename T>
inline wide_int_ref_storage::wide_int_ref_storage (const T &x)
  : storage_ref (wi::int_traits <T>::decompose (scratch,
						wi::get_precision (x), x))
{
}

/* Create a reference to integer X in precision PRECISION.  */
template <typename T>
inline wide_int_ref_storage::wide_int_ref_storage (const T &x,
						   unsigned int precision)
  : storage_ref (wi::int_traits <T>::decompose (scratch, precision, x))
{
}

namespace wi
{
  template <>
  struct int_traits <wide_int_ref_storage>
    : public int_traits <wi::storage_ref>
  {
  };
}

namespace wi
{
  unsigned int force_to_size (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			      unsigned int, unsigned int, unsigned int,
			      signop sgn);
  unsigned int from_array (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			   unsigned int, unsigned int, bool = true);
}

/* The storage used by wide_int.  */
class GTY(()) wide_int_storage
{
private:
  HOST_WIDE_INT val[WIDE_INT_MAX_ELTS];
  unsigned int len;
  unsigned int precision;

public:
  wide_int_storage ();
  template <typename T>
  wide_int_storage (const T &);

  /* The standard generic_wide_int storage methods.  */
  unsigned int get_precision () const;
  const HOST_WIDE_INT *get_val () const;
  unsigned int get_len () const;
  HOST_WIDE_INT *write_val ();
  void set_len (unsigned int);

  static wide_int from (const wide_int_ref &, unsigned int, signop);
  static wide_int from_array (const HOST_WIDE_INT *, unsigned int,
			      unsigned int, bool = true);
  static wide_int create (unsigned int);

  /* FIXME: target-dependent, so should disappear.  */
  wide_int bswap () const;
};

inline wide_int_storage::wide_int_storage () {}

/* Initialize the storage from integer X, in its natural precision.
   Note that we do not allow integers with host-dependent precision
   to become wide_ints; wide_ints must always be logically independent
   of the host.  */
template <typename T>
inline wide_int_storage::wide_int_storage (const T &x)
{
  STATIC_ASSERT (!wi::int_traits<T>::host_dependent_precision);
  wide_int_ref xi (x);
  precision = xi.precision;
  len = xi.len;
  for (unsigned int i = 0; i < len; ++i)
    val[i] = xi.val[i];
}

inline unsigned int
wide_int_storage::get_precision () const
{
  return precision;
}

inline const HOST_WIDE_INT *
wide_int_storage::get_val () const
{
  return val;
}

inline unsigned int
wide_int_storage::get_len () const
{
  return len;
}

inline HOST_WIDE_INT *
wide_int_storage::write_val ()
{
  return val;
}

inline void
wide_int_storage::set_len (unsigned int l)
{
  len = l;
}

/* Treat X as having signedness SGN and convert it to a PRECISION-bit
   number.  */
inline wide_int
wide_int_storage::from (const wide_int_ref &x, unsigned int precision,
			signop sgn)
{
  wide_int result = wide_int::create (precision);
  result.set_len (wi::force_to_size (result.write_val (), x.val, x.len,
				     x.precision, precision, sgn));
  return result;
}

/* Create a wide_int from the explicit block encoding given by VAL and LEN.
   PRECISION is the precision of the integer.  NEED_CANON_P is true if the
   encoding may have redundant trailing blocks.  */
inline wide_int
wide_int_storage::from_array (const HOST_WIDE_INT *val, unsigned int len,
			      unsigned int precision, bool need_canon_p)
{
  wide_int result = wide_int::create (precision);
  result.set_len (wi::from_array (result.write_val (), val, len, precision,
				  need_canon_p));
  return result;
}

/* Return an uninitialized wide_int with precision PRECISION.  */
inline wide_int
wide_int_storage::create (unsigned int precision)
{
  wide_int x;
  x.precision = precision;
  return x;
}

namespace wi
{
  template <>
  struct int_traits <wide_int_storage>
  {
    static const enum precision_type precision_type = VAR_PRECISION;
    /* Guaranteed by a static assert in the wide_int_storage constructor.  */
    static const bool host_dependent_precision = false;
    template <typename T1, typename T2>
    static wide_int get_binary_result (const T1 &, const T2 &);
  };
}

template <typename T1, typename T2>
inline wide_int
wi::int_traits <wide_int_storage>::get_binary_result (const T1 &x, const T2 &y)
{
  /* This shouldn't be used for two flexible-precision inputs.  */
  STATIC_ASSERT (wi::int_traits <T1>::precision_type != FLEXIBLE_PRECISION
		 || wi::int_traits <T2>::precision_type != FLEXIBLE_PRECISION);
  if (wi::int_traits <T1>::precision_type == FLEXIBLE_PRECISION)
    return wide_int::create (wi::get_precision (y));
  else
    return wide_int::create (wi::get_precision (x));
}

/* An N-bit integer.  Until we can use typedef templates, use this instead.  */
#define FIXED_WIDE_INT(N) \
  generic_wide_int < fixed_wide_int_storage <N> >

/* The storage used by FIXED_WIDE_INT (N).  */
template <int N>
class GTY(()) fixed_wide_int_storage
{
private:
  HOST_WIDE_INT val[(N + HOST_BITS_PER_WIDE_INT + 1) / HOST_BITS_PER_WIDE_INT];
  unsigned int len;

public:
  fixed_wide_int_storage ();
  template <typename T>
  fixed_wide_int_storage (const T &);

  /* The standard generic_wide_int storage methods.  */
  unsigned int get_precision () const;
  const HOST_WIDE_INT *get_val () const;
  unsigned int get_len () const;
  HOST_WIDE_INT *write_val ();
  void set_len (unsigned int);

  static FIXED_WIDE_INT (N) from (const wide_int_ref &, signop);
  static FIXED_WIDE_INT (N) from_array (const HOST_WIDE_INT *, unsigned int,
					bool = true);
};

typedef FIXED_WIDE_INT (ADDR_MAX_PRECISION) addr_wide_int;
typedef FIXED_WIDE_INT (MAX_BITSIZE_MODE_ANY_INT) max_wide_int;

template <int N>
inline fixed_wide_int_storage <N>::fixed_wide_int_storage () {}

/* Initialize the storage from integer X, in precision N.  */
template <int N>
template <typename T>
inline fixed_wide_int_storage <N>::fixed_wide_int_storage (const T &x)
{
  /* Check for type compatibility.  We don't want to initialize a
     fixed-width integer from something like a wide_int.  */
  WI_BINARY_RESULT (T, FIXED_WIDE_INT (N)) *assertion ATTRIBUTE_UNUSED;
  wide_int_ref xi (x, N);
  len = xi.len;
  for (unsigned int i = 0; i < len; ++i)
    val[i] = xi.val[i];
}

template <int N>
inline unsigned int
fixed_wide_int_storage <N>::get_precision () const
{
  return N;
}

template <int N>
inline const HOST_WIDE_INT *
fixed_wide_int_storage <N>::get_val () const
{
  return val;
}

template <int N>
inline unsigned int
fixed_wide_int_storage <N>::get_len () const
{
  return len;
}

template <int N>
inline HOST_WIDE_INT *
fixed_wide_int_storage <N>::write_val ()
{
  return val;
}

template <int N>
inline void
fixed_wide_int_storage <N>::set_len (unsigned int l)
{
  len = l;
}

/* Treat X as having signedness SGN and convert it to an N-bit number.  */
template <int N>
inline FIXED_WIDE_INT (N)
fixed_wide_int_storage <N>::from (const wide_int_ref &x, signop sgn)
{
  FIXED_WIDE_INT (N) result;
  result.set_len (wi::force_to_size (result.write_val (), x.val, x.len,
				     x.precision, N, sgn));
  return result;
}

/* Create a FIXED_WIDE_INT (N) from the explicit block encoding given by
   VAL and LEN.  NEED_CANON_P is true if the encoding may have redundant
   trailing blocks.  */
template <int N>
inline FIXED_WIDE_INT (N)
fixed_wide_int_storage <N>::from_array (const HOST_WIDE_INT *val,
					unsigned int len,
					bool need_canon_p)
{
  FIXED_WIDE_INT (N) result;
  result.set_len (wi::from_array (result.write_val (), val, len,
				  N, need_canon_p));
  return result;
}

namespace wi
{
  template <>
  template <int N>
  struct int_traits < fixed_wide_int_storage <N> >
  {
    static const enum precision_type precision_type = CONST_PRECISION;
    static const bool host_dependent_precision = false;
    static const unsigned int precision = N;
    template <typename T1, typename T2>
    static FIXED_WIDE_INT (N) get_binary_result (const T1 &, const T2 &);
  };
}

template <int N>
template <typename T1, typename T2>
inline FIXED_WIDE_INT (N)
wi::int_traits < fixed_wide_int_storage <N> >::
get_binary_result (const T1 &, const T2 &)
{
  return FIXED_WIDE_INT (N) ();
}

/* Specify the result type for each supported combination of binary inputs.
   Note that CONST_PRECISION and VAR_PRECISION cannot be mixed, in order to
   give stronger type checking.  When both inputs are CONST_PRECISION,
   they must have the same precision.  */
namespace wi
{
  template <>
  template <typename T1, typename T2>
  struct binary_traits <T1, T2, FLEXIBLE_PRECISION, FLEXIBLE_PRECISION>
  {
    typedef max_wide_int result_type;
  };

  template <>
  template <typename T1, typename T2>
  struct binary_traits <T1, T2, FLEXIBLE_PRECISION, VAR_PRECISION>
  {
    typedef wide_int result_type;
  };

  template <>
  template <typename T1, typename T2>
  struct binary_traits <T1, T2, FLEXIBLE_PRECISION, CONST_PRECISION>
  {
    /* Spelled out explicitly (rather than through FIXED_WIDE_INT)
       so as not to confuse gengtype.  */
    typedef generic_wide_int < fixed_wide_int_storage
			       <int_traits <T2>::precision> > result_type;
  };

  template <>
  template <typename T1, typename T2>
  struct binary_traits <T1, T2, VAR_PRECISION, FLEXIBLE_PRECISION>
  {
    typedef wide_int result_type;
  };

  template <>
  template <typename T1, typename T2>
  struct binary_traits <T1, T2, CONST_PRECISION, FLEXIBLE_PRECISION>
  {
    /* Spelled out explicitly (rather than through FIXED_WIDE_INT)
       so as not to confuse gengtype.  */
    typedef generic_wide_int < fixed_wide_int_storage
			       <int_traits <T1>::precision> > result_type;
  };

  template <>
  template <typename T1, typename T2>
  struct binary_traits <T1, T2, CONST_PRECISION, CONST_PRECISION>
  {
    /* Spelled out explicitly (rather than through FIXED_WIDE_INT)
       so as not to confuse gengtype.  */
    STATIC_ASSERT (int_traits <T1>::precision == int_traits <T2>::precision);
    typedef generic_wide_int < fixed_wide_int_storage
			       <int_traits <T1>::precision> > result_type;
  };

  template <>
  template <typename T1, typename T2>
  struct binary_traits <T1, T2, VAR_PRECISION, VAR_PRECISION>
  {
    typedef wide_int result_type;
  };
}

namespace wi
{
  /* Implementation of int_traits for primitive integer types like "int".  */
  template <typename T, bool signed_p>
  struct primitive_int_traits
  {
    static const enum precision_type precision_type = FLEXIBLE_PRECISION;
    static const bool host_dependent_precision = true;
    static unsigned int get_precision (T);
    static wi::storage_ref decompose (HOST_WIDE_INT *, unsigned int, T);
  };
}

template <typename T, bool signed_p>
inline unsigned int
wi::primitive_int_traits <T, signed_p>::get_precision (T)
{
  return sizeof (T) * CHAR_BIT;
}

template <typename T, bool signed_p>
inline wi::storage_ref
wi::primitive_int_traits <T, signed_p>::decompose (HOST_WIDE_INT *scratch,
						   unsigned int precision, T x)
{
  scratch[0] = x;
  if (signed_p || scratch[0] >= 0 || precision <= HOST_BITS_PER_WIDE_INT)
    return wi::storage_ref (scratch, 1, precision);
  scratch[1] = 0;
  return wi::storage_ref (scratch, 2, precision);
}

/* Allow primitive C types to be used in wi:: routines.  */
namespace wi
{
  template <>
  struct int_traits <int>
    : public primitive_int_traits <int, true> {};

  template <>
  struct int_traits <unsigned int>
    : public primitive_int_traits <unsigned int, false> {};

#if HOST_BITS_PER_INT != HOST_BITS_PER_WIDE_INT
  template <>
  struct int_traits <HOST_WIDE_INT>
    : public primitive_int_traits <HOST_WIDE_INT, true> {};

  template <>
  struct int_traits <unsigned HOST_WIDE_INT>
    : public primitive_int_traits <unsigned HOST_WIDE_INT, false> {};
#endif
}

namespace wi
{
  /* Stores HWI-sized integer VAL, treating it as having signedness SGN
     and precision PRECISION.  */
  struct hwi_with_prec
  {
    hwi_with_prec (HOST_WIDE_INT, unsigned int, signop);
    HOST_WIDE_INT val;
    unsigned int precision;
    signop sgn;
  };

  hwi_with_prec shwi (HOST_WIDE_INT, unsigned int);
  hwi_with_prec uhwi (unsigned HOST_WIDE_INT, unsigned int);

  hwi_with_prec minus_one (unsigned int);
  hwi_with_prec zero (unsigned int);
  hwi_with_prec one (unsigned int);
  hwi_with_prec two (unsigned int);
}

inline wi::hwi_with_prec::hwi_with_prec (HOST_WIDE_INT v, unsigned int p,
					 signop s)
  : val (v), precision (p), sgn (s)
{
}

/* Return a signed integer that has value VAL and precision PRECISION.  */
inline wi::hwi_with_prec
wi::shwi (HOST_WIDE_INT val, unsigned int precision)
{
  return hwi_with_prec (val, precision, SIGNED);
}

/* Return an unsigned integer that has value VAL and precision PRECISION.  */
inline wi::hwi_with_prec
wi::uhwi (unsigned HOST_WIDE_INT val, unsigned int precision)
{
  return hwi_with_prec (val, precision, UNSIGNED);
}

/* Return a wide int of -1 with precision PREC.  */
inline wi::hwi_with_prec
wi::minus_one (unsigned int precision)
{
  return wi::shwi (-1, precision);
}

/* Return a wide int of 0 with precision PREC.  */
inline wi::hwi_with_prec
wi::zero (unsigned int precision)
{
  return wi::shwi (0, precision);
}

/* Return a wide int of 1 with precision PREC.  */
inline wi::hwi_with_prec
wi::one (unsigned int precision)
{
  return wi::shwi (1, precision);
}

/* Return a wide int of 2 with precision PREC.  */
inline wi::hwi_with_prec
wi::two (unsigned int precision)
{
  return wi::shwi (2, precision);
}

namespace wi
{
  template <>
  struct int_traits <wi::hwi_with_prec>
  {
    /* Since we have a sign, we can extend or truncate the integer to
       other precisions where necessary.  */
    static const enum precision_type precision_type = FLEXIBLE_PRECISION;
    /* hwi_with_prec has an explicitly-given precision, rather than the
       precision of HOST_WIDE_INT.  */
    static const bool host_dependent_precision = false;
    static unsigned int get_precision (const wi::hwi_with_prec &);
    static wi::storage_ref decompose (HOST_WIDE_INT *, unsigned int,
    const wi::hwi_with_prec &);
  };
}

inline unsigned int
wi::int_traits <wi::hwi_with_prec>::get_precision (const wi::hwi_with_prec &x)
{
  return x.precision;
}

inline wi::storage_ref
wi::int_traits <wi::hwi_with_prec>::
decompose (HOST_WIDE_INT *scratch, unsigned int precision,
	   const wi::hwi_with_prec &x)
{
  scratch[0] = x.val;
  if (x.sgn == SIGNED || x.val >= 0 || precision <= HOST_BITS_PER_WIDE_INT)
    return wi::storage_ref (scratch, 1, precision);
  scratch[1] = 0;
  return wi::storage_ref (scratch, 2, precision);
}

/* Private functions for handling large cases out of line.  They take
   individual length and array parameters because that is cheaper for
   the inline caller than constructing an object on the stack and
   passing a reference to it.  (Although many callers use wide_int_refs,
   we generally want those to be removed by SRA.)  */
namespace wi
{
  bool eq_p_large (const HOST_WIDE_INT *, unsigned int,
		   const HOST_WIDE_INT *, unsigned int, unsigned int);
  bool lts_p_large (const HOST_WIDE_INT *, unsigned int, unsigned int,
		    const HOST_WIDE_INT *, unsigned int, unsigned int);
  bool ltu_p_large (const HOST_WIDE_INT *, unsigned int, unsigned int,
		    const HOST_WIDE_INT *, unsigned int, unsigned int);
  int cmps_large (const HOST_WIDE_INT *, unsigned int, unsigned int,
		  const HOST_WIDE_INT *, unsigned int, unsigned int);
  int cmpu_large (const HOST_WIDE_INT *, unsigned int, unsigned int,
		  const HOST_WIDE_INT *, unsigned int, unsigned int);
  unsigned int sext_large (HOST_WIDE_INT *, const HOST_WIDE_INT *, unsigned int,
			   unsigned int, unsigned int);
  unsigned int zext_large (HOST_WIDE_INT *, const HOST_WIDE_INT *, unsigned int,
			   unsigned int, unsigned int);
  unsigned int set_bit_large (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			      unsigned int, unsigned int, unsigned int);
  unsigned int lshift_large (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			     unsigned int, unsigned int, unsigned int);
  unsigned int lrshift_large (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			      unsigned int, unsigned int, unsigned int,
			      unsigned int);
  unsigned int arshift_large (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			      unsigned int, unsigned int, unsigned int,
			      unsigned int);
  unsigned int and_large (HOST_WIDE_INT *, const HOST_WIDE_INT *, unsigned int,
			  const HOST_WIDE_INT *, unsigned int, unsigned int);
  unsigned int and_not_large (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			      unsigned int, const HOST_WIDE_INT *,
			      unsigned int, unsigned int);
  unsigned int or_large (HOST_WIDE_INT *, const HOST_WIDE_INT *, unsigned int,
			 const HOST_WIDE_INT *, unsigned int, unsigned int);
  unsigned int or_not_large (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			     unsigned int, const HOST_WIDE_INT *,
			     unsigned int, unsigned int);
  unsigned int xor_large (HOST_WIDE_INT *, const HOST_WIDE_INT *, unsigned int,
			  const HOST_WIDE_INT *, unsigned int, unsigned int);
  unsigned int add_large (HOST_WIDE_INT *, const HOST_WIDE_INT *, unsigned int,
			  const HOST_WIDE_INT *, unsigned int, unsigned int,
			  signop, bool *);
  unsigned int sub_large (HOST_WIDE_INT *, const HOST_WIDE_INT *, unsigned int,
			  const HOST_WIDE_INT *, unsigned int, unsigned int,
			  signop, bool *);
  unsigned int mul_internal (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			     unsigned int, const HOST_WIDE_INT *,
			     unsigned int, unsigned int, signop, bool *,
			     bool, bool);
  unsigned int divmod_internal (HOST_WIDE_INT *, unsigned int *,
				HOST_WIDE_INT *, const HOST_WIDE_INT *,
				unsigned int, unsigned int,
				const HOST_WIDE_INT *,
				unsigned int, unsigned int,
				signop, bool *);
}

/* Return the number of bits that integer X can hold.  */
template <typename T>
inline unsigned int
wi::get_precision (const T &x)
{
  return wi::int_traits <T>::get_precision (x);
}

/* Return the number of bits that the result of a binary operation can hold
   when the input operands are X and Y.  */
template <typename T1, typename T2>
inline unsigned int
wi::get_binary_precision (const T1 &x, const T2 &y)
{
  return get_precision (wi::int_traits <WI_BINARY_RESULT (T1, T2)>::
			get_binary_result (x, y));
}

/* Extend undefined bits in X according to SGN.  */
template <typename T>
inline void
wi::clear_undef (T &x, signop sgn)
{
  HOST_WIDE_INT *val = x.write_val ();
  unsigned int precision = x.get_precision ();
  unsigned int len = x.get_len ();
  unsigned int small_prec = precision % HOST_BITS_PER_WIDE_INT;
  if (small_prec
      && len == ((precision + HOST_BITS_PER_WIDE_INT - 1)
		 / HOST_BITS_PER_WIDE_INT))
    {
      if (sgn == UNSIGNED)
	val[len - 1] = zext_hwi (val[len - 1], small_prec);
      else
	val[len - 1] = sext_hwi (val[len - 1], small_prec);
    }
}

/* Return true if X fits in a HOST_WIDE_INT with no loss of precision.  */
inline bool
wi::fits_shwi_p (const wide_int_ref &x)
{
  return x.len == 1;
}

/* Return true if X fits in an unsigned HOST_WIDE_INT with no loss of
   precision.  */
inline bool
wi::fits_uhwi_p (const wide_int_ref &x)
{
  if (x.precision <= HOST_BITS_PER_WIDE_INT)
    return true;
  if (x.len == 1)
    return x.sign_mask () == 0;
  if (x.precision < 2 * HOST_BITS_PER_WIDE_INT)
    return zext_hwi (x.uhigh (), x.precision % HOST_BITS_PER_WIDE_INT) == 0;
  return x.len == 2 && x.uhigh () == 0;
}

/* Return true if X is negative based on the interpretation of SGN.
   For UNSIGNED, this is always false.  */
inline bool
wi::neg_p (const wide_int_ref &x, signop sgn)
{
  if (sgn == UNSIGNED)
    return false;
  if (x.precision == 0)
    return false;
  if (x.len * HOST_BITS_PER_WIDE_INT > x.precision)
    return (x.uhigh () >> (x.precision % HOST_BITS_PER_WIDE_INT - 1)) & 1;
  return x.sign_mask () < 0;
}

/* Return -1 if the top bit of X is set and 0 if the top bit is clear.  */
inline HOST_WIDE_INT
wi::sign_mask (const wide_int_ref &x)
{
  return x.sign_mask ();
}

/* Return true if X == Y.  X and Y must be binary-compatible.  */
template <typename T1, typename T2>
inline bool
wi::eq_p (const T1 &x, const T2 &y)
{
  unsigned int precision = get_binary_precision (x, y);
  if (precision == 0)
    return true;
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      unsigned HOST_WIDE_INT diff = xi.ulow () ^ yi.ulow ();
      return (diff << (HOST_BITS_PER_WIDE_INT - precision)) == 0;
    }
  return eq_p_large (xi.val, xi.len, yi.val, yi.len, precision);
}

/* Return true if X != Y.  X and Y must be binary-compatible.  */
template <typename T1, typename T2>
inline bool
wi::ne_p (const T1 &x, const T2 &y)
{
  return !eq_p (x, y);
}

/* Return true if X < Y when both are treated as signed values.  */
inline bool
wi::lts_p (const wide_int_ref &x, const wide_int_ref &y)
{
  if (x.precision <= HOST_BITS_PER_WIDE_INT
      && y.precision <= HOST_BITS_PER_WIDE_INT)
    {
      HOST_WIDE_INT xl = sext_hwi (x.ulow (), x.precision);
      HOST_WIDE_INT yl = sext_hwi (y.ulow (), y.precision);
      return xl < yl;
    }
  return lts_p_large (x.val, x.len, x.precision, y.val, y.len,
		      y.precision);
}

/* Return true if X < Y when both are treated as unsigned values.  */
inline bool
wi::ltu_p (const wide_int_ref &x, const wide_int_ref &y)
{
  if (x.precision <= HOST_BITS_PER_WIDE_INT
      && y.precision <= HOST_BITS_PER_WIDE_INT)
    {
      unsigned HOST_WIDE_INT xl = zext_hwi (x.ulow (), x.precision);
      unsigned HOST_WIDE_INT yl = zext_hwi (y.ulow (), y.precision);
      return xl < yl;
    }
  return ltu_p_large (x.val, x.len, x.precision, y.val, y.len, y.precision);
}

/* Return true if X < Y.  Signedness of X and Y is indicated by SGN.  */
inline bool
wi::lt_p (const wide_int_ref &x, const wide_int_ref &y, signop sgn)
{
  if (sgn == SIGNED)
    return lts_p (x, y);
  else
    return ltu_p (x, y);
}

/* Return true if X <= Y when both are treated as signed values.  */
inline bool
wi::les_p (const wide_int_ref &x, const wide_int_ref &y)
{
  return !lts_p (y, x);
}

/* Return true if X <= Y when both are treated as unsigned values.  */
inline bool
wi::leu_p (const wide_int_ref &x, const wide_int_ref &y)
{
  return !ltu_p (y, x);
}

/* Return true if X <= Y.  Signedness of X and Y is indicated by SGN.  */
inline bool
wi::le_p (const wide_int_ref &x, const wide_int_ref &y, signop sgn)
{
  if (sgn == SIGNED)
    return les_p (x, y);
  else
    return leu_p (x, y);
}

/* Return true if X > Y when both are treated as signed values.  */
inline bool
wi::gts_p (const wide_int_ref &x, const wide_int_ref &y)
{
  return lts_p (y, x);
}

/* Return true if X > Y when both are treated as unsigned values.  */
inline bool
wi::gtu_p (const wide_int_ref &x, const wide_int_ref &y)
{
  return ltu_p (y, x);
}

/* Return true if X > Y.  Signedness of X and Y is indicated by SGN.  */
inline bool
wi::gt_p (const wide_int_ref &x, const wide_int_ref &y, signop sgn)
{
  if (sgn == SIGNED)
    return gts_p (x, y);
  else
    return gtu_p (x, y);
}

/* Return true if X >= Y when both are treated as signed values.  */
inline bool
wi::ges_p (const wide_int_ref &x, const wide_int_ref &y)
{
  return !lts_p (x, y);
}

/* Return true if X >= Y when both are treated as unsigned values.  */
inline bool
wi::geu_p (const wide_int_ref &x, const wide_int_ref &y)
{
  return !ltu_p (x, y);
}

/* Return true if X >= Y.  Signedness of X and Y is indicated by SGN.  */
inline bool
wi::ge_p (const wide_int_ref &x, const wide_int_ref &y, signop sgn)
{
  if (sgn == SIGNED)
    return ges_p (x, y);
  else
    return geu_p (x, y);
}

/* Return -1 if X < Y, 0 if X == Y and 1 if X > Y.  Treat both X and Y
   as signed values.  */
inline int
wi::cmps (const wide_int_ref &x, const wide_int_ref &y)
{
  if (x.precision <= HOST_BITS_PER_WIDE_INT
      && y.precision <= HOST_BITS_PER_WIDE_INT)
    {
      HOST_WIDE_INT xl = sext_hwi (x.ulow (), x.precision);
      HOST_WIDE_INT yl = sext_hwi (y.ulow (), y.precision);
      if (xl < yl)
	return -1;
      else if (xl > yl)
	return 1;
      else
	return 0;
    }
  return cmps_large (x.val, x.len, x.precision, y.val, y.len,
		     y.precision);
}

/* Return -1 if X < Y, 0 if X == Y and 1 if X > Y.  Treat both X and Y
   as unsigned values.  */
inline int
wi::cmpu (const wide_int_ref &x, const wide_int_ref &y)
{
  if (x.precision <= HOST_BITS_PER_WIDE_INT
      && y.precision <= HOST_BITS_PER_WIDE_INT)
    {
      unsigned HOST_WIDE_INT xl = zext_hwi (x.ulow (), x.precision);
      unsigned HOST_WIDE_INT yl = zext_hwi (y.ulow (), y.precision);
      if (xl < yl)
	return -1;
      else if (xl == yl)
	return 0;
      else
	return 1;
    }
  return cmpu_large (x.val, x.len, x.precision, y.val, y.len,
		     y.precision);
}

/* Return -1 if X < Y, 0 if X == Y and 1 if X > Y.  Signedness of
   X and Y indicated by SGN.  */
inline int
wi::cmp (const wide_int_ref &x, const wide_int_ref &y, signop sgn)
{
  if (sgn == SIGNED)
    return cmps (x, y);
  else
    return cmpu (x, y);
}

/* Return ~x.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::bit_not (const T &x)
{
  WI_UNARY_RESULT_VAR (result, val, T, x);
  wide_int_ref xi (x, get_precision (result));
  for (unsigned int i = 0; i < xi.len; ++i)
    val[i] = ~xi.val[i];
  result.set_len (xi.len);
  return result;
}

/* Return -x.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::neg (const T &x)
{
  return sub (0, x);
}

/* Return -x.  Indicate in *OVERFLOW if X is the minimum signed value.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::neg (const T &x, bool *overflow)
{
  *overflow = only_sign_bit_p (x);
  return sub (0, x);
}

/* Return the absolute value of x.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::abs (const T &x)
{
  if (neg_p (x))
    return neg (x);
  return x;
}

/* Return the result of sign-extending the low OFFSET bits of X.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::sext (const T &x, unsigned int offset)
{
  WI_UNARY_RESULT_VAR (result, val, T, x);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  if (offset <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = sext_hwi (xi.ulow (), offset);
      result.set_len (1);
    }
  else
    result.set_len (sext_large (val, xi.val, xi.len, precision, offset));
  return result;
}

/* Return the result of zero-extending the low OFFSET bits of X.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::zext (const T &x, unsigned int offset)
{
  WI_UNARY_RESULT_VAR (result, val, T, x);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  if (offset < HOST_BITS_PER_WIDE_INT)
    {
      val[0] = zext_hwi (xi.ulow (), offset);
      result.set_len (1);
    }
  else
    result.set_len (zext_large (val, xi.val, xi.len, precision, offset));
  return result;
}

/* Return the result of extending the low OFFSET bits of X according to
   signedness SGN.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::ext (const T &x, unsigned int offset, signop sgn)
{
  return sgn == SIGNED ? sext (x, offset) : zext (x, offset);
}

/* Return an integer that represents X | (1 << bit).  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::set_bit (const T &x, unsigned int bit)
{
  WI_UNARY_RESULT_VAR (result, val, T, x);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = xi.ulow () | ((unsigned HOST_WIDE_INT) 1 << bit);
      result.set_len (1);
    }
  else
    result.set_len (set_bit_large (val, xi.val, xi.len, precision, bit));
  return result;
}

/* Return the mininum of X and Y, treating them both as having
   signedness SGN.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::min (const T1 &x, const T2 &y, signop sgn)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  if (wi::le_p (x, y, sgn))
    {
      wide_int_ref xi (x, precision);
      for (unsigned int i = 0; i < xi.len; ++i)
	val[i] = xi.val[i];
      result.set_len (xi.len);
    }
  else
    {
      wide_int_ref yi (y, precision);
      for (unsigned int i = 0; i < yi.len; ++i)
	val[i] = yi.val[i];
      result.set_len (yi.len);
    }
  return result;
}

/* Return the minimum of X and Y, treating both as signed values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::smin (const T1 &x, const T2 &y)
{
  return min (x, y, SIGNED);
}

/* Return the minimum of X and Y, treating both as unsigned values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::umin (const T1 &x, const T2 &y)
{
  return min (x, y, UNSIGNED);
}

/* Return the maxinum of X and Y, treating them both as having
   signedness SGN.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::max (const T1 &x, const T2 &y, signop sgn)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  if (wi::ge_p (x, y, sgn))
    {
      wide_int_ref xi (x, precision);
      for (unsigned int i = 0; i < xi.len; ++i)
	val[i] = xi.val[i];
      result.set_len (xi.len);
    }
  else
    {
      wide_int_ref yi (y, precision);
      for (unsigned int i = 0; i < yi.len; ++i)
	val[i] = yi.val[i];
      result.set_len (yi.len);
    }
  return result;
}

/* Return the maximum of X and Y, treating both as signed values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::smax (const T1 &x, const T2 &y)
{
  return max (x, y, SIGNED);
}

/* Return the maximum of X and Y, treating both as unsigned values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::umax (const T1 &x, const T2 &y)
{
  return max (x, y, UNSIGNED);
}

/* Return X & Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::bit_and (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (xi.len + yi.len == 2)
    {
      val[0] = xi.ulow () & yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (and_large (val, xi.val, xi.len, yi.val, yi.len,
			       precision));
  return result;
}

/* Return X & ~Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::bit_and_not (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (xi.len + yi.len == 2)
    {
      val[0] = xi.ulow () & ~yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (and_not_large (val, xi.val, xi.len, yi.val, yi.len,
				   precision));
  return result;
}

/* Return X | Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::bit_or (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = xi.ulow () | yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (or_large (val, xi.val, xi.len, yi.val, yi.len, precision));
  return result;
}

/* Return X | ~Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::bit_or_not (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (xi.len + yi.len == 2)
    {
      val[0] = xi.ulow () | ~yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (or_not_large (val, xi.val, xi.len, yi.val, yi.len,
				  precision));
  return result;
}

/* Return X ^ Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::bit_xor (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (xi.len + yi.len == 2)
    {
      val[0] = xi.ulow () ^ yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (xor_large (val, xi.val, xi.len, yi.val, yi.len, precision));
  return result;
}

/* Return X + Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::add (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = xi.ulow () + yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (add_large (val, xi.val, xi.len, yi.val, yi.len, precision,
			       UNSIGNED, 0));
  return result;
}

/* Return X + Y.  Treat X and Y as having the signednes given by SGN
   and indicate in *OVERFLOW whether the operation overflowed.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::add (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      unsigned HOST_WIDE_INT xl = xi.ulow ();
      unsigned HOST_WIDE_INT yl = yi.ulow ();
      unsigned HOST_WIDE_INT resultl = xl + yl;
      if (precision == 0)
	*overflow = false;
      else if (sgn == SIGNED)
	*overflow = (((resultl ^ xl) & (resultl ^ yl)) >> (precision - 1)) & 1;
      else
	*overflow = ((resultl << (HOST_BITS_PER_WIDE_INT - precision))
		     < (xl << (HOST_BITS_PER_WIDE_INT - precision)));
      val[0] = resultl;
      result.set_len (1);
    }
  else
    result.set_len (add_large (val, xi.val, xi.len, yi.val, yi.len, precision,
			       sgn, overflow));
  return result;
}

/* Return X - Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::sub (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = xi.ulow () - yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (sub_large (val, xi.val, xi.len, yi.val, yi.len, precision,
			       UNSIGNED, 0));
  return result;
}

/* Return X - Y.  Treat X and Y as having the signednes given by SGN
   and indicate in *OVERFLOW whether the operation overflowed.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::sub (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      unsigned HOST_WIDE_INT xl = xi.ulow ();
      unsigned HOST_WIDE_INT yl = yi.ulow ();
      unsigned HOST_WIDE_INT resultl = xl - yl;
      if (precision == 0)
	*overflow = false;
      else if (sgn == SIGNED)
	*overflow = (((xl ^ yl) & (resultl ^ xl)) >> (precision - 1)) & 1;
      else
	*overflow = ((resultl << (HOST_BITS_PER_WIDE_INT - precision))
		     > (xl << (HOST_BITS_PER_WIDE_INT - precision)));
      val[0] = resultl;
      result.set_len (1);
    }
  else
    result.set_len (sub_large (val, xi.val, xi.len, yi.val, yi.len, precision,
			       sgn, overflow));
  return result;
}

/* Return X * Y.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::mul (const T1 &x, const T2 &y)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = xi.ulow () * yi.ulow ();
      result.set_len (1);
    }
  else
    result.set_len (mul_internal (val, xi.val, xi.len, yi.val, yi.len,
				  precision, UNSIGNED, 0, false, false));
  return result;
}

/* Return X * Y.  Treat X and Y as having the signednes given by SGN
   and indicate in *OVERFLOW whether the operation overflowed.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::mul (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  result.set_len (mul_internal (val, xi.val, xi.len, yi.val, yi.len, precision,
				sgn, overflow, false, false));
  return result;
}

/* Return X * Y, treating both X and Y as signed values.  Indicate in
   *OVERFLOW whether the operation overflowed.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::smul (const T1 &x, const T2 &y, bool *overflow)
{
  return mul (x, y, SIGNED, overflow);
}

/* Return X * Y, treating both X and Y as unsigned values.  Indicate in
   *OVERFLOW whether the operation overflowed.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::umul (const T1 &x, const T2 &y, bool *overflow)
{
  return mul (x, y, UNSIGNED, overflow);
}

/* Perform a widening multiplication of X and Y, extending the values
   according to SGN, and return the high part of the result.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::mul_high (const T1 &x, const T2 &y, signop sgn)
{
  WI_BINARY_RESULT_VAR (result, val, T1, x, T2, y);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y, precision);
  result.set_len (mul_internal (val, xi.val, xi.len, yi.val, yi.len, precision,
				sgn, 0, true, false));
  return result;
}

/* Return X / Y, rouding towards 0.  Treat X and Y as having the
   signedness given by SGN.  Indicate in *OVERFLOW if the result
   overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::div_trunc (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  quotient.set_len (divmod_internal (quotient_val, 0, 0, xi.val, xi.len,
				     precision,
				     yi.val, yi.len, yi.precision,
				     sgn, overflow));
  return quotient;
}

/* Return X / Y, rouding towards 0.  Treat X and Y as signed values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::sdiv_trunc (const T1 &x, const T2 &y)
{
  return div_trunc (x, y, SIGNED);
}

/* Return X / Y, rouding towards 0.  Treat X and Y as unsigned values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::udiv_trunc (const T1 &x, const T2 &y)
{
  return div_trunc (x, y, UNSIGNED);
}

/* Return X / Y, rouding towards -inf.  Treat X and Y as having the
   signedness given by SGN.  Indicate in *OVERFLOW if the result
   overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::div_floor (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  quotient.set_len (divmod_internal (quotient_val, &remainder_len,
				     remainder_val, xi.val, xi.len, precision,
				     yi.val, yi.len, yi.precision, sgn,
				     overflow));
  remainder.set_len (remainder_len);
  if (wi::neg_p (quotient, sgn) && remainder != 0)
    return quotient + 1;
  return quotient;
}

/* Return X / Y, rouding towards -inf.  Treat X and Y as signed values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::sdiv_floor (const T1 &x, const T2 &y)
{
  return div_floor (x, y, SIGNED);
}

/* Return X / Y, rouding towards -inf.  Treat X and Y as unsigned values.  */
/* ??? Why do we have both this and udiv_trunc.  Aren't they the same?  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::udiv_floor (const T1 &x, const T2 &y)
{
  return div_floor (x, y, UNSIGNED);
}

/* Return X / Y, rouding towards +inf.  Treat X and Y as having the
   signedness given by SGN.  Indicate in *OVERFLOW if the result
   overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::div_ceil (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  quotient.set_len (divmod_internal (quotient_val, &remainder_len,
				     remainder_val, xi.val, xi.len, precision,
				     yi.val, yi.len, yi.precision, sgn,
				     overflow));
  remainder.set_len (remainder_len);
  if (!wi::neg_p (quotient, sgn) && remainder != 0)
    return quotient + 1;
  return quotient;
}

/* Return X / Y, rouding towards nearest with ties away from zero.
   Treat X and Y as having the signedness given by SGN.  Indicate
   in *OVERFLOW if the result overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::div_round (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  quotient.set_len (divmod_internal (quotient_val, &remainder_len,
				     remainder_val, xi.val, xi.len, precision,
				     yi.val, yi.len, yi.precision, sgn,
				     overflow));
  remainder.set_len (remainder_len);

  if (remainder != 0)
    {
      if (sgn == SIGNED)
	{
	  if (wi::gts_p (wi::lrshift (wi::abs (y), 1),
			 wi::abs (remainder)))
	    {
	      if (wi::neg_p (quotient))
		return quotient - 1;
	      else
		return quotient + 1;
	    }
	}
      else
	{
	  if (wi::gtu_p (wi::lrshift (y, 1), remainder))
	    return quotient + 1;
	}
    }
  return quotient;
}

/* Return X / Y, rouding towards nearest with ties away from zero.
   Treat X and Y as having the signedness given by SGN.  Store the
   remainder in *REMAINDER_PTR.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::divmod_trunc (const T1 &x, const T2 &y, signop sgn,
		  WI_BINARY_RESULT (T1, T2) *remainder_ptr)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  quotient.set_len (divmod_internal (quotient_val, &remainder_len,
				     remainder_val, xi.val, xi.len, precision,
				     yi.val, yi.len, yi.precision, sgn, 0));
  remainder.set_len (remainder_len);

  *remainder_ptr = remainder;
  return quotient;
}

/* Compute X / Y, rouding towards 0, and return the remainder.
   Treat X and Y as having the signedness given by SGN.  Indicate
   in *OVERFLOW if the division overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::mod_trunc (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (remainder);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  divmod_internal (0, &remainder_len, remainder_val, xi.val, xi.len, precision,
		   yi.val, yi.len, yi.precision, sgn, overflow);
  remainder.set_len (remainder_len);

  return remainder;
}

/* Compute X / Y, rouding towards 0, and return the remainder.
   Treat X and Y as signed values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::smod_trunc (const T1 &x, const T2 &y)
{
  return mod_trunc (x, y, SIGNED);
}

/* Compute X / Y, rouding towards 0, and return the remainder.
   Treat X and Y as unsigned values.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::umod_trunc (const T1 &x, const T2 &y)
{
  return mod_trunc (x, y, UNSIGNED);
}

/* Compute X / Y, rouding towards -inf, and return the remainder.
   Treat X and Y as having the signedness given by SGN.  Indicate
   in *OVERFLOW if the division overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::mod_floor (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  quotient.set_len (divmod_internal (quotient_val, &remainder_len,
				     remainder_val, xi.val, xi.len, precision,
				     yi.val, yi.len, yi.precision, sgn,
				     overflow));
  remainder.set_len (remainder_len);

  if (wi::neg_p (quotient, sgn) && remainder != 0)
    return remainder + y;
  return remainder;
}

/* Compute X / Y, rouding towards -inf, and return the remainder.
   Treat X and Y as unsigned values.  */
/* ??? Why do we have both this and umod_trunc.  Aren't they the same?  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::umod_floor (const T1 &x, const T2 &y)
{
  return mod_floor (x, y, UNSIGNED);
}

/* Compute X / Y, rouding towards +inf, and return the remainder.
   Treat X and Y as having the signedness given by SGN.  Indicate
   in *OVERFLOW if the division overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::mod_ceil (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  quotient.set_len (divmod_internal (quotient_val, &remainder_len,
				     remainder_val, xi.val, xi.len, precision,
				     yi.val, yi.len, yi.precision, sgn,
				     overflow));
  remainder.set_len (remainder_len);

  if (!wi::neg_p (quotient, sgn) && remainder != 0)
    return remainder - y;
  return remainder;
}

/* Compute X / Y, rouding towards nearest with ties away from zero,
   and return the remainder.  Treat X and Y as having the signedness
   given by SGN.  Indicate in *OVERFLOW if the division overflows.  */
template <typename T1, typename T2>
inline WI_BINARY_RESULT (T1, T2)
wi::mod_round (const T1 &x, const T2 &y, signop sgn, bool *overflow)
{
  WI_BINARY_RESULT_VAR (quotient, quotient_val, T1, x, T2, y);
  WI_BINARY_RESULT_VAR (remainder, remainder_val, T1, x, T2, y);
  unsigned int precision = get_precision (quotient);
  wide_int_ref xi (x, precision);
  wide_int_ref yi (y);

  unsigned int remainder_len;
  quotient.set_len (divmod_internal (quotient_val, &remainder_len,
				     remainder_val, xi.val, xi.len, precision,
				     yi.val, yi.len, yi.precision, sgn,
				     overflow));
  remainder.set_len (remainder_len);

  if (remainder != 0)
    {
      if (sgn == SIGNED)
	{
	  if (wi::gts_p (wi::lrshift (wi::abs (y), 1),
			 wi::abs (remainder)))
	    {
	      if (wi::neg_p (quotient))
		return remainder + y;
	      else
		return remainder - y;
	    }
	}
      else
	{
	  if (wi::gtu_p (wi::lrshift (y, 1), remainder))
	    return remainder - y;
	}
    }
  return remainder;
}

/* Return true if X is a multiple of Y, storing X / Y in *RES if so.
   Treat X and Y as having the signedness given by SGN.  */
template <typename T1, typename T2>
inline bool
wi::multiple_of_p (const T1 &x, const T2 &y, signop sgn,
		   WI_BINARY_RESULT (T1, T2) *res)
{
  WI_BINARY_RESULT (T1, T2) remainder;
  WI_BINARY_RESULT (T1, T2) quotient = divmod_trunc (x, y, sgn, &remainder);
  if (remainder == 0)
    {
      *res = quotient;
      return true;
    }
  return false;
}

/* Truncate the value of shift value X so that the value is within BITSIZE.
   PRECISION is the number of bits in the value being shifted.  */
inline unsigned int
wi::trunc_shift (const wide_int_ref &x, unsigned int bitsize,
		 unsigned int precision)
{
  if (bitsize == 0)
    {
      gcc_checking_assert (!neg_p (x));
      if (geu_p (x, precision))
	return precision;
    }
  /* Flush out undefined bits.  */
  unsigned int shift = x.ulow ();
  if (x.precision < HOST_BITS_PER_WIDE_INT)
    shift = zext_hwi (shift, x.precision);
  return shift & (bitsize - 1);
}

/* Return X << Y.  If BITSIZE is nonzero, only use the low BITSIZE bits
   of Y.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::lshift (const T &x, const wide_int_ref &y, unsigned int bitsize)
{
  WI_UNARY_RESULT_VAR (result, val, T, x);
  unsigned int precision = get_precision (result);
  wide_int_ref xi (x, precision);
  unsigned int shift = trunc_shift (y, bitsize, precision);
  /* Handle the simple cases quickly.   */
  if (shift >= precision)
    {
      val[0] = 0;
      result.set_len (1);
    }
  else if (precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = xi.ulow () << shift;
      result.set_len (1);
    }
  else
    result.set_len (lshift_large (val, xi.val, xi.len, precision, shift));
  return result;
}

/* Return X >> Y, using a logical shift.  If BITSIZE is nonzero, only use
   the low BITSIZE bits of Y.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::lrshift (const T &x, const wide_int_ref &y, unsigned int bitsize)
{
  WI_UNARY_RESULT_VAR (result, val, T, x);
  /* Do things in the precision of the input rather than the output,
     since the result can be no larger than that.  */
  wide_int_ref xi (x);
  unsigned int shift = trunc_shift (y, bitsize, xi.precision);
  /* Handle the simple cases quickly.   */
  if (shift >= xi.precision)
    {
      val[0] = 0;
      result.set_len (1);
    }
  else if (xi.precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = zext_hwi (xi.ulow (), xi.precision) >> shift;
      result.set_len (1);
    }
  else
    result.set_len (lrshift_large (val, xi.val, xi.len, xi.precision,
				   get_precision (result), shift));
  return result;
}

/* Return X >> Y, using an arithmetic shift.  If BITSIZE is nonzero, only use
   the low BITSIZE bits of Y.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::arshift (const T &x, const wide_int_ref &y, unsigned int bitsize)
{
  WI_UNARY_RESULT_VAR (result, val, T, x);
  /* Do things in the precision of the input rather than the output,
     since the result can be no larger than that.  */
  wide_int_ref xi (x);
  unsigned int shift = trunc_shift (y, bitsize, xi.precision);
  /* Handle the simple case quickly.   */
  if (shift >= xi.precision)
    {
      val[0] = sign_mask (x);
      result.set_len (1);
    }
  else if (xi.precision <= HOST_BITS_PER_WIDE_INT)
    {
      val[0] = sext_hwi (zext_hwi (xi.ulow (), xi.precision) >> shift,
			 xi.precision - shift);
      result.set_len (1);
    }
  else
    result.set_len (arshift_large (val, xi.val, xi.len, xi.precision,
				   get_precision (result), shift));
  return result;
}

/* Return X >> Y, using an arithmetic shift if SGN is SIGNED and a logical
   shift otherwise.  If BITSIZE is nonzero, only use the low BITSIZE bits
   of Y.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::rshift (const T &x, const wide_int_ref &y, signop sgn,
	    unsigned int bitsize)
{
  if (sgn == UNSIGNED)
    return lrshift (x, y, bitsize);
  else
    return arshift (x, y, bitsize);
}

/* Return the result of rotating the low WIDTH bits of X left by Y bits
   and zero-extending the result.  Use a full-width rotate if WIDTH is
   zero.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::lrotate (const T &x, const wide_int_ref &y, unsigned int width)
{
  unsigned int precision = get_binary_precision (x, x);
  if (width == 0)
    width = precision;
  gcc_checking_assert ((width & -width) == width);
  WI_UNARY_RESULT (T) left = wi::lshift (x, y, width);
  WI_UNARY_RESULT (T) right = wi::lrshift (x, wi::sub (width, y), width);
  if (width != precision)
    return wi::zext (left, width) | wi::zext (right, width);
  return left | right;
}

/* Return the result of rotating the low WIDTH bits of X right by Y bits
   and zero-extending the result.  Use a full-width rotate if WIDTH is
   zero.  */
template <typename T>
inline WI_UNARY_RESULT (T)
wi::rrotate (const T &x, const wide_int_ref &y, unsigned int width)
{
  unsigned int precision = get_binary_precision (x, x);
  if (width == 0)
    width = precision;
  gcc_checking_assert ((width & -width) == width);
  WI_UNARY_RESULT (T) right = wi::lrshift (x, y, width);
  WI_UNARY_RESULT (T) left = wi::lshift (x, wi::sub (width, y), width);
  if (width != precision)
    return wi::zext (left, width) | wi::zext (right, width);
  return left | right;
}

/* Return 0 if the number of 1s in X is even and 1 if the number of 1s
   is odd.  */
inline int
wi::parity (const wide_int_ref &x)
{
  return popcount (x) & 1;
}

/* Extract WIDTH bits from X, starting at BITPOS.  */
template <typename T>
inline unsigned HOST_WIDE_INT
wi::extract_uhwi (const T &x, unsigned int bitpos,
		  unsigned int width)
{
  unsigned precision = get_precision (x);
  if (precision < bitpos + width)
    precision = bitpos + width;
  wide_int_ref xi (x, precision);

  /* Handle this rare case after the above, so that we assert about
     bogus BITPOS values.  */
  if (width == 0)
    return 0;

  unsigned int start = bitpos / HOST_BITS_PER_WIDE_INT;
  unsigned int shift = bitpos % HOST_BITS_PER_WIDE_INT;
  unsigned HOST_WIDE_INT res = xi.elt (start);
  res >>= shift;
  if (shift + width > HOST_BITS_PER_WIDE_INT)
    {
      unsigned HOST_WIDE_INT upper = xi.elt (start + 1);
      res |= upper << (-shift % HOST_BITS_PER_WIDE_INT);
    }
  return zext_hwi (res, width);
}

template<typename T>
void
gt_ggc_mx (generic_wide_int <T> *)
{
}

template<typename T>
void
gt_pch_nx (generic_wide_int <T> *)
{
}

template<typename T>
void
gt_pch_nx (generic_wide_int <T> *, void (*) (void *, void *), void *)
{
}

namespace wi
{
  /* Used for overloaded functions in which the only other acceptable
     scalar type is a pointer.  It stops a plain 0 from being treated
     as a null pointer.  */
  struct never_used1 {};
  struct never_used2 {};

  wide_int min_value (unsigned int, signop);
  wide_int min_value (never_used1 *);
  wide_int min_value (never_used2 *);
  wide_int max_value (unsigned int, signop);
  wide_int max_value (never_used1 *);
  wide_int max_value (never_used2 *);

  wide_int mul_full (const wide_int_ref &, const wide_int_ref &, signop);

  /* FIXME: this is target dependent, so should be elsewhere.
     It also seems to assume that CHAR_BIT == BITS_PER_UNIT.  */
  wide_int from_buffer (const unsigned char *, unsigned int);

#ifndef GENERATOR_FILE
  void to_mpz (wide_int, mpz_t, signop);
#endif

  wide_int mask (unsigned int, bool, unsigned int);
  wide_int shifted_mask (unsigned int, unsigned int, bool, unsigned int);
  wide_int set_bit_in_zero (unsigned int, unsigned int);
  wide_int insert (const wide_int &x, const wide_int &y, unsigned int,
		   unsigned int);

  template <typename T>
  T mask (unsigned int, bool);

  template <typename T>
  T shifted_mask (unsigned int, unsigned int, bool);

  template <typename T>
  T set_bit_in_zero (unsigned int);

  unsigned int mask (HOST_WIDE_INT *, unsigned int, bool, unsigned int);
  unsigned int shifted_mask (HOST_WIDE_INT *, unsigned int, unsigned int,
			     bool, unsigned int);
  unsigned int from_array (HOST_WIDE_INT *, const HOST_WIDE_INT *,
			   unsigned int, unsigned int, bool);
}

/* Perform a widening multiplication of X and Y, extending the values
   according according to SGN.  */
inline wide_int
wi::mul_full (const wide_int_ref &x, const wide_int_ref &y, signop sgn)
{
  gcc_checking_assert (x.precision == y.precision);
  wide_int result = wide_int::create (x.precision * 2);
  result.set_len (mul_internal (result.write_val (), x.val, x.len,
				y.val, y.len, x.precision,
				sgn, 0, false, true));
  return result;
}

/* Return a PRECISION-bit integer in which the low WIDTH bits are set
   and the other bits are clear, or the inverse if NEGATE_P.  */
inline wide_int
wi::mask (unsigned int width, bool negate_p, unsigned int precision)
{
  wide_int result = wide_int::create (precision);
  result.set_len (mask (result.write_val (), width, negate_p, precision));
  return result;
}

/* Return a PRECISION-bit integer in which the low START bits are clear,
   the next WIDTH bits are set, and the other bits are clear,
   or the inverse if NEGATE_P.  */
inline wide_int
wi::shifted_mask (unsigned int start, unsigned int width, bool negate_p,
		  unsigned int precision)
{
  wide_int result = wide_int::create (precision);
  result.set_len (shifted_mask (result.write_val (), start, width, negate_p,
				precision));
  return result;
}

/* Return a PRECISION-bit integer in which bit BIT is set and all the
   others are clear.  */
inline wide_int
wi::set_bit_in_zero (unsigned int bit, unsigned int precision)
{
  return shifted_mask (bit, 1, false, precision);
}

/* Return an integer of type T in which the low WIDTH bits are set
   and the other bits are clear, or the inverse if NEGATE_P.  */
template <typename T>
inline T
wi::mask (unsigned int width, bool negate_p)
{
  STATIC_ASSERT (wi::int_traits<T>::precision);
  T result;
  result.set_len (mask (result.write_val (), width, negate_p,
			wi::int_traits <T>::precision));
  return result;
}

/* Return an integer of type T in which the low START bits are clear,
   the next WIDTH bits are set, and the other bits are clear,
   or the inverse if NEGATE_P.  */
template <typename T>
inline T
wi::shifted_mask (unsigned int start, unsigned int width, bool negate_p)
{
  STATIC_ASSERT (wi::int_traits<T>::precision);
  T result;
  result.set_len (shifted_mask (result.write_val (), start, width, negate_p,
				wi::int_traits <T>::precision));
  return result;
}

/* Return an integer of type T in which bit BIT is set and all the
   others are clear.  */
template <typename T>
inline T
wi::set_bit_in_zero (unsigned int bit)
{
  return shifted_mask <T> (bit, 1, false);
}

#endif /* WIDE_INT_H */
