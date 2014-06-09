// Check if ObjC classes with non-POD C++ ivars are specially marked in the metadata.

// { dg-do run { target *-*-darwin* } }
// { dg-skip-if "" { *-*-* } { "-fgnu-runtime" } { "" } }
// { dg-options "-fobjc-call-cxx-cdtors -mmacosx-version-min=10.4" }
// This test has no equivalent or meaning for m64/ABI V2
// { dg-xfail-run-if "No Test Avail" { *-*-darwin* && lp64 } { "-fnext-runtime" } { "" } }

#include <objc/objc-runtime.h>
#include <stdlib.h>
#define CHECK_IF(expr) if(!(expr)) abort()

#ifndef CLS_HAS_CXX_STRUCTORS
#define CLS_HAS_CXX_STRUCTORS 0x2000L
#endif

struct cxx_struct {
  int a, b;
  cxx_struct (void) { a = b = 55; }
};

@interface Foo {
  int c;
  cxx_struct s;
}
@end

@interface Bar: Foo {
  float f;
}
@end

@implementation Foo
@end

@implementation Bar
@end

int main (void)
{
#ifndef __LP64__
  Class cls;

  cls = objc_getClass("Foo");
  CHECK_IF(cls->info & CLS_HAS_CXX_STRUCTORS);
  cls = objc_getClass("Bar");
  CHECK_IF(!(cls->info & CLS_HAS_CXX_STRUCTORS));

#else
  /* No test needed or available.  */
  abort ();
#endif
  return 0;
}
