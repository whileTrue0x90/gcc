
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_prng_CSPRNG$1__
#define __gnu_javax_crypto_prng_CSPRNG$1__

#pragma interface

#include <java/lang/Object.h>
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
            class CSPRNG$1;
        }
      }
    }
  }
}

class gnu::javax::crypto::prng::CSPRNG$1 : public ::java::lang::Object
{

public: // actually package-private
  CSPRNG$1(::java::lang::String *);
public:
  ::java::lang::Object * run();
private:
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) val$name;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_prng_CSPRNG$1__
