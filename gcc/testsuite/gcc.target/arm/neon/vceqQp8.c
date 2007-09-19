/* Test the `vceqQp8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vceqQp8 (void)
{
  uint8x16_t out_uint8x16_t;
  poly8x16_t arg0_poly8x16_t;
  poly8x16_t arg1_poly8x16_t;

  out_uint8x16_t = vceqq_p8 (arg0_poly8x16_t, arg1_poly8x16_t);
}

/* { dg-final { scan-assembler "vceq\.i8\[ 	\]+\[qQ\]\[0-9\]+, \[qQ\]\[0-9\]+, \[qQ\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
