/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** max_f16_m_tied1:
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_Z (max_f16_m_tied1, svfloat16_t,
		z0 = svmax_f16_m (p0, z0, z1),
		z0 = svmax_m (p0, z0, z1))

/*
** max_f16_m_tied2:
**	mov	(z[0-9]+)\.d, z0\.d
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, \1\.h
**	ret
*/
TEST_UNIFORM_Z (max_f16_m_tied2, svfloat16_t,
		z0 = svmax_f16_m (p0, z1, z0),
		z0 = svmax_m (p0, z1, z0))

/*
** max_f16_m_untied:
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, z2\.h
**	ret
*/
TEST_UNIFORM_Z (max_f16_m_untied, svfloat16_t,
		z0 = svmax_f16_m (p0, z1, z2),
		z0 = svmax_m (p0, z1, z2))

/*
** max_h4_f16_m_tied1:
**	mov	(z[0-9]+\.h), h4
**	fmax	z0\.h, p0/m, z0\.h, \1
**	ret
*/
TEST_UNIFORM_ZD (max_h4_f16_m_tied1, svfloat16_t, __fp16,
		 z0 = svmax_n_f16_m (p0, z0, d4),
		 z0 = svmax_m (p0, z0, d4))

/*
** max_h4_f16_m_untied:
**	mov	(z[0-9]+\.h), h4
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, \1
**	ret
*/
TEST_UNIFORM_ZD (max_h4_f16_m_untied, svfloat16_t, __fp16,
		 z0 = svmax_n_f16_m (p0, z1, d4),
		 z0 = svmax_m (p0, z1, d4))

/*
** max_0_f16_m_tied1:
**	fmax	z0\.h, p0/m, z0\.h, #0\.0
**	ret
*/
TEST_UNIFORM_Z (max_0_f16_m_tied1, svfloat16_t,
		z0 = svmax_n_f16_m (p0, z0, 0),
		z0 = svmax_m (p0, z0, 0))

/*
** max_0_f16_m_untied:
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, #0\.0
**	ret
*/
TEST_UNIFORM_Z (max_0_f16_m_untied, svfloat16_t,
		z0 = svmax_n_f16_m (p0, z1, 0),
		z0 = svmax_m (p0, z1, 0))

/*
** max_1_f16_m_tied1:
**	fmax	z0\.h, p0/m, z0\.h, #1\.0
**	ret
*/
TEST_UNIFORM_Z (max_1_f16_m_tied1, svfloat16_t,
		z0 = svmax_n_f16_m (p0, z0, 1),
		z0 = svmax_m (p0, z0, 1))

/*
** max_1_f16_m_untied:
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, #1\.0
**	ret
*/
TEST_UNIFORM_Z (max_1_f16_m_untied, svfloat16_t,
		z0 = svmax_n_f16_m (p0, z1, 1),
		z0 = svmax_m (p0, z1, 1))

/*
** max_2_f16_m:
**	fmov	(z[0-9]+\.h), #2\.0(?:e\+0)?
**	fmax	z0\.h, p0/m, z0\.h, \1
**	ret
*/
TEST_UNIFORM_Z (max_2_f16_m, svfloat16_t,
		z0 = svmax_n_f16_m (p0, z0, 2),
		z0 = svmax_m (p0, z0, 2))

/*
** max_f16_z_tied1:
**	movprfx	z0\.h, p0/z, z0\.h
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_Z (max_f16_z_tied1, svfloat16_t,
		z0 = svmax_f16_z (p0, z0, z1),
		z0 = svmax_z (p0, z0, z1))

/*
** max_f16_z_tied2:
**	movprfx	z0\.h, p0/z, z0\.h
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_Z (max_f16_z_tied2, svfloat16_t,
		z0 = svmax_f16_z (p0, z1, z0),
		z0 = svmax_z (p0, z1, z0))

/*
** max_f16_z_untied:
** (
**	movprfx	z0\.h, p0/z, z1\.h
**	fmax	z0\.h, p0/m, z0\.h, z2\.h
** |
**	movprfx	z0\.h, p0/z, z2\.h
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
** )
**	ret
*/
TEST_UNIFORM_Z (max_f16_z_untied, svfloat16_t,
		z0 = svmax_f16_z (p0, z1, z2),
		z0 = svmax_z (p0, z1, z2))

