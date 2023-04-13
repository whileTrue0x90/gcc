/* { dg-do run } 
 * { dg-options "-std=c99" }
 * */

static int f(int n, char (*x)[sizeof *(++n, (struct { char (*x)[n]; }){ 0 }).x]) /* { dg-warning "anonymous struct" } */
{
  return sizeof *x;
}

int main (void)
{
  if (2 != f(1, 0))
    __builtin_abort ();
  return 0;
}

