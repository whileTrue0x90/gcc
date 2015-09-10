/* { dg-options "-mshared -mabi=64 -mmicromips" } */
/* { dg-final { scan-assembler-not "(\\\$28|%gp_rel|%got)" } } */
/* { dg-final { scan-assembler-not "\tjrc?\t\\\$1\n" } } */

#include "branch-helper.h"

NOMIPS16 void
foo (int (*bar) (void), int *x)
{
  *x = bar ();
  if (__builtin_expect (*x == 0, 1))
    OCCUPY_0xfffc;
}
