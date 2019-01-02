// 1999-08-11 bkoz

// Copyright (C) 1999-2019 Free Software Foundation, Inc.
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

// 27.6.1.3 unformatted input functions

#include <istream>
#include <streambuf>
#include <testsuite_hooks.h>

// libstdc++/8258
class mybuf : public std::basic_streambuf<char> 
{ };

void test11()
{
  using namespace std;
  char arr[10];
  mybuf sbuf;
  basic_istream<char, char_traits<char> > istr(&sbuf);
  
  VERIFY(istr.rdstate() == ios_base::goodbit);
  VERIFY(istr.readsome(arr, 10) == 0);
  VERIFY(istr.rdstate() == ios_base::goodbit);
}
 
int 
main()
{
  test11();
  return 0;
}
