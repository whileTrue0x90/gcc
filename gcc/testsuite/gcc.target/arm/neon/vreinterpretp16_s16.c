/* Test the `vreinterpretp16_s16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretp16_s16 (void)
{
  poly16x4_t out_poly16x4_t;
  int16x4_t arg0_int16x4_t;

  out_poly16x4_t = vreinterpret_p16_s16 (arg0_int16x4_t);
}

/* { dg-final { cleanup-saved-temps } } */
