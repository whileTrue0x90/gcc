/* { dg-require-effective-target vect_float } */
#include <stdarg.h>
#include "tree-vect.h"

#define N 40
float image[N][N][N+1] __attribute__ ((__aligned__(16)));

void
foo (){
  int i,j,k;

 for (k=0; k<N; k++) {
  for (i = 0; i < N; i++) {
    for (j = 0; j < i+1; j++) {
      image[k][j][i] = j+i+k;
    }
  }
 }
}

int main (void)
{
  check_vect ();
  int i, j, k;

  foo ();

 for (k=0; k<N; k++) {
  for (i = 0; i < N; i++) {
    for (j = 0; j < i+1; j++) {
      if (image[k][j][i] != j+i+k)
	abort ();
    }
  }
 }

  return 0;
}

/* { dg-final { scan-tree-dump-times "OUTER LOOP VECTORIZED" 0 "vect" } } */
/* { dg-final { cleanup-tree-dump "vect" } } */
