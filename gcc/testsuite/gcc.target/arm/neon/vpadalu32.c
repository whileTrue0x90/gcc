/* Test the `vpadalu32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vpadalu32 (void)
{
  uint64x1_t out_uint64x1_t;
  uint64x1_t arg0_uint64x1_t;
  uint32x2_t arg1_uint32x2_t;

  out_uint64x1_t = vpadal_u32 (arg0_uint64x1_t, arg1_uint32x2_t);
}

/* { dg-final { scan-assembler "vpadal\.u32\[ 	\]+\[dD\]\[0-9\]+, \[dD\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
