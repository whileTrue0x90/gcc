/* { dg-do compile } */ 
/* { dg-options "-O1 -fscalar-evolutions -fdump-scalar-evolutions" } */

int main(void)
{
  int a;
  int b;
  int c;
  for (a = 22; a < 50; a+=1)
    {
      for (b = 23; b < 50; b+=5)
	{
	  /* The evolution of A depends on the evolution of B: we have to 
	     determine the number of iterations of loop 2 before analyzing 
	     the evolution of A.  */
	  ++a;
	  for (c = a; c < 100; c++)
	    {
	      
	    }
	}
    }
}

/* The analyzer has to detect the following scalar dependence graph:
   c -> a -> b
   
   Then, it has to detect the following evolution functions:
   b  ->  {23, +, 5}_2
   a  ->  {{22, +, 7}_1, +, 1}_2
   c  ->  {{{23, +, 7}_1, +, 1}_2, +, 1}_3
*/
/* { dg-final { diff-tree-dumps "scev" } } */
