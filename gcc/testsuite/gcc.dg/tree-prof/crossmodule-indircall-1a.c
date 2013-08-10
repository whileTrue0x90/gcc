/* { dg-require-effective-target lto } */
/* { dg-options "-O3 -flto -DDOJOB=1" } */
/* It seems there is no way to avoid the other source of mulitple
   source testcase from being compiled independently.  Just avoid
   error.  */
#ifdef DOJOB
extern int a;
void abort (void);

#ifdef _PROFILE_USE
__attribute__ ((externally_visible))
int constval=1,constval2=2;
#else
__attribute__ ((externally_visible))
int constval=3,constval2=2;
#endif


void
add(int i)
{
  /* Verify that inlining happens for first case.  */
  if (i==constval && !__builtin_constant_p (i))
    abort ();
  /* Second case has no dominating target; it should not inline.  */
  if (i==constval2 && __builtin_constant_p (i))
    abort ();
  a += i;
}
void
sub(int i)
{
  a -= i;
}
__attribute__ ((externally_visible))
void (*p[2])(int)={add, sub};
#else
main()
{
  return 0;
}
#endif
