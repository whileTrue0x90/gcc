/* Test the `vreinterpretf32_s8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretf32_s8 (void)
{
  float32x2_t out_float32x2_t;
  int8x8_t arg0_int8x8_t;

  out_float32x2_t = vreinterpret_f32_s8 (arg0_int8x8_t);
}

/* { dg-final { cleanup-saved-temps } } */
