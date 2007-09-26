/* Test the `vreinterpretu8_u16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vreinterpretu8_u16 (void)
{
  uint8x8_t out_uint8x8_t;
  uint16x4_t arg0_uint16x4_t;

  out_uint8x8_t = vreinterpret_u8_u16 (arg0_uint16x4_t);
}

/* { dg-final { cleanup-saved-temps } } */
