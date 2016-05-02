/* { dg-do compile } */
/* { dg-options "-mcpu=nps400 -mcmem" } */

#define CMEM_SECTION_ATTR __attribute__ ((section (".cmem")));

#include "cmem-st.inc"

/* { dg-final { scan-assembler "xst " } } */
/* { dg-final { scan-assembler "xstw " } } */
/* { dg-final { scan-assembler "xstb " } } */
