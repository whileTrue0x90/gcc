/* APPLE LOCAL 3729261 FSF candidate  */
/* Test lang in N_SO stab.  */
/* Contributed by Devang Patel  <dpatel@apple.com>  */

/* { dg-do compile } */
/* { dg-options "-gstabs" } */

int
main ()
{
  return 0;
}

/* { dg-final { scan-assembler ".stabs*100,0,2" } } */

