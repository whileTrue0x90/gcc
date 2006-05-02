/* { dg-do compile } */
/* { dg-options "-O1 -fdump-tree-optimized" } */

int f(int a)
{
  return 10/-a;
}


/* { dg-final { scan-tree-dump-times "10 / -a" 0 "optimized"} } */
/* { dg-final { scan-tree-dump-times "-10 / a" 1 "optimized"} } */

/* { dg-final { cleanup-tree-dump "optimized" } } */

