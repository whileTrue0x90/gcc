/* { dg-do compile } */
/* { dg-options "-mavx512f -O2" } */
/* { dg-final { scan-assembler-times "vcvtpd2udq\[ \\t\]+\[^\n\]*%zmm\[0-9\]\[^\n\]*%ymm\[0-9\]\[^\{\]" 2 } } */
/* { dg-final { scan-assembler-times "vcvtpd2udq\[ \\t\]+\[^\n\]*%zmm\[0-9\]\[^\n\]*%ymm\[0-9\]\{%k\[1-7\]\}\[^\{\]" 2 } } */
/* { dg-final { scan-assembler-times "vcvtpd2udq\[ \\t\]+\[^\n\]*%zmm\[0-9\]\[^\n\]*%ymm\[0-9\]\{%k\[1-7\]\}\{z\}" 2 } } */
/* { dg-final { scan-assembler-times "vcvtpd2udq\[ \\t\]+\[^\n\]*\{rn-sae\}\[^\n\]*%zmm\[0-9\]\[^\n\]*%ymm\[0-9\]" 1 } } */
/* { dg-final { scan-assembler-times "vcvtpd2udq\[ \\t\]+\[^\n\]*\{ru-sae\}\[^\n\]*%zmm\[0-9\]\[^\n\]*%ymm\[0-9\]\{%k\[1-7\]\}\[^\{\]" 1 } } */
/* { dg-final { scan-assembler-times "vcvtpd2udq\[ \\t\]+\[^\n\]*\{rz-sae\}\[^\n\]*%zmm\[0-9\]\[^\n\]*%ymm\[0-9\]\{%k\[1-7\]\}\{z\}" 1 } } */

#include <immintrin.h>

volatile __m512d s;
volatile __m256i res;
volatile __mmask8 m;

void extern
avx512f_test (void)
{
  res = _mm512_cvtpd_epu32 (s);
  res = _mm512_mask_cvtpd_epu32 (res, m, s);
  res = _mm512_maskz_cvtpd_epu32 (m, s);
  res = _mm512_cvt_roundpd_epu32 (s, _MM_FROUND_TO_NEAREST_INT);
  res = _mm512_mask_cvt_roundpd_epu32 (res, m, s, _MM_FROUND_TO_POS_INF);
  res = _mm512_maskz_cvt_roundpd_epu32 (m, s, _MM_FROUND_TO_ZERO);
}
