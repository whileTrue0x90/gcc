/* Test the `vreinterpretQf32_p16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vreinterpretQf32_p16 (void)
{
  float32x4_t out_float32x4_t;
  poly16x8_t arg0_poly16x8_t;

  out_float32x4_t = vreinterpretq_f32_p16 (arg0_poly16x8_t);
}

/* { dg-final { cleanup-saved-temps } } */
