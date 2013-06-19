/* { dg-do compile } */
/* { dg-require-effective-target powerpc_eabi_ok } */
/* { dg-options "-mcpu=8540 -msoft-float -msdata=eabi -G 8 -fno-common" } */

extern void f (void);

struct s
{
  int *p;
  int *q;
};

extern int a;

extern const struct s c;

const struct s c = { &a, 0 };

void
f (void)
{
  char buf[4] = { 0, 1, 2, 3 };
}
