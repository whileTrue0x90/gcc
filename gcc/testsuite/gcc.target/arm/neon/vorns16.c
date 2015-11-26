/* Test the `vorns16' ARM Neon intrinsic.  */
/* This file was autogenerated by neon-testgen.  */

/* { dg-do assemble } */
/* { dg-require-effective-target arm_neon_ok } */
/* { dg-options "-save-temps -O2" } */
/* { dg-add-options arm_neon } */

#include "arm_neon.h"

int16x4_t out_int16x4_t;
int16x4_t arg0_int16x4_t;
int16x4_t arg1_int16x4_t;
void test_vorns16 (void)
{

  out_int16x4_t = vorn_s16 (arg0_int16x4_t, arg1_int16x4_t);
}

/* { dg-final { scan-assembler "vorn\[ 	\]+\[dD\]\[0-9\]+, \[dD\]\[0-9\]+, \[dD\]\[0-9\]+!?\(\[ 	\]+@\[a-zA-Z0-9 \]+\)?\n" } } */
