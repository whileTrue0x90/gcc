// Copyright (C) 2016 Free Software Foundation, Inc.
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

// { dg-options "-std=gnu++11" }
// { dg-do compile }

#include <set>

using stype1 = std::set<int>;
static_assert(std::is_nothrow_default_constructible<stype1>::value, "Error");

struct cmp
{
  cmp() { }
  bool operator()(int, int) const;
};

using stype2 = std::set<int, cmp>;
static_assert( !std::is_nothrow_default_constructible<stype2>::value, "Error");
