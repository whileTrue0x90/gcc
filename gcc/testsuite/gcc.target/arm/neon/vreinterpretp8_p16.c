/* Test the `vreinterpretp8_p16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretp8_p16 (void)
{
  poly8x8_t out_poly8x8_t;
  poly16x4_t arg0_poly16x4_t;

  out_poly8x8_t = vreinterpret_p8_p16 (arg0_poly16x4_t);
}

