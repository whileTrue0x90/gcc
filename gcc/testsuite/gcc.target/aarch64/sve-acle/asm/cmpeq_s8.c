/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** cmpeq_s8_tied:
**	cmpeq	p0\.b, p0/z, (z0\.b, z1\.b|z1\.b, z0\.b)
**	ret
*/
TEST_COMPARE_Z (cmpeq_s8_tied, svint8_t,
		p0 = svcmpeq_s8 (p0, z0, z1),
		p0 = svcmpeq (p0, z0, z1))

/*
** cmpeq_s8_untied:
**	cmpeq	p0\.b, p1/z, (z0\.b, z1\.b|z1\.b, z0\.b)
**	ret
*/
TEST_COMPARE_Z (cmpeq_s8_untied, svint8_t,
		p0 = svcmpeq_s8 (p1, z0, z1),
		p0 = svcmpeq (p1, z0, z1))

/*
** cmpeq_x0_s8:
**	mov	(z[0-9]+\.d), x0
**	cmpeq	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpeq_x0_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, x0),
		 p0 = svcmpeq (p1, z0, x0))

/*
** cmpeq_d4_s8:
**	mov	(z[0-9]+\.d), d4
**	cmpeq	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpeq_d4_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, d4),
		 p0 = svcmpeq (p1, z0, d4))

/*
** cmpeq_0_s8:
**	cmpeq	p0\.b, p1/z, z0\.b, #0
**	ret
*/
TEST_COMPARE_ZS (cmpeq_0_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, 0),
		 p0 = svcmpeq (p1, z0, 0))

/*
** cmpeq_1_s8:
**	cmpeq	p0\.b, p1/z, z0\.b, #1
**	ret
*/
TEST_COMPARE_ZS (cmpeq_1_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, 1),
		 p0 = svcmpeq (p1, z0, 1))

/*
** cmpeq_15_s8:
**	cmpeq	p0\.b, p1/z, z0\.b, #15
**	ret
*/
TEST_COMPARE_ZS (cmpeq_15_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, 15),
		 p0 = svcmpeq (p1, z0, 15))

/*
** cmpeq_16_s8:
**	mov	(z[0-9]+\.d), #16
**	cmpeq	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpeq_16_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, 16),
		 p0 = svcmpeq (p1, z0, 16))

/*
** cmpeq_m1_s8:
**	cmpeq	p0\.b, p1/z, z0\.b, #-1
**	ret
*/
TEST_COMPARE_ZS (cmpeq_m1_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, -1),
		 p0 = svcmpeq (p1, z0, -1))

/*
** cmpeq_m16_s8:
**	cmpeq	p0\.b, p1/z, z0\.b, #-16
**	ret
*/
TEST_COMPARE_ZS (cmpeq_m16_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, -16),
		 p0 = svcmpeq (p1, z0, -16))

/*
** cmpeq_m17_s8:
**	mov	(z[0-9]+\.d), #-17
**	cmpeq	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpeq_m17_s8, svint8_t, int64_t,
		 p0 = svcmpeq_n_s8 (p1, z0, -17),
		 p0 = svcmpeq (p1, z0, -17))
