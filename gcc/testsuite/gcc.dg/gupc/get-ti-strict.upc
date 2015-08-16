/* { dg-do compile } */
/* { dg-options "-fdump-tree-original" } */
/* { dg-require-effective-target int128 } */

/* The base address of the UPC shared section */
extern char __upc_shared_start[1];

typedef unsigned int uint_ti_t __attribute__ ((__mode__(__TI__)));

/* UPC runtime remote access prototype */
typedef __SIZE_TYPE__ size_t;
extern uint_ti_t __getsblk3 (void *, upc_shared_ptr_t, size_t);

strict shared uint_ti_t x;

uint_ti_t p () {
  return x;
}

/* { dg-final { scan-tree-dump-times "getsblk3" 1 "original" } } */
/* { dg-final { scan-tree-dump-times "get|put" 1 "original" } } */
