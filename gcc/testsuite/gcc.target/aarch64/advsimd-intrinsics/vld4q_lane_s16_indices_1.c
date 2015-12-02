#include <arm_neon.h>

/* { dg-do compile } */
/* { dg-skip-if "" { *-*-* } { "-fno-fat-lto-objects" } } */

int16x8x4_t
f_vld4q_lane_s16 (int16_t * p, int16x8x4_t v)
{
  int16x8x4_t res;
  /* { dg-error "lane 8 out of range 0 - 7" "" { target *-*-* } 0 } */
  res = vld4q_lane_s16 (p, v, 8);
  /* { dg-error "lane -1 out of range 0 - 7" "" { target *-*-* } 0 } */
  res = vld4q_lane_s16 (p, v, -1);
  return res;
}
