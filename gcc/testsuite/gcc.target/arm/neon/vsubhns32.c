/* Test the `vsubhns32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vsubhns32 (void)
{
  int16x4_t out_int16x4_t;
  int32x4_t arg0_int32x4_t;
  int32x4_t arg1_int32x4_t;

  out_int16x4_t = vsubhn_s32 (arg0_int32x4_t, arg1_int32x4_t);
}

/* { dg-final { scan-assembler "vsubhn\.i32\[ 	\]+\[dD\]\[0-9\]+, \[qQ\]\[0-9\]+, \[qQ\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
