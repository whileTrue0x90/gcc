// { dg-do compile { target c++14 } }

// Copyright (C) 2014-2019 Free Software Foundation, Inc.
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

#include <experimental/optional>
#include <type_traits>
#include <stdexcept>

template<typename T>
  using check1_t = std::experimental::fundamentals_v1::optional<T>;

using check2_t = std::experimental::fundamentals_v1::in_place_t;
using check3_t = std::experimental::fundamentals_v1::nullopt_t;
using check4_t = std::experimental::fundamentals_v1::bad_optional_access;

static_assert(std::is_base_of<std::logic_error, check4_t>::value,
	      "bad_optional_access must derive from logic_error");
