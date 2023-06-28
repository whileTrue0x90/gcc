/* { dg-do compile } */
/* { dg-require-effective-target vect_int } */

#include <vector>

void f4 (std::vector<int> a, std::vector<int> b, int n)
{
    int i = 0;
#pragma GCC novector
    while (i < (n & -8))
      {
        a[i] += b[i];
        i++;
      }
}


void f5 (std::vector<int> a, std::vector<int> b, int n)
{
    int i = 0;
#pragma GCC novector
    for (auto x : b)
      {
        a[i] += x;
        i++;
      }
}

/* { dg-final { scan-tree-dump-not "LOOP VECTORIZED" "vect" } } */
