/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** len_x0_s64:
**	cntd	x0
**	ret
*/
TEST_REDUCTION_X (len_x0_s64, uint64_t, svint64_t,
		  x0 = svlen_s64 (z0),
		  x0 = svlen (z0))

/*
** len_d0_s64_tied:
**	cntd	(x[0-9]+)
**	fmov	d0, \1
**	ret
*/
TEST_REDUCTION_D (len_d0_s64_tied, uint64_t, svint64_t,
		  d0 = svlen_s64 (z0),
		  d0 = svlen (z0))

/*
** len_d0_s64_untied:
**	cntd	(x[0-9]+)
**	fmov	d0, \1
**	ret
*/
TEST_REDUCTION_D (len_d0_s64_untied, uint64_t, svint64_t,
		  d0 = svlen_s64 (z1),
		  d0 = svlen (z1))
