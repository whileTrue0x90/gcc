/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** cmpne_f64_tied:
**	fcmne	p0\.d, p0/z, (z0\.d, z1\.d|z1\.d, z0\.d)
**	ret
*/
TEST_COMPARE_Z (cmpne_f64_tied, svfloat64_t,
		p0 = svcmpne_f64 (p0, z0, z1),
		p0 = svcmpne (p0, z0, z1))

/*
** cmpne_f64_untied:
**	fcmne	p0\.d, p1/z, (z0\.d, z1\.d|z1\.d, z0\.d)
**	ret
*/
TEST_COMPARE_Z (cmpne_f64_untied, svfloat64_t,
		p0 = svcmpne_f64 (p1, z0, z1),
		p0 = svcmpne (p1, z0, z1))

/*
** cmpne_x0_f64:
**	mov	(z[0-9]+\.d), x0
**	fcmne	p0\.d, p1/z, (z0\.d, \1|\1, z0\.d)
**	ret
*/
TEST_COMPARE_ZS (cmpne_x0_f64, svfloat64_t, float64_t,
		 p0 = svcmpne_n_f64 (p1, z0, x0),
		 p0 = svcmpne (p1, z0, x0))

/*
** cmpne_d4_f64:
**	mov	(z[0-9]+\.d), d4
**	fcmne	p0\.d, p1/z, (z0\.d, \1|\1, z0\.d)
**	ret
*/
TEST_COMPARE_ZS (cmpne_d4_f64, svfloat64_t, float64_t,
		 p0 = svcmpne_n_f64 (p1, z0, d4),
		 p0 = svcmpne (p1, z0, d4))

/*
** cmpne_0_f64:
**	fcmne	p0\.d, p1/z, z0\.d, #0\.0
**	ret
*/
TEST_COMPARE_ZS (cmpne_0_f64, svfloat64_t, float64_t,
		 p0 = svcmpne_n_f64 (p1, z0, 0),
		 p0 = svcmpne (p1, z0, 0))

/*
** cmpne_1_f64:
**	fmov	(z[0-9]+\.d), #1\.0(?:e\+0)?
**	fcmne	p0\.d, p1/z, (z0\.d, \1|\1, z0\.d)
**	ret
*/
TEST_COMPARE_ZS (cmpne_1_f64, svfloat64_t, float64_t,
		 p0 = svcmpne_n_f64 (p1, z0, 1),
		 p0 = svcmpne (p1, z0, 1))
