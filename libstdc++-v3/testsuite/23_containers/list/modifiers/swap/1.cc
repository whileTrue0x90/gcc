// Copyright (C) 2004, 2009 Free Software Foundation
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

#include <list>
#include <testsuite_hooks.h>
 
struct T { int i; };

int swap_calls;

namespace std
{
  template<> 
    void 
    list<T, allocator<T> >::swap(list<T, allocator<T> >&) 
    { ++swap_calls; }
}

// Should use list specialization for swap.
template<typename _Tp>
void 
swap11()
{
  bool test __attribute__((unused)) = true;
  typedef _Tp list_type;

  list_type A;
  list_type B;
  swap_calls = 0;
  std::swap(A, B);
  VERIFY(1 == swap_calls);
}

// Should use list specialization for swap.
template<typename _Tp>
void 
swap12()
{
  using namespace std;
  bool test __attribute__((unused)) = true;
  typedef _Tp list_type;

  list_type A;
  list_type B;
  swap_calls = 0;
  swap(A, B);
  VERIFY(1 == swap_calls);
}

#if !__GXX_WEAK__ && _MT_ALLOCATOR_H
template class __gnu_cxx::__mt_alloc<std::_List_node<T> >;
#endif

// See c++/13658 for background info.
int main()
{
  swap11<std::list<T> >();
  swap12<std::list<T> >();
  return 0;
}
