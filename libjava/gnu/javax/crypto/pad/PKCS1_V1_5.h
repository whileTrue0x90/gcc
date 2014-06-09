
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_pad_PKCS1_V1_5__
#define __gnu_javax_crypto_pad_PKCS1_V1_5__

#pragma interface

#include <gnu/javax/crypto/pad/BasePad.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace security
      {
        namespace sig
        {
          namespace rsa
          {
              class EME_PKCS1_V1_5;
          }
        }
      }
    }
    namespace javax
    {
      namespace crypto
      {
        namespace pad
        {
            class PKCS1_V1_5;
        }
      }
    }
  }
}

class gnu::javax::crypto::pad::PKCS1_V1_5 : public ::gnu::javax::crypto::pad::BasePad
{

public: // actually package-private
  PKCS1_V1_5();
public:
  virtual void setup();
  virtual JArray< jbyte > * pad(JArray< jbyte > *, jint, jint);
  virtual jint unpad(JArray< jbyte > *, jint, jint);
  virtual jboolean selfTest();
private:
  static ::java::util::logging::Logger * log;
  ::gnu::java::security::sig::rsa::EME_PKCS1_V1_5 * __attribute__((aligned(__alignof__( ::gnu::javax::crypto::pad::BasePad)))) codec;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_pad_PKCS1_V1_5__
