
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_accessibility_AccessibleTextSequence__
#define __javax_accessibility_AccessibleTextSequence__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleTextSequence;
    }
  }
}

class javax::accessibility::AccessibleTextSequence : public ::java::lang::Object
{

public:
  AccessibleTextSequence(jint, jint, ::java::lang::String *);
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) text;
  jint startIndex;
  jint endIndex;
  static ::java::lang::Class class$;
};

#endif // __javax_accessibility_AccessibleTextSequence__
