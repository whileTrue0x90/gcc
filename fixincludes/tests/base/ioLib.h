/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/ioLib.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( VXWORKS_IOCTL_MACRO_CHECK )
extern int ioctl ( int asdf1234, int jkl , int qwerty ) ;
#endif  /* VXWORKS_IOCTL_MACRO_CHECK */


#if defined( VXWORKS_WRITE_CONST_CHECK )
extern int  write (int, const char*, size_t);
#endif  /* VXWORKS_WRITE_CONST_CHECK */
