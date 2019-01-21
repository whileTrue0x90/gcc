/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** lastb_x0_u8:
**	lastb	w0, p0, z0\.b
**	ret
*/
TEST_REDUCTION_X (lastb_x0_u8, uint8_t, svuint8_t,
		  x0 = svlastb_u8 (p0, z0),
		  x0 = svlastb (p0, z0))

/*
** lastb_d0_u8_tied:
**	lastb	b0, p0, z0\.b
**	ret
*/
TEST_REDUCTION_D (lastb_d0_u8_tied, uint8_t, svuint8_t,
		  d0 = svlastb_u8 (p0, z0),
		  d0 = svlastb (p0, z0))

/*
** lastb_d0_u8_untied:
**	lastb	b0, p0, z1\.b
**	ret
*/
TEST_REDUCTION_D (lastb_d0_u8_untied, uint8_t, svuint8_t,
		  d0 = svlastb_u8 (p0, z1),
		  d0 = svlastb (p0, z1))
