/*  DO NOT EDIT THIS FILE.

    It has been auto-edited by fixincludes from:

	"fixinc/tests/inc/testing.h"

    This had to be done to correct non-standard usages in the
    original, manufacturer supplied header file.  */



#if defined( ALPHA___EXTERN_PREFIX_CHECK )
#if (defined(__DECC) || defined(__PRAGMA_EXTERN_PREFIX))
#pragma extern_prefix "_P"
#   if  (defined(__DECC) || defined(__PRAGMA_EXTERN_PREFIX))
#     pragma extern_prefix "_E"
# if !defined(_LIBC_POLLUTION_H_) &&  (defined(__DECC) || defined(__PRAGMA_EXTERN_PREFIX))
#  pragma extern_prefix ""
#endif  /* ALPHA___EXTERN_PREFIX_CHECK */


#if defined( ALPHA_BAD_LVAL_CHECK )
#pragma extern_prefix "_FOO"
#define something _FOOsomething
#define mumble _FOOmumble
#endif  /* ALPHA_BAD_LVAL_CHECK */


#if defined( AVOID_WCHAR_T_TYPE_CHECK )
#ifndef __cplusplus
typedef unsigned short	wchar_t 	;
#endif /* wchar_t
 type */
#endif  /* AVOID_WCHAR_T_TYPE_CHECK */


#if defined( CTRL_QUOTES_DEF_CHECK )
#define BSD43_CTRL(n, x) ((n<<8)+x)

#endif  /* CTRL_QUOTES_DEF_CHECK */


#if defined( CTRL_QUOTES_USE_CHECK )
#define TCTRLFOO BSD43_CTRL('T', 1)
#endif  /* CTRL_QUOTES_USE_CHECK */


#if defined( DJGPP_WCHAR_H_CHECK )
#include <stddef.h>
#include <sys/djtypes.h>
extern __DJ_wint_t x;

#endif  /* DJGPP_WCHAR_H_CHECK */


#if defined( HPUX11_SIZE_T_CHECK )
#define _hpux_size_t size_t
       extern int getpwuid_r( char *, _hpux_size_t, struct passwd **);

#endif  /* HPUX11_SIZE_T_CHECK */


#if defined( IO_QUOTES_DEF_CHECK )
#define BSD43__IOWR(n, x) ((n<<8)+x)
#define _IOWN(x,y,t)  (_IOC_IN|(((t)&_IOCPARM_MASK)<<16)|(x<<8)|y)
#define _IO(x,y)      (x<<8|y)
#endif  /* IO_QUOTES_DEF_CHECK */


#if defined( IO_QUOTES_USE_CHECK )
#define TIOCFOO BSD43__IOWR('T', 1)
#define TIOCFOO \
BSD43__IOWR('T', 1) /* Some are multi-line */
#endif  /* IO_QUOTES_USE_CHECK */


#if defined( LIBC1_IFDEFD_MEMX_CHECK )
/* Copy N bytes of SRC to DEST.  */
extern __ptr_t memcpy __P ((__ptr_t __dest, __const __ptr_t __src,
                         size_t __n));
#endif  /* LIBC1_IFDEFD_MEMX_CHECK */


#if defined( MACHINE_ANSI_H_VA_LIST_CHECK )
 # define _BSD_VA_LIST_	__builtin_va_list
#endif  /* MACHINE_ANSI_H_VA_LIST_CHECK */


#if defined( MACHINE_NAME_CHECK )
/* MACH_DIFF: */
#if defined( i386 ) || defined( sparc ) || defined( vax )
/* no uniform test, so be careful  :-) */
#endif  /* MACHINE_NAME_CHECK */


#if defined( STRICT_ANSI_NOT_CHECK )
#if  !defined(__STRICT_ANSI__) 
#if  !defined(__STRICT_ANSI__)
#if  !defined(__STRICT_ANSI__)
#if  !defined(__STRICT_ANSI__)/* not std C */
int foo;

#end-end-end-end-if :-)
#endif  /* STRICT_ANSI_NOT_CHECK */


#if defined( STRICT_ANSI_ONLY_CHECK )
#if  defined(__STRICT_ANSI__) /* is std C
 */
int foo;
#endif
#endif  /* STRICT_ANSI_ONLY_CHECK */


#if defined( SYSV68_STRING_CHECK )
extern unsigned int strlen();
extern int ffs(int);
extern void
	*memccpy(),
	memcpy();
extern int
	strcmp(),
	strncmp();
extern unsigned int
	strlen(),
	strspn();
extern size_t
	strlen(), strspn();
#endif  /* SYSV68_STRING_CHECK */


#if defined( UNDEFINE_NULL_CHECK )
#ifndef NULL
#define NULL 0UL
#endif
#ifndef NULL
#define NULL	((void*)0)
#endif

#endif  /* UNDEFINE_NULL_CHECK */


#if defined( WINDISS_VALIST_CHECK )
#include <stdarg.h>
#endif  /* WINDISS_VALIST_CHECK */
