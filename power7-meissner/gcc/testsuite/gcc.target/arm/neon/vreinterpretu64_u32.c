/* Test the `vreinterpretu64_u32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vreinterpretu64_u32 (void)
{
  uint64x1_t out_uint64x1_t;
  uint32x2_t arg0_uint32x2_t;

  out_uint64x1_t = vreinterpret_u64_u32 (arg0_uint32x2_t);
}

/* { dg-final { cleanup-saved-temps } } */
