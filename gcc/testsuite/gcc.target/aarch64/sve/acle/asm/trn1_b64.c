/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** trn1_b64_tied1:
**	trn1	p0\.d, p0\.d, p1\.d
**	ret
*/
TEST_UNIFORM_P (trn1_b64_tied1,
		p0 = svtrn1_b64 (p0, p1),
		p0 = svtrn1_b64 (p0, p1))

/*
** trn1_b64_tied2:
**	trn1	p0\.d, p1\.d, p0\.d
**	ret
*/
TEST_UNIFORM_P (trn1_b64_tied2,
		p0 = svtrn1_b64 (p1, p0),
		p0 = svtrn1_b64 (p1, p0))

/*
** trn1_b64_untied:
**	trn1	p0\.d, p1\.d, p2\.d
**	ret
*/
TEST_UNIFORM_P (trn1_b64_untied,
		p0 = svtrn1_b64 (p1, p2),
		p0 = svtrn1_b64 (p1, p2))
