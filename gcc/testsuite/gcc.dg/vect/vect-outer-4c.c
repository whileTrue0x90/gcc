/* { dg-do compile } */

#define N 40
#define M 128
unsigned short in[N+M];
unsigned short coeff[M];
unsigned int out[N];

/* Outer-loop vectorization. */

void
foo (){
  int i,j;
  unsigned short diff;

  for (i = 0; i < N; i++) {
    diff = 0;
    for (j = 0; j < M; j+=8) {
      diff += in[j+i]*coeff[j]; 
    }
    out[i]=diff;
  }
}

/* FORNOW. Until we support the 0-stride access coeff[j].  should be:
   scan-tree-dump-times "OUTER LOOP VECTORIZED" 1 "vect" { target vect_short_mult } } } */

/* { dg-final { scan-tree-dump-times "OUTER LOOP VECTORIZED" 1 "vect" { xfail *-*-* } } } */
/* { dg-final { scan-tree-dump-times "zero step in outer loop." 1 "vect" } } */

/* { dg-final { cleanup-tree-dump "vect" } } */
