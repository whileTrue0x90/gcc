#ifndef FIXINC_STDIO_STDARG_H_CHECK
#define FIXINC_STDIO_STDARG_H_CHECK 1

#define __need___va_list
#include <stdarg.h>


#if defined( ALPHA_GETOPT_CHECK )
extern int getopt(int, char *const[], const char *);
#endif  /* ALPHA_GETOPT_CHECK */


#if defined( ISC_OMITS_WITH_STDC_CHECK )
#if !defined(_POSIX_SOURCE) /* ? ! */
int foo;
#endif
#endif  /* ISC_OMITS_WITH_STDC_CHECK */


#if defined( READ_RET_TYPE_CHECK )
extern unsigned int fread(), fwrite();
extern int	fclose(), fflush(), foo();
#endif  /* READ_RET_TYPE_CHECK */


#if defined( RS6000_PARAM_CHECK )
extern int rename(const char *_old, const char *_new);
#endif  /* RS6000_PARAM_CHECK */


#if defined( STDIO_STDARG_H_CHECK )

#endif  /* STDIO_STDARG_H_CHECK */

#endif  /* FIXINC_STDIO_STDARG_H_CHECK */
