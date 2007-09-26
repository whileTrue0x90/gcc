/* Test the `vreinterpretQu64_p8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vreinterpretQu64_p8 (void)
{
  uint64x2_t out_uint64x2_t;
  poly8x16_t arg0_poly8x16_t;

  out_uint64x2_t = vreinterpretq_u64_p8 (arg0_poly8x16_t);
}

/* { dg-final { cleanup-saved-temps } } */
