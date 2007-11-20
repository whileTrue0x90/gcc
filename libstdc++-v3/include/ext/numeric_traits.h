// -*- C++ -*-

// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software
// Foundation; either version 2, or (at your option) any later
// version.

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free
// software library without restriction.  Specifically, if other files
// instantiate templates or use macros or inline functions from this
// file, or you compile this file and link it with other files to
// produce an executable, this file does not by itself cause the
// resulting executable to be covered by the GNU General Public
// License.  This exception does not however invalidate any other
// reasons why the executable file might be covered by the GNU General
// Public License.

/** @file ext/numeric_traits.h
 *  This file is a GNU extension to the Standard C++ Library.
 */

#ifndef _EXT_NUMERIC_TRAITS
#define _EXT_NUMERIC_TRAITS 1

#pragma GCC system_header

#include <bits/cpp_type_traits.h>
#include <ext/type_traits.h>

_GLIBCXX_BEGIN_NAMESPACE(__gnu_cxx)

  // Compile time constants for builtin types.
  // Sadly std::numeric_limits member functions cannot be used for this.
#define __glibcxx_signed(_Tp) ((_Tp)(-1) < 0)
#define __glibcxx_digits(_Tp) \
  (sizeof(_Tp) * __CHAR_BIT__ - __glibcxx_signed(_Tp))

#define __glibcxx_min(_Tp) \
  (__glibcxx_signed(_Tp) ? (_Tp)1 << __glibcxx_digits(_Tp) : (_Tp)0)

#define __glibcxx_max(_Tp) \
  (__glibcxx_signed(_Tp) ? \
   (((((_Tp)1 << (__glibcxx_digits(_Tp) - 1)) - 1) << 1) + 1) : ~(_Tp)0)

  template<typename _Value>
    struct __numeric_traits_integer
    {
      // Only integers for initialization of member constant.
      static const _Value __min = __glibcxx_min(_Value);
      static const _Value __max = __glibcxx_max(_Value);

      // NB: these two also available in std::numeric_limits as compile
      // time constants, but <limits> is big and we avoid including it.
      static const bool __is_signed = __glibcxx_signed(_Value);
      static const int __digits = __glibcxx_digits(_Value);      
    };

  template<typename _Value>
    const _Value __numeric_traits_integer<_Value>::__min;

  template<typename _Value>
    const _Value __numeric_traits_integer<_Value>::__max;

  template<typename _Value>
    const bool __numeric_traits_integer<_Value>::__is_signed;

  template<typename _Value>
    const int __numeric_traits_integer<_Value>::__digits;

#undef __glibcxx_signed
#undef __glibcxx_digits
#undef __glibcxx_min
#undef __glibcxx_max

#define __glibcxx_floating(_Tp, _Fval, _Dval, _LDval) \
  (std::__are_same<_Tp, float>::__value ? _Fval \
   : std::__are_same<_Tp, double>::__value ? _Dval : _LDval)

#define __glibcxx_max_digits10(_Tp) \
  (2 + __glibcxx_floating(_Tp, __FLT_MANT_DIG__, __DBL_MANT_DIG__, \
			  __LDBL_MANT_DIG__) * 3010 / 10000)

#define __glibcxx_digits10(_Tp) \
  __glibcxx_floating(_Tp, __FLT_DIG__, __DBL_DIG__, __LDBL_DIG__)

#define __glibcxx_max_exponent10(_Tp) \
  __glibcxx_floating(_Tp, __FLT_MAX_10_EXP__, __DBL_MAX_10_EXP__, \
		     __LDBL_MAX_10_EXP__)

  template<typename _Value>
    struct __numeric_traits_floating
    {
      // Only floating point types. See N1822. 
      static const int __max_digits10 = __glibcxx_max_digits10(_Value);

      // See above comment...
      static const bool __is_signed = true;
      static const int __digits10 = __glibcxx_digits10(_Value);
      static const int __max_exponent10 = __glibcxx_max_exponent10(_Value);
    };

  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__max_digits10;

  template<typename _Value>
    const bool __numeric_traits_floating<_Value>::__is_signed;

  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__digits10;

  template<typename _Value>
    const int __numeric_traits_floating<_Value>::__max_exponent10;

  template<typename _Value>
    struct __numeric_traits
    : public __conditional_type<std::__is_integer<_Value>::__value,
				__numeric_traits_integer<_Value>,
				__numeric_traits_floating<_Value> >::__type
    { };

_GLIBCXX_END_NAMESPACE

#undef __glibcxx_floating
#undef __glibcxx_max_digits10
#undef __glibcxx_digits10
#undef __glibcxx_max_exponent10

#endif 
