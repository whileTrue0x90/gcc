/* { dg-do compile } */
/* { dg-options "-O2 -fdump-tree-reassoc1" } */

unsigned int test1 (unsigned int x, unsigned int y, unsigned int z,
		    unsigned int weight)
{
  unsigned int tmp1 = x * weight;
  unsigned int tmp2 = y * weight;
  unsigned int tmp3 = (x - y) * weight;
  return tmp1 + (tmp2 + tmp3);
}

unsigned int test2 (unsigned int x, unsigned int y, unsigned int z,
		    unsigned int weight)
{
  unsigned int tmp1 = x * weight;
  unsigned int tmp2 = y * weight * weight;
  unsigned int tmp3 = z * weight * weight * weight;
  return tmp1 + tmp2 + tmp3;
}

/* There should be one multiplication left in test1 and three in test2.  */

/* { dg-final { scan-tree-dump-times "\\\*" 4 "reassoc1" } } */
/* { dg-final { cleanup-tree-dump "reassoc1" } } */
