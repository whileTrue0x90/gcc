/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** orr_u8_m_tied1:
**	orr	z0\.b, p0/m, z0\.b, z1\.b
**	ret
*/
TEST_UNIFORM_Z (orr_u8_m_tied1, svuint8_t,
		z0 = svorr_u8_m (p0, z0, z1),
		z0 = svorr_m (p0, z0, z1))

/*
** orr_u8_m_tied2:
**	mov	(z[0-9]+)\.d, z0\.d
**	movprfx	z0, z1
**	orr	z0\.b, p0/m, z0\.b, \1\.b
**	ret
*/
TEST_UNIFORM_Z (orr_u8_m_tied2, svuint8_t,
		z0 = svorr_u8_m (p0, z1, z0),
		z0 = svorr_m (p0, z1, z0))

/*
** orr_u8_m_untied:
**	movprfx	z0, z1
**	orr	z0\.b, p0/m, z0\.b, z2\.b
**	ret
*/
TEST_UNIFORM_Z (orr_u8_m_untied, svuint8_t,
		z0 = svorr_u8_m (p0, z1, z2),
		z0 = svorr_m (p0, z1, z2))

/*
** orr_w0_u8_m_tied1:
**	mov	(z[0-9]+\.b), w0
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_ZS (orr_w0_u8_m_tied1, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_m (p0, z0, x0),
		 z0 = svorr_m (p0, z0, x0))

/*
** orr_w0_u8_m_untied:
**	mov	(z[0-9]+\.b), w0
**	movprfx	z0, z1
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_ZS (orr_w0_u8_m_untied, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_m (p0, z1, x0),
		 z0 = svorr_m (p0, z1, x0))

/*
** orr_b4_u8_m_tied1:
**	mov	(z[0-9]+\.b), b4
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_ZS (orr_b4_u8_m_tied1, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_m (p0, z0, d4),
		 z0 = svorr_m (p0, z0, d4))

/*
** orr_b4_u8_m_untied:
**	mov	(z[0-9]+\.b), b4
**	movprfx	z0, z1
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_ZS (orr_b4_u8_m_untied, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_m (p0, z1, d4),
		 z0 = svorr_m (p0, z1, d4))

/*
** orr_1_u8_m_tied1:
**	mov	(z[0-9]+\.b), #1
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_Z (orr_1_u8_m_tied1, svuint8_t,
		z0 = svorr_n_u8_m (p0, z0, 1),
		z0 = svorr_m (p0, z0, 1))

/*
** orr_1_u8_m_untied:
**	mov	(z[0-9]+\.b), #1
**	movprfx	z0, z1
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_Z (orr_1_u8_m_untied, svuint8_t,
		z0 = svorr_n_u8_m (p0, z1, 1),
		z0 = svorr_m (p0, z1, 1))

/*
** orr_m2_u8_m:
**	mov	(z[0-9]+\.b), #-2
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_Z (orr_m2_u8_m, svuint8_t,
		z0 = svorr_n_u8_m (p0, z0, -2),
		z0 = svorr_m (p0, z0, -2))

/*
** orr_u8_z_tied1:
**	movprfx	z0\.b, p0/z, z0\.b
**	orr	z0\.b, p0/m, z0\.b, z1\.b
**	ret
*/
TEST_UNIFORM_Z (orr_u8_z_tied1, svuint8_t,
		z0 = svorr_u8_z (p0, z0, z1),
		z0 = svorr_z (p0, z0, z1))

/*
** orr_u8_z_tied2:
**	movprfx	z0\.b, p0/z, z0\.b
**	orr	z0\.b, p0/m, z0\.b, z1\.b
**	ret
*/
TEST_UNIFORM_Z (orr_u8_z_tied2, svuint8_t,
		z0 = svorr_u8_z (p0, z1, z0),
		z0 = svorr_z (p0, z1, z0))

/*
** orr_u8_z_untied:
** (
**	movprfx	z0\.b, p0/z, z1\.b
**	orr	z0\.b, p0/m, z0\.b, z2\.b
** |
**	movprfx	z0\.b, p0/z, z2\.b
**	orr	z0\.b, p0/m, z0\.b, z1\.b
** )
**	ret
*/
TEST_UNIFORM_Z (orr_u8_z_untied, svuint8_t,
		z0 = svorr_u8_z (p0, z1, z2),
		z0 = svorr_z (p0, z1, z2))

