/* Test the `vreinterpretp64_u64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_crypto_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_crypto } */

#include "arm_neon.h"

void test_vreinterpretp64_u64 (void)
{
  poly64x1_t out_poly64x1_t;
  uint64x1_t arg0_uint64x1_t;

  out_poly64x1_t = vreinterpret_p64_u64 (arg0_uint64x1_t);
}

