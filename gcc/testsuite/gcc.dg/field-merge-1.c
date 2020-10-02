/* { dg-do run } */
/* { dg-options "-O -save-temps -fdump-tree-optimized" } */

/* Check that field loads compared with constants are merged, even if
   tested out of order, and when fields straddle across alignment
   boundaries.  */

struct TL {
  unsigned char p;
  unsigned int a;
  unsigned char q;
  unsigned int b;
  unsigned char r;
  unsigned int c;
  unsigned char s;
} __attribute__ ((packed, aligned (4), scalar_storage_order ("little-endian")));

struct TB {
  unsigned char p;
  unsigned int a;
  unsigned char q;
  unsigned int b;
  unsigned char r;
  unsigned int c;
  unsigned char s;
} __attribute__ ((packed, aligned (4), scalar_storage_order ("big-endian")));

#define vc 0xaa
#define vi 0x12345678

struct TL vL = { vc, vi, vc, vi, vc, vi, vc };
struct TB vB = { vc, vi, vc, vi, vc, vi, vc };

void f (void) {
  if (0
      || vL.p != vc
      || vL.a != vi
      || vL.q != vc
      || vL.b != vi
      || vL.r != vc
      || vL.c != vi
      || vL.s != vc
      || vB.p != vc
      || vB.a != vi
      || vB.q != vc
      || vB.b != vi
      || vB.r != vc
      || vB.c != vi
      || vB.s != vc
      )
    __builtin_abort ();
}

int main () {
  f ();
  return 0;
}

/* { dg-final { scan-tree-dump-times "BIT_FIELD_REF" 8 "optimized" } } */
/* { dg-final { scan-assembler-not "cmpb" { target { i*86-*-* || x86_64-*-* } } } } */
/* { dg-final { scan-assembler-times "cmpl" 8 { target { i*86-*-* || x86_64-*-* } } } } */
/* { dg-final { scan-assembler-times "cmpw" 8 { target { powerpc*-*-* || rs6000-*-* } } } } */
