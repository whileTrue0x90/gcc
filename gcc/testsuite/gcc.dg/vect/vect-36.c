/* { dg-require-effective-target vect_int } */

#include <stdarg.h>
#include "tree-vect.h"

#define N 16
 
int main1 ()
{  
  struct {
    char ca[N];
    char cb[N];
  } s;
  int i;

  for (i = 0; i < N; i++)
    {
      s.cb[i] = 3*i;
    }

  for (i = 0; i < N; i++)
    {
      s.ca[i] = s.cb[i];
    }

  /* check results:  */
  for (i = 0; i < N; i++)
    {
      if (s.ca[i] != s.cb[i])
        abort ();
    }

  return 0;
}

int main (void)
{ 
  check_vect ();
  
  return main1 ();
} 

/* { dg-final { scan-tree-dump-times "vectorized 1 loops" 1 "vect" } } */
