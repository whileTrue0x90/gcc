/* { dg-require-effective-target vect_condition } */

extern void abort (void) __attribute__ ((noreturn));

#define N 32

/* Simple condition reduction with a reversed loop.
   Will fail to vectorize to a simple case.  */

int
condition_reduction (int *a, int min_v)
{
  int last = -1;

  for (int i = N-1; i >=0; i--)
    if (a[i] < min_v)
      last = i;

  return last;
}

int
main (void)
{
  int a[N] = {
  17, 28, 13, 14, 15, 16, 17, 18, 19, 20,
  1, 2, -3, 4, 5, 6, 7, -8, 9, 10,
  21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
  31, 32
  };

  int ret = condition_reduction (a, 16);

  if (ret != 2)
    abort ();

  return 0;
}

/* { dg-final { scan-tree-dump-times "LOOP VECTORIZED" 2 "vect" { xfail { ! vect_max_reduc } } } } */
/* { dg-final { scan-tree-dump-not "condition expression based on integer induction." "vect" } } */
