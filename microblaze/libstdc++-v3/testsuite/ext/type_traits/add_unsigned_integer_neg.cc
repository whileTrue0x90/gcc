// { dg-do compile }
// -*- C++ -*-

// Copyright (C) 2006, 2007 Free Software Foundation, Inc.
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

#include <ext/type_traits.h>
#include <tr1/type_traits>

template<typename T>
  void
  check_add_unsigned()
  {
    typedef typename __gnu_cxx::__add_unsigned<T>::__type unsigned_type;
  }

int main()
{
  check_add_unsigned<bool>();  // { dg-error "instantiated from" }
  check_add_unsigned<wchar_t>();  // { dg-error "instantiated from" }
  return 0;
}

// { dg-error "invalid use of incomplete" "" { target *-*-* } 29 } 
// { dg-error "declaration of" "" { target *-*-* } 68 } 
