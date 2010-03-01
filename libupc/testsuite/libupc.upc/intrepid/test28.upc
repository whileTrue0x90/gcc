/*
UPC tests

Copyright (C) 2009
Written by Gary Funck <gary@intrepid.com>
and Nenad Vukicevic <nenad@intrepid.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#include <upc.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define N 1000
shared unsigned long A[N * THREADS];
shared unsigned long ix;

upc_lock_t *lock;

/* Burn a few random cycles */
static void
keep_busy ()
{
  volatile long busy_count;
  busy_count = 100 + (long) (random () % 900);
  while (--busy_count > 0) /* loop */ ;
}

/* Test for Bug 350: GCC/UPC 4.3.2 - post-increment inside array index,
   increments twice (for struct sptr rep). This test is deerived from
   the MTU "test_locks2" test, though does a better job of testing
   lock correctness. */

void
test28 ()
{
  int i;
  if (MYTHREAD == 0)
    ix = 0;
  lock = upc_all_lock_alloc ();
  if (!lock)
    {
      fprintf (stderr, "Error: upc_all_lock_alloc() failed.\n");
      abort ();
    }
  upc_barrier;
  /* random seed based on thread number.  */
  srand (211 * (MYTHREAD + 1));
  for (i = 0; i < N; ++i)
    {
      unsigned long ix_prev;
      /* delay a little to randomize sequencing on the lock */
      keep_busy ();
      upc_lock (lock);
      ix_prev = ix;
      A[ix++] = ix_prev;
      upc_unlock (lock);
    }
  upc_barrier;
  if (MYTHREAD == 0)
    {
      int fail = 0;
      int error_count = 0;
      for (i = 0; i < N * THREADS; ++i)
	{
	  unsigned long expected = i;
	  unsigned long got = A[i];
	  if (got != expected)
	    {
	      if (++error_count <= 20)
		fprintf (stderr, "A[%2d] = %2ld, expected %2ld\n", i, got,
			 expected);
	      else if (error_count == 21)
		fprintf (stderr, "[...]\n");
	      fail = 1;
	    }
	}
      if (fail)
	{
	  fprintf (stderr, "test28: failed.\n");
	  abort ();
	}
    }
}

int
main ()
{
  test28 ();
  upc_barrier;
  if (!MYTHREAD)
    {
      printf
	("test28: Test post-increment as shared array index - passed.\n");
    }
  return 0;
}
