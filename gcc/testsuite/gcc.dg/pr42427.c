/* { dg-do assemble { target c99_runtime } } */
/* { dg-options "-O2 -fexceptions -fnon-call-exceptions -fpeel-loops" } */
/* { dg-add-options c99_runtime } */
/* { dg-require-effective-target ilp32 } */

#include <complex.h>

extern double myabs (complex double);

void
test (double *help, complex double *wm, long nz)
{
  long k;
  double znew;
  double zold;
  for (k = 0; k < nz; k++)
    {
      znew = myabs (wm[k]);
      zold = help[k];
      help[k] = znew;
    }
}
