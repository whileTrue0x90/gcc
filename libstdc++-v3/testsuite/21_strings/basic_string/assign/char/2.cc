// 2001-10-30 Benjamin Kosnik  <bkoz@redhat.com>

// Copyright (C) 2001, 2003 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// 21.3.5 string modifiers

#include <string>
#include <cstdio>
#include <testsuite_hooks.h>

// assign(const basic_string& __str, size_type __pos, size_type __n)
void
test02()
{
  bool test __attribute__((unused)) = true;

  using namespace std;
  
  string one = "Selling England by the pound";
  string two = one;
  string three = "Brilliant trees";

  one.assign(one, 8, 100);
  VERIFY( one == "England by the pound" );

  one.assign(one, 8, 0);
  VERIFY( one == "" );
 
  one.assign(two, 8, 7);
  VERIFY( one == "England" );

  one.assign(three, 10, 100);
  VERIFY( one == "trees" );

  three.assign(one, 0, 3);
  VERIFY( three == "tre" );
}

int main()
{ 
  test02();
  return 0;
}
