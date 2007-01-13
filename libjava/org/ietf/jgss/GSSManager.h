
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_ietf_jgss_GSSManager__
#define __org_ietf_jgss_GSSManager__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace security
    {
        class Provider;
    }
  }
  namespace org
  {
    namespace ietf
    {
      namespace jgss
      {
          class GSSContext;
          class GSSCredential;
          class GSSManager;
          class GSSName;
          class Oid;
      }
    }
  }
}

class org::ietf::jgss::GSSManager : public ::java::lang::Object
{

public:
  GSSManager();
  static ::org::ietf::jgss::GSSManager * getInstance();
  virtual void addProviderAtEnd(::java::security::Provider *, ::org::ietf::jgss::Oid *) = 0;
  virtual void addProviderAtFront(::java::security::Provider *, ::org::ietf::jgss::Oid *) = 0;
  virtual ::org::ietf::jgss::GSSContext * createContext(JArray< jbyte > *) = 0;
  virtual ::org::ietf::jgss::GSSContext * createContext(::org::ietf::jgss::GSSCredential *) = 0;
  virtual ::org::ietf::jgss::GSSContext * createContext(::org::ietf::jgss::GSSName *, ::org::ietf::jgss::Oid *, ::org::ietf::jgss::GSSCredential *, jint) = 0;
  virtual ::org::ietf::jgss::GSSCredential * createCredential(jint) = 0;
  virtual ::org::ietf::jgss::GSSCredential * createCredential(::org::ietf::jgss::GSSName *, jint, ::org::ietf::jgss::Oid *, jint) = 0;
  virtual ::org::ietf::jgss::GSSCredential * createCredential(::org::ietf::jgss::GSSName *, jint, JArray< ::org::ietf::jgss::Oid * > *, jint) = 0;
  virtual ::org::ietf::jgss::GSSName * createName(JArray< jbyte > *, ::org::ietf::jgss::Oid *) = 0;
  virtual ::org::ietf::jgss::GSSName * createName(JArray< jbyte > *, ::org::ietf::jgss::Oid *, ::org::ietf::jgss::Oid *) = 0;
  virtual ::org::ietf::jgss::GSSName * createName(::java::lang::String *, ::org::ietf::jgss::Oid *) = 0;
  virtual ::org::ietf::jgss::GSSName * createName(::java::lang::String *, ::org::ietf::jgss::Oid *, ::org::ietf::jgss::Oid *) = 0;
  virtual JArray< ::org::ietf::jgss::Oid * > * getMechs() = 0;
  virtual JArray< ::org::ietf::jgss::Oid * > * getMechsForName(::org::ietf::jgss::Oid *) = 0;
  virtual JArray< ::org::ietf::jgss::Oid * > * getNamesForMech(::org::ietf::jgss::Oid *) = 0;
  static ::java::lang::Class class$;
};

#endif // __org_ietf_jgss_GSSManager__
