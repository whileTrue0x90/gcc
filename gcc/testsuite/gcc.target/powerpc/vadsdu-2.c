/* { dg-do compile { target { powerpc*-*-* } } } */
/* { dg-skip-if "do not override -mcpu" { powerpc*-*-* } { "-mcpu=*" } { "-mcpu=power9" } } */
/* { dg-require-effective-target p9vector_hw } */
/* { dg-options "-mcpu=power9" } */

/* This test should succeed on both 32- and 64-bit configurations.  */
#include <altivec.h>

__vector unsigned short
doAbsoluteDifferenceUnsignedShort (__vector unsigned short *p,
				   __vector unsigned short *q)
{
  __vector unsigned short source_1, source_2;
  __vector unsigned short result;

  source_1 = *p;
  source_2 = *q;

  result = __builtin_vec_vadu (source_1, source_2);
  return result;
}

/* { dg-final { scan-assembler "vabsduh" } } */
