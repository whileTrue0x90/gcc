
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_SecurityPermission__
#define __java_security_SecurityPermission__

#pragma interface

#include <java/security/BasicPermission.h>
extern "Java"
{
  namespace java
  {
    namespace security
    {
        class SecurityPermission;
    }
  }
}

class java::security::SecurityPermission : public ::java::security::BasicPermission
{

public:
  SecurityPermission(::java::lang::String *);
  SecurityPermission(::java::lang::String *, ::java::lang::String *);
private:
  static const jlong serialVersionUID = 5236109936224050470LL;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_SecurityPermission__
