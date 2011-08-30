/* { dg-do run } */
/* { dg-require-effective-target avx2 } */
/* { dg-options "-O2 -mavx2" } */

#include <string.h>
#include "avx2-check.h"

static void
compute_i32gatherd256 (int *s1, int *s2, int scale, int *r)
{
  int i;

  for (i = 0; i < 8; ++i)
    r[i] = *(int *) (((unsigned char *) s1) + s2[i] * scale);
}

void static
avx2_test (void)
{
  int i;
  union256i_d idx;
  union256i_d res;
  int s1[8], res_ref[8] = { 0 };

  for (i = 0; i < 8; ++i)
    {
      /* Set some stuff */
      s1[i] = 1973 * (i + 1) * (i + 2);

      /* About to gather in reverse order,
         divide by 2 to demonstrate scale */
      idx.a[i] = (32 - (i + 1) * 4) >> 1;
    }

  res.x = _mm256_i32gather_epi32 (s1, idx.x, 2);

  compute_i32gatherd256 (s1, idx.a, 2, res_ref);

  if (check_union256i_d (res, res_ref) != 0)
    abort ();
}
