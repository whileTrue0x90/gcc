/* { dg-do run { target i?86-*-* x86_64-*-* } } */
/* { dg-require-effective-target sse4 } */
/* { dg-options "-O2 -msse4.2" } */

#include "sse4_2-check.h"
#include "sse4_2-pcmpstr.h"

#define NUM 1024

#define IMM_VAL0 \
  (SIDD_SBYTE_OPS | SIDD_CMP_RANGES | SIDD_MASKED_POSITIVE_POLARITY)
#define IMM_VAL1 \
 (SIDD_UBYTE_OPS | SIDD_CMP_EQUAL_EACH | SIDD_NEGATIVE_POLARITY \
  | SIDD_MOST_SIGNIFICANT)
#define IMM_VAL2 \
 (SIDD_UWORD_OPS | SIDD_CMP_EQUAL_ANY | SIDD_MASKED_NEGATIVE_POLARITY)
#define IMM_VAL3 \
  (SIDD_SWORD_OPS | SIDD_CMP_EQUAL_ORDERED \
   | SIDD_MASKED_NEGATIVE_POLARITY | SIDD_MOST_SIGNIFICANT)


static void
sse4_2_test (void)
{
  union
    {
      __m128i x[NUM];
      char c[NUM *16];
    } src1, src2;
  int res, correct, correct_flags;
  int flags, cf, zf, sf, of, af;
  int i;

  for (i = 0; i < NUM *16; i++)
    {
      src1.c[i] = rand ();
      src2.c[i] = rand ();
    }

  for (i = 0; i < NUM; i++)
    {
      switch ((rand () % 4))
	{
	case 0:
	  res = _mm_cmpistri (src1.x[i], src2.x[i], IMM_VAL0);
	  cf = _mm_cmpistrc (src1.x[i], src2.x[i], IMM_VAL0);
	  zf = _mm_cmpistrz (src1.x[i], src2.x[i], IMM_VAL0);
	  sf = _mm_cmpistrs (src1.x[i], src2.x[i], IMM_VAL0);
	  of = _mm_cmpistro (src1.x[i], src2.x[i], IMM_VAL0);
	  af = _mm_cmpistra (src1.x[i], src2.x[i], IMM_VAL0);
	  correct = cmp_ii (&src1.x[i], &src2.x[i], IMM_VAL0,
			    &correct_flags);
	  break;

	case 1:
	  res = _mm_cmpistri (src1.x[i], src2.x[i], IMM_VAL1);
	  cf = _mm_cmpistrc (src1.x[i], src2.x[i], IMM_VAL1);
	  zf = _mm_cmpistrz (src1.x[i], src2.x[i], IMM_VAL1);
	  sf = _mm_cmpistrs (src1.x[i], src2.x[i], IMM_VAL1);
	  of = _mm_cmpistro (src1.x[i], src2.x[i], IMM_VAL1);
	  af = _mm_cmpistra (src1.x[i], src2.x[i], IMM_VAL1);
	  correct = cmp_ii (&src1.x[i], &src2.x[i], IMM_VAL1,
			    &correct_flags);
	  break;

	case 2:
	  res = _mm_cmpistri (src1.x[i], src2.x[i], IMM_VAL2);
	  cf = _mm_cmpistrc (src1.x[i], src2.x[i], IMM_VAL2);
	  zf = _mm_cmpistrz (src1.x[i], src2.x[i], IMM_VAL2);
	  sf = _mm_cmpistrs (src1.x[i], src2.x[i], IMM_VAL2);
	  of = _mm_cmpistro (src1.x[i], src2.x[i], IMM_VAL2);
	  af = _mm_cmpistra (src1.x[i], src2.x[i], IMM_VAL2);
	  correct = cmp_ii (&src1.x[i], &src2.x[i], IMM_VAL2,
			    &correct_flags);
	  break;

	default:
	  res = _mm_cmpistri (src1.x[i], src2.x[i], IMM_VAL3);
	  cf = _mm_cmpistrc (src1.x[i], src2.x[i], IMM_VAL3);
	  zf = _mm_cmpistrz (src1.x[i], src2.x[i], IMM_VAL3);
	  sf = _mm_cmpistrs (src1.x[i], src2.x[i], IMM_VAL3);
	  of = _mm_cmpistro (src1.x[i], src2.x[i], IMM_VAL3);
	  af = _mm_cmpistra (src1.x[i], src2.x[i], IMM_VAL3);
	  correct = cmp_ii (&src1.x[i], &src2.x[i], IMM_VAL3,
			    &correct_flags);
	  break;
	}
      
      if (correct != res)
	abort ();

      flags = 0;
      if (cf)
	flags |= CFLAG;
      if (zf)
	flags |= ZFLAG;
      if (sf)
	flags |= SFLAG;
      if (of)
	flags |= OFLAG;
      
      if (flags != correct_flags
	  || (af && (cf || zf))
	  || (!af && !(cf || zf)))
	abort ();
    }
}
