
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_crypto_sasl_IllegalMechanismStateException__
#define __gnu_javax_crypto_sasl_IllegalMechanismStateException__

#pragma interface

#include <javax/security/sasl/AuthenticationException.h>
extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace crypto
      {
        namespace sasl
        {
            class IllegalMechanismStateException;
        }
      }
    }
  }
}

class gnu::javax::crypto::sasl::IllegalMechanismStateException : public ::javax::security::sasl::AuthenticationException
{

public:
  IllegalMechanismStateException();
  IllegalMechanismStateException(::java::lang::String *);
  IllegalMechanismStateException(::java::lang::String *, ::java::lang::Throwable *);
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_crypto_sasl_IllegalMechanismStateException__
