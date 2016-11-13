#include <altivec.h>

/* { dg-do compile { target { powerpc64*-*-* && lp64 } } } */
/* { dg-skip-if "do not override -mcpu" { powerpc*-*-* } { "-mcpu=*" } { "-mcpu=power9" } } */
/* { dg-require-effective-target powerpc_p9vector_ok } */
/* { dg-options "-mcpu=power9 -O2 -mupper-regs-di -mvsx-small-integer" } */

vector short
insert_0_0 (vector short v)
{
  return vec_insert (0, v, 0);
}

vector short
insert_m1_1 (vector short v)
{
  return vec_insert (-1, v, 1);
}

vector short
insert_5_2 (vector short v)
{
  return vec_insert (5, v, 2);
}

vector short
insert_mem_7 (vector short v, short *p)
{
  return vec_insert (*p, v, 7);
}

/* { dg-final { scan-assembler     "vinserth" } } */
/* { dg-final { scan-assembler     "xxspltib" } } */
/* { dg-final { scan-assembler     "vspltish" } } */
/* { dg-final { scan-assembler-not "mtvsrd"   } } */
/* { dg-final { scan-assembler-not "mtvsrdd"  } } */
/* { dg-final { scan-assembler-not "mtvsrwa"  } } */
/* { dg-final { scan-assembler-not "mtvsrwz"  } } */
/* { dg-final { scan-assembler-not "mfvsrd"   } } */
/* { dg-final { scan-assembler-not "mfvsrwz"  } } */
