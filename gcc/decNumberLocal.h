/* ------------------------------------------------------------------ */
/* decNumber package local type, tuning, and macro definitions        */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2004.  All rights reserved.   */
/*                                                                    */
/* This software is made available under the terms of the IBM         */
/* alphaWorks License Agreement (distributed with this software as    */
/* alphaWorks-License.txt).  Your use of this software indicates      */
/* your acceptance of the terms and conditions of that Agreement.     */
/*                                                                    */
/* The description and User's Guide ("The decNumber C Library") for   */
/* this software is included in the package as decNumber.pdf.  This   */
/* document is also available in HTML, together with specifications,  */
/* testcases, and Web links, at: http://www2.hursley.ibm.com/decimal  */
/*                                                                    */
/* Please send comments, suggestions, and corrections to the author:  */
/*   mfc@uk.ibm.com                                                   */
/*   Mike Cowlishaw, IBM Fellow                                       */
/*   IBM UK, PO Box 31, Birmingham Road, Warwick CV34 5JL, UK         */
/* ------------------------------------------------------------------ */
/* This header file is included by all modules in the decNumber       */
/* library, and contains local type definitions, tuning parameters,   */
/* etc.  It must only be included once, and should not need to be     */
/* used by application programs.  decNumber.h must be included first. */
/* ------------------------------------------------------------------ */

#if !defined(DECNUMBERLOC)
  #define DECNUMBERLOC
  #define DECNLAUTHOR   "Mike Cowlishaw"              /* Who to blame */

  /* Local names for common types -- decNumber modules do not use int or
     long directly */
  #define Flag   uint8_t
  #define Byte   int8_t
  #define uByte  uint8_t
  #define Short  int16_t
  #define uShort uint16_t
  #define Int    int32_t
  #define uInt   uint32_t
  #define Unit   decNumberUnit


  /* Tuning parameter */
  #define DECBUFFER 36        // Maximum size basis for local buffers.
                              // Should be a common maximum precision
                              // rounded up to a multiple of 4; must
                              // be non-negative.

  /* Conditional code flags -- set these to 0 for best performance */
  #define DECCHECK  0         // 1 to enable robust checking
  #define DECALLOC  0         // 1 to enable memory allocation accounting
  #define DECTRACE  0         // 1 to trace critical intermediates, etc.


  /* Development use defines */
  #if DECALLOC
     // if these interfere with your C includes, just comment them out
     #define  int ?           // enable to ensure we do not use plain C
     #define  long ??         // .. 'int' or 'long' types from here on
  #endif

  /* Limits and constants */
  #define DECNUMMAXP 999999999  // maximum precision we can handle (9 digits)
  #define DECNUMMAXE 999999999  // maximum adjusted exponent ditto (9 digits)
  #define DECNUMMINE -999999999 // minimum adjusted exponent ditto (9 digits)
  #if (DECNUMMAXP != DEC_MAX_DIGITS)
    #error Maximum digits mismatch
  #endif
  #if (DECNUMMAXE != DEC_MAX_EMAX)
    #error Maximum exponent mismatch
  #endif
  #if (DECNUMMINE != DEC_MIN_EMIN)
    #error Minimum exponent mismatch
  #endif

  /* Set DECDPUNMAX -- the maximum integer that fits in DECDPUN digits */
  #if   DECDPUN==1
    #define DECDPUNMAX 9
  #elif DECDPUN==2
    #define DECDPUNMAX 99
  #elif DECDPUN==3
    #define DECDPUNMAX 999
  #elif DECDPUN==4
    #define DECDPUNMAX 9999
  #elif DECDPUN==5
    #define DECDPUNMAX 99999
  #elif DECDPUN==6
    #define DECDPUNMAX 999999
  #elif DECDPUN==7
    #define DECDPUNMAX 9999999
  #elif DECDPUN==8
    #define DECDPUNMAX 99999999
  #elif DECDPUN==9
    #define DECDPUNMAX 999999999
  #elif defined(DECDPUN)
    #error DECDPUN must be in the range 1-9
  #endif


  /* ----- Shared data ----- */
  // The powers of of ten array (powers[n]==10**n, 0<=n<=10)
  extern const uInt powers[];

  /* ----- Macros ----- */
  // ISZERO -- return true if decNumber dn is a zero
  // [performance-critical in some situations]
  #define ISZERO(dn) decNumberIsZero(dn)     // now just a local name

  // X10 and X100 -- multiply integer i by 10 or 100
  // [shifts are usually faster than multiply; could be conditional]
  #define X10(i)  (((i)<<1)+((i)<<3))
  #define X100(i) (((i)<<2)+((i)<<5)+((i)<<6))

  // D2U -- return the number of Units needed to hold d digits
  #if DECDPUN==8
    #define D2U(d) ((unsigned)((d)+7)>>3)
  #elif DECDPUN==4
    #define D2U(d) ((unsigned)((d)+3)>>2)
  #else
    #define D2U(d) (((d)+DECDPUN-1)/DECDPUN)
  #endif

#else
  #error decNumberLocal included more than once
#endif
