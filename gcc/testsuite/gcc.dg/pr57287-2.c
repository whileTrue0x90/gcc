/* { dg-do compile } */
/* { dg-options "-O2 -Wall" } */

#include <setjmp.h>

struct node
{
  struct node *next;
  char *name;
} *list;

struct node *list;
struct node *head (void);

sigjmp_buf *bar (void);

int baz (void)
{
  struct node *n;
  int varseen = 0;

  list = head ();
  for (n = list; n; n = n->next)
    {
      if (!varseen)
	varseen = 1;

      sigjmp_buf *buf = bar ();  /* { dg-bogus "may be used uninitialized" "" } */
      __sigsetjmp (*buf, 1);
    }

  if (!varseen)
    return 0;
  return 1;
}
