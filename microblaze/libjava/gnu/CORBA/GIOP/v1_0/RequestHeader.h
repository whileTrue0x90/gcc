
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_GIOP_v1_0_RequestHeader__
#define __gnu_CORBA_GIOP_v1_0_RequestHeader__

#pragma interface

#include <gnu/CORBA/GIOP/RequestHeader.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
      namespace CDR
      {
          class AbstractCdrInput;
          class AbstractCdrOutput;
      }
      namespace GIOP
      {
        namespace v1_0
        {
            class RequestHeader;
        }
      }
    }
  }
}

class gnu::CORBA::GIOP::v1_0::RequestHeader : public ::gnu::CORBA::GIOP::RequestHeader
{

public:
  RequestHeader();
  virtual void setResponseExpected(jboolean);
  virtual jboolean isResponseExpected();
  virtual ::java::lang::String * bytes(JArray< jbyte > *);
  virtual ::java::lang::String * contexts();
  virtual void read(::gnu::CORBA::CDR::AbstractCdrInput *);
  virtual ::java::lang::String * toString();
  virtual void write(::gnu::CORBA::CDR::AbstractCdrOutput *);
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_GIOP_v1_0_RequestHeader__
