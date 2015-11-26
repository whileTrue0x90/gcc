/* Test the `vreinterpretQu32_u64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretQu32_u64 (void)
{
  uint32x4_t out_uint32x4_t;
  uint64x2_t arg0_uint64x2_t;

  out_uint32x4_t = vreinterpretq_u32_u64 (arg0_uint64x2_t);
}

