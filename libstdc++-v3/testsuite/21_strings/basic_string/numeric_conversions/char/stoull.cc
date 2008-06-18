// { dg-options "-std=gnu++0x" }
// 2008-06-15  Paolo Carlini  <paolo.carlini@oracle.com>

// Copyright (C) 2008 Free Software Foundation, Inc.
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
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// 21.4 Numeric Conversions [string.conversions]

#include <string>
#include <limits>
#include <stdexcept>
#include <testsuite_hooks.h>

void
test01()
{
#ifdef _GLIBCXX_USE_C99

  bool test __attribute__((unused)) = false;
  using namespace std;

  try
    {
      string one;
      stoull(one);      
    }
  catch(std::invalid_argument)
    {
      test = true;
    }
  catch(...)
    {
    }
  VERIFY( test );
  
  test = false;
  try
    {
      string one("a");
      stoull(one);      
    }
  catch(std::invalid_argument)
    {
      test = true;
    }
  catch(...)
    {
    }
  VERIFY( test );

  unsigned long long ull1 = 0;
  try
    {
      string one("a");
      ull1 = stoull(one, 0, 16);      
    }
  catch(...)
    {
      test = false;
    }
  VERIFY( test );
  VERIFY( ull1 == 10 );

  size_t idx1 = 0;
  try
    {
      string one("78");
      ull1 = stoull(one, &idx1, 8);      
    }
  catch(...)
    {
      test = false;
    }
  VERIFY( test );
  VERIFY( ull1 == 7 );
  VERIFY( idx1 = 1 );

  try
    {
      string one("10112");
      ull1 = stoull(one, &idx1, 2);      
    }
  catch(...)
    {
      test = false;
    }
  VERIFY( test );
  VERIFY( ull1 == 11 );
  VERIFY( idx1 == 4 );

  try
    {
      string one("0XE");
      ull1 = stoull(one, &idx1, 0);      
    }
  catch(...)
    {
      test = false;
    }
  VERIFY( test );
  VERIFY( ull1 == 14 );
  VERIFY( idx1 == 3 );

  test = false;
  try
    {
      string one(1000, '9');
      ull1 = stoull(one);
    }
  catch(std::out_of_range)
    {
      test = true;
    }
  catch(...)
    {
    }
  VERIFY( test );
  VERIFY( ull1 == 14 );

  try
    {
      ull1 = numeric_limits<unsigned long long>::max();
      string one(to_string(ull1));
      ull1 = stoull(one);
    }
  catch(...)
    {
      test = false;
    }
  VERIFY( test );
  VERIFY( ull1 == numeric_limits<unsigned long long>::max() );

#endif
}

int main()
{
  test01();
  return 0;
}
