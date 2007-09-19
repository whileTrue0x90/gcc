/* Test the `vset_lanes64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vset_lanes64 (void)
{
  int64x1_t out_int64x1_t;
  int64_t arg0_int64_t;
  int64x1_t arg1_int64x1_t;

  out_int64x1_t = vset_lane_s64 (arg0_int64_t, arg1_int64x1_t, 0);
}

/* { dg-final { scan-assembler "vmov\[ 	\]+\[dD\]\[0-9\]+, \[rR\]\[0-9\]+, \[rR\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
