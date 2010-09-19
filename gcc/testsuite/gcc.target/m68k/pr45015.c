/* PR debug/45015 */
/* { dg-do compile } */
/* { dg-options "-O2 -g" } */

unsigned int
foo (unsigned int *x, const unsigned int *y, int z, unsigned int w)
{
  unsigned int a, b, c, s;
  int j;
  j = -z;
  x -= j;
  y -= j;
  a = 0;
  do
    {
      __asm__ ("move.l %2, %0; move.l %3, %1" : "=d" (b), "=d" (c) : "g<>" (y[j]), "d" (w));
      c += a;
      a = (c < a) + b;
      s = x[j];
      c = s + c;
      a += (c < s);
      x[j] = c;
    }
  while (++j != 0);
  return a;
}
