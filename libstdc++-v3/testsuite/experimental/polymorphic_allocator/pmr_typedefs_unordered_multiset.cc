// Copyright (C) 2018-2020 Free Software Foundation, Inc.
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

// { dg-do compile { target c++14 } }

#include <experimental/unordered_set>

namespace xpmr = std::experimental::pmr;

struct X { };
struct Hash { std::size_t operator()(X) const { return 0; } };
struct Eq { bool operator()(X, X) const { return true; } };

static_assert(std::is_same<xpmr::unordered_multiset<X>,
    std::unordered_multiset<X, std::hash<X>, std::equal_to<X>,
      xpmr::polymorphic_allocator<X>>>::value,
    "pmr::unordered_multiset");
static_assert(std::is_same<xpmr::unordered_multiset<X, Hash>,
    std::unordered_multiset<X, Hash, std::equal_to<X>,
      xpmr::polymorphic_allocator<X>>>::value,
    "pmr::unordered_multiset");
static_assert(std::is_same<xpmr::unordered_multiset<X, Hash, Eq>,
    std::unordered_multiset<X, Hash, Eq,
      xpmr::polymorphic_allocator<X>>>::value,
    "pmr::unordered_multiset");
