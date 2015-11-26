/* Test the `vreinterpretQp64_u32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_crypto_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_crypto } */

#include "arm_neon.h"

void test_vreinterpretQp64_u32 (void)
{
  poly64x2_t out_poly64x2_t;
  uint32x4_t arg0_uint32x4_t;

  out_poly64x2_t = vreinterpretq_p64_u32 (arg0_uint32x4_t);
}

