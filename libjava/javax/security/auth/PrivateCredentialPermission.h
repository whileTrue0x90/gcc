
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_security_auth_PrivateCredentialPermission__
#define __javax_security_auth_PrivateCredentialPermission__

#pragma interface

#include <java/security/Permission.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace security
    {
        class Permission;
        class PermissionCollection;
    }
  }
  namespace javax
  {
    namespace security
    {
      namespace auth
      {
          class PrivateCredentialPermission;
      }
    }
  }
}

class javax::security::auth::PrivateCredentialPermission : public ::java::security::Permission
{

public:
  PrivateCredentialPermission(::java::lang::String *, ::java::lang::String *);
  jboolean equals(::java::lang::Object *);
  ::java::lang::String * getActions();
  ::java::lang::String * getCredentialClass();
  JArray< JArray< ::java::lang::String * > * > * getPrincipals();
  jint hashCode();
  jboolean implies(::java::security::Permission *);
  ::java::security::PermissionCollection * newPermissionCollection();
private:
  static const jlong serialVersionUID = 5284372143517237068LL;
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::security::Permission)))) credentialClass;
  ::java::util::Set * principals;
  jboolean testing;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_security_auth_PrivateCredentialPermission__
