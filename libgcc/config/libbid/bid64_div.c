/* Copyright (C) 2007  Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs,
and to distribute those combinations without any restriction coming
from the use of this file.  (The General Public License restrictions
do apply in other respects; for example, they cover modification of
the file, and distribution when not linked into a combine
executable.)

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/*****************************************************************************
 *    BID64 divide
 *****************************************************************************
 *
 *  Algorithm description:
 *
 *  if(coefficient_x<coefficient_y)
 *    p = number_digits(coefficient_y) - number_digits(coefficient_x)
 *    A = coefficient_x*10^p
 *    B = coefficient_y
 *    CA= A*10^(15+j), j=0 for A>=B, 1 otherwise
 *    Q = 0
 *  else
 *    get Q=(int)(coefficient_x/coefficient_y) 
 *        (based on double precision divide)
 *    check for exact divide case
 *    Let R = coefficient_x - Q*coefficient_y
 *    Let m=16-number_digits(Q)
 *    CA=R*10^m, Q=Q*10^m
 *    B = coefficient_y
 *  endif
 *    if (CA<2^64)
 *      Q += CA/B  (64-bit unsigned divide)
 *    else 
 *      get final Q using double precision divide, followed by 3 integer 
 *          iterations
 *    if exact result, eliminate trailing zeros
 *    check for underflow
 *    round coefficient to nearest
 *
 ****************************************************************************/

#include "bid_internal.h"

extern UINT32 __bid_convert_table[5][128][2];
extern SINT8 __bid_factors[][2];
extern UINT8 __bid_packed_10000_zeros[];


#if DECIMAL_CALL_BY_REFERENCE

