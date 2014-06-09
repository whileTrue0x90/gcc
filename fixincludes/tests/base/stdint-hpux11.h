/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/stdint-hpux11.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( HPUX_C99_INTPTR_CHECK )
#define PTRDIFF_MAX (2147483647l)
#define PTRDIFF_MIN (-PTRDIFF_MAX - 1)
#define INTPTR_MAX (2147483647l)
#define INTPTR_MIN (-INTPTR_MAX - 1)
#define UINTPTR_MAX (4294967295ul)
#define SIZE_MAX (4294967295ul)

#endif  /* HPUX_C99_INTPTR_CHECK */


#if defined( HPUX_C99_INTTYPES2_CHECK )
#define INT8_C(__c) (__c)
#define UINT8_C(__c) (__c)
#define INT16_C(__c) (__c)
#define UINT16_C(__c) (__c)

#endif  /* HPUX_C99_INTTYPES2_CHECK */


#if defined( HPUX_STDINT_LEAST_FAST_CHECK )
#  define	UINT_FAST64_MAX	__UINT64_MAX__
#  define	UINT_LEAST64_MAX	__UINT64_MAX__

#endif  /* HPUX_STDINT_LEAST_FAST_CHECK */
