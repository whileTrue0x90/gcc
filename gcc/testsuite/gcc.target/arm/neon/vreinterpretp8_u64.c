/* Test the `vreinterpretp8_u64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretp8_u64 (void)
{
  poly8x8_t out_poly8x8_t;
  uint64x1_t arg0_uint64x1_t;

  out_poly8x8_t = vreinterpret_p8_u64 (arg0_uint64x1_t);
}

