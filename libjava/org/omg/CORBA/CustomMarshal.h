
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_CORBA_CustomMarshal__
#define __org_omg_CORBA_CustomMarshal__

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
          class CustomMarshal;
          class DataInputStream;
          class DataOutputStream;
      }
    }
  }
}

class org::omg::CORBA::CustomMarshal : public ::java::lang::Object
{

public:
  virtual void marshal(::org::omg::CORBA::DataOutputStream *) = 0;
  virtual void unmarshal(::org::omg::CORBA::DataInputStream *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __org_omg_CORBA_CustomMarshal__
