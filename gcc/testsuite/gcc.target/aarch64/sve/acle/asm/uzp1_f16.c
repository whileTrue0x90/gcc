/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** uzp1_f16_tied1:
**	uzp1	z0\.h, z0\.h, z1\.h
**	ret
*/
TEST_UNIFORM_Z (uzp1_f16_tied1, svfloat16_t,
		z0 = svuzp1_f16 (z0, z1),
		z0 = svuzp1 (z0, z1))

/*
** uzp1_f16_tied2:
**	uzp1	z0\.h, z1\.h, z0\.h
**	ret
*/
TEST_UNIFORM_Z (uzp1_f16_tied2, svfloat16_t,
		z0 = svuzp1_f16 (z1, z0),
		z0 = svuzp1 (z1, z0))

/*
** uzp1_f16_untied:
**	uzp1	z0\.h, z1\.h, z2\.h
**	ret
*/
TEST_UNIFORM_Z (uzp1_f16_untied, svfloat16_t,
		z0 = svuzp1_f16 (z1, z2),
		z0 = svuzp1 (z1, z2))
