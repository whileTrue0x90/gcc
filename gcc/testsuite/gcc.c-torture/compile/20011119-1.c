extern inline int foo (void)
{
  return 23;
}
extern int foo (void) __attribute__ ((weak, alias ("xxx")));
