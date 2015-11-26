/* Test the `vshll_nu8' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vshll_nu8 (void)
{
  uint16x8_t out_uint16x8_t;
  uint8x8_t arg0_uint8x8_t;

  out_uint16x8_t = vshll_n_u8 (arg0_uint8x8_t, 1);
}

/* { dg-final { scan-assembler "vshll\.u8\[ 	\]+\[qQ\]\[0-9\]+, \[dD\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
