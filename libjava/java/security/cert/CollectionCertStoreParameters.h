
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_cert_CollectionCertStoreParameters__
#define __java_security_cert_CollectionCertStoreParameters__

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
          class CollectionCertStoreParameters;
      }
    }
  }
}

class java::security::cert::CollectionCertStoreParameters : public ::java::lang::Object
{

public:
  CollectionCertStoreParameters();
  CollectionCertStoreParameters(::java::util::Collection *);
  virtual ::java::lang::Object * clone();
  virtual ::java::util::Collection * getCollection();
  virtual ::java::lang::String * toString();
private:
  ::java::util::Collection * __attribute__((aligned(__alignof__( ::java::lang::Object)))) collection;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_cert_CollectionCertStoreParameters__
