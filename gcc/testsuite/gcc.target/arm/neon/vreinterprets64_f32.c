/* Test the `vreinterprets64_f32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterprets64_f32 (void)
{
  int64x1_t out_int64x1_t;
  float32x2_t arg0_float32x2_t;

  out_int64x1_t = vreinterpret_s64_f32 (arg0_float32x2_t);
}

