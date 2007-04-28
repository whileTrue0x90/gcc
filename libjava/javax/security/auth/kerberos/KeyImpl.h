
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_security_auth_kerberos_KeyImpl__
#define __javax_security_auth_kerberos_KeyImpl__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace security
    {
      namespace auth
      {
        namespace kerberos
        {
            class KeyImpl;
        }
      }
    }
  }
}

class javax::security::auth::kerberos::KeyImpl : public ::java::lang::Object
{

public:
  KeyImpl(JArray< jbyte > *, jint);
  KeyImpl(JArray< jchar > *, ::java::lang::String *);
  ::java::lang::String * getAlgorithm();
  JArray< jbyte > * getEncoded();
  ::java::lang::String * getFormat();
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) algorithm;
  jint type;
  JArray< jbyte > * key;
  static ::java::lang::Class class$;
};

#endif // __javax_security_auth_kerberos_KeyImpl__
