/* APPLE LOCAL file CW asm blocks */
/* { dg-do assemble { target i?86*-*-darwin* } } */
/* { dg-options { -fasm-blocks -msse3 } } */
/* Radar 4319881 */

void foo() {
  static const int aMoveMasks[4] = { 0 };
  asm {
    movdqa xmm6, [aMoveMasks+edx*8]
  }
}
