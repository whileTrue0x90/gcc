
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_omg_CORBA_DATA_CONVERSION__
#define __org_omg_CORBA_DATA_CONVERSION__

#pragma interface

#include <org/omg/CORBA/SystemException.h>
extern "Java"
{
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
          class CompletionStatus;
          class DATA_CONVERSION;
      }
    }
  }
}

class org::omg::CORBA::DATA_CONVERSION : public ::org::omg::CORBA::SystemException
{

public:
  DATA_CONVERSION(::java::lang::String *);
  DATA_CONVERSION();
  DATA_CONVERSION(jint, ::org::omg::CORBA::CompletionStatus *);
  DATA_CONVERSION(::java::lang::String *, jint, ::org::omg::CORBA::CompletionStatus *);
private:
  static const jlong serialVersionUID = 1874869932271600956LL;
public:
  static ::java::lang::Class class$;
};

#endif // __org_omg_CORBA_DATA_CONVERSION__
