/* { dg-do compile } */
/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** msb_f32_m_tied1:
**	fmsb	z0\.s, p0/m, z1\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_m_tied1, svfloat32_t,
		z0 = svmsb_f32_m (p0, z0, z1, z2),
		z0 = svmsb_m (p0, z0, z1, z2))

/*
** msb_f32_m_tied2:
**	mov	(z[0-9]+)\.d, z1\.d
**	movprfx	z1, z0
**	fmsb	z1\.s, p0/m, \1\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_m_tied2, svfloat32_t,
		z1 = svmsb_f32_m (p0, z0, z1, z2),
		z1 = svmsb_m (p0, z0, z1, z2))

/*
** msb_f32_m_tied3:
**	mov	(z[0-9]+)\.d, z2\.d
**	movprfx	z2, z0
**	fmsb	z2\.s, p0/m, z1\.s, \1\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_m_tied3, svfloat32_t,
		z2 = svmsb_f32_m (p0, z0, z1, z2),
		z2 = svmsb_m (p0, z0, z1, z2))

/*
** msb_f32_m_untied:
**	movprfx	z0, z1
**	fmsb	z0\.s, p0/m, z2\.s, z3\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_m_untied, svfloat32_t,
		z0 = svmsb_f32_m (p0, z1, z2, z3),
		z0 = svmsb_m (p0, z1, z2, z3))

/*
** msb_w0_f32_m_tied1:
**	mov	(z[0-9]+\.s), w0
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_m_tied1, svfloat32_t, float,
		 z0 = svmsb_n_f32_m (p0, z0, z1, x0),
		 z0 = svmsb_m (p0, z0, z1, x0))

/*
** msb_w0_f32_m_tied2:
**	mov	(z[0-9]+\.s), w0
**	mov	(z[0-9]+)\.d, z1\.d
**	movprfx	z1, z0
**	fmsb	z1\.s, p0/m, \2\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_m_tied2, svfloat32_t, float,
		 z1 = svmsb_n_f32_m (p0, z0, z1, x0),
		 z1 = svmsb_m (p0, z0, z1, x0))

/*
** msb_w0_f32_m_untied:
**	mov	(z[0-9]+\.s), w0
**	movprfx	z0, z1
**	fmsb	z0\.s, p0/m, z2\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_m_untied, svfloat32_t, float,
		 z0 = svmsb_n_f32_m (p0, z1, z2, x0),
		 z0 = svmsb_m (p0, z1, z2, x0))

/*
** msb_s0_f32_m_tied1:
**	mov	(z[0-9]+\.s), s0
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_m_tied1, svfloat32_t, float,
		 z0 = svmsb_n_f32_m (p0, z0, z1, d0),
		 z0 = svmsb_m (p0, z0, z1, d0))

/*
** msb_s0_f32_m_tied2:
**	mov	(z[0-9]+\.s), s0
**	mov	(z[0-9]+)\.d, z1\.d
**	movprfx	z1, z0
**	fmsb	z1\.s, p0/m, \2\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_m_tied2, svfloat32_t, float,
		 z1 = svmsb_n_f32_m (p0, z0, z1, d0),
		 z1 = svmsb_m (p0, z0, z1, d0))

/*
** msb_s0_f32_m_untied:
**	mov	(z[0-9]+\.s), s0
**	movprfx	z1, z0
**	fmsb	z1\.s, p0/m, z2\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_m_untied, svfloat32_t, float,
		 z1 = svmsb_n_f32_m (p0, z0, z2, d0),
		 z1 = svmsb_m (p0, z0, z2, d0))

/*
** msb_2_f32_m_tied1:
**	fmov	(z[0-9]+\.s), #2.0(e\+0)?
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_Z (msb_2_f32_m_tied1, svfloat32_t,
		z0 = svmsb_n_f32_m (p0, z0, z1, 2),
		z0 = svmsb_m (p0, z0, z1, 2))

/*
** msb_2_f32_m_tied2:
**	fmov	(z[0-9]+\.s), #2.0(e\+0)?
**	mov	(z[0-9]+)\.d, z1\.d
**	movprfx	z1, z0
**	fmsb	z1\.s, p0/m, \3\.s, \1
**	ret
*/
TEST_UNIFORM_Z (msb_2_f32_m_tied2, svfloat32_t,
		z1 = svmsb_n_f32_m (p0, z0, z1, 2),
		z1 = svmsb_m (p0, z0, z1, 2))

