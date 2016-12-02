/* { dg-do compile { target lp64 } } */
/* { dg-options "-O2" } */

/* { dg-final { scan-assembler-times {(?n)^\s+[a-z]} 1799 } } */
/* { dg-final { scan-assembler-times {(?n)^\s+blr} 900 } } */

/* { dg-final { scan-assembler-times {(?n)^\s+rldic} 423 } } */
/* { dg-final { scan-assembler-times {(?n)^\s+sldi} 23 } } */

/* { dg-final { scan-assembler-times {(?n)^\s+mulli} 453 } } */


#define CR
#define SL

#include "rldicx.h"
