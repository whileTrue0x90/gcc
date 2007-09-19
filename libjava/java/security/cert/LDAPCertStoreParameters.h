
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_cert_LDAPCertStoreParameters__
#define __java_security_cert_LDAPCertStoreParameters__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace security
    {
      namespace cert
      {
          class LDAPCertStoreParameters;
      }
    }
  }
}

class java::security::cert::LDAPCertStoreParameters : public ::java::lang::Object
{

public:
  LDAPCertStoreParameters();
  LDAPCertStoreParameters(::java::lang::String *);
  LDAPCertStoreParameters(::java::lang::String *, jint);
  virtual ::java::lang::Object * clone();
  virtual ::java::lang::String * getServerName();
  virtual jint getPort();
  virtual ::java::lang::String * toString();
private:
  static const jint LDAP_PORT = 389;
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) serverName;
  jint port;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_cert_LDAPCertStoreParameters__
