/* { dg-do run } */

#ifdef __MSP430X__
#include "isr-push-pop-isr-430x.c"
#include "isr-push-pop-leaf-isr-430x.c"
#else
#include "isr-push-pop-isr-430.c"
#include "isr-push-pop-leaf-isr-430.c"
#endif

/* Test that ISRs which call other functions do not save extraneous registers.
   They only need to save the caller-saved regs R11->R15.
   We use a lot of asm statements to hide what is going on from the compiler to
   more accurately simulate an interrupt.  */

/* Store the register number in each general register R4->R15, so they can be
   later checked their value has been kept.  */
#define SETUP_REGS		\
  __asm__ ("mov #4, r4");	\
  __asm__ ("mov #5, r5");	\
  __asm__ ("mov #6, r6");	\
  __asm__ ("mov #7, r7");	\
  __asm__ ("mov #8, r8");	\
  __asm__ ("mov #9, r9");	\
  __asm__ ("mov #10, r10");	\
  __asm__ ("mov #11, r11");	\
  __asm__ ("mov #12, r12");	\
  __asm__ ("mov #13, r13");	\
  __asm__ ("mov #14, r14");	\
  __asm__ ("mov #15, r15");

/* Write an arbitrary value to all general regs.  */
#define TRASH_REGS				\
  __asm__ ("mov #0xFFFF, r4" : : : "R4");	\
  __asm__ ("mov #0xFFFF, r5" : : : "R5");	\
  __asm__ ("mov #0xFFFF, r6" : : : "R6");	\
  __asm__ ("mov #0xFFFF, r7" : : : "R7");	\
  __asm__ ("mov #0xFFFF, r8" : : : "R8");	\
  __asm__ ("mov #0xFFFF, r9" : : : "R9");	\
  __asm__ ("mov #0xFFFF, r10" : : : "R10");	\
  __asm__ ("mov #0xFFFF, r11" : : : "R11");	\
  __asm__ ("mov #0xFFFF, r12" : : : "R12");	\
  __asm__ ("mov #0xFFFF, r13" : : : "R13");	\
  __asm__ ("mov #0xFFFF, r14" : : : "R14");	\
  __asm__ ("mov #0xFFFF, r15" : : : "R15");

/* Check the value in all general registers is the same as that set in
   SETUP_REGS.  */
#define CHECK_REGS			\
  __asm__ ("cmp #4,  r4 { jne ABORT");	\
  __asm__ ("cmp #5,  r5 { jne ABORT");	\
  __asm__ ("cmp #6,  r6 { jne ABORT");	\
  __asm__ ("cmp #7,  r7 { jne ABORT");	\
  __asm__ ("cmp #8,  r8 { jne ABORT");	\
  __asm__ ("cmp #9,  r9 { jne ABORT");	\
  __asm__ ("cmp #10, r10 { jne ABORT");	\
  __asm__ ("cmp #11, r11 { jne ABORT");	\
  __asm__ ("cmp #12, r12 { jne ABORT");	\
  __asm__ ("cmp #13, r13 { jne ABORT");	\
  __asm__ ("cmp #14, r14 { jne ABORT");	\
  __asm__ ("cmp #15, r15 { jne ABORT");

void __attribute__((noinline))
callee (void)
{
  /* Here were modify all the regs, but tell the compiler that we are since
     this is just a way to simulate a function that happens to modify all the
     registers.  */
  TRASH_REGS
}
int 
#ifdef __MSP430X_LARGE__ 
__attribute__((lower))
#endif
main (void)
{
  SETUP_REGS

  /* A surprise branch to the ISR that the compiler cannot prepare for.
     We must first simulate the interrupt acceptance procedure that the
     hardware would normally take care of.
     So push the desired PC return address, and then the SR (R2).
     MSP430X expects the high bits 19:16 of the PC return address to be stored
     in bits 12:15 of the SR stack slot.  This is hard to handle in hand-rolled
     assembly code, so we always place main() in lower memory so the return
     address is 16-bits.  */
  __asm__ ("push #CHECK1");
  __asm__ ("push r2");
  __asm__ ("br #isr");

  __asm__ ("CHECK1:");
  /* If any of the regs R4->R15 don't match their original value, this will
     jump to ABORT.  */
  CHECK_REGS

  /* Now test that an interrupt function that is a leaf also works
     correctly.  */
  __asm__ ("push #CHECK2");
  __asm__ ("push r2");
  __asm__ ("br #isr_leaf");

  __asm__ ("CHECK2:");
  CHECK_REGS

  /* The values in R4->R15 were successfully checked, now jump to FINISH to run
     the prologue generated by the compiler.  */
  __asm__ ("jmp FINISH");

  /* CHECK_REGS will branch here if a register holds the wrong value.  */
  __asm__ ("ABORT:");
#ifdef __MSP430X_LARGE__ 
  __asm__ ("calla #abort");
#else
  __asm__ ("call #abort");
#endif

  __asm__ ("FINISH:");
  return 0;
}

