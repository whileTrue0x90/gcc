/* { dg-do compile } */
/* { dg-options "-O2" } */

typedef struct {
  volatile unsigned long a:8;
  volatile unsigned long b:8;
  volatile unsigned long c:16;
} BitStruct;

BitStruct bits;

unsigned long foo ()
{
  return bits.c;
}

/* { dg-final { scan-assembler "ldr\[\\t \]+\[^\n\]*,\[\\t \]*\\\[\[^\n\]*\\\]" } } */
