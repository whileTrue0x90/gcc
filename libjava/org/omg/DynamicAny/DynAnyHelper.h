
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_DynamicAny_DynAnyHelper__
#define __org_omg_DynamicAny_DynAnyHelper__

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
          class Object;
          class TypeCode;
        namespace portable
        {
            class InputStream;
            class OutputStream;
        }
      }
      namespace DynamicAny
      {
          class DynAny;
          class DynAnyHelper;
      }
    }
  }
}

class org::omg::DynamicAny::DynAnyHelper : public ::java::lang::Object
{

public:
  DynAnyHelper();
  static ::org::omg::DynamicAny::DynAny * narrow(::org::omg::CORBA::Object *);
  static ::org::omg::DynamicAny::DynAny * unchecked_narrow(::org::omg::CORBA::Object *);
  static ::org::omg::CORBA::TypeCode * type();
  static void insert(::org::omg::CORBA::Any *, ::org::omg::DynamicAny::DynAny *);
  static ::org::omg::DynamicAny::DynAny * extract(::org::omg::CORBA::Any *);
  static ::java::lang::String * id();
  static ::org::omg::DynamicAny::DynAny * read(::org::omg::CORBA::portable::InputStream *);
  static void write(::org::omg::CORBA::portable::OutputStream *, ::org::omg::DynamicAny::DynAny *);
  static ::java::lang::Class class$;
};

#endif // __org_omg_DynamicAny_DynAnyHelper__
