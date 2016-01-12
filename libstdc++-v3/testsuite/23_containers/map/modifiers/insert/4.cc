// { dg-options "-std=gnu++11" }

// 2010-11-10  Paolo Carlini  <paolo.carlini@oracle.com> 
//
// Copyright (C) 2010-2016 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

#include <iterator>
#include <map>
#include <testsuite_hooks.h>
#include <testsuite_rvalref.h>

void test01()
{
  bool test __attribute__((unused)) = true;
  using __gnu_test::rvalstruct;

  typedef std::map<int, rvalstruct> Map;
  typedef std::pair<const int, rvalstruct> Pair;

  Map m;
  VERIFY( m.empty());

  Map::iterator p = m.insert(m.begin(), Pair(1, rvalstruct(3)));
  VERIFY( m.size() == 1 );
  VERIFY( std::distance(m.begin(), m.end()) == 1 );
  VERIFY( p == m.begin() );
  VERIFY( p->first == 1 );
  VERIFY( (p->second).val == 3 );
}

void test02()
{
  bool test __attribute__((unused)) = true;
  using __gnu_test::rvalstruct;

  typedef std::map<int, rvalstruct> Map;
  typedef std::pair<const int, rvalstruct> Pair;

  Map m;
  VERIFY( m.empty() );

  Map::iterator p1 = m.insert(m.begin(), Pair(2, rvalstruct(3)));
  Map::iterator p2 = m.insert(p1, Pair(2, rvalstruct(7)));
  VERIFY( m.size() == 1 );
  VERIFY( p1 == p2 );
  VERIFY( p1->first == 2 );
  VERIFY( (p2->second).val == 3 );
}

int main()
{
  test01();
  test02();
  return 0;
}
