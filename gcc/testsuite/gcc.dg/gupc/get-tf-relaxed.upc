/* { dg-do compile } */
/* { dg-options "-fdump-tree-original" } */
/* { dg-require-effective-target longdouble128 } */

/* The base address of the UPC shared section */
extern char __upc_shared_start[1];


/* UPC runtime remote access prototype */
extern long double __gettf2 (upc_shared_ptr_t);
extern long double __getxf2 (upc_shared_ptr_t);

relaxed shared long double x;

long double p () {
  return x;
}

/* { dg-final { scan-tree-dump-times "get\[tx\]f2" 1 "original" } } */
/* { dg-final { scan-tree-dump-times "get|put" 1 "original" } } */
