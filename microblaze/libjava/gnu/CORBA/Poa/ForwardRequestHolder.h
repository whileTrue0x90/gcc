
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_Poa_ForwardRequestHolder__
#define __gnu_CORBA_Poa_ForwardRequestHolder__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
      namespace Poa
      {
          class ForwardRequestHolder;
      }
    }
  }
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class TypeCode;
        namespace portable
        {
            class InputStream;
            class OutputStream;
        }
      }
      namespace PortableServer
      {
          class ForwardRequest;
      }
    }
  }
}

class gnu::CORBA::Poa::ForwardRequestHolder : public ::java::lang::Object
{

public:
  ForwardRequestHolder();
  ForwardRequestHolder(::org::omg::PortableServer::ForwardRequest *);
  virtual void _read(::org::omg::CORBA::portable::InputStream *);
  virtual ::org::omg::CORBA::TypeCode * _type();
  virtual void _write(::org::omg::CORBA::portable::OutputStream *);
  ::org::omg::PortableServer::ForwardRequest * __attribute__((aligned(__alignof__( ::java::lang::Object)))) value;
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_Poa_ForwardRequestHolder__
