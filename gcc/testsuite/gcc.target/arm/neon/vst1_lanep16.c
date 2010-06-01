/* Test the `vst1_lanep16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vst1_lanep16 (void)
{
  poly16_t *arg0_poly16_t;
  poly16x4_t arg1_poly16x4_t;

  vst1_lane_p16 (arg0_poly16_t, arg1_poly16x4_t, 1);
}

/* { dg-final { scan-assembler "vst1\.16\[ 	\]+((\\\{\[dD\]\[0-9\]+\\\[\[0-9\]+\\\]\\\})|(\[dD\]\[0-9\]+\\\[\[0-9\]+\\\])), \\\[\[rR\]\[0-9\]+\\\]!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
