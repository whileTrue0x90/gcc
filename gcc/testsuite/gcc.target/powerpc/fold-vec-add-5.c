/* Verify that overloaded built-ins for vec_add with float
   inputs produce the right results.  */

/* { dg-do compile } */
/* { dg-require-effective-target powerpc_altivec_ok } */
/* { dg-additional-options "-mno-vsx" } */

#include <altivec.h>

vector float
test1 (vector float x, vector float y)
{
  return vec_add (x, y);
}

/* { dg-final { scan-assembler-times "vaddfp" 1 } } */
