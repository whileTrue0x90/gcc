// 1999-09-20 bkoz

// Copyright (C) 1999, 2003 Free Software Foundation, Inc.
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

// 27.4.4.2 basic_ios member functions

// NB: Don't include any other headers in this file.
#include <ios>
#include <testsuite_hooks.h>

// 27.4.4.3 basic_ios iostate flags function
void test02()
{
  bool test = true;

  typedef std::ios_base::fmtflags fmtflags;
  typedef std::ios_base::iostate iostate;
  using std::ios_base;

  iostate iostate02, iostate03;
  const iostate iostate01 = std::ios_base::badbit | std::ios_base::eofbit;
  const iostate iostate04 = std::ios_base::badbit;

  // basic_ios& copyfmt(const basic_ios& rhs)
  {
    std::ios ios_01(NULL);
    std::ios ios_02(NULL);  
    ios_01.exceptions(std::ios_base::eofbit);
    ios_02.exceptions(std::ios_base::eofbit);
    
    try {
    ios_01.copyfmt(ios_02);
    }		 
    catch(...) {
      VERIFY( false );
    }
  }

  {
    std::ios ios_01(NULL);
    std::ios ios_02(NULL);  
    ios_01.clear(std::ios_base::eofbit);
    ios_02.exceptions(std::ios_base::eofbit);

    try {
      ios_01.copyfmt(ios_02);
      VERIFY( false );
    }		 
    catch(std::ios_base::failure& fail) {
      VERIFY( true );
    }
    catch(...) {
      VERIFY( false );
    }
  }
}

int main() 
{
  test02();
  return 0;
}
