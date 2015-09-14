/* { dg-do compile { target aarch64*-*-* x86_64-*-* }  } */
/* { dg-options "-fdump-rtl-ce1 -O2" } */

typedef long long s64;

int
foo (s64 a, s64 b, s64 c)
{
 s64 d = a - b;

  if (d == 0)
    return a + c;
  else
    return b + d + c;
}

/* This test can be reduced to just return a + c;  */
/* { dg-final { scan-rtl-dump "3 true changes made" "ce1" } } */
/* { dg-final { scan-assembler-not "sub\.*\tx\[0-9\]+, x\[0-9\]+, x\[0-9\]+\.*" { target { aarch64*-*-* } } } } */
