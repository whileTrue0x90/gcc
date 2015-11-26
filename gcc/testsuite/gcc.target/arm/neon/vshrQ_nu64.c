/* Test the `vshrQ_nu64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vshrQ_nu64 (void)
{
  uint64x2_t out_uint64x2_t;
  uint64x2_t arg0_uint64x2_t;

  out_uint64x2_t = vshrq_n_u64 (arg0_uint64x2_t, 1);
}

/* { dg-final { scan-assembler "vshr\.u64\[ 	\]+\[qQ\]\[0-9\]+, \[qQ\]\[0-9\]+, #\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
