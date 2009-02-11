
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_prng_ARCFour__
#define __gnu_javax_crypto_prng_ARCFour__

#pragma interface

#include <gnu/java/security/prng/BasePRNG.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace crypto
      {
        namespace prng
        {
            class ARCFour;
        }
      }
    }
  }
}

class gnu::javax::crypto::prng::ARCFour : public ::gnu::java::security::prng::BasePRNG
{

public:
  ARCFour();
  virtual void setup(::java::util::Map *);
  virtual void fillBlock();
  static ::java::lang::String * ARCFOUR_KEY_MATERIAL;
  static const jint ARCFOUR_SBOX_SIZE = 256;
private:
  JArray< jbyte > * __attribute__((aligned(__alignof__( ::gnu::java::security::prng::BasePRNG)))) s;
  jbyte m;
  jbyte n;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_prng_ARCFour__
