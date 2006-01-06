// { dg-require-namedlocale "" }

// 2001-09-21 Benjamin Kosnik  <bkoz@redhat.com>

// Copyright (C) 2001, 2002, 2003, 2004, 2005, 2006 Free Software Foundation
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

// 22.2.5.1.1 time_get members

#include <locale>
#include <sstream>
#include <testsuite_hooks.h>

void test02()
{
  using namespace std;
  bool test __attribute__((unused)) = true;

  typedef istreambuf_iterator<char> iterator_type;

  // basic construction and sanity checks.
  locale loc_c = locale::classic();
  locale loc_hk = locale("en_HK");
  locale loc_de = locale("de_DE");
  VERIFY( loc_hk != loc_c );
  VERIFY( loc_hk != loc_de );

  const string empty;

  // create an ostream-derived object, cache the time_get facet
  iterator_type end;

  istringstream iss;
  const time_get<char>& tim_get = use_facet<time_get<char> >(iss.getloc()); 

  const ios_base::iostate good = ios_base::goodbit;
  ios_base::iostate errorstate = good;

  // create "C" time objects
  const tm time_bday = __gnu_test::test_tm(0);

  // inspection of named locales, de_DE
  iss.imbue(loc_de);
  iss.str("Sonntag");
  iterator_type is_it10(iss);
  tm time10;
  errorstate = good;
  tim_get.get_weekday(is_it10, end, iss, errorstate, &time10);
  VERIFY( time10.tm_wday == time_bday.tm_wday );
  VERIFY( errorstate == ios_base::eofbit );

  // inspection of named locales, en_HK
  iss.imbue(loc_hk);
  iss.str("Sunday"); 
  iterator_type is_it20(iss);
  tm time20;
  errorstate = good;
  tim_get.get_weekday(is_it20, end, iss, errorstate, &time20);
  VERIFY( time20.tm_wday == time_bday.tm_wday );
  VERIFY( errorstate == ios_base::eofbit );
}

int main()
{
  test02();
  return 0;
}
