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

#define BID_128RES
#include "div_macros.h"

extern UINT32 __bid_convert_table[5][128][2];
extern SINT8 __bid_factors[][2];
extern UINT8 __bid_packed_10000_zeros[];

BID128_FUNCTION_ARG2(__bid128_div, x, y)

  UINT256 CA4, CA4r, P256;
  UINT128 CX, CY, T128, CQ, CR, CA, TP128, Qh, Ql, res;
  UINT64 sign_x, sign_y, T, carry64, D, Q_high, Q_low, QX, X, PD;
  int_float fx, fy, f64;
  UINT32 QX32, tdigit[3], digit, digit_h, digit_low;
  int exponent_x = 0, exponent_y, bin_index, bin_expon, diff_expon, ed2,
    digits_q, amount;
  int nzeros, i, j, k, d5;
  unsigned rmode;


  // unpack arguments, check for NaN or Infinity
  if (!unpack_BID128_value (&sign_x, &exponent_x, &CX, x)) {
    // test if x is NaN
    if ((x.w[1] & 0x7c00000000000000ull) == 0x7c00000000000000ull) {
#ifdef SET_STATUS_FLAGS
      if ((x.w[1] & 0x7e00000000000000ull) == 0x7e00000000000000ull || // sNaN
          (y.w[1] & 0x7e00000000000000ull) == 0x7e00000000000000ull)
        __set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
      res.w[1] = (x.w[1]) & QUIET_MASK64;
      res.w[0] = x.w[0];
      BID_RETURN (res);
    }
    // x is Infinity?
    if ((x.w[1] & 0x7800000000000000ull) == 0x7800000000000000ull) {
      // check if y is Inf. 
      if (((y.w[1] & 0x7c00000000000000ull) == 0x7800000000000000ull))
        // return NaN 
      {
#ifdef SET_STATUS_FLAGS
        __set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
        res.w[1] = 0x7c00000000000000ull;
        res.w[0] = 0;
        BID_RETURN (res);
      }
      // y is NaN?
      if (((y.w[1] & 0x7c00000000000000ull) != 0x7c00000000000000ull))
        // return NaN 
      {
        // return +/-Inf
        res.w[1] = ((x.w[1] ^ y.w[1]) & 0x8000000000000000ull) | 
            0x7800000000000000ull;
        res.w[0] = 0;
        BID_RETURN (res);
      }
    }
    // x is 0
    if ((y.w[1] & 0x7800000000000000ull) < 0x7800000000000000ull) {
      if ((!y.w[0]) && !(y.w[1] & 0x0001ffffffffffffull)) {
#ifdef SET_STATUS_FLAGS
        __set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
        // x=y=0, return NaN
        res.w[1] = 0x7c00000000000000ull;
        res.w[0] = 0;
        BID_RETURN (res);
      }
      // return 0
      res.w[1] = (x.w[1] ^ y.w[1]) & 0x8000000000000000ull;
      X = ((y.w[1]) << 1) >> 50;
      exponent_x = exponent_x - (int) X + DECIMAL_EXPONENT_BIAS_128;
      if (exponent_x > DECIMAL_MAX_EXPON_128)
        exponent_x = DECIMAL_MAX_EXPON_128;
      else if (exponent_x < 0)
        exponent_x = 0;
      res.w[1] |= (((UINT64) exponent_x) << 49);
      res.w[0] = 0;
      BID_RETURN (res);
    }
  }
  if (!unpack_BID128_value (&sign_y, &exponent_y, &CY, y)) {
    // y is Inf. or NaN

    // test if y is NaN
    if ((y.w[1] & 0x7c00000000000000ull) == 0x7c00000000000000ull) {
#ifdef SET_STATUS_FLAGS
      if ((y.w[1] & 0x7e00000000000000ull) == 0x7e00000000000000ull) // sNaN
        __set_status_flags (pfpsf, INVALID_EXCEPTION);
#endif
      res.w[1] = y.w[1] & QUIET_MASK64;
      res.w[0] = y.w[0];
      BID_RETURN (res);
    }
    // y is Infinity?
    if ((y.w[1] & 0x7800000000000000ull) == 0x7800000000000000ull) {
      // return +/-0
      res.w[1] = sign_x ^ sign_y;
      res.w[0] = 0;
      BID_RETURN (res);
    }
    // y is 0, return +/-Inf
#ifdef SET_STATUS_FLAGS
    __set_status_flags (pfpsf, ZERO_DIVIDE_EXCEPTION);
#endif
    res.w[1] =
      ((x.w[1] ^ y.w[1]) & 0x8000000000000000ull) | 0x7800000000000000ull;
    res.w[0] = 0;
    BID_RETURN (res);
  }
  diff_expon = exponent_x - exponent_y + DECIMAL_EXPONENT_BIAS_128;

  if (__unsigned_compare_gt_128 (CY, CX)) {
    // CX < CY

    // 2^64
    f64.i = 0x5f800000;

    // fx ~ CX,   fy ~ CY
    fx.d = (float) CX.w[1] * f64.d + (float) CX.w[0];
    fy.d = (float) CY.w[1] * f64.d + (float) CY.w[0];
    // expon_cy - expon_cx
    bin_index = (fy.i - fx.i) >> 23;

    if (CX.w[1]) {
      T = __bid_power10_index_binexp_128[bin_index].w[0];
      __mul_64x128_short (CA, T, CX);
    } else {
      T128 = __bid_power10_index_binexp_128[bin_index];
      __mul_64x128_short (CA, CX.w[0], T128);
    }

    ed2 = 33;
    if (__unsigned_compare_gt_128 (CY, CA))
      ed2++;

    T128 = __bid_power10_table_128[ed2];
    __mul_128x128_to_256 (CA4, CA, T128);

    ed2 += __bid_estimate_decimal_digits[bin_index];
    CQ.w[0] = CQ.w[1] = 0;
    diff_expon = diff_expon - ed2;

  } else {
    // get CQ = CX/CY
    __div_128_by_128 (&CQ, &CR, CX, CY);

    if (!CR.w[1] && !CR.w[0]) {
      get_BID128 (&res, sign_x ^ sign_y, diff_expon, CQ, &rnd_mode,
                  pfpsf);
      BID_RETURN (res);
    }
    // get number of decimal digits in CQ
    // 2^64
    f64.i = 0x5f800000;
    fx.d = (float) CQ.w[1] * f64.d + (float) CQ.w[0];
    // binary expon. of CQ
    bin_expon = (fx.i - 0x3f800000) >> 23;

    digits_q = __bid_estimate_decimal_digits[bin_expon];
    TP128.w[0] = __bid_power10_index_binexp_128[bin_expon].w[0];
    TP128.w[1] = __bid_power10_index_binexp_128[bin_expon].w[1];
    if (__unsigned_compare_ge_128 (CQ, TP128))
      digits_q++;

    ed2 = 34 - digits_q;
    T128.w[0] = __bid_power10_table_128[ed2].w[0];
    T128.w[1] = __bid_power10_table_128[ed2].w[1];
    __mul_128x128_to_256 (CA4, CR, T128);
    diff_expon = diff_expon - ed2;
    __mul_128x128_low (CQ, CQ, T128);

  }

  __div_256_by_128 (&CQ, &CA4, CY);

