// APPLE LOCAL file Objective-C++
// Check if ivars may be accessed via the C++ dot notation.
// { dg-do run }
// { dg-options "-fno-objc-call-cxx-cdtors" }

#include <objc/Object.h>
#include <stdlib.h>
#define CHECK_IF(expr) if(!(expr)) abort()

struct cxx_struct {
  int a, b;
  void set_values (int _a, int _b = 3) {
    a = _a; b = _b;
  }
  ~cxx_struct (void) {
    a = b = 99;
  }
};

@interface Manip : Object {
  int c;
  cxx_struct s;   // { dg-warning "user-defined destructor" }
                  // { dg-warning "constructors and destructors will not be invoked" "" { target *-*-* } 22 }
}
- (void) manipulate_ivars;
@end

@implementation Manip
- (void) manipulate_ivars {
  s.set_values (7);
  CHECK_IF (s.a == 7 && s.b == 3);
  s.~cxx_struct();
  CHECK_IF (s.a == 99 && s.b == 99);
}
@end

int main (void)
{
  Manip *obj = [Manip new];
  [obj manipulate_ivars];
  [obj free];
}
