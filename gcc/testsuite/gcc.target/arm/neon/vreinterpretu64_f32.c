/* Test the `vreinterpretu64_f32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretu64_f32 (void)
{
  uint64x1_t out_uint64x1_t;
  float32x2_t arg0_float32x2_t;

  out_uint64x1_t = vreinterpret_u64_f32 (arg0_float32x2_t);
}

