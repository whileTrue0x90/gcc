#ifndef FIXINC_SUNOS_MATHERR_DECL_CHECK
#define FIXINC_SUNOS_MATHERR_DECL_CHECK 1

struct exception;
#ifndef FIXINC_MATH_EXCEPTION_CHECK
#define FIXINC_MATH_EXCEPTION_CHECK 1

#ifdef __cplusplus
#define exception __math_exception
#endif


#if defined( BROKEN_CABS_CHECK )
#ifdef __STDC__

#else

#endif
 /* This is a comment
                         and it ends here. */
#endif  /* BROKEN_CABS_CHECK */


#if defined( FIX_HEADER_BREAKAGE_CHECK )
extern double floor(), ceil(), fmod(), fabs _PARAMS((double));
#endif  /* FIX_HEADER_BREAKAGE_CHECK */


#if defined( HPUX11_FABSF_CHECK )
#ifdef _PA_RISC
#ifndef __cplusplus
#  define fabsf(x) ((float)fabs((double)(float)(x)))
#endif
#endif
#endif  /* HPUX11_FABSF_CHECK */


#if defined( HPUX8_BOGUS_INLINES_CHECK )
extern "C" int abs(int);

#endif  /* HPUX8_BOGUS_INLINES_CHECK */


#if defined( ISC_FMOD_CHECK )
extern double	fmod(double, double);
#endif  /* ISC_FMOD_CHECK */


#if defined( M88K_BAD_HYPOT_OPT_CHECK )
extern double hypot();
/* Workaround a stupid Motorola optimization if one
   of x or y is 0.0 and the other is negative!  */
#ifdef __STDC__
static __inline__ double fake_hypot (double x, double y)
#else
static __inline__ double fake_hypot (x, y)
	double x, y;
#endif
{
	return fabs (hypot (x, y));
}
#define hypot	fake_hypot
#endif  /* M88K_BAD_HYPOT_OPT_CHECK */


#if defined( MATH_EXCEPTION_CHECK )
typedef struct exception t_math_exception;
#endif  /* MATH_EXCEPTION_CHECK */


#if defined( MATH_HUGE_VAL_IFNDEF_CHECK )
#ifndef HUGE_VAL
# define	HUGE_VAL 3.4e+40
#endif
#endif  /* MATH_HUGE_VAL_IFNDEF_CHECK */


#if defined( RS6000_DOUBLE_CHECK )
#ifndef __cplusplus
extern int class();
#endif
#endif  /* RS6000_DOUBLE_CHECK */


#if defined( SUNOS_MATHERR_DECL_CHECK )
extern int matherr();
#endif  /* SUNOS_MATHERR_DECL_CHECK */


#if defined( SVR4__P_CHECK )
#ifndef __P
#define __P(a) a
#endif
#endif  /* SVR4__P_CHECK */
#ifdef __cplusplus
#undef exception
#endif

#endif  /* FIXINC_MATH_EXCEPTION_CHECK */

#endif  /* FIXINC_SUNOS_MATHERR_DECL_CHECK */
