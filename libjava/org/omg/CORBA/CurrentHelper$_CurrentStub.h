
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_CORBA_CurrentHelper$_CurrentStub__
#define __org_omg_CORBA_CurrentHelper$_CurrentStub__

#pragma interface

#include <org/omg/CORBA/portable/ObjectImpl.h>
#include <gcj/array.h>

extern "Java"
{
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class CurrentHelper$_CurrentStub;
        namespace portable
        {
            class Delegate;
        }
      }
    }
  }
}

class org::omg::CORBA::CurrentHelper$_CurrentStub : public ::org::omg::CORBA::portable::ObjectImpl
{

public:
  CurrentHelper$_CurrentStub(::org::omg::CORBA::portable::Delegate *);
  virtual JArray< ::java::lang::String * > * _ids();
  static ::java::lang::Class class$;
};

#endif // __org_omg_CORBA_CurrentHelper$_CurrentStub__
