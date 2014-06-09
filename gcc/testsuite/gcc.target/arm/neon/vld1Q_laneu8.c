/* Test the `vld1Q_laneu8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vld1Q_laneu8 (void)
{
  uint8x16_t out_uint8x16_t;
  uint8x16_t arg1_uint8x16_t;

  out_uint8x16_t = vld1q_lane_u8 (0, arg1_uint8x16_t, 1);
}

/* { dg-final { scan-assembler "vld1\.8\[ 	\]+((\\\{\[dD\]\[0-9\]+\\\[\[0-9\]+\\\]\\\})|(\[dD\]\[0-9\]+\\\[\[0-9\]+\\\])), \\\[\[rR\]\[0-9\]+\(:\[0-9\]+\)?\\\]!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
