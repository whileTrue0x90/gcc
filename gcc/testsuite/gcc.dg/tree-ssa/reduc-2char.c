/* { dg-do compile } */
/* { dg-options "-O2 -ftree-parallelize-loops=4 -fdump-tree-parloops-details -fdump-tree-final_cleanup" } */

#include <stdarg.h>
#include <stdlib.h>

#define N 16
#define DIFF 121

__attribute__ ((noinline))
void main1 (signed char x, signed char max_result, signed char min_result)
{
  int i;
  signed char b[N] = {1,2,3,6,8,10,12,14,16,18,20,22,24,26,28,30};
  signed char c[N] = {1,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
  signed char diff = 2;
  signed char max = x;
  signed char min = x;

  for (i = 0; i < N; i++) {
    diff += (signed char)(b[i] - c[i]);
  }

  for (i = 0; i < N; i++) {
    max = max < c[i] ? c[i] : max;
  }

  for (i = 0; i < N; i++) {
    min = min > c[i] ? c[i] : min;
  }

  /* check results:  */
  if (diff != DIFF)
    abort ();
  if (max != max_result)
    abort ();
  if (min != min_result)
    abort ();
}

int main (void)
{ 
  main1 (100, 100, 1);
  main1 (0, 15, 0);
  return 0;
}

/* { dg-final { scan-tree-dump-times "Detected reduction" 2 "parloops" } } */
/* { dg-final { scan-tree-dump-times "SUCCESS: may be parallelized" 2 "parloops" } } */
/* { dg-final { cleanup-tree-dump "parloops" } } */
/* { dg-final { cleanup-tree-dump "final_cleanup" } } */


