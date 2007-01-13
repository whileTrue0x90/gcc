
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_PortableServer_Servant__
#define __org_omg_PortableServer_Servant__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class ORB;
          class Object;
      }
      namespace PortableServer
      {
          class POA;
          class Servant;
        namespace portable
        {
            class Delegate;
        }
      }
    }
  }
}

class org::omg::PortableServer::Servant : public ::java::lang::Object
{

public:
  Servant();
  virtual JArray< ::java::lang::String * > * _all_interfaces(::org::omg::PortableServer::POA *, JArray< jbyte > *) = 0;
  virtual ::org::omg::PortableServer::portable::Delegate * _get_delegate();
  virtual ::org::omg::CORBA::Object * _get_interface_def();
  virtual jboolean _is_a(::java::lang::String *);
  virtual jboolean _non_existent();
  virtual ::org::omg::CORBA::ORB * _orb();
  virtual ::org::omg::PortableServer::POA * _default_POA();
  virtual JArray< jbyte > * _object_id();
  virtual ::org::omg::PortableServer::POA * _poa();
  virtual void _set_delegate(::org::omg::PortableServer::portable::Delegate *);
  virtual ::org::omg::CORBA::Object * _this_object(::org::omg::CORBA::ORB *);
  virtual ::org::omg::CORBA::Object * _this_object();
private:
  ::org::omg::PortableServer::portable::Delegate * __attribute__((aligned(__alignof__( ::java::lang::Object)))) delegate;
public:
  static ::java::lang::Class class$;
};

#endif // __org_omg_PortableServer_Servant__
