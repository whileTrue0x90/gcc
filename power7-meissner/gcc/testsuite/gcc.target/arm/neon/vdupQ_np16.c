/* Test the `vdupQ_np16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vdupQ_np16 (void)
{
  poly16x8_t out_poly16x8_t;
  poly16_t arg0_poly16_t;

  out_poly16x8_t = vdupq_n_p16 (arg0_poly16_t);
}

/* { dg-final { scan-assembler "vdup\.16\[ 	\]+\[qQ\]\[0-9\]+, \[rR\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
