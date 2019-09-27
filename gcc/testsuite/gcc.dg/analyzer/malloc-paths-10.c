#include <stdlib.h>

int test (int flag)
{
  int other_flag;
  if (flag)
    other_flag = 1;
  else
    other_flag = 0;

  /* With state-merging, we lose the relationship between 'flag' and 'other_flag'.  */
  __analyzer_dump_exploded_nodes (0); /* { dg-warning "2 exploded nodes" } */
  __analyzer_dump_exploded_nodes (0); /* { dg-warning "1 exploded node" } */

  if (other_flag)
    __analyzer_eval (flag); /* { dg-warning "UNKNOWN" } */
  else
    __analyzer_eval (flag); /* { dg-warning "UNKNOWN" } */
}
