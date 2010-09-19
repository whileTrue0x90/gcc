/* Test the `vreinterprets64_s16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vreinterprets64_s16 (void)
{
  int64x1_t out_int64x1_t;
  int16x4_t arg0_int16x4_t;

  out_int64x1_t = vreinterpret_s64_s16 (arg0_int16x4_t);
}

/* { dg-final { cleanup-saved-temps } } */
