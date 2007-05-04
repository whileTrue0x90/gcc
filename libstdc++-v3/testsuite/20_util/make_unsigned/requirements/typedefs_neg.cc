// { dg-do compile }
// { dg-options "-std=gnu++0x" }

// 2007-05-03  Benjamin Kosnik  <bkoz@redhat.com>
//
// Copyright (C) 2007 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

#include <type_traits>
#include <testsuite_character.h>

enum test_enum { first_selection };

void test01()
{
  using std::make_unsigned;

  // Negative  tests.
  typedef make_unsigned<bool>::type     	test1_type;

  typedef make_unsigned<__gnu_test::pod_uint>::type     	test2_type;

  typedef make_unsigned<int[4]>::type     test3_type;

  typedef void (fn_type) ();
  typedef make_unsigned<fn_type>::type  	test4_type;
}

// { dg-error "does not name a type" "" { target *-*-* } 34 }
// { dg-error "instantiated from here" "" { target *-*-* } 36 }
// { dg-error "instantiated from here" "" { target *-*-* } 38 }
// { dg-error "instantiated from here" "" { target *-*-* } 41 }

// { dg-error "invalid use of incomplete type" "" { target *-*-* } 223 }
// { dg-error "declaration of" "" { target *-*-* } 170 }

// { dg-excess-errors "At global scope" }
// { dg-excess-errors "In instantiation of" }