/*
** msb_2_f32_m_untied:
**	fmov	(z[0-9]+\.s), #2.0(e\+0)?
**	movprfx	z0, z1
**	fmsb	z0\.s, p0/m, z2\.s, \1
**	ret
*/
TEST_UNIFORM_Z (msb_2_f32_m_untied, svfloat32_t,
		z0 = svmsb_n_f32_m (p0, z1, z2, 2),
		z0 = svmsb_m (p0, z1, z2, 2))

/*
** msb_f32_z_tied1:
**	movprfx	z0\.s, p0/z, z0\.s
**	fmsb	z0\.s, p0/m, z1\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_z_tied1, svfloat32_t,
		z0 = svmsb_f32_z (p0, z0, z1, z2),
		z0 = svmsb_z (p0, z0, z1, z2))

/*
** msb_f32_z_tied2:
**	movprfx	z1\.s, p0/z, z1\.s
**	fmsb	z1\.s, p0/m, z0\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_z_tied2, svfloat32_t,
		z1 = svmsb_f32_z (p0, z0, z1, z2),
		z1 = svmsb_z (p0, z0, z1, z2))

/*
** msb_f32_z_tied3:
**	movprfx	z2\.s, p0/z, z2\.s
**	fmls	z2\.s, p0/m, z0\.s, z1\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_z_tied3, svfloat32_t,
		z2 = svmsb_f32_z (p0, z0, z1, z2),
		z2 = svmsb_z (p0, z0, z1, z2))

/*
** msb_f32_z_untied:
**	movprfx	z0\.s, p0/z, z3\.s
**	fmls	z0\.s, p0/m, z1\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_z_untied, svfloat32_t,
		z0 = svmsb_f32_z (p0, z1, z2, z3),
		z0 = svmsb_z (p0, z1, z2, z3))

/*
** msb_w0_f32_z_tied1:
**	mov	(z[0-9]+\.s), w0
**	movprfx	z0\.s, p0/z, z0\.s
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_z_tied1, svfloat32_t, float,
		 z0 = svmsb_n_f32_z (p0, z0, z1, x0),
		 z0 = svmsb_z (p0, z0, z1, x0))

/*
** msb_w0_f32_z_tied2:
**	mov	(z[0-9]+\.s), w0
**	movprfx	z1\.s, p0/z, z1\.s
**	fmsb	z1\.s, p0/m, z0\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_z_tied2, svfloat32_t, float,
		 z1 = svmsb_n_f32_z (p0, z0, z1, x0),
		 z1 = svmsb_z (p0, z0, z1, x0))

/*
** msb_w0_f32_z_untied:
**	mov	(z[0-9]+\.s), w0
**	movprfx	z0\.s, p0/z, \1
**	fmls	z0\.s, p0/m, z1\.s, z2\.s
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_z_untied, svfloat32_t, float,
		 z0 = svmsb_n_f32_z (p0, z1, z2, x0),
		 z0 = svmsb_z (p0, z1, z2, x0))

/*
** msb_s0_f32_z_tied1:
**	mov	(z[0-9]+\.s), s0
**	movprfx	z0\.s, p0/z, z0\.s
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_z_tied1, svfloat32_t, float,
		 z0 = svmsb_n_f32_z (p0, z0, z1, d0),
		 z0 = svmsb_z (p0, z0, z1, d0))

/*
** msb_s0_f32_z_tied2:
**	mov	(z[0-9]+\.s), s0
**	movprfx	z1\.s, p0/z, z1\.s
**	fmsb	z1\.s, p0/m, z0\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_z_tied2, svfloat32_t, float,
		 z1 = svmsb_n_f32_z (p0, z0, z1, d0),
		 z1 = svmsb_z (p0, z0, z1, d0))

/*
** msb_s0_f32_z_untied:
**	mov	(z[0-9]+\.s), s0
**	movprfx	z1\.s, p0/z, \1
**	fmls	z1\.s, p0/m, z0\.s, z2\.s
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_z_untied, svfloat32_t, float,
		 z1 = svmsb_n_f32_z (p0, z0, z2, d0),
		 z1 = svmsb_z (p0, z0, z2, d0))

/*
** msb_f32_x_tied1:
**	fmsb	z0\.s, p0/m, z1\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_x_tied1, svfloat32_t,
		z0 = svmsb_f32_x (p0, z0, z1, z2),
		z0 = svmsb_x (p0, z0, z1, z2))

/*
** msb_f32_x_tied2:
**	fmsb	z1\.s, p0/m, z0\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_x_tied2, svfloat32_t,
		z1 = svmsb_f32_x (p0, z0, z1, z2),
		z1 = svmsb_x (p0, z0, z1, z2))

/*
** msb_f32_x_tied3:
**	fmls	z2\.s, p0/m, z0\.s, z1\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_x_tied3, svfloat32_t,
		z2 = svmsb_f32_x (p0, z0, z1, z2),
		z2 = svmsb_x (p0, z0, z1, z2))

/*
** msb_f32_x_untied:
**	movprfx	z3, z2
**	fmls	z3\.s, p0/m, z0\.s, z1\.s
**	ret
*/
TEST_UNIFORM_Z (msb_f32_x_untied, svfloat32_t,
		z3 = svmsb_f32_x (p0, z0, z1, z2),
		z3 = svmsb_x (p0, z0, z1, z2))

