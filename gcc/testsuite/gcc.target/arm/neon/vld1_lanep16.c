/* Test the `vld1_lanep16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vld1_lanep16 (void)
{
  poly16x4_t out_poly16x4_t;
  poly16x4_t arg1_poly16x4_t;

  out_poly16x4_t = vld1_lane_p16 (0, arg1_poly16x4_t, 1);
}

/* { dg-final { scan-assembler "vld1\.16\[ 	\]+((\\\{\[dD\]\[0-9\]+\\\[\[0-9\]+\\\]\\\})|(\[dD\]\[0-9\]+\\\[\[0-9\]+\\\])), \\\[\[rR\]\[0-9\]+\(:\[0-9\]+\)?\\\]!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
