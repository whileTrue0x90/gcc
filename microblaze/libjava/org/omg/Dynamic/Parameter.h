
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_Dynamic_Parameter__
#define __org_omg_Dynamic_Parameter__

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
          class ParameterMode;
      }
      namespace Dynamic
      {
          class Parameter;
      }
    }
  }
}

class org::omg::Dynamic::Parameter : public ::java::lang::Object
{

public:
  Parameter();
  Parameter(::org::omg::CORBA::Any *, ::org::omg::CORBA::ParameterMode *);
private:
  static const jlong serialVersionUID = 892191606993734699LL;
public:
  ::org::omg::CORBA::Any * __attribute__((aligned(__alignof__( ::java::lang::Object)))) argument;
  ::org::omg::CORBA::ParameterMode * mode;
  static ::java::lang::Class class$;
};

#endif // __org_omg_Dynamic_Parameter__
