/* { dg-do compile } */ 
/* { dg-options "-O1 -fdump-tree-optimized" } */

extern void link_error (void);

/* Check for cprop on fields of the same struct.  */

struct s
{
  char d;
  int a, b;
  double m;
};


void foo (struct s*  r)
{
  r->a = 0;
  r->b = 1;
  r->a++;
  r->b++;
  if (r->a != 1)
    link_error ();
  if (r->b != 2)
    link_error ();
}

/* There should be no link_error calls.  */
/* { dg-final { scan-tree-dump-times "link_error" 0 "optimized" { xfail *-*-* } } } */
