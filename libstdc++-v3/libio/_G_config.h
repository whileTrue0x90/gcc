/* This file is needed by libio to define various configuration parameters.
   These are always the same in the GNU C library.  */

#ifndef _G_config_h
#define _G_config_h 1

#ifndef _LIBC
# include <bits/c++config.h>
# define _IO_MTSAFE_IO
#endif

/* Define types for libio in terms of the standard internal type names.  */

#include <sys/types.h>
#define __need_size_t
#define __need_wchar_t
#define __need_wint_t
#define __need_NULL
#include <bits/std_cstddef.h>


/* For use as part of glibc (native) or as part of libstdc++ (maybe
   not glibc) */
#if __GLIBC__ < 2 || (__GLIBC__ == 2  && __GLIBC_MINOR__ <= 1)
# ifdef _GLIBCPP_USE_WCHAR_T
#  include <bits/std_cwchar.h>
typedef mbstate_t __mbstate_t;
# endif
#endif

#ifndef _WINT_T
/* Integral type unchanged by default argument promotions that can
   hold any value corresponding to members of the extended character
   set, as well as at least one value that does not correspond to any
   member of the extended character set.  */
# define _WINT_T
typedef unsigned int wint_t;
#endif
#define __need_mbstate_t
#include <bits/std_cwchar.h>
#define _G_size_t	size_t
#if defined _LIBC || defined _GLIBCPP_USE_WCHAR_T
typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} _G_fpos64_t;
#else
typedef __off_t _G_fpos_t;
typedef __off64_t _G_fpos64_t;
#endif
#define _G_ssize_t	__ssize_t
#define _G_off_t	__off_t
#define _G_off64_t	__off64_t
#define	_G_pid_t	__pid_t
#define	_G_uid_t	__uid_t
#define _G_wchar_t	wchar_t
#define _G_wint_t	wint_t
#define _G_stat64	stat64
#if defined _LIBC || defined _GLIBCPP_USE_WCHAR_T
# include <iconv.h>
typedef iconv_t _G_iconv_t;
# if 0
/* XXX Commented out because outside glibc we have to use iconv()
   and not gconv().  */
typedef union
{
  struct __gconv_info __cd;
  struct
  {
    struct __gconv_info __cd;
    struct __gconv_step_data __data;
  } __combined;
} _G_iconv_t;
# endif
#endif

typedef int _G_int16_t __attribute__ ((__mode__ (__HI__)));
typedef int _G_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int _G_uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int _G_uint32_t __attribute__ ((__mode__ (__SI__)));

#define _G_HAVE_BOOL 1


/* These library features are always available in the GNU C library.  */
#define _G_HAVE_ATEXIT 1
#define _G_HAVE_SYS_CDEFS 1
#define _G_HAVE_SYS_WAIT 1
#define _G_NEED_STDARG_H 1
#define _G_va_list __gnuc_va_list

#define _G_HAVE_PRINTF_FP 1
#define _G_HAVE_MMAP 1
#define _G_HAVE_LONG_DOUBLE_IO 1
#define _G_HAVE_IO_FILE_OPEN 1
#define _G_HAVE_IO_GETLINE_INFO 1

#define _G_IO_IO_FILE_VERSION 0x20001

//#define _G_OPEN64	__open64
//#define _G_LSEEK64	__lseek64
//#define _G_FSTAT64(fd,buf) __fxstat64 (_STAT_VER, fd, buf)

/* This is defined by <bits/stat.h> if `st_blksize' exists.  */
/*#define _G_HAVE_ST_BLKSIZE defined (_STATBUF_ST_BLKSIZE)*/

#define _G_BUFSIZ 8192

/* These are the vtbl details for ELF.  */
#define _G_NAMES_HAVE_UNDERSCORE 0
#define _G_VTABLE_LABEL_HAS_LENGTH 1
#define _G_USING_THUNKS	1
#define _G_VTABLE_LABEL_PREFIX "__vt_"
#define _G_VTABLE_LABEL_PREFIX_ID __vt_

#define _G_INTERNAL_CCS	"UCS4"
#define _G_HAVE_WEAK_SYMBOL 1
#define _G_STDIO_USES_LIBIO 1

#if defined __cplusplus || defined __STDC__
# define _G_ARGS(ARGLIST) ARGLIST
#else
# define _G_ARGS(ARGLIST) ()
#endif

#endif	/* _G_config.h */
