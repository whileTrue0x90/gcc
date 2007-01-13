
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_SessionImpl__
#define __gnu_javax_net_ssl_provider_SessionImpl__

#pragma interface

#include <gnu/javax/net/ssl/Session.h>
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
            class Session$ID;
          namespace provider
          {
              class CipherSuite;
              class MaxFragmentLength;
              class ProtocolVersion;
              class SessionImpl;
              class SessionImpl$PrivateData;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace security
    {
        class SecureRandom;
      namespace cert
      {
          class Certificate;
      }
    }
  }
  namespace javax
  {
    namespace crypto
    {
        class SealedObject;
    }
  }
}

class gnu::javax::net::ssl::provider::SessionImpl : public ::gnu::javax::net::ssl::Session
{

public:
  SessionImpl();
public: // actually package-private
  virtual ::java::security::SecureRandom * random();
public:
  virtual ::java::lang::String * getProtocol();
  virtual void prepare(JArray< jchar > *);
  virtual void repair(JArray< jchar > *);
  virtual ::javax::crypto::SealedObject * privateData();
  virtual void setPrivateData(::javax::crypto::SealedObject *);
public: // actually package-private
  virtual void setApplicationBufferSize(jint);
  virtual void setRandom(::java::security::SecureRandom *);
  virtual void setTruncatedMac(jboolean);
  virtual void setId(::gnu::javax::net::ssl::Session$ID *);
  virtual void setLocalCertificates(JArray< ::java::security::cert::Certificate * > *);
  virtual void setPeerCertificates(JArray< ::java::security::cert::Certificate * > *);
  virtual void setPeerVerified(jboolean);
  static const jlong serialVersionUID = 8932976607588442485LL;
  ::gnu::javax::net::ssl::provider::CipherSuite * __attribute__((aligned(__alignof__( ::gnu::javax::net::ssl::Session)))) suite;
  ::gnu::javax::net::ssl::provider::ProtocolVersion * version;
  JArray< jbyte > * privateDataSalt;
  ::javax::crypto::SealedObject * sealedPrivateData;
  ::gnu::javax::net::ssl::provider::MaxFragmentLength * maxLength;
  ::gnu::javax::net::ssl::provider::SessionImpl$PrivateData * privateData__;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_SessionImpl__
