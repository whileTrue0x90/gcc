/* Test the `vreinterpretQs32_f32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretQs32_f32 (void)
{
  int32x4_t out_int32x4_t;
  float32x4_t arg0_float32x4_t;

  out_int32x4_t = vreinterpretq_s32_f32 (arg0_float32x4_t);
}

