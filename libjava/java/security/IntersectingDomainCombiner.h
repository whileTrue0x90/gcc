
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_IntersectingDomainCombiner__
#define __java_security_IntersectingDomainCombiner__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace security
    {
        class IntersectingDomainCombiner;
        class ProtectionDomain;
    }
  }
}

class java::security::IntersectingDomainCombiner : public ::java::lang::Object
{

  IntersectingDomainCombiner();
public:
  JArray< ::java::security::ProtectionDomain * > * combine(JArray< ::java::security::ProtectionDomain * > *, JArray< ::java::security::ProtectionDomain * > *);
public: // actually package-private
  static ::java::security::IntersectingDomainCombiner * SINGLETON;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_IntersectingDomainCombiner__
