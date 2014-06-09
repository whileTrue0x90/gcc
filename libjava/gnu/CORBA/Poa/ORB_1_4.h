
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_Poa_ORB_1_4__
#define __gnu_CORBA_Poa_ORB_1_4__

#pragma interface

#include <gnu/CORBA/OrbFunctional.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
        class Connected_objects$cObject;
      namespace DynAn
      {
          class gnuDynAnyFactory;
      }
        class IOR;
      namespace Interceptor
      {
          class gnuIcCurrent;
      }
      namespace Poa
      {
          class ORB_1_4;
          class gnuPOA;
          class gnuPoaCurrent;
      }
    }
  }
  namespace java
  {
    namespace applet
    {
        class Applet;
    }
  }
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class Any;
          class Object;
          class Policy;
      }
    }
  }
}

class gnu::CORBA::Poa::ORB_1_4 : public ::gnu::CORBA::OrbFunctional
{

public:
  ORB_1_4();
  virtual ::java::lang::String * object_to_string(::org::omg::CORBA::Object *);
  virtual void destroy();
public: // actually protected
  virtual void registerInterceptors(::java::util::Properties *, JArray< ::java::lang::String * > *);
  virtual ::gnu::CORBA::IOR * createIOR(::gnu::CORBA::Connected_objects$cObject *);
public:
  virtual ::org::omg::CORBA::Policy * create_policy(jint, ::org::omg::CORBA::Any *);
public: // actually protected
  virtual void set_parameters(::java::applet::Applet *, ::java::util::Properties *);
  virtual void set_parameters(JArray< ::java::lang::String * > *, ::java::util::Properties *);
public:
  virtual void set_delegate(::java::lang::Object *);
  ::gnu::CORBA::Poa::gnuPOA * __attribute__((aligned(__alignof__( ::gnu::CORBA::OrbFunctional)))) rootPOA;
  ::gnu::CORBA::Poa::gnuPoaCurrent * currents;
  ::gnu::CORBA::Interceptor::gnuIcCurrent * ic_current;
  ::gnu::CORBA::DynAn::gnuDynAnyFactory * factory;
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_Poa_ORB_1_4__
