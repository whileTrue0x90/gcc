/* { dg-do run } 
 * { dg-options "-std=c99" }
 * */

static int f(int n, char (*x)[sizeof (*(++n, (char (*)[n])0))])
{
  return sizeof *x;
}

int main (void)
{
  if (2 != f(1, 0))
    __builtin_abort ();
  return 0;
}