#ifdef SET_STATUS_FLAGS
  if (CA4.w[0] || CA4.w[1]) {
    // set status flags
    __set_status_flags (pfpsf, INEXACT_EXCEPTION);
  }
#ifndef LEAVE_TRAILING_ZEROS
  else
#endif
#else
#ifndef LEAVE_TRAILING_ZEROS
  if (!CA4.w[0] && !CA4.w[1])
#endif
#endif
#ifndef LEAVE_TRAILING_ZEROS
    // check whether result is exact
  {
    // check whether CX, CY are short
    if (!CX.w[1] && !CY.w[1] && (CX.w[0] <= 1024) && (CY.w[0] <= 1024)) {
      i = (int) CY.w[0] - 1;
      j = (int) CX.w[0] - 1;
      // difference in powers of 2 __bid_factors for Y and X
      nzeros = ed2 - __bid_factors[i][0] + __bid_factors[j][0];
      // difference in powers of 5 __bid_factors
      d5 = ed2 - __bid_factors[i][1] + __bid_factors[j][1];
      if (d5 < nzeros)
        nzeros = d5;
      // get P*(2^M[extra_digits])/10^extra_digits
      __mul_128x128_full (Qh, Ql, CQ, __bid_reciprocals10_128[nzeros]);

      // now get P/10^extra_digits: shift Q_high right by M[extra_digits]-128
      amount = __bid_recip_scale[nzeros];
      __shr_128_long (CQ, Qh, amount);

      diff_expon += nzeros;
    } else {
      // decompose Q as Qh*10^17 + Ql
      //T128 = __bid_reciprocals10_128[17];
      T128.w[0] = 0x44909befeb9fad49ull;
      T128.w[1] = 0x000b877aa3236a4bull;
      __mul_128x128_to_256 (P256, CQ, T128);
      //amount = __bid_recip_scale[17];
      Q_high = (P256.w[2] >> 44) | (P256.w[3] << (64 - 44));
      Q_low = CQ.w[0] - Q_high * 100000000000000000ull;

      if (!Q_low) {
        diff_expon += 17;

        tdigit[0] = Q_high & 0x3ffffff;
        tdigit[1] = 0;
        QX = Q_high >> 26;
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

        if (tdigit[1] >= 100000000) {
          tdigit[1] -= 100000000;
          if (tdigit[1] >= 100000000)
            tdigit[1] -= 100000000;
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
          __mul_64x64_to_128 (CQ, Q_high, __bid_reciprocals10_64[nzeros]);

          // now get P/10^extra_digits: shift C64 right by M[extra_digits]-64
          amount = __bid_short_recip_scale[nzeros];
          CQ.w[0] = CQ.w[1] >> amount;
        } else
          CQ.w[0] = Q_high;
        CQ.w[1] = 0;

        diff_expon += nzeros;
      } else {
        tdigit[0] = Q_low & 0x3ffffff;
        tdigit[1] = 0;
        QX = Q_low >> 26;
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

        if (tdigit[1] >= 100000000) {
          tdigit[1] -= 100000000;
          if (tdigit[1] >= 100000000)
            tdigit[1] -= 100000000;
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
          // get P*(2^M[extra_digits])/10^extra_digits
          __mul_128x128_full (Qh, Ql, CQ, __bid_reciprocals10_128[nzeros]);

          //now get P/10^extra_digits: shift Q_high right by M[extra_digits]-128
          amount = __bid_recip_scale[nzeros];
          __shr_128 (CQ, Qh, amount);
        }
        diff_expon += nzeros;

      }
    }
    get_BID128 (&res, sign_x ^ sign_y, diff_expon, CQ, &rnd_mode,
                pfpsf);
    BID_RETURN (res);
  }
