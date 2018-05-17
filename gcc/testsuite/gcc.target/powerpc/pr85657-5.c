/* { dg-do compile { target { powerpc*-*-linux* } } } */
/* { dg-require-effective-target ppc_float128_sw } */
/* { dg-options "-mvsx -mfloat128 -O2" } */

/* PR 85657 -- test __builtin_unpack_ibm128.  Each call should generate just 1
   move.  */

double
unpack0 (double dummy, __ibm128 x)
{
  return __builtin_unpack_ibm128 (x, 0);
}

double
unpack1 (double dummy, __ibm128 x)
{
  /* Should just generate some moves.  */
  return __builtin_unpack_ibm128 (x, 1);
}

/* { dg-final { scan-assembler-times {\m(mfmr|xxlor)\M} 2 } } */
/* { dg-final { scan-assembler-not   {\mbl\M} } } */
/* { dg-final { scan-assembler-not   {\m(mstfd|stxsd)x?\M} } } */
/* { dg-final { scan-assembler-not   {\m(lfd|lxsd)x?\M} } } */
/* { dg-final { scan-assembler-not   {\m(mmtvsrd|mfvsrd)\M} } } */
