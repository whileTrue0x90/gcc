/* { dg-require-effective-target vect_int } */

#include <stdarg.h>
#include "tree-vect.h"

#define N 16
 
__attribute__ ((noinline))
int main1 ()
{  
  union {
    unsigned char a[N] __attribute__ ((__aligned__(16)));
    unsigned char b[N] __attribute__ ((__aligned__(16)));
  } s;
  int i;

  /* Initialization.  */
  for (i = 0; i < N; i++)
    {
      s.b[i] = i;
    }

  /* Dependence analysis fails cause s.a and s.b may overlap.
     Use runtime aliasing test with versioning.  */
  for (i = 0; i < N; i++)
    {
      s.a[i] = s.b[i] + 1;
    }

  /* check results:  */
  for (i = 0; i < N; i++)
    {
      if (s.a[i] != i + 1)
	abort ();
    }

  return 0;
}

int main (void)
{ 
  check_vect ();
  
  return main1 ();
} 


/* { dg-final { scan-tree-dump-times "vectorized 2 loops" 1 "vect"  { xfail { ia64-*-* sparc*-*-* } } } } */
/* { dg-final { scan-tree-dump-times "can't determine dependence between" 1 "vect" } } */
/* { dg-final { cleanup-tree-dump "vect" } } */