/*
** orr_w0_u8_z_tied1:
**	mov	(z[0-9]+\.b), w0
**	movprfx	z0\.b, p0/z, z0\.b
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_ZS (orr_w0_u8_z_tied1, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_z (p0, z0, x0),
		 z0 = svorr_z (p0, z0, x0))

/*
** orr_w0_u8_z_untied:
**	mov	(z[0-9]+\.b), w0
** (
**	movprfx	z0\.b, p0/z, z1\.b
**	orr	z0\.b, p0/m, z0\.b, \1
** |
**	movprfx	z0\.b, p0/z, \1
**	orr	z0\.b, p0/m, z0\.b, z1\.b
** )
**	ret
*/
TEST_UNIFORM_ZS (orr_w0_u8_z_untied, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_z (p0, z1, x0),
		 z0 = svorr_z (p0, z1, x0))

/*
** orr_b4_u8_z_tied1:
**	mov	(z[0-9]+\.b), b4
**	movprfx	z0\.b, p0/z, z0\.b
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_ZS (orr_b4_u8_z_tied1, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_z (p0, z0, d4),
		 z0 = svorr_z (p0, z0, d4))

/*
** orr_b4_u8_z_untied:
**	mov	(z[0-9]+\.b), b4
** (
**	movprfx	z0\.b, p0/z, z1\.b
**	orr	z0\.b, p0/m, z0\.b, \1
** |
**	movprfx	z0\.b, p0/z, \1
**	orr	z0\.b, p0/m, z0\.b, z1\.b
** )
**	ret
*/
TEST_UNIFORM_ZS (orr_b4_u8_z_untied, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_z (p0, z1, d4),
		 z0 = svorr_z (p0, z1, d4))

/*
** orr_1_u8_z_tied1:
**	mov	(z[0-9]+\.b), #1
**	movprfx	z0\.b, p0/z, z0\.b
**	orr	z0\.b, p0/m, z0\.b, \1
**	ret
*/
TEST_UNIFORM_Z (orr_1_u8_z_tied1, svuint8_t,
		z0 = svorr_n_u8_z (p0, z0, 1),
		z0 = svorr_z (p0, z0, 1))

/*
** orr_1_u8_z_untied:
**	mov	(z[0-9]+\.b), #1
** (
**	movprfx	z0\.b, p0/z, z1\.b
**	orr	z0\.b, p0/m, z0\.b, \1
** |
**	movprfx	z0\.b, p0/z, \1
**	orr	z0\.b, p0/m, z0\.b, z1\.b
** )
**	ret
*/
TEST_UNIFORM_Z (orr_1_u8_z_untied, svuint8_t,
		z0 = svorr_n_u8_z (p0, z1, 1),
		z0 = svorr_z (p0, z1, 1))

/*
** orr_u8_x_tied1:
**	orr	z0\.d, (z0\.d, z1\.d|z1\.d, z0\.d)
**	ret
*/
TEST_UNIFORM_Z (orr_u8_x_tied1, svuint8_t,
		z0 = svorr_u8_x (p0, z0, z1),
		z0 = svorr_x (p0, z0, z1))

/*
** orr_u8_x_tied2:
**	orr	z0\.d, (z0\.d, z1\.d|z1\.d, z0\.d)
**	ret
*/
TEST_UNIFORM_Z (orr_u8_x_tied2, svuint8_t,
		z0 = svorr_u8_x (p0, z1, z0),
		z0 = svorr_x (p0, z1, z0))

/*
** orr_u8_x_untied:
**	orr	z0\.d, (z1\.d, z2\.d|z2\.d, z1\.d)
**	ret
*/
TEST_UNIFORM_Z (orr_u8_x_untied, svuint8_t,
		z0 = svorr_u8_x (p0, z1, z2),
		z0 = svorr_x (p0, z1, z2))