/*
** msb_w0_f32_x_tied1:
**	mov	(z[0-9]+\.s), w0
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_x_tied1, svfloat32_t, float,
		 z0 = svmsb_n_f32_x (p0, z0, z1, x0),
		 z0 = svmsb_x (p0, z0, z1, x0))

/*
** msb_w0_f32_x_tied2:
**	mov	(z[0-9]+\.s), w0
**	fmsb	z1\.s, p0/m, z0\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_x_tied2, svfloat32_t, float,
		 z1 = svmsb_n_f32_x (p0, z0, z1, x0),
		 z1 = svmsb_x (p0, z0, z1, x0))

/*
** msb_w0_f32_x_untied:
**	mov	z2\.s, w0
**	fmls	z2\.s, p0/m, z0\.s, z1\.s
**	ret
*/
TEST_UNIFORM_ZS (msb_w0_f32_x_untied, svfloat32_t, float,
		 z2 = svmsb_n_f32_x (p0, z0, z1, x0),
		 z2 = svmsb_x (p0, z0, z1, x0))

/*
** msb_s0_f32_x_tied1:
**	mov	(z[0-9]+\.s), s0
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_x_tied1, svfloat32_t, float,
		 z0 = svmsb_n_f32_x (p0, z0, z1, d0),
		 z0 = svmsb_x (p0, z0, z1, d0))

/*
** msb_s0_f32_x_tied2:
**	mov	(z[0-9]+\.s), s0
**	fmsb	z1\.s, p0/m, z0\.s, \1
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_x_tied2, svfloat32_t, float,
		 z1 = svmsb_n_f32_x (p0, z0, z1, d0),
		 z1 = svmsb_x (p0, z0, z1, d0))

/*
** msb_s0_f32_x_untied:
**	mov	z2\.s, s0
**	fmls	z2\.s, p0/m, z0\.s, z1\.s
**	ret
*/
TEST_UNIFORM_ZS (msb_s0_f32_x_untied, svfloat32_t, float,
		 z2 = svmsb_n_f32_x (p0, z0, z1, d0),
		 z2 = svmsb_x (p0, z0, z1, d0))

/*
** msb_2_f32_x_tied1:
**	fmov	(z[0-9]+\.s), #2.0(e\+0)?
**	fmsb	z0\.s, p0/m, z1\.s, \1
**	ret
*/
TEST_UNIFORM_Z (msb_2_f32_x_tied1, svfloat32_t,
		z0 = svmsb_n_f32_x (p0, z0, z1, 2),
		z0 = svmsb_x (p0, z0, z1, 2))

/*
** msb_2_f32_x_tied2:
**	fmov	(z[0-9]+\.s), #2.0(e\+0)?
**	fmsb	z1\.s, p0/m, z0\.s, \1
**	ret
*/
TEST_UNIFORM_Z (msb_2_f32_x_tied2, svfloat32_t,
		z1 = svmsb_n_f32_x (p0, z0, z1, 2),
		z1 = svmsb_x (p0, z0, z1, 2))

/*
** msb_2_f32_x_untied:
**	fmov	z0\.s, #2.0(e\+0)?
**	fmls	z0\.s, p0/m, z1\.s, z2\.s
**	ret
*/
TEST_UNIFORM_Z (msb_2_f32_x_untied, svfloat32_t,
		z0 = svmsb_n_f32_x (p0, z1, z2, 2),
		z0 = svmsb_x (p0, z1, z2, 2))

