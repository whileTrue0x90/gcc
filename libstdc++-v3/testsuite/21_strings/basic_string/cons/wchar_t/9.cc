// Copyright (C) 2016-2017 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// { dg-options "-std=gnu++17" }
// { dg-do run { target c++17 } }

#include <string>
#include <testsuite_hooks.h>

void
test01()
{
  using C = wchar_t;
  using string_type = std::basic_string<C>;
  using view_type = std::basic_string_view<C>;

  std::allocator<C> alloc;
  VERIFY( string_type(view_type(L"string")) == L"string" );
  VERIFY( string_type(view_type(L"string"), alloc) == L"string" );

  // LWG 2742
  VERIFY( string_type(L"substring", 3, 6) == L"string" );
  VERIFY( string_type(L"substring", 3, 6, alloc) == L"string" );
  VERIFY( string_type(view_type(L"substring"), 3, 6) == L"string" );
  VERIFY( string_type(view_type(L"substring"), 3, 6, alloc) == L"string" );
}

int
main()
{
  test01();
}
