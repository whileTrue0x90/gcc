/* Test the `vsubQu32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vsubQu32 (void)
{
  uint32x4_t out_uint32x4_t;
  uint32x4_t arg0_uint32x4_t;
  uint32x4_t arg1_uint32x4_t;

  out_uint32x4_t = vsubq_u32 (arg0_uint32x4_t, arg1_uint32x4_t);
}

/* { dg-final { scan-assembler "vsub\.i32\[ 	\]+\[qQ\]\[0-9\]+, \[qQ\]\[0-9\]+, \[qQ\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
