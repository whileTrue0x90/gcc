// { dg-do link { target c++11 } }
// { dg-require-cstdint "" }
//
// 2009-09-29  Paolo Carlini <paolo.carlini@oracle.com>
//
// Copyright (C) 2009-2019 Free Software Foundation, Inc.
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

#include <random>

void test01()
{
  std::minstd_rand0 lc;

  const void* p = &lc.multiplier;
  p = &lc.increment;
  p = &lc.modulus;
  p = &lc.default_seed;
  p = p; // Suppress unused warning.
}

int main()
{
  test01();
  return 0;
}
