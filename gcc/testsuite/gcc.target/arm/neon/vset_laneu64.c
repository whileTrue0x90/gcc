/* Test the `vset_laneu64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vset_laneu64 (void)
{
  uint64x1_t out_uint64x1_t;
  uint64_t arg0_uint64_t;
  uint64x1_t arg1_uint64x1_t;

  out_uint64x1_t = vset_lane_u64 (arg0_uint64_t, arg1_uint64x1_t, 0);
}

