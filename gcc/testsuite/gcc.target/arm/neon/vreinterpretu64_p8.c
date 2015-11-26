/* Test the `vreinterpretu64_p8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretu64_p8 (void)
{
  uint64x1_t out_uint64x1_t;
  poly8x8_t arg0_poly8x8_t;

  out_uint64x1_t = vreinterpret_u64_p8 (arg0_poly8x8_t);
}

