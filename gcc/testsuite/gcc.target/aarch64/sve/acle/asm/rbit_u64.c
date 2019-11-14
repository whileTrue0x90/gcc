/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** rbit_u64_m_tied12:
**	rbit	z0\.d, p0/m, z0\.d
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_m_tied12, svuint64_t,
		z0 = svrbit_u64_m (z0, p0, z0),
		z0 = svrbit_m (z0, p0, z0))

/*
** rbit_u64_m_tied1:
**	rbit	z0\.d, p0/m, z1\.d
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_m_tied1, svuint64_t,
		z0 = svrbit_u64_m (z0, p0, z1),
		z0 = svrbit_m (z0, p0, z1))

/*
** rbit_u64_m_tied2:
**	mov	(z[0-9]+\.d), z0\.d
**	movprfx	z0, z1
**	rbit	z0\.d, p0/m, \1
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_m_tied2, svuint64_t,
		z0 = svrbit_u64_m (z1, p0, z0),
		z0 = svrbit_m (z1, p0, z0))

/*
** rbit_u64_m_untied:
**	movprfx	z0, z2
**	rbit	z0\.d, p0/m, z1\.d
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_m_untied, svuint64_t,
		z0 = svrbit_u64_m (z2, p0, z1),
		z0 = svrbit_m (z2, p0, z1))

/*
** rbit_u64_z_tied1:
**	mov	(z[0-9]+\.d), z0\.d
**	movprfx	z0\.d, p0/z, \1
**	rbit	z0\.d, p0/m, \1
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_z_tied1, svuint64_t,
		z0 = svrbit_u64_z (p0, z0),
		z0 = svrbit_z (p0, z0))

/*
** rbit_u64_z_untied:
**	movprfx	z0\.d, p0/z, z1\.d
**	rbit	z0\.d, p0/m, z1\.d
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_z_untied, svuint64_t,
		z0 = svrbit_u64_z (p0, z1),
		z0 = svrbit_z (p0, z1))

/*
** rbit_u64_x_tied1:
**	rbit	z0\.d, p0/m, z0\.d
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_x_tied1, svuint64_t,
		z0 = svrbit_u64_x (p0, z0),
		z0 = svrbit_x (p0, z0))

/*
** rbit_u64_x_untied:
**	rbit	z0\.d, p0/m, z1\.d
**	ret
*/
TEST_UNIFORM_Z (rbit_u64_x_untied, svuint64_t,
		z0 = svrbit_u64_x (p0, z1),
		z0 = svrbit_x (p0, z1))
