
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_io_ObjectIdentityWrapper__
#define __gnu_java_io_ObjectIdentityWrapper__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace io
      {
          class ObjectIdentityWrapper;
      }
    }
  }
}

class gnu::java::io::ObjectIdentityWrapper : public ::java::lang::Object
{

public:
  ObjectIdentityWrapper(::java::lang::Object *);
  virtual jint hashCode();
  virtual jboolean equals(::java::lang::Object *);
  virtual ::java::lang::String * toString();
  ::java::lang::Object * __attribute__((aligned(__alignof__( ::java::lang::Object)))) object;
  static ::java::lang::Class class$;
};

#endif // __gnu_java_io_ObjectIdentityWrapper__
