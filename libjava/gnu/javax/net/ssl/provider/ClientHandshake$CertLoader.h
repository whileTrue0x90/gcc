
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_ClientHandshake$CertLoader__
#define __gnu_javax_net_ssl_provider_ClientHandshake$CertLoader__

#pragma interface

#include <gnu/javax/net/ssl/provider/DelegatedTask.h>
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
              class ClientHandshake;
              class ClientHandshake$CertLoader;
          }
        }
      }
    }
  }
}

class gnu::javax::net::ssl::provider::ClientHandshake$CertLoader : public ::gnu::javax::net::ssl::provider::DelegatedTask
{

public: // actually package-private
  ClientHandshake$CertLoader(::gnu::javax::net::ssl::provider::ClientHandshake *, ::java::util::List *, ::java::util::List *);
public:
  virtual void implRun();
private:
  ::java::util::List * __attribute__((aligned(__alignof__( ::gnu::javax::net::ssl::provider::DelegatedTask)))) keyTypes;
  ::java::util::List * issuers;
public: // actually package-private
  ::gnu::javax::net::ssl::provider::ClientHandshake * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_ClientHandshake$CertLoader__
