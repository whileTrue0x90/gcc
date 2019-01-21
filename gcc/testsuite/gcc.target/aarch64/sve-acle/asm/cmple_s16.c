/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** cmple_s16_tied:
** (
**	cmpge	p0\.h, p0/z, z1\.h, z0\.h
** |
**	cmple	p0\.h, p0/z, z0\.h, z1\.h
** )
**	ret
*/
TEST_COMPARE_Z (cmple_s16_tied, svint16_t,
		p0 = svcmple_s16 (p0, z0, z1),
		p0 = svcmple (p0, z0, z1))

/*
** cmple_s16_untied:
** (
**	cmpge	p0\.h, p1/z, z1\.h, z0\.h
** |
**	cmple	p0\.h, p1/z, z0\.h, z1\.h
** )
**	ret
*/
TEST_COMPARE_Z (cmple_s16_untied, svint16_t,
		p0 = svcmple_s16 (p1, z0, z1),
		p0 = svcmple (p1, z0, z1))

/*
** cmple_x0_s16:
**	mov	(z[0-9]+\.d), x0
**	cmple	p0\.h, p1/z, z0\.h, \1
**	ret
*/
TEST_COMPARE_ZS (cmple_x0_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, x0),
		 p0 = svcmple (p1, z0, x0))

/*
** cmple_d4_s16:
**	mov	(z[0-9]+\.d), d4
**	cmple	p0\.h, p1/z, z0\.h, \1
**	ret
*/
TEST_COMPARE_ZS (cmple_d4_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, d4),
		 p0 = svcmple (p1, z0, d4))

/*
** cmple_0_s16:
**	cmple	p0\.h, p1/z, z0\.h, #0
**	ret
*/
TEST_COMPARE_ZS (cmple_0_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, 0),
		 p0 = svcmple (p1, z0, 0))

/*
** cmple_1_s16:
**	cmple	p0\.h, p1/z, z0\.h, #1
**	ret
*/
TEST_COMPARE_ZS (cmple_1_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, 1),
		 p0 = svcmple (p1, z0, 1))

/*
** cmple_15_s16:
**	cmple	p0\.h, p1/z, z0\.h, #15
**	ret
*/
TEST_COMPARE_ZS (cmple_15_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, 15),
		 p0 = svcmple (p1, z0, 15))

/*
** cmple_16_s16:
**	mov	(z[0-9]+\.d), #16
**	cmple	p0\.h, p1/z, z0\.h, \1
**	ret
*/
TEST_COMPARE_ZS (cmple_16_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, 16),
		 p0 = svcmple (p1, z0, 16))

/*
** cmple_m1_s16:
**	cmple	p0\.h, p1/z, z0\.h, #-1
**	ret
*/
TEST_COMPARE_ZS (cmple_m1_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, -1),
		 p0 = svcmple (p1, z0, -1))

/*
** cmple_m16_s16:
**	cmple	p0\.h, p1/z, z0\.h, #-16
**	ret
*/
TEST_COMPARE_ZS (cmple_m16_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, -16),
		 p0 = svcmple (p1, z0, -16))

/*
** cmple_m17_s16:
**	mov	(z[0-9]+\.d), #-17
**	cmple	p0\.h, p1/z, z0\.h, \1
**	ret
*/
TEST_COMPARE_ZS (cmple_m17_s16, svint16_t, int64_t,
		 p0 = svcmple_n_s16 (p1, z0, -17),
		 p0 = svcmple (p1, z0, -17))
