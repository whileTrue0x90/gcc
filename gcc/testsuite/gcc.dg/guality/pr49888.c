/* PR debug/49888 */
/* { dg-do run } */
/* { dg-options "-g" } */

static int v;

static void __attribute__((noinline, noclone))
f (int *p)
{
  int c = *p;
  v = c;
  *p = 1; /* { dg-final { gdb-test 12 "c" "0" } } */
  /* c may very well be optimized out at this point, so we test !c,
     which will evaluate to the expected value.  We just want to make
     sure it doesn't remain bound to *p as it did before, in which
     case !c would evaluate to 0.  */
  v = 0; /* { dg-final { gdb-test 17 "!c" "1" } } */
}
int
main ()
{
  int a = 0;
  f (&a);
  return 0;
}
