// { dg-do run { target c++11 } }

// Copyright (C) 2007-2020 Free Software Foundation, Inc.
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

// 20.6.6.2 Template class shared_ptr [util.smartptr.shared]

#include <memory>
#include <testsuite_hooks.h>
#include <testsuite_allocator.h>

using __gnu_test::tracker_allocator_counter;
using __gnu_test::tracker_allocator;

struct A
{
  A(int i, double d, char c = '\0') : i(i), d(d), c(c) { ++ctor_count; }
  explicit A(int i) : i(i), d(), c() { ++ctor_count; }
  A() : i(), d(), c() { ++ctor_count; }
  ~A() { ++dtor_count; }
  int i;
  double d;
  char c;
  static int ctor_count;
  static int dtor_count;
};
int A::ctor_count = 0;
int A::dtor_count = 0;

struct reset_count_struct
{
  ~reset_count_struct()
  {
    A::ctor_count = 0;
    A::dtor_count = 0;
    tracker_allocator_counter::reset();
  }
};

// 20.6.6.2.6 shared_ptr creation [util.smartptr.shared.create]

void
test01()
{
  reset_count_struct __attribute__((unused)) reset;

  {
    std::shared_ptr<A> p1 = std::allocate_shared<A>(tracker_allocator<A>());
    VERIFY( p1.get() != 0 );
    VERIFY( p1.use_count() == 1 );
    VERIFY( A::ctor_count == 1 );
    VERIFY( tracker_allocator_counter::get_allocation_count() > sizeof(A) );
  }
  VERIFY( A::ctor_count == A::dtor_count );
  VERIFY( tracker_allocator_counter::get_allocation_count()
	  == tracker_allocator_counter::get_deallocation_count() );
}

void
test02()
{
  reset_count_struct __attribute__((unused)) reset;

  std::shared_ptr<A> p1;
  
  p1 = std::allocate_shared<A>(tracker_allocator<A>(), 1);
  VERIFY( A::ctor_count == 1 );
  VERIFY( tracker_allocator_counter::get_allocation_count() > sizeof(A) );

  p1 = std::allocate_shared<A>(tracker_allocator<A>(), 1, 2.0);
  VERIFY( A::ctor_count == 2 );
  VERIFY( A::dtor_count == 1 );
  VERIFY( tracker_allocator_counter::get_deallocation_count() > sizeof(A) );

  p1 = std::allocate_shared<A>(tracker_allocator<A>(), 1, 2.0, '3');
  VERIFY( A::ctor_count == 3 );
  VERIFY( A::dtor_count == 2 );
  VERIFY( p1->i == 1 );
  VERIFY( p1->d == 2.0 );
  VERIFY( p1->c == '3' );

  p1 = std::shared_ptr<A>();
  VERIFY( A::ctor_count == A::dtor_count );
  VERIFY( tracker_allocator_counter::get_allocation_count()
	  == tracker_allocator_counter::get_deallocation_count() );
}

void
test03()
{
  __gnu_test::CustomPointerAlloc<int> alloc;
  auto p = std::allocate_shared<int>(alloc, 1);
  VERIFY( *p == 1 );
}

int
main()
{
  test01();
  test02();
  test03();
  return 0;
}
