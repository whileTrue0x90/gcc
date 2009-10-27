/* { dg-do compile { target microblaze*-*-* } } */

/* { dg-options "-mxl-gp-opt" } */

#include <stdio.h>
#include <string.h>

/* { dg-final { scan-assembler "\.rodata*" } } */
/* { dg-final { scan-assembler "\.sdata\[^2]+" } } */
char *string1 = "string1";

int testfunc (char *cptr)
{
/* { dg-final { scan-assembler "\lwi\tr(\[0-9]\|\[1-2]\[0-9]\|3\[0-1]),r13" } } */
    strcpy (string1, cptr);
}