/*
** max_h4_f16_z_tied1:
**	mov	(z[0-9]+\.h), h4
**	movprfx	z0\.h, p0/z, z0\.h
**	fmax	z0\.h, p0/m, z0\.h, \1
**	ret
*/
TEST_UNIFORM_ZD (max_h4_f16_z_tied1, svfloat16_t, __fp16,
		 z0 = svmax_n_f16_z (p0, z0, d4),
		 z0 = svmax_z (p0, z0, d4))

/*
** max_h4_f16_z_untied:
**	mov	(z[0-9]+\.h), h4
** (
**	movprfx	z0\.h, p0/z, z1\.h
**	fmax	z0\.h, p0/m, z0\.h, \1
** |
**	movprfx	z0\.h, p0/z, \1
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
** )
**	ret
*/
TEST_UNIFORM_ZD (max_h4_f16_z_untied, svfloat16_t, __fp16,
		 z0 = svmax_n_f16_z (p0, z1, d4),
		 z0 = svmax_z (p0, z1, d4))

/*
** max_0_f16_z_tied1:
**	movprfx	z0\.h, p0/z, z0\.h
**	fmax	z0\.h, p0/m, z0\.h, #0\.0
**	ret
*/
TEST_UNIFORM_Z (max_0_f16_z_tied1, svfloat16_t,
		z0 = svmax_n_f16_z (p0, z0, 0),
		z0 = svmax_z (p0, z0, 0))

/*
** max_0_f16_z_untied:
**	movprfx	z0\.h, p0/z, z1\.h
**	fmax	z0\.h, p0/m, z0\.h, #0\.0
**	ret
*/
TEST_UNIFORM_Z (max_0_f16_z_untied, svfloat16_t,
		z0 = svmax_n_f16_z (p0, z1, 0),
		z0 = svmax_z (p0, z1, 0))

/*
** max_1_f16_z_tied1:
**	movprfx	z0\.h, p0/z, z0\.h
**	fmax	z0\.h, p0/m, z0\.h, #1\.0
**	ret
*/
TEST_UNIFORM_Z (max_1_f16_z_tied1, svfloat16_t,
		z0 = svmax_n_f16_z (p0, z0, 1),
		z0 = svmax_z (p0, z0, 1))

/*
** max_1_f16_z_untied:
**	movprfx	z0\.h, p0/z, z1\.h
**	fmax	z0\.h, p0/m, z0\.h, #1\.0
**	ret
*/
TEST_UNIFORM_Z (max_1_f16_z_untied, svfloat16_t,
		z0 = svmax_n_f16_z (p0, z1, 1),
		z0 = svmax_z (p0, z1, 1))

/*
** max_2_f16_z:
**	fmov	(z[0-9]+\.h), #2\.0(?:e\+0)?
**	movprfx	z0\.h, p0/z, z0\.h
**	fmax	z0\.h, p0/m, z0\.h, \1
**	ret
*/
TEST_UNIFORM_Z (max_2_f16_z, svfloat16_t,
		z0 = svmax_n_f16_z (p0, z0, 2),
		z0 = svmax_z (p0, z0, 2))

/*
** max_f16_x_tied1:
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_Z (max_f16_x_tied1, svfloat16_t,
		z0 = svmax_f16_x (p0, z0, z1),
		z0 = svmax_x (p0, z0, z1))

/*
** max_f16_x_tied2:
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_Z (max_f16_x_tied2, svfloat16_t,
		z0 = svmax_f16_x (p0, z1, z0),
		z0 = svmax_x (p0, z1, z0))

/*
** max_f16_x_untied:
** (
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, z2\.h
** |
**	movprfx	z0, z2
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
** )
**	ret
*/
TEST_UNIFORM_Z (max_f16_x_untied, svfloat16_t,
		z0 = svmax_f16_x (p0, z1, z2),
		z0 = svmax_x (p0, z1, z2))

/*
** max_h4_f16_x_tied1:
**	mov	(z[0-9]+\.h), h4
**	fmax	z0\.h, p0/m, z0\.h, \1
**	ret
*/
TEST_UNIFORM_ZD (max_h4_f16_x_tied1, svfloat16_t, __fp16,
		 z0 = svmax_n_f16_x (p0, z0, d4),
		 z0 = svmax_x (p0, z0, d4))

/*
** max_h4_f16_x_untied:
**	mov	z0\.h, h4
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_ZD (max_h4_f16_x_untied, svfloat16_t, __fp16,
		 z0 = svmax_n_f16_x (p0, z1, d4),
		 z0 = svmax_x (p0, z1, d4))

/*
** max_0_f16_x_tied1:
**	fmax	z0\.h, p0/m, z0\.h, #0\.0
**	ret
*/
TEST_UNIFORM_Z (max_0_f16_x_tied1, svfloat16_t,
		z0 = svmax_n_f16_x (p0, z0, 0),
		z0 = svmax_x (p0, z0, 0))

