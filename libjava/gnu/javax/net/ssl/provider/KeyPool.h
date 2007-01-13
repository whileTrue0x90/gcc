
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_KeyPool__
#define __gnu_javax_net_ssl_provider_KeyPool__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace net
      {
        namespace ssl
        {
          namespace provider
          {
              class KeyPool;
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
        class KeyPair;
        class SecureRandom;
    }
  }
}

class gnu::javax::net::ssl::provider::KeyPool : public ::java::lang::Object
{

  KeyPool();
public: // actually package-private
  static ::java::security::KeyPair * generateRSAKeyPair();
private:
  static void nextBytes(JArray< jbyte > *);
  static ::java::math::BigInteger * ONE;
  static ::java::math::BigInteger * TWO;
  static ::java::math::BigInteger * E;
  static ::java::security::SecureRandom * RANDOM;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_KeyPool__
