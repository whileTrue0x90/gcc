/* Test the `vextf32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vextf32 (void)
{
  float32x2_t out_float32x2_t;
  float32x2_t arg0_float32x2_t;
  float32x2_t arg1_float32x2_t;

  out_float32x2_t = vext_f32 (arg0_float32x2_t, arg1_float32x2_t, 0);
}

/* { dg-final { scan-assembler "vext\.32\[ 	\]+\[dD\]\[0-9\]+, \[dD\]\[0-9\]+, \[dD\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
