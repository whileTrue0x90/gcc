
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_cert_PKIXCertPathChecker__
#define __java_security_cert_PKIXCertPathChecker__

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
          class Certificate;
          class PKIXCertPathChecker;
      }
    }
  }
}

class java::security::cert::PKIXCertPathChecker : public ::java::lang::Object
{

public: // actually protected
  PKIXCertPathChecker();
public:
  virtual ::java::lang::Object * clone();
  virtual void init(jboolean) = 0;
  virtual jboolean isForwardCheckingSupported() = 0;
  virtual ::java::util::Set * getSupportedExtensions() = 0;
  virtual void check(::java::security::cert::Certificate *, ::java::util::Collection *) = 0;
  static ::java::lang::Class class$;
};

#endif // __java_security_cert_PKIXCertPathChecker__
