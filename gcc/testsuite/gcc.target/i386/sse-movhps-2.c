/* { dg-do run } */
/* { dg-options "-O2 -msse" } */

#ifndef CHECK_H
#define CHECK_H "sse-check.h"
#endif

#ifndef TEST
#define TEST sse_test
#endif

#include CHECK_H

#include <xmmintrin.h>

static void
__attribute__((noinline, unused))
test (__m64 *p, __m128 a)
{
  return _mm_storeh_pi (p, a); 
}

static void
TEST (void)
{
  union128 s1;
  float e[2];
  float d[2];

  s1.x = _mm_set_ps (5.13, 6.12, 7.11, 8.9);
 
  test ((__m64 *)d, s1.x);

  e[0] = s1.a[2];
  e[1] = s1.a[3];

  if (checkVf (d, e, 2))
    abort ();
}
