/* { dg-do run } */
/* { dg-options "-DEXPENSIVE" { target run_expensive_tests } } */
/* { dg-skip-if "" { ! run_expensive_tests } { "*" } { "-O2" } } */
/* { dg-skip-if "" { ! run_expensive_tests } { "-flto" } { "" } } */

typedef unsigned char V __attribute__((vector_size(8)));
typedef V VI;

#include "vshuf-8.inc"
#include "vshuf-main.inc"
