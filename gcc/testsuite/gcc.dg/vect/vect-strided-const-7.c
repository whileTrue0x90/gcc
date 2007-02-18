/* { dg-require-effective-target vect_int } */

#include <stdarg.h>
#include <stdio.h>
#include "tree-vect.h"

#define N 8

typedef struct {
   unsigned short a;
   unsigned short b;
} s;

typedef struct {
   unsigned int a;
   unsigned int b;
} ii;

int
main1 (s *arr, ii *iarr)
{
  s *ptr = arr;
  ii *iptr = iarr;
  s res[N];
  ii ires[N];
  int i;

  for (i = 0; i < N; i++)
    {
      ires[i].a = 4;
      ires[i].b = 6;
      res[i].b = ptr->a + ptr->b;
      res[i].a = ptr->a + 5;
      iptr++;
      ptr++;
    }
  
  /* check results:  */
  for (i = 0; i < N; i++)
    {
      if (res[i].b != arr[i].a + arr[i].b
          || ires[i].a != 4 
          || res[i].a != arr[i].a + 5 
          || ires[i].b != 6)  
        abort ();
    }

  return 0;
}

int main (void)
{
  int i;
  s arr[N];
  ii iarr[N];

  check_vect ();

  for (i = 0; i < N; i++)
    {
      arr[i].a = i;
      arr[i].b = i * 2;
      iarr[i].a = i;
      iarr[i].b = i * 3;
      if (arr[i].a == 178)
         abort();
    }

  main1 (arr, iarr); 
  
  return 0;
}

/* { dg-final { scan-tree-dump-times "vectorized 1 loops" 1 "vect" { target { vect_interleave && vect_extract_even_odd } } } } */
/* { dg-final { scan-tree-dump-times "Constant stores in interleaving" 1 "vect"  } } */
/* { dg-final { cleanup-tree-dump "vect" } } */

