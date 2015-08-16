/* { dg-do compile } */
/* { dg-options "-fdump-tree-original" } */

/* The base address of the UPC shared section */
extern char __upc_shared_start[1];

typedef unsigned int uint_qi_t __attribute__ ((__mode__(__QI__)));

/* UPC runtime remote access prototype */
extern uint_qi_t __getsqi2 (upc_shared_ptr_t);

strict shared uint_qi_t x;

uint_qi_t p () {
  return x;
}

/* { dg-final { scan-tree-dump-times "getsqi2" 1 "original" } } */
/* { dg-final { scan-tree-dump-times "get|put" 1 "original" } } */
