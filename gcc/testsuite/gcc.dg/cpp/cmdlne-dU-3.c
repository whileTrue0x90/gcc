/* { dg-do preprocess } */
/* { dg-options "-P -dU" } */
/* { dg-final { scan-file cmdlne-dU-3.i "^\n*#define A B\n*$" } } */
#define A B
#ifndef A
#endif
