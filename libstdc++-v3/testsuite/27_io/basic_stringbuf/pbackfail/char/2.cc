// 2004-10-01  Paolo Carlini  <pcarlini@suse.de>

// Copyright (C) 2004 Free Software Foundation, Inc.
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

// 27.7.1.3  Overridden virtual functions  [lib.stringbuf.virtuals]

#include <sstream>
#include <testsuite_hooks.h>

class my_stringbuf : public std::stringbuf
{
public:
  my_stringbuf(const std::string& str, std::ios_base::openmode mode)
  : std::stringbuf(str, mode) { }

  int_type 
  pub_pbackfail(int_type __c) 
  { return this->pbackfail(__c); }
};

// We weren't enforcing 27.7.1.3/2, bullet 2: "... and if
// mode & ios_base::out is nonzero, ..."
void test01()
{
  bool test __attribute__((unused)) = true;
  using namespace std;

  typedef my_stringbuf::int_type    int_type;
  typedef my_stringbuf::traits_type traits_type;

  my_stringbuf sbuf("any", ios_base::in);
  
  int_type c = sbuf.sbumpc();
  VERIFY( c == 'a' );

  c = sbuf.pub_pbackfail('x');
  VERIFY( c == traits_type::eof() );
  VERIFY( sbuf.str() == "any" );
  c = sbuf.sgetc();
  VERIFY( c == 'n' );
}


int main()
{
  test01();
  return 0;
}
