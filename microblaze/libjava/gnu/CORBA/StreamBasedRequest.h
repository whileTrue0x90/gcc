
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_StreamBasedRequest__
#define __gnu_CORBA_StreamBasedRequest__

#pragma interface

#include <gnu/CORBA/CDR/BufferedCdrOutput.h>
extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
        class StreamBasedRequest;
        class gnuRequest;
    }
  }
}

class gnu::CORBA::StreamBasedRequest : public ::gnu::CORBA::CDR::BufferedCdrOutput
{

public:
  StreamBasedRequest();
  ::gnu::CORBA::gnuRequest * __attribute__((aligned(__alignof__( ::gnu::CORBA::CDR::BufferedCdrOutput)))) request;
  jboolean response_expected;
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_StreamBasedRequest__
