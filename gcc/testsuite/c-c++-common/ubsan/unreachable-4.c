/* { dg-do run } */
/* { dg-options "-fsanitize=undefined" } */
/* { dg-shouldfail "ubsan" } */

int
main (void)
{
  __builtin_unreachable ();
}
 /* { dg-output "execution reached a __builtin_unreachable\\(\\) call" } */
