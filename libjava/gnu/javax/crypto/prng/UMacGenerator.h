
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_prng_UMacGenerator__
#define __gnu_javax_crypto_prng_UMacGenerator__

#pragma interface

#include <gnu/java/security/prng/BasePRNG.h>
extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace crypto
      {
        namespace cipher
        {
            class IBlockCipher;
        }
        namespace prng
        {
            class UMacGenerator;
        }
      }
    }
  }
}

class gnu::javax::crypto::prng::UMacGenerator : public ::gnu::java::security::prng::BasePRNG
{

public:
  UMacGenerator();
  virtual void setup(::java::util::Map *);
  virtual void fillBlock();
  static ::java::lang::String * INDEX;
  static ::java::lang::String * CIPHER;
private:
  ::gnu::javax::crypto::cipher::IBlockCipher * __attribute__((aligned(__alignof__( ::gnu::java::security::prng::BasePRNG)))) cipher;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_prng_UMacGenerator__