/*
** orr_w0_u8_x_tied1:
**	mov	(z[0-9]+)\.b, w0
**	orr	z0\.d, (z0\.d, \1\.d|\1\.d, z0\.d)
**	ret
*/
TEST_UNIFORM_ZS (orr_w0_u8_x_tied1, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_x (p0, z0, x0),
		 z0 = svorr_x (p0, z0, x0))

/*
** orr_w0_u8_x_untied:
**	mov	(z[0-9]+)\.b, w0
**	orr	z0\.d, (z1\.d, \1\.d|\1\.d, z1\.d)
**	ret
*/
TEST_UNIFORM_ZS (orr_w0_u8_x_untied, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_x (p0, z1, x0),
		 z0 = svorr_x (p0, z1, x0))

/*
** orr_b4_u8_x_tied1:
**	mov	(z[0-9]+)\.b, b4
**	orr	z0\.d, (z0\.d, \1\.d|\1\.d, z0\.d)
**	ret
*/
TEST_UNIFORM_ZS (orr_b4_u8_x_tied1, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_x (p0, z0, d4),
		 z0 = svorr_x (p0, z0, d4))

/*
** orr_b4_u8_x_untied:
**	mov	(z[0-9]+)\.b, b4
**	orr	z0\.d, (z1\.d, \1\.d|\1\.d, z1\.d)
**	ret
*/
TEST_UNIFORM_ZS (orr_b4_u8_x_untied, svuint8_t, uint8_t,
		 z0 = svorr_n_u8_x (p0, z1, d4),
		 z0 = svorr_x (p0, z1, d4))

/*
** orr_1_u8_x_tied1:
**	orr	z0\.b, z0\.b, #0x1
**	ret
*/
TEST_UNIFORM_Z (orr_1_u8_x_tied1, svuint8_t,
		z0 = svorr_n_u8_x (p0, z0, 1),
		z0 = svorr_x (p0, z0, 1))

/*
** orr_1_u8_x_untied:
**	movprfx	z0, z1
**	orr	z0\.b, z0\.b, #0x1
**	ret
*/
TEST_UNIFORM_Z (orr_1_u8_x_untied, svuint8_t,
		z0 = svorr_n_u8_x (p0, z1, 1),
		z0 = svorr_x (p0, z1, 1))

/*
** orr_127_u8_x:
**	orr	z0\.b, z0\.b, #0x7f
**	ret
*/
TEST_UNIFORM_Z (orr_127_u8_x, svuint8_t,
		z0 = svorr_n_u8_x (p0, z0, 127),
		z0 = svorr_x (p0, z0, 127))

/*
** orr_128_u8_x:
**	orr	z0\.b, z0\.b, #0x80
**	ret
*/
TEST_UNIFORM_Z (orr_128_u8_x, svuint8_t,
		z0 = svorr_n_u8_x (p0, z0, 128),
		z0 = svorr_x (p0, z0, 128))

/*
** orr_255_u8_x:
**	mov	z0\.b, #-1
**	ret
*/
TEST_UNIFORM_Z (orr_255_u8_x, svuint8_t,
		z0 = svorr_n_u8_x (p0, z0, 255),
		z0 = svorr_x (p0, z0, 255))

/*
** orr_m127_u8_x:
**	orr	z0\.b, z0\.b, #0x81
**	ret
*/
TEST_UNIFORM_Z (orr_m127_u8_x, svuint8_t,
		z0 = svorr_n_u8_x (p0, z0, -127),
		z0 = svorr_x (p0, z0, -127))

/*
** orr_m128_u8_x:
**	orr	z0\.b, z0\.b, #0x80
**	ret
*/
TEST_UNIFORM_Z (orr_m128_u8_x, svuint8_t,
		z0 = svorr_n_u8_x (p0, z0, -128),
		z0 = svorr_x (p0, z0, -128))

/*
** orr_5_u8_x:
**	mov	(z[0-9]+)\.b, #5
**	orr	z0\.d, (z0\.d, \1\.d|\1\.d, z0\.d)
**	ret
*/
TEST_UNIFORM_Z (orr_5_u8_x, svuint8_t,
		z0 = svorr_n_u8_x (p0, z0, 5),
		z0 = svorr_x (p0, z0, 5))
