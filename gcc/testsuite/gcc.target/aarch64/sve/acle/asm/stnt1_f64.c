/* { dg-final { check-function-bodies "**" "" "-DCHECK_ASM" } } */

#include "test_sve_acle.h"

/*
** stnt1_f64_base:
**	stnt1d	z0\.d, p0, \[x0\]
**	ret
*/
TEST_STORE (stnt1_f64_base, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0, z0),
	    svstnt1 (p0, x0, z0))

/*
** stnt1_f64_index:
**	stnt1d	z0\.d, p0, \[x0, x1, lsl 3\]
**	ret
*/
TEST_STORE (stnt1_f64_index, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0 + x1, z0),
	    svstnt1 (p0, x0 + x1, z0))

/*
** stnt1_f64_1:
**	stnt1d	z0\.d, p0, \[x0, #1, mul vl\]
**	ret
*/
TEST_STORE (stnt1_f64_1, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0 + svcntd (), z0),
	    svstnt1 (p0, x0 + svcntd (), z0))

/*
** stnt1_f64_7:
**	stnt1d	z0\.d, p0, \[x0, #7, mul vl\]
**	ret
*/
TEST_STORE (stnt1_f64_7, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0 + svcntd () * 7, z0),
	    svstnt1 (p0, x0 + svcntd () * 7, z0))

/* Moving the constant into a register would also be OK.  */
/*
** stnt1_f64_8:
**	incb	x0, all, mul #8
**	stnt1d	z0\.d, p0, \[x0\]
**	ret
*/
TEST_STORE (stnt1_f64_8, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0 + svcntd () * 8, z0),
	    svstnt1 (p0, x0 + svcntd () * 8, z0))

/*
** stnt1_f64_m1:
**	stnt1d	z0\.d, p0, \[x0, #-1, mul vl\]
**	ret
*/
TEST_STORE (stnt1_f64_m1, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0 - svcntd (), z0),
	    svstnt1 (p0, x0 - svcntd (), z0))

/*
** stnt1_f64_m8:
**	stnt1d	z0\.d, p0, \[x0, #-8, mul vl\]
**	ret
*/
TEST_STORE (stnt1_f64_m8, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0 - svcntd () * 8, z0),
	    svstnt1 (p0, x0 - svcntd () * 8, z0))

/* Moving the constant into a register would also be OK.  */
/*
** stnt1_f64_m9:
**	decb	x0, all, mul #9
**	stnt1d	z0\.d, p0, \[x0\]
**	ret
*/
TEST_STORE (stnt1_f64_m9, svfloat64_t, float64_t,
	    svstnt1_f64 (p0, x0 - svcntd () * 9, z0),
	    svstnt1 (p0, x0 - svcntd () * 9, z0))

/*
** stnt1_vnum_f64_0:
**	stnt1d	z0\.d, p0, \[x0\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_0, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, 0, z0),
	    svstnt1_vnum (p0, x0, 0, z0))

/*
** stnt1_vnum_f64_1:
**	stnt1d	z0\.d, p0, \[x0, #1, mul vl\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_1, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, 1, z0),
	    svstnt1_vnum (p0, x0, 1, z0))

/*
** stnt1_vnum_f64_7:
**	stnt1d	z0\.d, p0, \[x0, #7, mul vl\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_7, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, 7, z0),
	    svstnt1_vnum (p0, x0, 7, z0))

/* Moving the constant into a register would also be OK.  */
/*
** stnt1_vnum_f64_8:
**	incb	x0, all, mul #8
**	stnt1d	z0\.d, p0, \[x0\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_8, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, 8, z0),
	    svstnt1_vnum (p0, x0, 8, z0))

/*
** stnt1_vnum_f64_m1:
**	stnt1d	z0\.d, p0, \[x0, #-1, mul vl\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_m1, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, -1, z0),
	    svstnt1_vnum (p0, x0, -1, z0))

/*
** stnt1_vnum_f64_m8:
**	stnt1d	z0\.d, p0, \[x0, #-8, mul vl\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_m8, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, -8, z0),
	    svstnt1_vnum (p0, x0, -8, z0))

/* Moving the constant into a register would also be OK.  */
/*
** stnt1_vnum_f64_m9:
**	decb	x0, all, mul #9
**	stnt1d	z0\.d, p0, \[x0\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_m9, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, -9, z0),
	    svstnt1_vnum (p0, x0, -9, z0))

/* Using MUL to calculate an index would also be OK.  */
/*
** stnt1_vnum_f64_x1:
**	cntb	(x[0-9]+)
**	madd	(x[0-9]+), (x1, \1|\1, x1), x0
**	stnt1d	z0\.d, p0, \[\2\]
**	ret
*/
TEST_STORE (stnt1_vnum_f64_x1, svfloat64_t, float64_t,
	    svstnt1_vnum_f64 (p0, x0, x1, z0),
	    svstnt1_vnum (p0, x0, x1, z0))
