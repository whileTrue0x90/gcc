/* { dg-do compile } */
/* { dg-options "-pedantic-errors" } */
#include "pr36901.h"
void foo(void)
{
  int s = sc;
}
/* { dg-message "file included" "In file included" { target *-*-* } 0 } */
/* { dg-warning "overflow" "overflow" { target *-*-* } 0 } */
/* { dg-error "overflow" "overflow" { target *-*-* } 0 } */
