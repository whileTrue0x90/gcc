/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** cmpne_u8_tied:
**	cmpne	p0\.b, p0/z, (z0\.b, z1\.b|z1\.b, z0\.b)
**	ret
*/
TEST_COMPARE_Z (cmpne_u8_tied, svuint8_t,
		p0 = svcmpne_u8 (p0, z0, z1),
		p0 = svcmpne (p0, z0, z1))

/*
** cmpne_u8_untied:
**	cmpne	p0\.b, p1/z, (z0\.b, z1\.b|z1\.b, z0\.b)
**	ret
*/
TEST_COMPARE_Z (cmpne_u8_untied, svuint8_t,
		p0 = svcmpne_u8 (p1, z0, z1),
		p0 = svcmpne (p1, z0, z1))

/*
** cmpne_x0_u8:
**	mov	(z[0-9]+\.d), x0
**	cmpne	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_x0_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, x0),
		 p0 = svcmpne (p1, z0, x0))

/*
** cmpne_d4_u8:
**	mov	(z[0-9]+\.d), d4
**	cmpne	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_d4_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, d4),
		 p0 = svcmpne (p1, z0, d4))

/*
** cmpne_0_u8:
**	cmpne	p0\.b, p1/z, z0\.b, #0
**	ret
*/
TEST_COMPARE_ZS (cmpne_0_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, 0),
		 p0 = svcmpne (p1, z0, 0))

/*
** cmpne_1_u8:
**	cmpne	p0\.b, p1/z, z0\.b, #1
**	ret
*/
TEST_COMPARE_ZS (cmpne_1_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, 1),
		 p0 = svcmpne (p1, z0, 1))

/*
** cmpne_15_u8:
**	cmpne	p0\.b, p1/z, z0\.b, #15
**	ret
*/
TEST_COMPARE_ZS (cmpne_15_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, 15),
		 p0 = svcmpne (p1, z0, 15))

/*
** cmpne_16_u8:
**	mov	(z[0-9]+\.d), #16
**	cmpne	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_16_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, 16),
		 p0 = svcmpne (p1, z0, 16))

/*
** cmpne_m1_u8:
**	cmpne	p0\.b, p1/z, z0\.b, #-1
**	ret
*/
TEST_COMPARE_ZS (cmpne_m1_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, -1),
		 p0 = svcmpne (p1, z0, -1))

/*
** cmpne_m16_u8:
**	cmpne	p0\.b, p1/z, z0\.b, #-16
**	ret
*/
TEST_COMPARE_ZS (cmpne_m16_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, -16),
		 p0 = svcmpne (p1, z0, -16))

/*
** cmpne_m17_u8:
**	mov	(z[0-9]+\.d), #-17
**	cmpne	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpne_m17_u8, svuint8_t, uint64_t,
		 p0 = svcmpne_n_u8 (p1, z0, -17),
		 p0 = svcmpne (p1, z0, -17))
