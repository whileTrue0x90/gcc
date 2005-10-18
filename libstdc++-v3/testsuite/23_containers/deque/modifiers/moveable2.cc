// Copyright (C) 2005 Free Software Foundation, Inc.
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

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

// { dg-do compile }

#include <deque>
#include <testsuite_hooks.h>
#include <testsuite_rvalref.h>

using namespace __gnu_test;

void
test01(std::deque<rvalstruct>& d, rvalstruct* begin, rvalstruct* end)
{
  d.assign(__gnu_cxx::__make_move_iterator(begin),
	   __gnu_cxx::__make_move_iterator(end));
}

// Test deque::insert(iterator, iterator, iterator) makes no unneeded copies.
void
test02(std::deque<rvalstruct>& d, rvalstruct* begin, rvalstruct* end)
{
  d.insert(d.begin(), __gnu_cxx::__make_move_iterator(begin), 
	   __gnu_cxx::__make_move_iterator(end));
}

// Erasing a single iterator
void
test03(std::deque<rvalstruct>& d, std::deque<rvalstruct>::iterator i)
{ d.erase(i); }

// Erasing a range of iterators
void
test04(std::deque<rvalstruct>& d, std::deque<rvalstruct>::iterator begin,
				   std::deque<rvalstruct>::iterator end)
{ d.erase(begin, end); }

// Clearing a deque
void
test05(std::deque<rvalstruct>& d)
{ d.clear(); }
