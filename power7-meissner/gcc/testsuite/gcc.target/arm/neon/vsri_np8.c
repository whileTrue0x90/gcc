/* Test the `vsri_np8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vsri_np8 (void)
{
  poly8x8_t out_poly8x8_t;
  poly8x8_t arg0_poly8x8_t;
  poly8x8_t arg1_poly8x8_t;

  out_poly8x8_t = vsri_n_p8 (arg0_poly8x8_t, arg1_poly8x8_t, 1);
}

/* { dg-final { scan-assembler "vsri\.8\[ 	\]+\[dD\]\[0-9\]+, \[dD\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
