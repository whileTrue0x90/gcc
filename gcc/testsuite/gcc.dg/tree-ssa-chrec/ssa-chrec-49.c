/* { dg-do compile } */ 
/* { dg-options "-O1 -fscalar-evolutions -fdump-scalar-evolutions" } */

int 
foo (int *c)
{
  int i = 0;
  int j = 10;
  
  while (1)
    {
      if (i == j)
	break;
      
      i++;
      j--;
    }
  
  return j;
}

/* i  ->  {0, +, 1}_1  */
/* j  ->  {10, +, -1}_1  */

/* { dg-final { diff-tree-dumps "scev" } } */
