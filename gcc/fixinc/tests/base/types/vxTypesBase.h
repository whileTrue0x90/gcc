/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/types/vxTypesBase.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( VXWORKS_GCC_PROBLEM_CHECK )
#if 1
#ifndef _GCC_SIZE_T
#define _GCC_SIZE_T
typedef unsigned int size_t;
#endif
#ifndef _GCC_PTRDIFF_T
#define _GCC_PTRDIFF_T
typedef long ptrdiff_t;
#endif
#ifndef _GCC_WCHAR_T
#define _GCC_WCHAR_T
typedef unsigned short wchar_t;
#endif
#endif /* __GNUC_TYPEOF_FEATURE_BROKEN_USE_DEFAULT_UNTIL_FIXED__ */

#endif  /* VXWORKS_GCC_PROBLEM_CHECK */
