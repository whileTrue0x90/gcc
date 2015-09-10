/* Have to enable optimizations, as otherwise builtins won't be expanded.  */
/* { dg-additional-options "-O -fdump-rtl-expand" } */

#if __cplusplus
extern "C" {
#endif

typedef enum acc_device_t { acc_device_X = 123 } acc_device_t;
extern int acc_on_device (acc_device_t);

#if __cplusplus
}
#endif

int
f (void)
{
  const acc_device_t dev = acc_device_X;
  return acc_on_device (dev);
}

/* With -fopenacc, we're expecting the builtin to be expanded, so no calls.
   TODO: in C++, even under extern "C", the use of enum for acc_device_t
   perturbs expansion as a builtin, which expects an int parameter.  It's fine
   when changing acc_device_t to plain int, but that's not what we're doing in
   <openacc.h>.

   { dg-final { scan-rtl-dump-times "\\\(call \[^\\n\]* acc_on_device" 0 "expand" { xfail c++ } } } */
