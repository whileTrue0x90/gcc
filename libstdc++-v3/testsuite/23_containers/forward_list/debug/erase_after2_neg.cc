// { dg-options "-std=gnu++0x" }
// { dg-require-debug-mode "" }
// { dg-do run { xfail *-*-* } }

// Copyright (C) 2010 Free Software Foundation, Inc.
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

#include <forward_list>

void
test01()
{
  std::forward_list<int> fl1{1, 2, 3};
  fl1.erase_after(fl1.end());
}

int
main()
{
  test01();
  return 0;
}
