#ifdef DBG
#include <stdio.h>
#define DEBUG_FPUTS(x) fputs (x, stdout)
#define DEBUG_DOT putc ('.', stdout)
#define DEBUG_NL putc ('\n', stdout)
#else
#define DEBUG_FPUTS(x)
#define DEBUG_DOT
#define DEBUG_NL
#endif

#define T(TYPE)							\
TYPE g01##TYPE, g02##TYPE, g03##TYPE, g04##TYPE;		\
TYPE g05##TYPE, g06##TYPE, g07##TYPE, g08##TYPE;		\
TYPE g09##TYPE, g10##TYPE, g11##TYPE, g12##TYPE;		\
TYPE g13##TYPE, g14##TYPE, g15##TYPE, g16##TYPE;		\
								\
extern void init##TYPE (TYPE *p, int i);			\
extern void checkg##TYPE (void);				\
extern TYPE test0##TYPE (void);					\
extern TYPE test1##TYPE (TYPE);					\
extern TYPE testva##TYPE (int n, ...);				\
								\
void								\
testit##TYPE (void)						\
{								\
  TYPE rslt;							\
  DEBUG_FPUTS (#TYPE);						\
  init##TYPE  (&g01##TYPE,  1);					\
  init##TYPE  (&g02##TYPE,  2);					\
  init##TYPE  (&g03##TYPE,  3);					\
  init##TYPE  (&g04##TYPE,  4);					\
  init##TYPE  (&g05##TYPE,  5);					\
  init##TYPE  (&g06##TYPE,  6);					\
  init##TYPE  (&g07##TYPE,  7);					\
  init##TYPE  (&g08##TYPE,  8);					\
  init##TYPE  (&g09##TYPE,  9);					\
  init##TYPE  (&g10##TYPE, 10);					\
  init##TYPE  (&g11##TYPE, 11);					\
  init##TYPE  (&g12##TYPE, 12);					\
  init##TYPE  (&g13##TYPE, 13);					\
  init##TYPE  (&g14##TYPE, 14);					\
  init##TYPE  (&g15##TYPE, 15);					\
  init##TYPE  (&g16##TYPE, 16);					\
  checkg##TYPE ();						\
  DEBUG_FPUTS (" test0");					\
  rslt = test0##TYPE ();					\
  check##TYPE (rslt, 1);					\
  DEBUG_FPUTS (" test1");					\
  rslt = test1##TYPE (g01##TYPE);				\
  check##TYPE (rslt, 1);					\
  DEBUG_FPUTS (" testva");					\
  rslt = testva##TYPE (1, g01##TYPE);				\
  check##TYPE (rslt, 1);					\
  rslt = testva##TYPE (5, g01##TYPE, g02##TYPE,			\
			  g03##TYPE, g04##TYPE,			\
			  g05##TYPE);				\
  check##TYPE (rslt, 5);					\
  rslt = testva##TYPE (9, g01##TYPE, g02##TYPE,			\
			  g03##TYPE, g04##TYPE,			\
			  g05##TYPE, g06##TYPE,			\
			  g07##TYPE, g08##TYPE,			\
			  g09##TYPE);				\
  check##TYPE (rslt, 9);					\
  rslt = testva##TYPE (16, g01##TYPE, g02##TYPE,		\
			  g03##TYPE, g04##TYPE,			\
			  g05##TYPE, g06##TYPE,			\
			  g07##TYPE, g08##TYPE,			\
			  g09##TYPE, g10##TYPE,			\
			  g11##TYPE, g12##TYPE,			\
			  g13##TYPE, g14##TYPE,			\
			  g15##TYPE, g16##TYPE);		\
  check##TYPE (rslt, 16);					\
  DEBUG_NL;							\
}

extern void abort (void);

#include "small-struct-defs.h"
#include "small-struct-check.h"

T(Sc)
T(Ss)
T(Si)
T(Scs)
T(Ssc)
T(Sic)
T(Sci)
T(Ssi)
T(Sis)
T(Scsi)
T(Scis)
T(Ssci)
T(Ssic)
T(Sisc)
T(Sics)

#undef T

void
struct_return_3_x ()
{
#define T(TYPE) testit##TYPE ();

T(Sc)
T(Ss)
T(Si)
T(Scs)
T(Ssc)
T(Sic)
T(Sci)
T(Ssi)
T(Sis)
T(Scsi)
T(Scis)
T(Ssci)
T(Ssic)
T(Sisc)
T(Sics)

#undef T
}
