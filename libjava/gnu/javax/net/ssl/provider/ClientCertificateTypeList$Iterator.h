
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_ClientCertificateTypeList$Iterator__
#define __gnu_javax_net_ssl_provider_ClientCertificateTypeList$Iterator__

#pragma interface

#include <java/lang/Object.h>
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
              class CertificateRequest$ClientCertificateType;
              class ClientCertificateTypeList;
              class ClientCertificateTypeList$Iterator;
          }
        }
      }
    }
  }
}

class gnu::javax::net::ssl::provider::ClientCertificateTypeList$Iterator : public ::java::lang::Object
{

public: // actually package-private
  ClientCertificateTypeList$Iterator(::gnu::javax::net::ssl::provider::ClientCertificateTypeList *);
public:
  virtual void ClientCertificateTypeList$Iterator$add(::gnu::javax::net::ssl::provider::CertificateRequest$ClientCertificateType *);
  virtual jboolean hasNext();
  virtual jboolean hasPrevious();
  virtual ::gnu::javax::net::ssl::provider::CertificateRequest$ClientCertificateType * ClientCertificateTypeList$Iterator$next();
  virtual jint nextIndex();
  virtual ::gnu::javax::net::ssl::provider::CertificateRequest$ClientCertificateType * ClientCertificateTypeList$Iterator$previous();
  virtual jint previousIndex();
  virtual void remove();
  virtual void ClientCertificateTypeList$Iterator$set(::gnu::javax::net::ssl::provider::CertificateRequest$ClientCertificateType *);
  virtual ::java::lang::Object * next();
  virtual ::java::lang::Object * previous();
  virtual void add(::java::lang::Object *);
  virtual void set(::java::lang::Object *);
private:
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) index;
  jint modCount;
public: // actually package-private
  ::gnu::javax::net::ssl::provider::ClientCertificateTypeList * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_ClientCertificateTypeList$Iterator__
