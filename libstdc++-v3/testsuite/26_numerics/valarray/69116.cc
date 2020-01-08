// Copyright (C) 2016-2020 Free Software Foundation, Inc.
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

// { dg-do compile }
// { dg-options "-std=gnu++98" }

// libstdc++/69116

#include <exception>
#include <valarray>

template<typename T>
  void foo(const T&) { }

struct X : std::exception // makes namespace std an associated namespace
{
  virtual void pure() = 0;

  typedef void(*func_type)(const X&);

  void operator+(func_type) const;
  void operator-(func_type) const;
  void operator*(func_type) const;
  void operator/(func_type) const;
  void operator%(func_type) const;
  void operator<<(func_type) const;
  void operator>>(func_type) const;
};

void foo(X& x)
{
  x + foo;
  x - foo;
  x * foo;
  x / foo;
  x % foo;
  x << foo;
  x >> foo;
}
