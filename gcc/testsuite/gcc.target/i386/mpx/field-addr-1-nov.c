/* { dg-do run } */
/* { dg-options "-fcheck-pointer-bounds -mmpx" } */


#include "mpx-check.h"

struct S {
  int a;
  int b[100];
  int c;
} S;

int foo (int *i, int k)
{
  printf ("%d\n", i[k]);
  return i[k];
}

int mpx_test (int argc, const char **argv)
{
  struct S s;

  foo(&s.a, 0);
  foo(&s.a, 101);

  return 0;
}
