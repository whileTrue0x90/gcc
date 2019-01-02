// 1999-10-11 bkoz

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


#include <fstream>
#include <ostream>
#include <testsuite_hooks.h>

// libstdc++/3599
class testbuf : public std::filebuf
{
public:
  typedef std::filebuf::traits_type traits_type;

  testbuf() : std::filebuf() { }
 
protected:
  int_type 
  overflow(int_type c __attribute__((unused)) = traits_type::eof()) 
  { return traits_type::not_eof(0); }
};

void
test07()
{
  testbuf ob;
  std::ostream out(&ob); 

  out << "gasp";
  VERIFY(out.good());

  out << std::endl;
  VERIFY(out.good());
}

int main() 
{
  test07();
  return 0;
}
