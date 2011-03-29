/* { dg-do compile } */
/* { dg-options "-fstack-usage" } */

/* This is aimed at testing basic support for -fstack-usage in the back-ends.
   See the SPARC back-end for an example (grep flag_stack_usage in sparc.c).
   Once it is implemented, adjust SIZE below so that the stack usage for the
   function FOO is reported as 256 or 264 in the stack usage (.su) file.
   Then check that this is the actual stack usage in the assembly file.  */

#if defined(__i386__)
#  define SIZE 248
#elif defined(__x86_64__)
#  define SIZE 356
#elif defined (__sparc__)
#  if defined (__arch64__)
#    define SIZE 76
#  else
#    define SIZE 160
#  endif
#elif defined(__hppa__)
#  define SIZE 192
#elif defined (__alpha__)
#  define SIZE 240
#elif defined (__ia64__)
#  define SIZE 272
#elif defined(__mips__)
#  if defined (__mips_abicalls) \
      || (defined _MIPS_SIM && (_MIPS_SIM ==_ABIN32 || _MIPS_SIM==_ABI64))
#    define SIZE 240
#  else
#    define SIZE 248
#  endif
#elif defined (__powerpc64__) || defined (__PPC64__)
#  define SIZE 180
#elif defined (__powerpc__) || defined (__PPC__) || defined (__ppc__) \
      || defined (__POWERPC__) || defined (PPC) || defined (_IBMR2)
#  if defined (__ALTIVEC__)
#    define SIZE 220
#  else
#    define SIZE 240
#  endif
#elif defined (__AVR__)
#  define SIZE 254
#elif defined (__s390x__)
#  define SIZE 96  /* 256 - 160 bytes for register save area */
#elif defined (__s390__)
#  define SIZE 160 /* 256 -  96 bytes for register save area */
#elif defined (__SPU__)
#  define SIZE 224
#else
#  define SIZE 256
#endif

int foo (void)
{
  char arr[SIZE];
  arr[0] = 1;
  return 0;
}

/* { dg-final { scan-stack-usage "foo\t\(256|264\)\tstatic" } } */
/* { dg-final { cleanup-stack-usage } } */
