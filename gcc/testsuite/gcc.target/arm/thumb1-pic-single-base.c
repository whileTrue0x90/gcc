/* { dg-do compile } */
/* { dg-skip-if "-mpure-code and -fpic incompatible" { *-*-* } { "-mpure-code" } } */
/* { dg-require-effective-target arm_thumb1_ok } */
/* { dg-options "-mthumb -fpic -msingle-pic-base" } */

int g_test;

int
foo (int par)
{
    g_test = par;
}
