// { dg-do run { target c++11 } }

//
// 2013-09-02  Tim Shen <timshen91@gmail.com>
//
// Copyright (C) 2013-2019 Free Software Foundation, Inc.
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

// 28.11.2 regex_match
// Tests ECMAScript "." against a std::string.

#include <regex>
#include <testsuite_hooks.h>
#include <testsuite_regex.h>

using namespace __gnu_test;
using namespace std;

void
test01()
{
#define TESTL(res, s) \
  {\
    wregex re(res);\
    wstring st(s);\
    VERIFY(!regex_match_debug(st, re));\
  }
  TESTL(L".", L"\u2028");
  TESTL(L".", L"\u2029");
}

int
main()
{
  test01();
  return 0;
}
