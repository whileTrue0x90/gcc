
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_CosNaming_IstringHelper__
#define __org_omg_CosNaming_IstringHelper__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class Any;
          class TypeCode;
        namespace portable
        {
            class InputStream;
            class OutputStream;
        }
      }
      namespace CosNaming
      {
          class IstringHelper;
      }
    }
  }
}

class org::omg::CosNaming::IstringHelper : public ::java::lang::Object
{

public:
  IstringHelper();
  static void insert(::org::omg::CORBA::Any *, ::java::lang::String *);
  static ::java::lang::String * extract(::org::omg::CORBA::Any *);
  static ::org::omg::CORBA::TypeCode * type();
  static ::java::lang::String * id();
  static ::java::lang::String * read(::org::omg::CORBA::portable::InputStream *);
  static void write(::org::omg::CORBA::portable::OutputStream *, ::java::lang::String *);
  static ::java::lang::Class class$;
};

#endif // __org_omg_CosNaming_IstringHelper__
