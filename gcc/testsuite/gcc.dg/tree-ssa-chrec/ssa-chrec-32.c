/* { dg-do compile } */ 
/* { dg-options "-O1 -fscalar-evolutions -fdump-scalar-evolutions -fall-data-deps -fdump-all-data-deps" } */

void bar (short);

#define N 100
#define NPad 10
#define M 32
void foo()
{
  short coef[M];
  short input[N];
  short output[N];
  
  int i,j,k;                              
  int sum;  
  
  for (i = 0; i < N; i++) {
    sum = 0;
    for (j = 0; j < M; j++) {
      sum += input[i+NPad-j] * coef[j];
    }
    output[i] = sum;
  }
  bar (sum);
}

/* { dg-final { diff-tree-dumps "scev" } } */
/* { dg-final { diff-tree-dumps "alldd" } } */
