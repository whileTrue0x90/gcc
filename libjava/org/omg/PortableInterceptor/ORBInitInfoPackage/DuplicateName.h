
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_PortableInterceptor_ORBInitInfoPackage_DuplicateName__
#define __org_omg_PortableInterceptor_ORBInitInfoPackage_DuplicateName__

#pragma interface

#include <org/omg/CORBA/UserException.h>
extern "Java"
{
  namespace org
  {
    namespace omg
    {
      namespace PortableInterceptor
      {
        namespace ORBInitInfoPackage
        {
            class DuplicateName;
        }
      }
    }
  }
}

class org::omg::PortableInterceptor::ORBInitInfoPackage::DuplicateName : public ::org::omg::CORBA::UserException
{

public:
  DuplicateName();
  DuplicateName(::java::lang::String *, ::java::lang::String *);
  DuplicateName(::java::lang::String *);
private:
  static const jlong serialVersionUID = 7748239257677851130LL;
public:
  ::java::lang::String * __attribute__((aligned(__alignof__( ::org::omg::CORBA::UserException)))) name;
  static ::java::lang::Class class$;
};

#endif // __org_omg_PortableInterceptor_ORBInitInfoPackage_DuplicateName__
