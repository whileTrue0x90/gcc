/* Test the `vcreatep64' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_crypto_ok } */
/* { dg-options "-save-temps -O0" } */
/* { dg-add-options arm_crypto } */

#include "arm_neon.h"

void test_vcreatep64 (void)
{
  poly64x1_t out_poly64x1_t;
  uint64_t arg0_uint64_t;

  out_poly64x1_t = vcreate_p64 (arg0_uint64_t);
}

