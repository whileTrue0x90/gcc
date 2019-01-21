/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** lastb_x0_s64:
**	lastb	x0, p0, z0\.d
**	ret
*/
TEST_REDUCTION_X (lastb_x0_s64, int64_t, svint64_t,
		  x0 = svlastb_s64 (p0, z0),
		  x0 = svlastb (p0, z0))

/*
** lastb_d0_s64_tied:
**	lastb	d0, p0, z0\.d
**	ret
*/
TEST_REDUCTION_D (lastb_d0_s64_tied, int64_t, svint64_t,
		  d0 = svlastb_s64 (p0, z0),
		  d0 = svlastb (p0, z0))

/*
** lastb_d0_s64_untied:
**	lastb	d0, p0, z1\.d
**	ret
*/
TEST_REDUCTION_D (lastb_d0_s64_untied, int64_t, svint64_t,
		  d0 = svlastb_s64 (p0, z1),
		  d0 = svlastb (p0, z1))
