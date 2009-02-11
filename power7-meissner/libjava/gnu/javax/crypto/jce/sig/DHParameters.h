
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_jce_sig_DHParameters__
#define __gnu_javax_crypto_jce_sig_DHParameters__

#pragma interface

#include <java/security/AlgorithmParametersSpi.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace crypto
      {
        namespace jce
        {
          namespace sig
          {
              class DHParameters;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace math
    {
        class BigInteger;
    }
    namespace security
    {
      namespace spec
      {
          class AlgorithmParameterSpec;
      }
    }
  }
}

class gnu::javax::crypto::jce::sig::DHParameters : public ::java::security::AlgorithmParametersSpi
{

public:
  DHParameters();
public: // actually protected
  virtual void engineInit(::java::security::spec::AlgorithmParameterSpec *);
  virtual void engineInit(JArray< jbyte > *);
  virtual void engineInit(JArray< jbyte > *, ::java::lang::String *);
  virtual ::java::security::spec::AlgorithmParameterSpec * engineGetParameterSpec(::java::lang::Class *);
  virtual JArray< jbyte > * engineGetEncoded();
  virtual JArray< jbyte > * engineGetEncoded(::java::lang::String *);
  virtual ::java::lang::String * engineToString();
private:
  ::java::math::BigInteger * __attribute__((aligned(__alignof__( ::java::security::AlgorithmParametersSpi)))) p;
  ::java::math::BigInteger * g;
  ::java::math::BigInteger * q;
  jint l;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_jce_sig_DHParameters__
