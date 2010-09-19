/* { dg-do compile { target { { powerpc*-*-* } && { ! powerpc*-apple-darwin* } } } } */
/* { dg-options "-O2 -mrecip -ffast-math -mcpu=power5" } */
/* { dg-final { scan-assembler-times "frsqrtes" 1 } } */
/* { dg-final { scan-assembler-times "fmsubs" 1 } } */
/* { dg-final { scan-assembler-times "fmuls" 6 } } */
/* { dg-final { scan-assembler-times "fnmsubs" 3 } } */
/* { dg-final { scan-assembler-times "fsqrt" 1 } } */

/* power5 resqrte is not accurate enough, and should not be generated by
   default for -mrecip.  */
double
rsqrt_d (double a)
{
  return 1.0 / __builtin_sqrt (a);
}

float
rsqrt_f (float a)
{
  return 1.0f / __builtin_sqrtf (a);
}
