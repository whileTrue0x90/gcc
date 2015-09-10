#include <arm_neon.h>

/* { dg-do compile } */
/* { dg-skip-if "" { *-*-* } { "-fno-fat-lto-objects" } } */
/* { dg-excess-errors "" { xfail arm*-*-* } } */

float16x4x2_t
f_vld2_lane_f16 (float16_t * p, float16x4x2_t v)
{
  float16x4x2_t res;
  /* { dg-error "lane 4 out of range 0 - 3" "" { xfail arm*-*-* } 0 } */
  res = vld2_lane_f16 (p, v, 4);
  /* { dg-error "lane -1 out of range 0 - 3" "" { xfail arm*-*-* } 0 } */
  res = vld2_lane_f16 (p, v, -1);
  return res;
}
