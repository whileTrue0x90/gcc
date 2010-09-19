/* Test the `vreinterpretQp16_u8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretQp16_u8 (void)
{
  poly16x8_t out_poly16x8_t;
  uint8x16_t arg0_uint8x16_t;

  out_poly16x8_t = vreinterpretq_p16_u8 (arg0_uint8x16_t);
}

/* { dg-final { cleanup-saved-temps } } */
