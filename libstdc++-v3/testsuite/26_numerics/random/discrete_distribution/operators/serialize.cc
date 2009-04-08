// { dg-options "-std=c++0x" }
//
// 2008-12-05  Edward M. Smith-Rowland <3dw4rd@verizon.net>
//
// Copyright (C) 2008 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// 26.4.8.5.1 Class template discrete_distribution [rand.dist.samp.discrete]
// 26.4.2.4 Concept RandomNumberDistribution [rand.concept.dist]

#include <random>
#include <sstream>
#include <testsuite_hooks.h>

void
test01()
{
  bool test __attribute__((unused)) = true;

  std::stringstream str;
  std::vector<double> wt = {0.5, 1.0, 2.5, 1.5, 0.5};
  std::discrete_distribution<int> u(wt.begin(), wt.end()), v;
  std::minstd_rand0 rng;

  u(rng); // advance
  str << u;

  VERIFY( !(u == v) );

  str >> v;
  VERIFY( u == v );
}

int main()
{
  test01();
  return 0;
}
