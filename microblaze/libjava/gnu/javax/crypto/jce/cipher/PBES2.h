
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_jce_cipher_PBES2__
#define __gnu_javax_crypto_jce_cipher_PBES2__

#pragma interface

#include <gnu/javax/crypto/jce/cipher/CipherAdapter.h>
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
          namespace cipher
          {
              class PBES2;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace security
    {
        class AlgorithmParameters;
        class Key;
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
      namespace interfaces
      {
          class PBEKey;
      }
      namespace spec
      {
          class SecretKeySpec;
      }
    }
  }
}

class gnu::javax::crypto::jce::cipher::PBES2 : public ::gnu::javax::crypto::jce::cipher::CipherAdapter
{

public: // actually protected
  PBES2(::java::lang::String *, jint, ::java::lang::String *);
  PBES2(::java::lang::String *, ::java::lang::String *);
  virtual void engineInit(jint, ::java::security::Key *, ::java::security::SecureRandom *);
  virtual void engineInit(jint, ::java::security::Key *, ::java::security::spec::AlgorithmParameterSpec *, ::java::security::SecureRandom *);
  virtual void engineInit(jint, ::java::security::Key *, ::java::security::AlgorithmParameters *, ::java::security::SecureRandom *);
private:
  ::javax::crypto::spec::SecretKeySpec * genkey(::javax::crypto::interfaces::PBEKey *);
public: // actually protected
  ::java::lang::String * __attribute__((aligned(__alignof__( ::gnu::javax::crypto::jce::cipher::CipherAdapter)))) macName;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_jce_cipher_PBES2__
