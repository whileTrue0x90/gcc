
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_GuardedObject__
#define __java_security_GuardedObject__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace security
    {
        class Guard;
        class GuardedObject;
    }
  }
}

class java::security::GuardedObject : public ::java::lang::Object
{

public:
  GuardedObject(::java::lang::Object *, ::java::security::Guard *);
  virtual ::java::lang::Object * getObject();
private:
  void writeObject(::java::io::ObjectOutputStream *);
  static const jlong serialVersionUID = -5240450096227834308LL;
  ::java::security::Guard * __attribute__((aligned(__alignof__( ::java::lang::Object)))) guard;
  ::java::lang::Object * object;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_GuardedObject__
