
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_Spring$2__
#define __javax_swing_Spring$2__

#pragma interface

#include <javax/swing/Spring.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Component;
    }
  }
  namespace javax
  {
    namespace swing
    {
        class Spring$2;
    }
  }
}

class javax::swing::Spring$2 : public ::javax::swing::Spring
{

public: // actually package-private
  Spring$2(::java::awt::Component *);
public:
  jint getMaximumValue();
  jint getMinimumValue();
  jint getPreferredValue();
  jint getValue();
  void setValue(jint);
private:
  ::java::awt::Component * __attribute__((aligned(__alignof__( ::javax::swing::Spring)))) val$component;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_Spring$2__