/*
** max_0_f16_x_untied:
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, #0\.0
**	ret
*/
TEST_UNIFORM_Z (max_0_f16_x_untied, svfloat16_t,
		z0 = svmax_n_f16_x (p0, z1, 0),
		z0 = svmax_x (p0, z1, 0))

/*
** max_1_f16_x_tied1:
**	fmax	z0\.h, p0/m, z0\.h, #1\.0
**	ret
*/
TEST_UNIFORM_Z (max_1_f16_x_tied1, svfloat16_t,
		z0 = svmax_n_f16_x (p0, z0, 1),
		z0 = svmax_x (p0, z0, 1))

/*
** max_1_f16_x_untied:
**	movprfx	z0, z1
**	fmax	z0\.h, p0/m, z0\.h, #1\.0
**	ret
*/
TEST_UNIFORM_Z (max_1_f16_x_untied, svfloat16_t,
		z0 = svmax_n_f16_x (p0, z1, 1),
		z0 = svmax_x (p0, z1, 1))

/*
** max_2_f16_x_tied1:
**	fmov	(z[0-9]+\.h), #2\.0(?:e\+0)?
**	fmax	z0\.h, p0/m, z0\.h, \1
**	ret
*/
TEST_UNIFORM_Z (max_2_f16_x_tied1, svfloat16_t,
		z0 = svmax_n_f16_x (p0, z0, 2),
		z0 = svmax_x (p0, z0, 2))

/*
** max_2_f16_x_untied:
**	fmov	z0\.h, #2\.0(?:e\+0)?
**	fmax	z0\.h, p0/m, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_Z (max_2_f16_x_untied, svfloat16_t,
		z0 = svmax_n_f16_x (p0, z1, 2),
		z0 = svmax_x (p0, z1, 2))

/*
** ptrue_max_f16_x_tied1:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_f16_x_tied1, svfloat16_t,
		z0 = svmax_f16_x (svptrue_b16 (), z0, z1),
		z0 = svmax_x (svptrue_b16 (), z0, z1))

/*
** ptrue_max_f16_x_tied2:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_f16_x_tied2, svfloat16_t,
		z0 = svmax_f16_x (svptrue_b16 (), z1, z0),
		z0 = svmax_x (svptrue_b16 (), z1, z0))

/*
** ptrue_max_f16_x_untied:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_f16_x_untied, svfloat16_t,
		z0 = svmax_f16_x (svptrue_b16 (), z1, z2),
		z0 = svmax_x (svptrue_b16 (), z1, z2))

/*
** ptrue_max_0_f16_x_tied1:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_0_f16_x_tied1, svfloat16_t,
		z0 = svmax_n_f16_x (svptrue_b16 (), z0, 0),
		z0 = svmax_x (svptrue_b16 (), z0, 0))

/*
** ptrue_max_0_f16_x_untied:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_0_f16_x_untied, svfloat16_t,
		z0 = svmax_n_f16_x (svptrue_b16 (), z1, 0),
		z0 = svmax_x (svptrue_b16 (), z1, 0))

/*
** ptrue_max_1_f16_x_tied1:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_1_f16_x_tied1, svfloat16_t,
		z0 = svmax_n_f16_x (svptrue_b16 (), z0, 1),
		z0 = svmax_x (svptrue_b16 (), z0, 1))

/*
** ptrue_max_1_f16_x_untied:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_1_f16_x_untied, svfloat16_t,
		z0 = svmax_n_f16_x (svptrue_b16 (), z1, 1),
		z0 = svmax_x (svptrue_b16 (), z1, 1))

/*
** ptrue_max_2_f16_x_tied1:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_2_f16_x_tied1, svfloat16_t,
		z0 = svmax_n_f16_x (svptrue_b16 (), z0, 2),
		z0 = svmax_x (svptrue_b16 (), z0, 2))

/*
** ptrue_max_2_f16_x_untied:
**	...
**	ptrue	p[0-9]+\.b[^\n]*
**	...
**	ret
*/
TEST_UNIFORM_Z (ptrue_max_2_f16_x_untied, svfloat16_t,
		z0 = svmax_n_f16_x (svptrue_b16 (), z1, 2),
		z0 = svmax_x (svptrue_b16 (), z1, 2))
