
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_SRPManagerParameters__
#define __gnu_javax_net_ssl_SRPManagerParameters__

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
        namespace sasl
        {
          namespace srp
          {
              class PasswordFile;
          }
        }
      }
      namespace net
      {
        namespace ssl
        {
            class SRPManagerParameters;
        }
      }
    }
  }
}

class gnu::javax::net::ssl::SRPManagerParameters : public ::java::lang::Object
{

public:
  SRPManagerParameters(::gnu::javax::crypto::sasl::srp::PasswordFile *);
  virtual ::gnu::javax::crypto::sasl::srp::PasswordFile * getPasswordFile();
private:
  ::gnu::javax::crypto::sasl::srp::PasswordFile * __attribute__((aligned(__alignof__( ::java::lang::Object)))) file;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_SRPManagerParameters__
