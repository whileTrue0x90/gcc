/* { dg-do compile } */
/* { dg-options "-fcilkplus" } */

int main (void)
{
  int array[10][10], array2[10];
  
  array2[:] = array2[1::] ;  /* { dg-error "expected expression before" } */

  return 0; /* { dg-error "expected ';' before" } */
}
