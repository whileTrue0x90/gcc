/* Test the `vshrn_ns16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vshrn_ns16 (void)
{
  int8x8_t out_int8x8_t;
  int16x8_t arg0_int16x8_t;

  out_int8x8_t = vshrn_n_s16 (arg0_int16x8_t, 1);
}

/* { dg-final { scan-assembler "vshrn\.i16\[ 	\]+\[dD\]\[0-9\]+, \[qQ\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
