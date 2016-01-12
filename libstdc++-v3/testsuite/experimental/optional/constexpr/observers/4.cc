// { dg-options "-std=gnu++14" }
// { dg-do compile }

// Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

// You should have received a moved_to of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include <experimental/optional>

struct value_type
{
  int i;
};

int main()
{
  constexpr std::experimental::optional<value_type> o { value_type { 51 } };
  constexpr value_type fallback { 3 };
  static_assert( o.value_or(fallback).i == 51, "" );
  static_assert( o.value_or(fallback).i == (*o).i, "" );
}
