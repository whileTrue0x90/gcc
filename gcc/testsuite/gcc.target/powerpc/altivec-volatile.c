/* { dg-do compile { target powerpc*-*-* } } */
/* { dg-require-effective-target powerpc_altivec_ok } */
/* { dg-options "-maltivec" } */

/* Check that "volatile" type qualifier is propagated to vector type.  */

#include <altivec.h>

vector float *f (volatile vector float *a)
{
  return a;  /* { dg-warning "discards qualifiers" } */
}

