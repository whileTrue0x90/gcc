/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/rtldef/decc$types.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( VMS_DEFINE_CAN_USE_EXTERN_PREFIX_CHECK )
# else
#    if defined(__DECCXX)
#	define __CAN_USE_EXTERN_PREFIX 1
#    elif defined (__GNUC__)
#	define __CAN_USE_EXTERN_PREFIX 1
#    endif
# endif

#endif  /* VMS_DEFINE_CAN_USE_EXTERN_PREFIX_CHECK */
