/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** minv_x0_u8:
**	uminv	b([0-9]+), p0, z0\.b
**	umov	w0, v\1\.b\[0\]
**	ret
*/
TEST_REDUCTION_X (minv_x0_u8, uint8_t, svuint8_t,
		  x0 = svminv_u8 (p0, z0),
		  x0 = svminv (p0, z0))
