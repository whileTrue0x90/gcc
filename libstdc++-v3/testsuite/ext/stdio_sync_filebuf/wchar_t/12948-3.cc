// 2003-05-01 Petur Runolfsson <peturr02@ru.is>

// Copyright (C) 2003 Free Software Foundation
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

#include <ext/stdio_sync_filebuf.h>
#include <testsuite_hooks.h>

// libstdc++/12048
void test04()
{
  bool test __attribute__((unused)) = true;
  const char* name = "cin_unget-1.txt";

  std::FILE* file = std::fopen(name, "r");
  __gnu_cxx::stdio_sync_filebuf<wchar_t> sbuf(file);
  wchar_t buf[2];
  VERIFY( sbuf.sgetn(buf, 2) == 2 );
  std::wint_t c2 = sbuf.sungetc();
  VERIFY( c2 != WEOF );
  std::wint_t c3 = sbuf.sbumpc();
  VERIFY( c3 == std::char_traits<wchar_t>::to_int_type(buf[1]) );

  std::fclose(file);
}

int main ()
{
  test04();
  return 0;
}
