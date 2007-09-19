// { dg-require-iconv "ISO-8859-1" }

// Copyright (C) 2004, 2007 Free Software Foundation
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

#include <locale>
#include <cstring>
#include <testsuite_hooks.h>
#include <ext/enc_filebuf.h>

int main()
{
  bool test __attribute__((unused)) = true;
  typedef char char_type;
  typedef __gnu_cxx::enc_filebuf<char_type> filebuf_type;
  typedef filebuf_type::state_type state_type;

  const char* str = "Hello, world!\n";
  std::locale loc(std::locale::classic(),
		  new std::codecvt<char, char, __gnu_cxx::encoding_state>());
  state_type st("ISO-8859-1", "ISO-8859-1");
  filebuf_type fb(st);
  fb.pubimbue(loc);

  fb.open("tmp_13598", std::ios_base::out);
  std::streamsize n = fb.sputn(str, std::strlen(str));
  int s = fb.pubsync();
  fb.close();
  
  VERIFY( n == std::strlen(str) );
  VERIFY( s == 0 );
  
  return 0;
}
