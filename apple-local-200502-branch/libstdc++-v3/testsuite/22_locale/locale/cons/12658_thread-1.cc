// { dg-do run { target *-*-freebsd* *-*-netbsd* *-*-linux* *-*-solaris* *-*-cygwin *-*-darwin* alpha*-*-osf* } }
// { dg-options "-pthread" { target *-*-freebsd* *-*-netbsd* *-*-linux* alpha*-*-osf* } }
// { dg-options "-pthreads" { target *-*-solaris* } }

// Copyright (C) 2004 Free Software Foundation
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

// 22.1.1.2 locale constructors and destructors [lib.locale.cons]

// { dg-do run { xfail *-*-linux* } }

#include <locale>
#include <pthread.h>
#include <testsuite_hooks.h>

const int max_thread_count = 20;
//const int max_loop_count = 1000000; // orig value
const int max_loop_count = 100000;
const int max_locales = 10;
 
void* thread_main(void*)
{
  try
    {
      std::locale loc_c = std::locale::classic();
      std::locale loc[max_locales];
      for (int j = 0; j < max_locales; ++j)
	loc[j] = __gnu_test::try_named_locale(j % 2 ? "en_US" : "fr_FR");
      
      for (int i = 0; i < max_loop_count; ++i)
	{
	  int k = i % max_locales;
	  loc[k] = std::locale::global(loc[k]);
	  
	  if (i % 37 == 0)
	    loc[k] = loc[k].combine<std::ctype<char> >(loc_c);
	}
    }
  catch (...) { }
  return 0;
}
 
int
main()
{
  pthread_t tid[max_thread_count];
  
  for (int i = 0; i < max_thread_count; i++)
    pthread_create (&tid[i], NULL, thread_main, 0);
  
  for (int i = 0; i < max_thread_count; i++)
    pthread_join (tid[i], NULL);

  return 0;
}
