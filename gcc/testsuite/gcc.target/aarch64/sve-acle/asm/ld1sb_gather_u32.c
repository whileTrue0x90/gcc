/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** ld1sb_gather_u32_tied1:
**	ld1sb	z0\.s, p0/z, \[z0\.s\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_u32_tied1, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_u32 (p0, z0),
		     z0_res = svld1sb_gather_u32 (p0, z0))

/*
** ld1sb_gather_u32_untied:
**	ld1sb	z0\.s, p0/z, \[z1\.s\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_u32_untied, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_u32 (p0, z1),
		     z0_res = svld1sb_gather_u32 (p0, z1))

/*
** ld1sb_gather_x0_u32_offset:
**	ld1sb	z0\.s, p0/z, \[x0, z0\.s, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_x0_u32_offset, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_offset_u32 (p0, z0, x0),
		     z0_res = svld1sb_gather_offset_u32 (p0, z0, x0))

/*
** ld1sb_gather_d4_u32_offset:
**	fmov	(x[0-9]+), d4
**	ld1sb	z0\.s, p0/z, \[\1, z0\.s, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_d4_u32_offset, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_offset_u32 (p0, z0, d4),
		     z0_res = svld1sb_gather_offset_u32 (p0, z0, d4))

/*
** ld1sb_gather_m1_u32_offset:
**	mov	(x[0-9]+), #?-1
**	ld1sb	z0\.s, p0/z, \[\1, z0\.s, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_m1_u32_offset, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_offset_u32 (p0, z0, -1),
		     z0_res = svld1sb_gather_offset_u32 (p0, z0, -1))

/*
** ld1sb_gather_0_u32_offset:
**	ld1sb	z0\.s, p0/z, \[z0\.s\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_0_u32_offset, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_offset_u32 (p0, z0, 0),
		     z0_res = svld1sb_gather_offset_u32 (p0, z0, 0))

/*
** ld1sb_gather_5_u32_offset:
**	ld1sb	z0\.s, p0/z, \[z0\.s, #5\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_5_u32_offset, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_offset_u32 (p0, z0, 5),
		     z0_res = svld1sb_gather_offset_u32 (p0, z0, 5))

/*
** ld1sb_gather_31_u32_offset:
**	ld1sb	z0\.s, p0/z, \[z0\.s, #31\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_31_u32_offset, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_offset_u32 (p0, z0, 31),
		     z0_res = svld1sb_gather_offset_u32 (p0, z0, 31))

/*
** ld1sb_gather_32_u32_offset:
**	mov	(x[0-9]+), #?32
**	ld1sb	z0\.s, p0/z, \[\1, z0\.s, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_ZS (ld1sb_gather_32_u32_offset, svuint32_t, svuint32_t,
		     z0_res = svld1sb_gather_u32base_offset_u32 (p0, z0, 32),
		     z0_res = svld1sb_gather_offset_u32 (p0, z0, 32))

/*
** ld1sb_gather_d4_u32_s32offset:
**	fmov	(x[0-9]+), d4
**	ld1sb	z0\.s, p0/z, \[\1, z0\.s, sxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_d4_u32_s32offset, svuint32_t, int8_t, svint32_t,
		     z0_res = svld1sb_gather_s32offset_u32 (p0, d4, z0),
		     z0_res = svld1sb_gather_offset_u32 (p0, d4, z0))

/*
** ld1sb_gather_tied1_u32_s32offset:
**	ld1sb	z0\.s, p0/z, \[x0, z0\.s, sxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_tied1_u32_s32offset, svuint32_t, int8_t, svint32_t,
		     z0_res = svld1sb_gather_s32offset_u32 (p0, x0, z0),
		     z0_res = svld1sb_gather_offset_u32 (p0, x0, z0))

/*
** ld1sb_gather_untied_u32_s32offset:
**	ld1sb	z0\.s, p0/z, \[x0, z1\.s, sxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_untied_u32_s32offset, svuint32_t, int8_t, svint32_t,
		     z0_res = svld1sb_gather_s32offset_u32 (p0, x0, z1),
		     z0_res = svld1sb_gather_offset_u32 (p0, x0, z1))

/*
** ld1sb_gather_d4_u32_u32offset:
**	fmov	(x[0-9]+), d4
**	ld1sb	z0\.s, p0/z, \[\1, z0\.s, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_d4_u32_u32offset, svuint32_t, int8_t, svuint32_t,
		     z0_res = svld1sb_gather_u32offset_u32 (p0, d4, z0),
		     z0_res = svld1sb_gather_offset_u32 (p0, d4, z0))

/*
** ld1sb_gather_tied1_u32_u32offset:
**	ld1sb	z0\.s, p0/z, \[x0, z0\.s, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_tied1_u32_u32offset, svuint32_t, int8_t, svuint32_t,
		     z0_res = svld1sb_gather_u32offset_u32 (p0, x0, z0),
		     z0_res = svld1sb_gather_offset_u32 (p0, x0, z0))

/*
** ld1sb_gather_untied_u32_u32offset:
**	ld1sb	z0\.s, p0/z, \[x0, z1\.s, uxtw\]
**	ret
*/
TEST_LOAD_GATHER_SZ (ld1sb_gather_untied_u32_u32offset, svuint32_t, int8_t, svuint32_t,
		     z0_res = svld1sb_gather_u32offset_u32 (p0, x0, z1),
		     z0_res = svld1sb_gather_offset_u32 (p0, x0, z1))
