/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** ld1sb_gather_s64_tied1:
**	ld1sb	z0\.d, p0/z, \[z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_s64_tied1, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_s64 (p0, z0),
		     z0_res = svld1sb_gather_s64 (p0, z0))

/*
** ld1sb_gather_s64_untied:
**	ld1sb	z0\.d, p0/z, \[z1\.d\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_s64_untied, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_s64 (p0, z1),
		     z0_res = svld1sb_gather_s64 (p0, z1))

/*
** ld1sb_gather_x0_s64_offset:
**	ld1sb	z0\.d, p0/z, \[x0, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_x0_s64_offset, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_offset_s64 (p0, z0, x0),
		     z0_res = svld1sb_gather_offset_s64 (p0, z0, x0))

/*
** ld1sb_gather_d4_s64_offset:
**	fmov	(x[0-9]+), d4
**	ld1sb	z0\.d, p0/z, \[\1, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_d4_s64_offset, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_offset_s64 (p0, z0, d4),
		     z0_res = svld1sb_gather_offset_s64 (p0, z0, d4))

/*
** ld1sb_gather_m1_s64_offset:
**	mov	(x[0-9]+), #?-1
**	ld1sb	z0\.d, p0/z, \[\1, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_m1_s64_offset, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_offset_s64 (p0, z0, -1),
		     z0_res = svld1sb_gather_offset_s64 (p0, z0, -1))

/*
** ld1sb_gather_0_s64_offset:
**	ld1sb	z0\.d, p0/z, \[z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_0_s64_offset, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_offset_s64 (p0, z0, 0),
		     z0_res = svld1sb_gather_offset_s64 (p0, z0, 0))

/*
** ld1sb_gather_5_s64_offset:
**	ld1sb	z0\.d, p0/z, \[z0\.d, #5\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_5_s64_offset, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_offset_s64 (p0, z0, 5),
		     z0_res = svld1sb_gather_offset_s64 (p0, z0, 5))

/*
** ld1sb_gather_31_s64_offset:
**	ld1sb	z0\.d, p0/z, \[z0\.d, #31\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_31_s64_offset, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_offset_s64 (p0, z0, 31),
		     z0_res = svld1sb_gather_offset_s64 (p0, z0, 31))

/*
** ld1sb_gather_32_s64_offset:
**	mov	(x[0-9]+), #?32
**	ld1sb	z0\.d, p0/z, \[\1, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_32_s64_offset, svint64_t, svuint64_t,
		     z0_res = svld1sb_gather_u64base_offset_s64 (p0, z0, 32),
		     z0_res = svld1sb_gather_offset_s64 (p0, z0, 32))

/*
** ld1sb_gather_d4_s64_s64offset:
**	fmov	(x[0-9]+), d4
**	ld1sb	z0\.d, p0/z, \[\1, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_d4_s64_s64offset, svint64_t, int8_t, svint64_t,
		     z0_res = svld1sb_gather_s64offset_s64 (p0, d4, z0),
		     z0_res = svld1sb_gather_offset_s64 (p0, d4, z0))

/*
** ld1sb_gather_tied1_s64_s64offset:
**	ld1sb	z0\.d, p0/z, \[x0, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_tied1_s64_s64offset, svint64_t, int8_t, svint64_t,
		     z0_res = svld1sb_gather_s64offset_s64 (p0, x0, z0),
		     z0_res = svld1sb_gather_offset_s64 (p0, x0, z0))

/*
** ld1sb_gather_untied_s64_s64offset:
**	ld1sb	z0\.d, p0/z, \[x0, z1\.d\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_untied_s64_s64offset, svint64_t, int8_t, svint64_t,
		     z0_res = svld1sb_gather_s64offset_s64 (p0, x0, z1),
		     z0_res = svld1sb_gather_offset_s64 (p0, x0, z1))

/*
** ld1sb_gather_ext_s64_s64offset:
**	ld1sb	z0\.d, p0/z, \[x0, z1\.d, sxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_ext_s64_s64offset, svint64_t, int8_t, svint64_t,
		     z0_res = svld1sb_gather_s64offset_s64 (p0, x0, svextw_s64_x (p0, z1)),
		     z0_res = svld1sb_gather_offset_s64 (p0, x0, svextw_x (p0, z1)))

/*
** ld1sb_gather_d4_s64_u64offset:
**	fmov	(x[0-9]+), d4
**	ld1sb	z0\.d, p0/z, \[\1, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_d4_s64_u64offset, svint64_t, int8_t, svuint64_t,
		     z0_res = svld1sb_gather_u64offset_s64 (p0, d4, z0),
		     z0_res = svld1sb_gather_offset_s64 (p0, d4, z0))

/*
** ld1sb_gather_tied1_s64_u64offset:
**	ld1sb	z0\.d, p0/z, \[x0, z0\.d\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_tied1_s64_u64offset, svint64_t, int8_t, svuint64_t,
		     z0_res = svld1sb_gather_u64offset_s64 (p0, x0, z0),
		     z0_res = svld1sb_gather_offset_s64 (p0, x0, z0))

/*
** ld1sb_gather_untied_s64_u64offset:
**	ld1sb	z0\.d, p0/z, \[x0, z1\.d\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_untied_s64_u64offset, svint64_t, int8_t, svuint64_t,
		     z0_res = svld1sb_gather_u64offset_s64 (p0, x0, z1),
		     z0_res = svld1sb_gather_offset_s64 (p0, x0, z1))

/*
** ld1sb_gather_ext_s64_u64offset:
**	ld1sb	z0\.d, p0/z, \[x0, z1\.d, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_ext_s64_u64offset, svint64_t, int8_t, svuint64_t,
		     z0_res = svld1sb_gather_u64offset_s64 (p0, x0, svextw_u64_x (p0, z1)),
		     z0_res = svld1sb_gather_offset_s64 (p0, x0, svextw_x (p0, z1)))
