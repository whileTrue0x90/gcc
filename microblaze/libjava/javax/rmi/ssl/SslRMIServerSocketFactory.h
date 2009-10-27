
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_rmi_ssl_SslRMIServerSocketFactory__
#define __javax_rmi_ssl_SslRMIServerSocketFactory__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace net
    {
        class ServerSocket;
    }
  }
  namespace javax
  {
    namespace net
    {
      namespace ssl
      {
          class SSLServerSocketFactory;
      }
    }
    namespace rmi
    {
      namespace ssl
      {
          class SslRMIServerSocketFactory;
      }
    }
  }
}

class javax::rmi::ssl::SslRMIServerSocketFactory : public ::java::lang::Object
{

public:
  SslRMIServerSocketFactory();
  SslRMIServerSocketFactory(JArray< ::java::lang::String * > *, JArray< ::java::lang::String * > *, jboolean);
  virtual ::java::net::ServerSocket * createServerSocket(jint);
  virtual jboolean equals(::java::lang::Object *);
public: // actually package-private
  static jboolean cmpStrArray(JArray< ::java::lang::String * > *, JArray< ::java::lang::String * > *);
public:
  virtual JArray< ::java::lang::String * > * getEnabledCipherSuites();
  virtual JArray< ::java::lang::String * > * getEnabledProtocols();
  virtual jboolean getNeedClientAuth();
  virtual jint hashCode();
private:
  JArray< ::java::lang::String * > * __attribute__((aligned(__alignof__( ::java::lang::Object)))) enabledCipherSuites;
  JArray< ::java::lang::String * > * enabledProtocols;
  jboolean needClientAuth;
  static ::javax::net::ssl::SSLServerSocketFactory * socketFactory;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_rmi_ssl_SslRMIServerSocketFactory__
