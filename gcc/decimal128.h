/* ------------------------------------------------------------------ */
/* Decimal 128-bit format module header                               */
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

#if !defined(DECIMAL128)
  #define DECIMAL128
  #define DEC128NAME     "decimal128"                 /* Short name */
  #define DEC128FULLNAME "Decimal 128-bit Number"     /* Verbose name */
  #define DEC128AUTHOR   "Mike Cowlishaw"             /* Who to blame */

  #if defined(DECIMAL32)
    #error decimal128.h must precede decimal32.h for correct DECNUMDIGITS
  #else
    #if defined(DECIMAL64)
      #error decimal128.h must precede decimal64.h for correct DECNUMDIGITS
    #endif
  #endif

  /* parameters for decimal128s */
  #define DECIMAL128_Bytes  16          // length
  #define DECIMAL128_Pmax   34          // maximum precision (digits)
  #define DECIMAL128_Emax   6144        // maximum adjusted exponent
  #define DECIMAL128_Emin  -6143        // minimum adjusted exponent
  #define DECIMAL128_Bias   6176        // bias for the exponent
  #define DECIMAL128_String 43          // maximum string length, +1
  // highest biased exponent (Elimit-1)
  #define DECIMAL128_Ehigh  (DECIMAL128_Emax+DECIMAL128_Bias-DECIMAL128_Pmax+1)

  #ifndef DECNUMDIGITS
    #define DECNUMDIGITS DECIMAL128_Pmax // size if not already defined
  #endif
  #ifndef DECNUMBER
    #include "decNumber.h"              // context and number library
  #endif

  /* Decimal 128-bit type, accessible by bytes */
  typedef struct {
    uint8_t bytes[DECIMAL128_Bytes];    // decimal128: 1, 5, 12, 110 bits
    } decimal128;

  /* special values [top byte excluding sign bit; last two bits are
     don't-care for Infinity on input, last bit don't-care for NaN] */
  #if !defined(DECIMAL_NaN)
    #define DECIMAL_NaN     0x7c        // 0 11111 00 NaN
    #define DECIMAL_sNaN    0x7e        // 0 11111 10 sNaN
    #define DECIMAL_Inf     0x78        // 0 11110 00 Infinity
  #endif

  /* Macros for accessing decimal128 fields.  These assume the argument
     is a reference (pointer) to the decimal128 structure */
  // Get sign
  #define decimal128Sign(d)       ((unsigned)(d)->bytes[0]>>7)

  // Get combination field
  #define decimal128Comb(d)       (((d)->bytes[0] & 0x7c)>>2)

  // Get exponent continuation [does not remove bias]
  #define decimal128ExpCon(d)     ((((d)->bytes[0] & 0x03)<<10)       \
                                | ((unsigned)(d)->bytes[1]<<2)        \
                                | ((unsigned)(d)->bytes[2]>>6))

  // Set sign [this assumes sign previously 0]
  #define decimal128SetSign(d, b) {                                   \
    (d)->bytes[0]|=((unsigned)(b)<<7);}

  // Set exponent continuation [does not apply bias]
  // This assumes range has been checked and exponent previously 0;
  // type of exponent must be unsigned
  #define decimal128SetExpCon(d, e) {                                 \
    (d)->bytes[0]|=(uint8_t)((e)>>10);                                \
    (d)->bytes[1] =(uint8_t)(((e)&0x3fc)>>2);                         \
    (d)->bytes[2]|=(uint8_t)(((e)&0x03)<<6);}

  /* ------------------------------------------------------------------ */
  /* Routines                                                           */
  /* ------------------------------------------------------------------ */
  // String conversions
  decimal128 * decimal128FromString(decimal128 *, char *, decContext *);
  char * decimal128ToString(decimal128 *, char *);
  char * decimal128ToEngString(decimal128 *, char *);

  // decNumber conversions
  decimal128 * decimal128FromNumber(decimal128 *, decNumber *, decContext *);
  decNumber * decimal128ToNumber(decimal128 *, decNumber *);

#endif
