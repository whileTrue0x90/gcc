/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** cmpne_s32_tied:
**	cmpne	p0\.s, p0/z, (z0\.s, z1\.s|z1\.s, z0\.s)
**	ret
*/
TEST_COMPARE_Z (cmpne_s32_tied, svint32_t,
		p0 = svcmpne_s32 (p0, z0, z1),
		p0 = svcmpne (p0, z0, z1))

/*
** cmpne_s32_untied:
**	cmpne	p0\.s, p1/z, (z0\.s, z1\.s|z1\.s, z0\.s)
**	ret
*/
TEST_COMPARE_Z (cmpne_s32_untied, svint32_t,
		p0 = svcmpne_s32 (p1, z0, z1),
		p0 = svcmpne (p1, z0, z1))

/*
** cmpne_x0_s32:
**	mov	(z[0-9]+\.d), x0
**	cmpne	p0\.s, p1/z, z0\.s, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_x0_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, x0),
		 p0 = svcmpne (p1, z0, x0))

/*
** cmpne_d4_s32:
**	mov	(z[0-9]+\.d), d4
**	cmpne	p0\.s, p1/z, z0\.s, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_d4_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, d4),
		 p0 = svcmpne (p1, z0, d4))

/*
** cmpne_0_s32:
**	cmpne	p0\.s, p1/z, z0\.s, #0
**	ret
*/
TEST_COMPARE_ZS (cmpne_0_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, 0),
		 p0 = svcmpne (p1, z0, 0))

/*
** cmpne_1_s32:
**	cmpne	p0\.s, p1/z, z0\.s, #1
**	ret
*/
TEST_COMPARE_ZS (cmpne_1_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, 1),
		 p0 = svcmpne (p1, z0, 1))

/*
** cmpne_15_s32:
**	cmpne	p0\.s, p1/z, z0\.s, #15
**	ret
*/
TEST_COMPARE_ZS (cmpne_15_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, 15),
		 p0 = svcmpne (p1, z0, 15))

/*
** cmpne_16_s32:
**	mov	(z[0-9]+\.d), #16
**	cmpne	p0\.s, p1/z, z0\.s, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_16_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, 16),
		 p0 = svcmpne (p1, z0, 16))

/*
** cmpne_m1_s32:
**	cmpne	p0\.s, p1/z, z0\.s, #-1
**	ret
*/
TEST_COMPARE_ZS (cmpne_m1_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, -1),
		 p0 = svcmpne (p1, z0, -1))

/*
** cmpne_m16_s32:
**	cmpne	p0\.s, p1/z, z0\.s, #-16
**	ret
*/
TEST_COMPARE_ZS (cmpne_m16_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, -16),
		 p0 = svcmpne (p1, z0, -16))

/*
** cmpne_m17_s32:
**	mov	(z[0-9]+\.d), #-17
**	cmpne	p0\.s, p1/z, z0\.s, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_m17_s32, svint32_t, int64_t,
		 p0 = svcmpne_n_s32 (p1, z0, -17),
		 p0 = svcmpne (p1, z0, -17))
