/* { dg-do compile } */
/* { dg-options "-O3 -march=z14 -mzvector -mzarch" } */

#include "autovec.h"

AUTOVEC_FLOAT (QUIET_GE);

/* { dg-final { scan-assembler {\n\tvfchesb\t} } } */
