// 2001-11-21 Benjamin Kosnik  <bkoz@redhat.com>

// Copyright (C) 2001, 2002, 2003 Free Software Foundation
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

// 22.2.2.1.1  num_get members

#include <locale>
#include <sstream>
#include <testsuite_hooks.h>

// libstdc++/5816
void test06()
{
  using namespace std;
  bool test = true;

  double d = 0.0;

  istringstream iss;
  locale loc_de("de_DE");
  iss.imbue(loc_de);

  const num_get<char>& ng = use_facet<num_get<char> >(iss.getloc()); 
  const ios_base::iostate goodbit = ios_base::goodbit;
  ios_base::iostate err = ios_base::goodbit;

  iss.str("1234,5 ");
  err = goodbit;
  ng.get(iss.rdbuf(), 0, iss, err, d);
  VERIFY( err == goodbit );
  VERIFY( d == 1234.5 );
}

int main()
{
  __gnu_cxx_test::run_test_wrapped_generic_locale_exception_catcher(test06);
  return 0;
}


// Kathleen Hannah, humanitarian, woman, art-thief