#endif

  if (diff_expon >= 0) {
#ifdef IEEE_ROUND_NEAREST
    // rounding
    // 2*CA4 - CY
    CA4r.w[1] = (CA4.w[1] + CA4.w[1]) | (CA4.w[0] >> 63);
    CA4r.w[0] = CA4.w[0] + CA4.w[0];
    __sub_borrow_out (CA4r.w[0], carry64, CA4r.w[0], CY.w[0]);
    CA4r.w[1] = CA4r.w[1] - CY.w[1] - carry64;

    D = (CA4r.w[1] | CA4r.w[0]) ? 1 : 0;
    carry64 = (1 + (((SINT64) CA4r.w[1]) >> 63)) & ((CQ.w[0]) | D);

    CQ.w[0] += carry64;
    if (CQ.w[0] < carry64)
      CQ.w[1]++;
#else
#ifdef IEEE_ROUND_NEAREST_TIES_AWAY
    // rounding
    // 2*CA4 - CY
    CA4r.w[1] = (CA4.w[1] + CA4.w[1]) | (CA4.w[0] >> 63);
    CA4r.w[0] = CA4.w[0] + CA4.w[0];
    __sub_borrow_out (CA4r.w[0], carry64, CA4r.w[0], CY.w[0]);
    CA4r.w[1] = CA4r.w[1] - CY.w[1] - carry64;

    D = (CA4r.w[1] | CA4r.w[0]) ? 0 : 1;
    carry64 = (1 + (((SINT64) CA4r.w[1]) >> 63)) | D;

    CQ.w[0] += carry64;
    if (CQ.w[0] < carry64)
      CQ.w[1]++;
#else
    rmode = rnd_mode;
    if (sign_x ^ sign_y && (unsigned) (rmode - 1) < 2)
      rmode = 3 - rmode;
    switch (rmode) {
    case ROUNDING_TO_NEAREST: // round to nearest code
      // rounding
      // 2*CA4 - CY
      CA4r.w[1] = (CA4.w[1] + CA4.w[1]) | (CA4.w[0] >> 63);
      CA4r.w[0] = CA4.w[0] + CA4.w[0];
      __sub_borrow_out (CA4r.w[0], carry64, CA4r.w[0], CY.w[0]);
      CA4r.w[1] = CA4r.w[1] - CY.w[1] - carry64;
      D = (CA4r.w[1] | CA4r.w[0]) ? 1 : 0;
      carry64 = (1 + (((SINT64) CA4r.w[1]) >> 63)) & ((CQ.w[0]) | D);
      CQ.w[0] += carry64;
      if (CQ.w[0] < carry64)
        CQ.w[1]++;
      break;
    case ROUNDING_TIES_AWAY:
      // rounding
      // 2*CA4 - CY
      CA4r.w[1] = (CA4.w[1] + CA4.w[1]) | (CA4.w[0] >> 63);
      CA4r.w[0] = CA4.w[0] + CA4.w[0];
      __sub_borrow_out (CA4r.w[0], carry64, CA4r.w[0], CY.w[0]);
      CA4r.w[1] = CA4r.w[1] - CY.w[1] - carry64;
      D = (CA4r.w[1] | CA4r.w[0]) ? 0 : 1;
      carry64 = (1 + (((SINT64) CA4r.w[1]) >> 63)) | D;
      CQ.w[0] += carry64;
      if (CQ.w[0] < carry64)
        CQ.w[1]++;
      break;
    case ROUNDING_DOWN:
    case ROUNDING_TO_ZERO:
      break;
    default: // rounding up
      CQ.w[0]++;
      if (!CQ.w[0])
        CQ.w[1]++;
      break;
    }
#endif
#endif

  } else {
#ifdef SET_STATUS_FLAGS
    if (CA4.w[0] || CA4.w[1]) {
      // set status flags
      __set_status_flags (pfpsf, INEXACT_EXCEPTION);
    }
#endif

    handle_UF_128_rem (&res, sign_x ^ sign_y, diff_expon, CQ,
                       CA4.w[1] | CA4.w[0], &rnd_mode, pfpsf);
    BID_RETURN (res);

  }

  get_BID128 (&res, sign_x ^ sign_y, diff_expon, CQ, &rnd_mode, pfpsf);
  BID_RETURN (res);

}
