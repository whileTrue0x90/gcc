/* Verify the return instruction is mret.  */
/* { dg-do compile } */
/* { dg-options "-O" } */
void __attribute__ ((interrupt ("supervisor")))
foo (void)
{
}
/* { dg-final { scan-assembler "sret" } } */
