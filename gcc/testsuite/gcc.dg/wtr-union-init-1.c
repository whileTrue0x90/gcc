/* Test for -Wtraditional warnings on union initialization.
   Note, gcc should omit these warnings in system header files.
   By Kaveh R. Ghazi <ghazi@caip.rutgers.edu> 8/22/2000.  */
/* { dg-do compile } */
/* { dg-options "-Wtraditional" } */

union foo
{
  int i;
  long l;
};

void
testfunc (void)
{
  /* Note we only warn for non-zero initializers.  */
  static union foo f1 = { 0 };
  static union foo f2 = { 1 }; /* { dg-warning "traditional C rejects initialization of unions" "initialization of unions" } */

#line 21 "sys-header.h" 3
/* We are in system headers now, no -Wtraditional warnings should issue.  */

  static union foo f3 = { 0 };
  static union foo f4 = { 1 };
}
