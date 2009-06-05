// Copyright (C) 2006, 2007, 2008, 2009 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without Pred the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// 6.3.4.4  Class template unordered_map

#include <tr1/unordered_map>
#include <testsuite_hooks.h>

// libstdc++/29134
void test01()
{
  bool test __attribute__((unused)) = true;

  std::tr1::unordered_map<int, int> um;

  VERIFY( (um.max_size() == std::allocator<std::tr1::__detail::_Hash_node<
 	   std::pair<const int, int>, false> >().max_size()));
}

int main()
{
  test01();
  return 0;
}
