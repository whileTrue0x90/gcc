
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_crypto_KeyGeneratorSpi__
#define __javax_crypto_KeyGeneratorSpi__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace security
    {
        class SecureRandom;
      namespace spec
      {
          class AlgorithmParameterSpec;
      }
    }
  }
  namespace javax
  {
    namespace crypto
    {
        class KeyGeneratorSpi;
        class SecretKey;
    }
  }
}

class javax::crypto::KeyGeneratorSpi : public ::java::lang::Object
{

public:
  KeyGeneratorSpi();
public: // actually protected
  virtual ::javax::crypto::SecretKey * engineGenerateKey() = 0;
  virtual void engineInit(::java::security::spec::AlgorithmParameterSpec *, ::java::security::SecureRandom *) = 0;
  virtual void engineInit(jint, ::java::security::SecureRandom *) = 0;
  virtual void engineInit(::java::security::SecureRandom *) = 0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_crypto_KeyGeneratorSpi__
