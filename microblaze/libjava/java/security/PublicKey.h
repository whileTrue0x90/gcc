
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_PublicKey__
#define __java_security_PublicKey__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace security
    {
        class PublicKey;
    }
  }
}

class java::security::PublicKey : public ::java::lang::Object
{

public:
  virtual ::java::lang::String * getAlgorithm() = 0;
  virtual ::java::lang::String * getFormat() = 0;
  virtual JArray< jbyte > * getEncoded() = 0;
  static const jlong serialVersionUID = 7187392471159151072LL;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __java_security_PublicKey__
