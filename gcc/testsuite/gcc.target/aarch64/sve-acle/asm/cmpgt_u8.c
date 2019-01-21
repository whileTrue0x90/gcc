/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** cmpgt_u8_tied:
** (
**	cmphi	p0\.b, p0/z, z0\.b, z1\.b
** |
**	cmplo	p0\.b, p0/z, z1\.b, z0\.b
** )
**	ret
*/
TEST_COMPARE_Z (cmpgt_u8_tied, svuint8_t,
		p0 = svcmpgt_u8 (p0, z0, z1),
		p0 = svcmpgt (p0, z0, z1))

/*
** cmpgt_u8_untied:
** (
**	cmphi	p0\.b, p1/z, z0\.b, z1\.b
** |
**	cmplo	p0\.b, p1/z, z1\.b, z0\.b
** )
**	ret
*/
TEST_COMPARE_Z (cmpgt_u8_untied, svuint8_t,
		p0 = svcmpgt_u8 (p1, z0, z1),
		p0 = svcmpgt (p1, z0, z1))

/*
** cmpgt_x0_u8:
**	mov	(z[0-9]+\.d), x0
**	cmphi	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpgt_x0_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, x0),
		 p0 = svcmpgt (p1, z0, x0))

/*
** cmpgt_d4_u8:
**	mov	(z[0-9]+\.d), d4
**	cmphi	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpgt_d4_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, d4),
		 p0 = svcmpgt (p1, z0, d4))

/*
** cmpgt_0_u8:
**	cmphi	p0\.b, p1/z, z0\.b, #0
**	ret
*/
TEST_COMPARE_ZS (cmpgt_0_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, 0),
		 p0 = svcmpgt (p1, z0, 0))

/*
** cmpgt_1_u8:
**	cmphi	p0\.b, p1/z, z0\.b, #1
**	ret
*/
TEST_COMPARE_ZS (cmpgt_1_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, 1),
		 p0 = svcmpgt (p1, z0, 1))

/*
** cmpgt_15_u8:
**	cmphi	p0\.b, p1/z, z0\.b, #15
**	ret
*/
TEST_COMPARE_ZS (cmpgt_15_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, 15),
		 p0 = svcmpgt (p1, z0, 15))

/*
** cmpgt_16_u8:
**	cmphi	p0\.b, p1/z, z0\.b, #16
**	ret
*/
TEST_COMPARE_ZS (cmpgt_16_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, 16),
		 p0 = svcmpgt (p1, z0, 16))

/*
** cmpgt_127_u8:
**	cmphi	p0\.b, p1/z, z0\.b, #127
**	ret
*/
TEST_COMPARE_ZS (cmpgt_127_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, 127),
		 p0 = svcmpgt (p1, z0, 127))

/*
** cmpgt_128_u8:
**	mov	(z[0-9]+\.d), #128
**	cmphi	p0\.b, p1/z, z0\.b, \1
**	ret
*/
TEST_COMPARE_ZS (cmpgt_128_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, 128),
		 p0 = svcmpgt (p1, z0, 128))

/*
** cmpgt_m1_u8:
**	mov	(z[0-9]+)\.b, #-1
**	cmphi	p0\.b, p1/z, z0\.b, \1\.d
**	ret
*/
TEST_COMPARE_ZS (cmpgt_m1_u8, svuint8_t, uint64_t,
		 p0 = svcmpgt_n_u8 (p1, z0, -1),
		 p0 = svcmpgt (p1, z0, -1))
