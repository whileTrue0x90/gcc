/* Test the `vreinterpretQp128_s64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_crypto_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_crypto } */

#include "arm_neon.h"

void test_vreinterpretQp128_s64 (void)
{
  poly128_t out_poly128_t;
  int64x2_t arg0_int64x2_t;

  out_poly128_t = vreinterpretq_p128_s64 (arg0_int64x2_t);
}

