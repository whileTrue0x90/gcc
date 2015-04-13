/* Functional tests for the function hotpatching feature.  */

/* { dg-do compile } */
/* { dg-options "-O3 -mzarch -mhotpatch=0,2" } */

#include <stdio.h>

void hp1(void)
{
  printf("hello, world!\n");
}

/* Check number of occurences of certain instructions.  */
/* { dg-final { scan-assembler-not "pre-label NOPs" } } */
/* { dg-final { scan-assembler "post-label.*(2 halfwords)" } } */
/* { dg-final { scan-assembler-not "nopr\t%r7" } } */
/* { dg-final { scan-assembler-times "nop\t0" 1 } } */
/* { dg-final { scan-assembler-not "brcl\t0, 0" } } */
