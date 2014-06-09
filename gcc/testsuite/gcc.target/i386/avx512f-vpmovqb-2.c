/* { dg-do run } */
/* { dg-options "-O2 -mavx512f" } */
/* { dg-require-effective-target avx512f } */

#define AVX512F

#include "avx512f-helper.h"

#define SIZE (AVX512F_LEN / 64)
#include "avx512f-mask-type.h"

CALC (char *r, long long *s, int mem)
{
  int i;
  /* Don't zero out upper half if destination is memory.  */
  int len = mem ? 8 : 16;
  for (i = 0; i < len; i++)
    {
      r[i] = (i < SIZE) ? (char) s[i] : 0;
    }
}

void static
TEST (void)
{
  int i, sign;
  UNION_TYPE (128, i_b) res1, res2, res3;
  char res4[16];
  UNION_TYPE (AVX512F_LEN, i_q) src;
  MASK_TYPE mask = MASK_VALUE;
  char res_ref[16];
  char res_ref2[16];

  sign = -1;
  for (i = 0; i < SIZE; i++)
    {
      src.a[i] = 1 + 34 * i * sign;
      sign = sign * -1;
      res2.a[i] = DEFAULT_VALUE;
      res4[i] = DEFAULT_VALUE;
    }

  for (i = SIZE; i < 16; i++)
    {
      /* To check that memory is not touched.  */
      res4[i] = DEFAULT_VALUE * 2;
      res_ref2[i] = DEFAULT_VALUE * 2;
    }

  res1.x = INTRINSIC (_cvtepi64_epi8) (src.x);
  res2.x = INTRINSIC (_mask_cvtepi64_epi8) (res2.x, mask, src.x);
  res3.x = INTRINSIC (_maskz_cvtepi64_epi8) (mask, src.x);

  CALC (res_ref, src.a, 0);

  if (UNION_CHECK (128, i_b) (res1, res_ref))
    abort ();

  MASK_MERGE (i_b) (res_ref, mask, SIZE);
  if (UNION_CHECK (128, i_b) (res2, res_ref))
    abort ();

  MASK_ZERO (i_b) (res_ref, mask, SIZE);
  if (UNION_CHECK (128, i_b) (res3, res_ref))
    abort ();


  INTRINSIC (_mask_cvtepi64_storeu_epi8) (res4, mask, src.x);

  CALC (res_ref2, src.a, 1);
  MASK_MERGE (i_b) (res_ref2, mask, SIZE);

  if (checkVc (res4, res_ref2, 16))
    abort ();
}
