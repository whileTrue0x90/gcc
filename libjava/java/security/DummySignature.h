
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_DummySignature__
#define __java_security_DummySignature__

#pragma interface

#include <java/security/Signature.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace security
    {
        class DummySignature;
        class PrivateKey;
        class PublicKey;
        class SignatureSpi;
    }
  }
}

class java::security::DummySignature : public ::java::security::Signature
{

public:
  DummySignature(::java::security::SignatureSpi *, ::java::lang::String *);
  ::java::lang::Object * clone();
public: // actually protected
  void engineInitVerify(::java::security::PublicKey *);
  void engineInitSign(::java::security::PrivateKey *);
  void engineUpdate(jbyte);
  void engineUpdate(JArray< jbyte > *, jint, jint);
  JArray< jbyte > * engineSign();
  jboolean engineVerify(JArray< jbyte > *);
  void engineSetParameter(::java::lang::String *, ::java::lang::Object *);
  ::java::lang::Object * engineGetParameter(::java::lang::String *);
private:
  ::java::security::SignatureSpi * __attribute__((aligned(__alignof__( ::java::security::Signature)))) sigSpi;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_DummySignature__
