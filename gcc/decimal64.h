/* ------------------------------------------------------------------ */
/* Decimal 64-bit format module header                                */
/* ------------------------------------------------------------------ */
/* Copyright (c) IBM Corporation, 2000, 2003.  All rights reserved.   */
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

#if !defined(DECIMAL64)
  #define DECIMAL64
  #define DEC64NAME     "decimal64"                   /* Short name */
  #define DEC64FULLNAME "Decimal 64-bit Number"       /* Verbose name */
  #define DEC64AUTHOR   "Mike Cowlishaw"              /* Who to blame */

  #if defined(DECIMAL32)
    #error decimal64.h must precede decimal32.h for correct DECNUMDIGITS
  #endif

  /* parameters for decimal64s */
  #define DECIMAL64_Bytes  8            // length
  #define DECIMAL64_Pmax   16           // maximum precision (digits)
  #define DECIMAL64_Emax   384          // maximum adjusted exponent
  #define DECIMAL64_Emin  -383          // minimum adjusted exponent
  #define DECIMAL64_Bias   398          // bias for the exponent
  #define DECIMAL64_String 24           // maximum string length, +1
  // highest biased exponent (Elimit-1)
  #define DECIMAL64_Ehigh  (DECIMAL64_Emax+DECIMAL64_Bias-DECIMAL64_Pmax+1)

  #ifndef DECNUMDIGITS
    #define DECNUMDIGITS DECIMAL64_Pmax // size if not already defined
  #endif
  #ifndef DECNUMBER
    #include "decNumber.h"              // context and number library
  #endif

  /* Decimal 64-bit type, accessible by bytes */
  typedef struct {
    uint8_t bytes[DECIMAL64_Bytes];     // decimal64: 1, 5, 8, 50 bits
    } decimal64;

  /* special values [top byte excluding sign bit; last two bits are
     don't-care for Infinity on input, last bit don't-care for NaN] */
  #if !defined(DECIMAL_NaN)
    #define DECIMAL_NaN     0x7c        // 0 11111 00 NaN
    #define DECIMAL_sNaN    0x7e        // 0 11111 10 sNaN
    #define DECIMAL_Inf     0x78        // 0 11110 00 Infinity
  #endif

  /* Macros for accessing decimal64 fields.  These assume the argument
     is a reference (pointer) to the decimal64 structure */
  // Get sign
  #define decimal64Sign(d)       ((unsigned)(d)->bytes[0]>>7)

  // Get combination field
  #define decimal64Comb(d)       (((d)->bytes[0] & 0x7c)>>2)

  // Get exponent continuation [does not remove bias]
  #define decimal64ExpCon(d)     ((((d)->bytes[0] & 0x03)<<6)         \
                               | ((unsigned)(d)->bytes[1]>>2))

  // Set sign [this assumes sign previously 0]
  #define decimal64SetSign(d, b) {                                    \
    (d)->bytes[0]|=((unsigned)(b)<<7);}

  // Set exponent continuation [does not apply bias]
  // This assumes range has been checked and exponent previously 0; type
  // of exponent must be unsigned
  #define decimal64SetExpCon(d, e) {                                  \
    (d)->bytes[0]|=(uint8_t)((e)>>6);                                 \
    (d)->bytes[1]|=(uint8_t)(((e)&0x3F)<<2);}

  /* ------------------------------------------------------------------ */
  /* Routines                                                           */
  /* ------------------------------------------------------------------ */
  // String conversions
  decimal64 * decimal64FromString(decimal64 *, char *, decContext *);
  char * decimal64ToString(decimal64 *, char *);
  char * decimal64ToEngString(decimal64 *, char *);

  // decNumber conversions
  decimal64 * decimal64FromNumber(decimal64 *, decNumber *, decContext *);
  decNumber * decimal64ToNumber(decimal64 *, decNumber *);

#endif
