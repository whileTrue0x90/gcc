/* Test the `vreinterpretp8_u8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterpretp8_u8 (void)
{
  poly8x8_t out_poly8x8_t;
  uint8x8_t arg0_uint8x8_t;

  out_poly8x8_t = vreinterpret_p8_u8 (arg0_uint8x8_t);
}

