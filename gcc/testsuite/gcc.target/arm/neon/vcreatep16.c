/* Test the `vcreatep16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

void test_vcreatep16 (void)
{
  poly16x4_t out_poly16x4_t;
  uint64_t arg0_uint64_t;

  out_poly16x4_t = vcreate_p16 (arg0_uint64_t);
}

