/* This should not warn about the case label being out of range.  */
/* { dg-do run } */
/* { dg-options "-O0" } */

extern void abort (void);
extern void exit (int);

int
foo (unsigned int i)
{
  switch (i)
  {
    case 123456123456ULL: /* { dg-warning "large integer implicitly truncated to unsigned type" } */
      return 0;
    default:
      return 3;
  }
}

int
main (void)
{
  if (foo (10) != 3)
    abort ();
  exit (0);
}
