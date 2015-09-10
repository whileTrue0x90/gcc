/* { dg-do compile } */
/* { dg-options "-march=armv8-a+nolse -O2" } */

#include "atomic-op-consume.x"

/* Scan for ldaxr is a PR59448 consume workaround.  */
/* { dg-final { scan-assembler-times "ldaxr\tw\[0-9\]+, \\\[x\[0-9\]+\\\]" 6 } } */
/* { dg-final { scan-assembler-times "stxr\tw\[0-9\]+, w\[0-9\]+, \\\[x\[0-9\]+\\\]" 6 } } */
