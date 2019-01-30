/* { dg-additional-options "-fopenacc-kernels=parloops" } as this is
   specifically testing "parloops" handling.  */
/* { dg-additional-options "-O2 -foffload-alias=pointer" } */
/* { dg-additional-options "-fdump-tree-ealias-all -fdump-tree-optimized" } */

#define N 2

void
foo (void)
{
  unsigned int a[N];
  unsigned int *p = &a[0];

#pragma acc kernels pcopyin (a, p[0:2])
  {
    a[0] = 0;
    *p = 1;
  }
}

/* { dg-final { scan-tree-dump-times " = 0" 1 "optimized" } } */

/* { dg-final { scan-tree-dump-times "clique 1 base 1" 2 "ealias" } } */
/* { dg-final { scan-tree-dump-times "clique 1 base 2" 1 "ealias" } } */
/* { dg-final { scan-tree-dump-times "clique 1 base 3" 1 "ealias" { xfail *-*-* } } } */
/* { dg-final { scan-tree-dump-times "(?n)clique .* base .*" 4 "ealias" } } */
