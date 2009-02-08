/* Check that malloc's alignment honors what we trust it
   minimally should.  */

/* { dg-do run } */
/* { dg-options "-fno-builtin-malloc" } */

#include <stdlib.h>                                                            
typedef int word __attribute__((mode(word)));

int main()
{
    if ((size_t)malloc (1) & (sizeof(word)-1))
	abort ();
    return 0;
}                                                                              
