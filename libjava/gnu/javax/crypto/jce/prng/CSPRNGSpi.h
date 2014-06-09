
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_jce_prng_CSPRNGSpi__
#define __gnu_javax_crypto_jce_prng_CSPRNGSpi__

#pragma interface

#include <java/security/SecureRandomSpi.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace security
      {
        namespace prng
        {
            class IRandom;
        }
      }
    }
    namespace javax
    {
      namespace crypto
      {
        namespace jce
        {
          namespace prng
          {
              class CSPRNGSpi;
          }
        }
      }
    }
  }
}

class gnu::javax::crypto::jce::prng::CSPRNGSpi : public ::java::security::SecureRandomSpi
{

public:
  CSPRNGSpi();
public: // actually protected
  virtual JArray< jbyte > * engineGenerateSeed(jint);
  virtual void engineNextBytes(JArray< jbyte > *);
  virtual void engineSetSeed(JArray< jbyte > *);
private:
  ::gnu::java::security::prng::IRandom * __attribute__((aligned(__alignof__( ::java::security::SecureRandomSpi)))) adaptee;
  jboolean virgin;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_jce_prng_CSPRNGSpi__
