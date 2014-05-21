/*
 * This should never be included directly.  It is included only from gc.h.
 * We separate it only to make gc.h more suitable as documentation.
 */
#if !defined(_REENTRANT) && (defined(GC_SOLARIS_THREADS) \
		             || defined(GC_SOLARIS_PTHREADS) \
			     || defined(GC_HPUX_THREADS) \
			     || defined(GC_AIX_THREADS) \
			     || defined(GC_LINUX_THREADS) \
			     || defined(GC_GNU_THREADS))
# define _REENTRANT
	/* Better late than never.  This fails if system headers that	*/
	/* depend on this were previously included.			*/
#endif

#if defined(GC_DGUX386_THREADS) && !defined(_POSIX4A_DRAFT10_SOURCE)
# define _POSIX4A_DRAFT10_SOURCE 1
#endif

# if defined(GC_SOLARIS_PTHREADS) || defined(GC_FREEBSD_THREADS) || \
	defined(GC_IRIX_THREADS) || defined(GC_LINUX_THREADS) || \
	defined(GC_GNU_THREADS) || \
	defined(GC_HPUX_THREADS) || defined(GC_OSF1_THREADS) || \
	defined(GC_DGUX386_THREADS) || defined(GC_DARWIN_THREADS) || \
	defined(GC_AIX_THREADS) || \
        (defined(GC_WIN32_THREADS) && defined(__CYGWIN__))
#   define GC_PTHREADS
# endif

#if defined(GC_THREADS) && !defined(GC_PTHREADS)
# if defined(__linux__)
#   define GC_LINUX_THREADS
#   define GC_PTHREADS
# endif
# if !defined(LINUX) && (defined(_PA_RISC1_1) || defined(_PA_RISC2_0) \
                         || defined(hppa) || defined(__HPPA))
#   define GC_HPUX_THREADS
#   define GC_PTHREADS
# endif
# if !defined(__linux__) && (defined(__alpha) || defined(__alpha__))
#   define GC_OSF1_THREADS
#   define GC_PTHREADS
# endif
# if defined(__mips) && !defined(__linux__)
#   define GC_IRIX_THREADS
#   define GC_PTHREADS
# endif
# if defined(__sparc) && !defined(__linux__)
#   define GC_SOLARIS_PTHREADS
#   define GC_PTHREADS
# endif
# if defined(__APPLE__) && defined(__MACH__) && defined(__ppc__)
#   define GC_DARWIN_THREADS
#   define GC_PTHREADS
# endif
# if !defined(GC_PTHREADS) && defined(__FreeBSD__)
#   define GC_FREEBSD_THREADS
#   define GC_PTHREADS
# endif
# if defined(DGUX) && (defined(i386) || defined(__i386__))
#   define GC_DGUX386_THREADS
#   define GC_PTHREADS
# endif
# if defined(_AIX)
#   define GC_AIX_THREADS
#   define GC_PTHREADS
# endif
#endif /* GC_THREADS */

#if defined(GC_THREADS) && !defined(GC_PTHREADS) && \
    (defined(_WIN32) || defined(_MSC_VER) || defined(__CYGWIN__) \
     || defined(__MINGW32__) || defined(__BORLANDC__) \
     || defined(_WIN32_WCE))
# define GC_WIN32_THREADS
#endif

# define __GC
# ifndef _WIN32_WCE
#   include <stddef.h>
# else /* ! _WIN32_WCE */
/* Yet more kluges for WinCE */
#   include <stdlib.h>		/* size_t is defined here */
    typedef long ptrdiff_t;	/* ptrdiff_t is not defined */
# endif

#if ((defined(_DLL) && !defined (__MINGW32__)) \
     || (defined (DLL_EXPORT) && defined (GC_BUILD))) \
    && !defined(GC_DLL)
# define GC_DLL
#endif

#if defined(__MINGW32__) && defined(GC_DLL)
# ifdef GC_BUILD
#   define GC_API __declspec(dllexport)
# else
#   define GC_API __declspec(dllimport)
# endif
#endif

#if (defined(__DMC__) || defined(_MSC_VER)) && defined(GC_DLL)
# ifdef GC_BUILD
#   define GC_API extern __declspec(dllexport)
# else
#   define GC_API __declspec(dllimport)
# endif
#endif

#if defined(__WATCOMC__) && defined(GC_DLL)
# ifdef GC_BUILD
#   define GC_API extern __declspec(dllexport)
# else
#   define GC_API extern __declspec(dllimport)
# endif
#endif

#ifndef GC_API
#define GC_API extern
#endif

