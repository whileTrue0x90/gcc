/* Test the `vreinterpretQs8_p128' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_crypto_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_crypto } */

#include "arm_neon.h"

void test_vreinterpretQs8_p128 (void)
{
  int8x16_t out_int8x16_t;
  poly128_t arg0_poly128_t;

  out_int8x16_t = vreinterpretq_s8_p128 (arg0_poly128_t);
}

