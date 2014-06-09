/* Turn the truncate,zero_extend,lshiftrt sequence before the or into a
   zero_extract.  The truncate is due to TARGET_PROMOTE_PROTOTYPES, the
   zero_extend to PROMOTE_MODE.  */
/* { dg-do compile } */
/* { dg-options "isa_rev>=2 -mgp64 -mlong64" } */
/* { dg-skip-if "code quality test" { *-*-* } { "-O0" } { "" } } */
/* { dg-final { scan-assembler "\tdext\t" } } */
/* { dg-final { scan-assembler-not "\tand" } } */
/* { dg-final { scan-assembler-not "\td?srl" } } */

NOMIPS16 void
f (unsigned char x, unsigned char *r)
{
  *r = 0x50 | (x >> 4);
}
