#include <stdarg.h>

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

/* Turn off checking for variable arguments with -DSKIPVA.  */
#ifdef SKIPVA
const int test_va = 0;
#else
const int test_va = 1;
#endif

typedef struct { char c; } Sc;
typedef struct { short s; } Ss;
typedef struct { int i; } Si;
typedef struct { short s; char c; } Ssc;
typedef struct { int i; short s; } Sis;
typedef struct { char c; short s; int i; } Scsi;
typedef struct { char c; int i; short s; } Scis;

void initSc (Sc *p, int i) { p->c = i/16; }
void initSs (Ss *p, int i) { p->s = i; }
void initSi (Si *p, int i) { p->i = i; }
void initSsc (Ssc *p, int i) { p->s = i; p->c = (i/16)+1; }
void initSis (Sis *p, int i) { p->i = i; p->s = i+1; }
void initScsi (Scsi *p, int i) { p->c = i/16; p->s = i+1; p->i = i+2; }
void initScis (Scis *p, int i) { p->c = i/16; p->i = i+1; p->s = i+2; }

#define T(N, TYPE)						\
struct S##TYPE##N { TYPE i[N]; };				\
								\
extern struct S##TYPE##N g1s##TYPE##N, g2s##TYPE##N;		\
extern struct S##TYPE##N g3s##TYPE##N, g4s##TYPE##N;		\
extern struct S##TYPE##N g5s##TYPE##N, g6s##TYPE##N;		\
extern struct S##TYPE##N g7s##TYPE##N, g8s##TYPE##N;		\
extern struct S##TYPE##N g9s##TYPE##N, g10s##TYPE##N;		\
extern struct S##TYPE##N g11s##TYPE##N, g12s##TYPE##N;		\
extern struct S##TYPE##N g13s##TYPE##N, g14s##TYPE##N;		\
extern struct S##TYPE##N g15s##TYPE##N, g16s##TYPE##N;		\
								\
extern void check##TYPE (TYPE x, int i);			\
extern void							\
check##TYPE##N (struct S##TYPE##N *p, int i);			\
								\
void								\
checkg##TYPE##N (void)						\
{								\
  check##TYPE##N ( &g1s##TYPE##N,  1*16);			\
  check##TYPE##N ( &g2s##TYPE##N,  2*16);			\
  check##TYPE##N ( &g3s##TYPE##N,  3*16);			\
  check##TYPE##N ( &g4s##TYPE##N,  4*16);			\
  check##TYPE##N ( &g5s##TYPE##N,  5*16);			\
  check##TYPE##N ( &g6s##TYPE##N,  6*16);			\
  check##TYPE##N ( &g7s##TYPE##N,  7*16);			\
  check##TYPE##N ( &g8s##TYPE##N,  8*16);			\
  check##TYPE##N ( &g9s##TYPE##N,  9*16);			\
  check##TYPE##N (&g10s##TYPE##N, 10*16);			\
  check##TYPE##N (&g11s##TYPE##N, 11*16);			\
  check##TYPE##N (&g12s##TYPE##N, 12*16);			\
  check##TYPE##N (&g13s##TYPE##N, 13*16);			\
  check##TYPE##N (&g14s##TYPE##N, 14*16);			\
  check##TYPE##N (&g15s##TYPE##N, 15*16);			\
  check##TYPE##N (&g16s##TYPE##N, 16*16);			\
}								\
								\
void								\
test##TYPE##N (struct S##TYPE##N s1, struct S##TYPE##N s2,	\
	       struct S##TYPE##N s3, struct S##TYPE##N s4,	\
	       struct S##TYPE##N s5, struct S##TYPE##N s6,	\
	       struct S##TYPE##N s7, struct S##TYPE##N s8,	\
	       struct S##TYPE##N s9, struct S##TYPE##N s10,	\
	       struct S##TYPE##N s11, struct S##TYPE##N s12,	\
	       struct S##TYPE##N s13, struct S##TYPE##N s14,	\
	       struct S##TYPE##N s15, struct S##TYPE##N s16)	\
{								\
  DEBUG_DOT;							\
  check##TYPE##N (&s1, 1*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s2, 2*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s3, 3*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s4, 4*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s5, 5*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s6, 6*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s7, 7*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s8, 8*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s9, 9*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s10, 10*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s11, 11*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s12, 12*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s13, 13*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s14, 14*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s15, 15*16);					\
  DEBUG_DOT;							\
  check##TYPE##N (&s16, 16*16);					\
}								\
								\
void								\
testva##TYPE##N (int n, ...)					\
{								\
  int i;							\
  va_list ap;							\
  if (test_va)							\
    {								\
      va_start (ap, n);						\
      for (i = 0; i < n; i++)					\
	{							\
	  struct S##TYPE##N t = va_arg (ap, struct S##TYPE##N);	\
	  DEBUG_DOT;						\
	  check##TYPE##N (&t, (i+1)*16);			\
	}							\
      va_end (ap);						\
    }								\
}

T(0, Sc)
T(1, Sc)
T(2, Sc)
T(3, Sc)
T(4, Sc)
T(5, Sc)
T(6, Sc)
T(7, Sc)
T(8, Sc)
T(9, Sc)
T(10, Sc)
T(11, Sc)
T(12, Sc)
T(13, Sc)
T(14, Sc)
T(15, Sc)
T(0, Ss)
T(1, Ss)
T(2, Ss)
T(3, Ss)
T(4, Ss)
T(5, Ss)
T(6, Ss)
T(7, Ss)
T(8, Ss)
T(9, Ss)
T(10, Ss)
T(11, Ss)
T(12, Ss)
T(13, Ss)
T(14, Ss)
T(15, Ss)
T(0, Si)
T(1, Si)
T(2, Si)
T(3, Si)
T(4, Si)
T(5, Si)
T(6, Si)
T(7, Si)
T(8, Si)
T(9, Si)
T(10, Si)
T(11, Si)
T(12, Si)
T(13, Si)
T(14, Si)
T(15, Si)
T(0, Ssc)
T(1, Ssc)
T(2, Ssc)
T(3, Ssc)
T(4, Ssc)
T(5, Ssc)
T(6, Ssc)
T(7, Ssc)
T(8, Ssc)
T(9, Ssc)
T(10, Ssc)
T(11, Ssc)
T(12, Ssc)
T(13, Ssc)
T(14, Ssc)
T(15, Ssc)
T(0, Sis)
T(1, Sis)
T(2, Sis)
T(3, Sis)
T(4, Sis)
T(5, Sis)
T(6, Sis)
T(7, Sis)
T(8, Sis)
T(9, Sis)
T(10, Sis)
T(11, Sis)
T(12, Sis)
T(13, Sis)
T(14, Sis)
T(15, Sis)
T(0, Scsi)
T(1, Scsi)
T(2, Scsi)
T(3, Scsi)
T(4, Scsi)
T(5, Scsi)
T(6, Scsi)
T(7, Scsi)
T(8, Scsi)
T(9, Scsi)
T(10, Scsi)
T(11, Scsi)
T(12, Scsi)
T(13, Scsi)
T(14, Scsi)
T(15, Scsi)
T(0, Scis)
T(1, Scis)
T(2, Scis)
T(3, Scis)
T(4, Scis)
T(5, Scis)
T(6, Scis)
T(7, Scis)
T(8, Scis)
T(9, Scis)
T(10, Scis)
T(11, Scis)
T(12, Scis)
T(13, Scis)
T(14, Scis)
T(15, Scis)
