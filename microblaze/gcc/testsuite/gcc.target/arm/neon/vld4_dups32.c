/* Test the `vld4_dups32' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0 -mfpu=neon -mfloat-abi=softfp" } */

#include "arm_neon.h"

void test_vld4_dups32 (void)
{
  int32x2x4_t out_int32x2x4_t;

  out_int32x2x4_t = vld4_dup_s32 (0);
}

/* { dg-final { scan-assembler "vld4\.32\[ 	\]+\\\{((\[dD\]\[0-9\]+\\\[\\\]-\[dD\]\[0-9\]+\\\[\\\])|(\[dD\]\[0-9\]+\\\[\\\], \[dD\]\[0-9\]+\\\[\\\], \[dD\]\[0-9\]+\\\[\\\], \[dD\]\[0-9\]+\\\[\\\]))\\\}, \\\[\[rR\]\[0-9\]+\\\]!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
/* { dg-final { cleanup-saved-temps } } */