void
__bid64_div (UINT64 * pres, UINT64 * px,
	   UINT64 *
	   py _RND_MODE_PARAM _EXC_FLAGS_PARAM _EXC_MASKS_PARAM
	   _EXC_INFO_PARAM) {
  UINT64 x, y;
#else

UINT64
__bid64_div (UINT64 x,
	   UINT64 y _RND_MODE_PARAM _EXC_FLAGS_PARAM
	   _EXC_MASKS_PARAM _EXC_INFO_PARAM) {
#endif
  UINT128 CA, CT;
  UINT64 sign_x, sign_y, coefficient_x, coefficient_y, A, B, QX, PD;
  UINT64 A2, Q, Q2, B2, B4, B5, R, T, DU, res;
  UINT64 valid_x, valid_y;
  SINT64 D;
  int_double t_scale, tempq, temp_b;
  int_float tempx, tempy;
  double da, db, dq, da_h, da_l;
  int exponent_x = 0, exponent_y = 0, bin_expon_cx;
  int diff_expon, ed1, ed2, bin_index;
  int rmode, amount;
  int nzeros, i, j, k, d5;
  UINT32 QX32, tdigit[3], digit, digit_h, digit_low;

#if DECIMAL_CALL_BY_REFERENCE
#if !DECIMAL_GLOBAL_ROUNDING
  _IDEC_round rnd_mode = *prnd_mode;
#endif
  x = *px;
  y = *py;
#endif

  valid_x = unpack_BID64 (&sign_x, &exponent_x, &coefficient_x, x);
  valid_y = unpack_BID64 (&sign_y, &exponent_y, &coefficient_y, y);

  // unpack arguments, check for NaN or Infinity
  if (!valid_x) {
    // x is Inf. or NaN
#ifdef SET_STATUS_FLAGS
    if ((y & SNAN_MASK64) == SNAN_MASK64)	// y is sNaN
      __set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif

    // test if x is NaN
    if ((x & NAN_MASK64) == NAN_MASK64) {
#ifdef SET_STATUS_FLAGS
      if ((x & SNAN_MASK64) == SNAN_MASK64)	// sNaN
	__set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
      BID_RETURN (x & QUIET_MASK64);
    }
    // x is Infinity?
    if ((x & INFINITY_MASK64) == INFINITY_MASK64) {
      // check if y is Inf or NaN
      if ((y & INFINITY_MASK64) == INFINITY_MASK64) {
	// y==Inf, return NaN 
#ifdef SET_STATUS_FLAGS
	if ((y & NAN_MASK64) == INFINITY_MASK64)	// Inf/Inf
	  __set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
	BID_RETURN (NAN_MASK64);
      }
      // otherwise return +/-Inf
      BID_RETURN (((x ^ y) & 0x8000000000000000ull) | INFINITY_MASK64);
    }
    // x==0
    if (((y & SPECIAL_ENCODING_MASK64) != SPECIAL_ENCODING_MASK64)
	&& !(y << (64 - 53))) {
      // y==0 , return NaN
#ifdef SET_STATUS_FLAGS
      __set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
      BID_RETURN (NAN_MASK64);
    }
    if (((y & INFINITY_MASK64) != INFINITY_MASK64)) {
      if ((y & SPECIAL_ENCODING_MASK64) == SPECIAL_ENCODING_MASK64)
	exponent_y = ((UINT32) (y >> 51)) & 0x3ff;
      else
	exponent_y = ((UINT32) (y >> 53)) & 0x3ff;
      sign_y = y & 0x8000000000000000ull;

      exponent_x = exponent_x - exponent_y + DECIMAL_EXPONENT_BIAS;
      if (exponent_x > DECIMAL_MAX_EXPON_64)
	exponent_x = DECIMAL_MAX_EXPON_64;
      else if (exponent_x < 0)
	exponent_x = 0;
      BID_RETURN ((sign_x ^ sign_y) | (((UINT64) exponent_x) << 53));
    }

  }
  if (!valid_y) {
    // y is Inf. or NaN

    // test if y is NaN
    if ((y & NAN_MASK64) == NAN_MASK64) {
#ifdef SET_STATUS_FLAGS
      if ((y & SNAN_MASK64) == SNAN_MASK64)	// sNaN
	__set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
      BID_RETURN (y & QUIET_MASK64);
    }
    // y is Infinity?
    if ((y & INFINITY_MASK64) == INFINITY_MASK64) {
      // return +/-0
      BID_RETURN (((x ^ y) & 0x8000000000000000ull));
    }
    // y is 0
#ifdef SET_STATUS_FLAGS
    __set_status_flags (pfpsf, ZERO_DIVIDE_EXCEPTION);
#endif
    BID_RETURN ((sign_x ^ sign_y) | INFINITY_MASK64);
  }

  diff_expon = exponent_x - exponent_y + DECIMAL_EXPONENT_BIAS;

  if (coefficient_x < coefficient_y) {
    // get number of decimal digits for c_x, c_y

    //--- get number of bits in the coefficients of x and y ---
    tempx.d = (float) coefficient_x;
    tempy.d = (float) coefficient_y;
    bin_index = (tempy.i - tempx.i) >> 23;

    A = coefficient_x * __bid_power10_index_binexp[bin_index];
    B = coefficient_y;

    temp_b.d = (double) B;

    // compare A, B
    DU = (A - B) >> 63;
    ed1 = 15 + (int) DU;
    ed2 = __bid_estimate_decimal_digits[bin_index] + ed1;
    T = __bid_power10_table_128[ed1].w[0];
    __mul_64x64_to_128 (CA, A, T);

    Q = 0;
    diff_expon = diff_expon - ed2;

    // adjust double precision db, to ensure that later A/B - (int)(da/db) > -1
    if (coefficient_y < 0x0020000000000000ull) {
      temp_b.i += 1;
      db = temp_b.d;
    } else
      db = (double) (B + 2 + (B & 1));

  } else {
    // get c_x/c_y

    //  set last bit before conversion to DP
    A2 = coefficient_x | 1;
    da = (double) A2;

    db = (double) coefficient_y;

    tempq.d = da / db;
    Q = (UINT64) tempq.d;

    R = coefficient_x - coefficient_y * Q;

    // will use to get number of dec. digits of Q
    bin_expon_cx = (tempq.i >> 52) - 0x3ff;

    // R<0 ?
    D = ((SINT64) R) >> 63;
    Q += D;
    R += (coefficient_y & D);

    // exact result ?
    if (((SINT64) R) <= 0) {
      // can have R==-1 for coeff_y==1
      res =
	get_BID64 (sign_x ^ sign_y, diff_expon, (Q + R), rnd_mode,
		   pfpsf);
      BID_RETURN (res);
    }
    // get decimal digits of Q
    DU = __bid_power10_index_binexp[bin_expon_cx] - Q - 1;
    DU >>= 63;

    ed2 = 16 - __bid_estimate_decimal_digits[bin_expon_cx] - (int) DU;

    T = __bid_power10_table_128[ed2].w[0];
    __mul_64x64_to_128 (CA, R, T);
    B = coefficient_y;

    Q *= __bid_power10_table_128[ed2].w[0];
    diff_expon -= ed2;

  }

  if (!CA.w[1]) {
    Q2 = CA.w[0] / B;
    B2 = B + B;
    B4 = B2 + B2;
    R = CA.w[0] - Q2 * B;
    Q += Q2;
  } else {

    // 2^64
    t_scale.i = 0x43f0000000000000ull;
    // convert CA to DP
    da_h = CA.w[1];
    da_l = CA.w[0];
    da = da_h * t_scale.d + da_l;

    // quotient
    dq = da / db;
    Q2 = (UINT64) dq;

    // get w[0] remainder
    R = CA.w[0] - Q2 * B;

    // R<0 ?
    D = ((SINT64) R) >> 63;
    Q2 += D;
    R += (B & D);

    // now R<6*B

    // quick divide

    // 4*B
    B2 = B + B;
    B4 = B2 + B2;

    R = R - B4;
    // R<0 ?
    D = ((SINT64) R) >> 63;
    // restore R if negative
    R += (B4 & D);
    Q2 += ((~D) & 4);

    R = R - B2;
    // R<0 ?
    D = ((SINT64) R) >> 63;
    // restore R if negative
    R += (B2 & D);
    Q2 += ((~D) & 2);

    R = R - B;
    // R<0 ?
    D = ((SINT64) R) >> 63;
    // restore R if negative
    R += (B & D);
    Q2 += ((~D) & 1);

    Q += Q2;
  }

#ifdef SET_STATUS_FLAGS
  if (R) {
    // set status flags
    __set_status_flags (pfpsf, INEXACT_EXCEPTION);
  }
#ifndef LEAVE_TRAILING_ZEROS
  else
#endif
#else
#ifndef LEAVE_TRAILING_ZEROS
  if (!R)
#endif
#endif
#ifndef LEAVE_TRAILING_ZEROS
  {
    // eliminate trailing zeros

    // check whether CX, CY are short
    if ((coefficient_x <= 1024) && (coefficient_y <= 1024)) {
      i = (int) coefficient_y - 1;
      j = (int) coefficient_x - 1;
      // difference in powers of 2 __bid_factors for Y and X
      nzeros = ed2 - __bid_factors[i][0] + __bid_factors[j][0];
      // difference in powers of 5 __bid_factors
      d5 = ed2 - __bid_factors[i][1] + __bid_factors[j][1];
      if (d5 < nzeros)
	nzeros = d5;

      __mul_64x64_to_128 (CT, Q, __bid_reciprocals10_64[nzeros]);

      // now get P/10^extra_digits: shift C64 right by M[extra_digits]-128
      amount = __bid_short_recip_scale[nzeros];
      Q = CT.w[1] >> amount;

      diff_expon += nzeros;
    } else {
      tdigit[0] = Q & 0x3ffffff;
      tdigit[1] = 0;
      QX = Q >> 26;
      QX32 = QX;
      nzeros = 0;

      for (j = 0; QX32; j++, QX32 >>= 7) {
	k = (QX32 & 127);
	tdigit[0] += __bid_convert_table[j][k][0];
	tdigit[1] += __bid_convert_table[j][k][1];
	if (tdigit[0] >= 100000000) {
	  tdigit[0] -= 100000000;
	  tdigit[1]++;
	}
      }

      digit = tdigit[0];
      if (!digit && !tdigit[1])
	nzeros += 16;
      else {
	if (!digit) {
	  nzeros += 8;
	  digit = tdigit[1];
	}
	// decompose digit
	PD = (UINT64) digit *0x068DB8BBull;
	digit_h = (UINT32) (PD >> 40);
	digit_low = digit - digit_h * 10000;

	if (!digit_low)
	  nzeros += 4;
	else
	  digit_h = digit_low;

	if (!(digit_h & 1))
	  nzeros +=
	    3 & (UINT32) (__bid_packed_10000_zeros[digit_h >> 3] >>
			  (digit_h & 7));
      }

      if (nzeros) {
	__mul_64x64_to_128 (CT, Q, __bid_reciprocals10_64[nzeros]);

	// now get P/10^extra_digits: shift C64 right by M[extra_digits]-128
	amount = __bid_short_recip_scale[nzeros];
	Q = CT.w[1] >> amount;
      }
      diff_expon += nzeros;

    }
    if (diff_expon >= 0) {
      res =
	fast_get_BID64_check_OF (sign_x ^ sign_y, diff_expon, Q,
				 rnd_mode, pfpsf);
      BID_RETURN (res);
    }
  }
#endif

  if (diff_expon >= 0) {
#ifdef IEEE_ROUND_NEAREST
    // round to nearest code
    // R*10
    R += R;
    R = (R << 2) + R;
    B5 = B4 + B;

    // compare 10*R to 5*B
    R = B5 - R;
    // correction for (R==0 && (Q&1))
    R -= (Q & 1);
    // R<0 ?
    D = ((UINT64) R) >> 63;
    Q += D;
#else
#ifdef IEEE_ROUND_NEAREST_TIES_AWAY
    // round to nearest code
    // R*10
    R += R;
    R = (R << 2) + R;
    B5 = B4 + B;

    // compare 10*R to 5*B
    R = B5 - R;
    // correction for (R==0 && (Q&1))
    R -= (Q & 1);
    // R<0 ?
    D = ((UINT64) R) >> 63;
    Q += D;
#else
    rmode = rnd_mode;
    if (sign_x ^ sign_y && (unsigned) (rmode - 1) < 2)
      rmode = 3 - rmode;
    switch (rmode) {
    case 0:	// round to nearest code
    case ROUNDING_TIES_AWAY:
      // R*10
      R += R;
      R = (R << 2) + R;
      B5 = B4 + B;
      // compare 10*R to 5*B
      R = B5 - R;
      // correction for (R==0 && (Q&1))
      R -= ((Q | (rmode >> 2)) & 1);
      // R<0 ?
      D = ((UINT64) R) >> 63;
      Q += D;
      break;
    case ROUNDING_DOWN:
    case ROUNDING_TO_ZERO:
      break;
    default:	// rounding up
      Q++;
      break;
    }
#endif
#endif

    res =
      fast_get_BID64_check_OF (sign_x ^ sign_y, diff_expon, Q, rnd_mode,
			       pfpsf);
    BID_RETURN (res);
  } else {
    // UF occurs

#ifdef SET_STATUS_FLAGS
    if ((diff_expon + 16 < 0)) {
      // set status flags
      __set_status_flags (pfpsf, INEXACT_EXCEPTION);
    }
#endif
    rmode = rnd_mode;
    res =
      get_BID64_UF (sign_x ^ sign_y, diff_expon, Q, R, rmode, pfpsf);
    BID_RETURN (res);

  }


}
