/* Test the `vld3_lanes16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vld3_lanes16 (void)
{
  int16x4x3_t out_int16x4x3_t;
  int16x4x3_t arg1_int16x4x3_t;

  out_int16x4x3_t = vld3_lane_s16 (0, arg1_int16x4x3_t, 1);
}

/* { dg-final { scan-assembler "vld3\.16\[ 	\]+\\\{((\[dD\]\[0-9\]+\\\[\[0-9\]+\\\]-\[dD\]\[0-9\]+\\\[\[0-9\]+\\\])|(\[dD\]\[0-9\]+\\\[\[0-9\]+\\\], \[dD\]\[0-9\]+\\\[\[0-9\]+\\\], \[dD\]\[0-9\]+\\\[\[0-9\]+\\\]))\\\}, \\\[\[rR\]\[0-9\]+\(:\[0-9\]+\)?\\\]!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
