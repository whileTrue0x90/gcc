/* Test the `vcombinep16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vcombinep16 (void)
{
  poly16x8_t out_poly16x8_t;
  poly16x4_t arg0_poly16x4_t;
  poly16x4_t arg1_poly16x4_t;

  out_poly16x8_t = vcombine_p16 (arg0_poly16x4_t, arg1_poly16x4_t);
}

/* { dg-final { cleanup-saved-temps } } */
