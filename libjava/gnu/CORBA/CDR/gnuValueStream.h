
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_CDR_gnuValueStream__
#define __gnu_CORBA_CDR_gnuValueStream__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
      namespace CDR
      {
          class gnuRuntime;
          class gnuValueStream;
      }
    }
  }
}

class gnu::CORBA::CDR::gnuValueStream : public ::java::lang::Object
{

public:
  virtual jint getPosition() = 0;
  virtual void seek(jint) = 0;
  virtual ::gnu::CORBA::CDR::gnuRuntime * getRunTime() = 0;
  virtual void setRunTime(::gnu::CORBA::CDR::gnuRuntime *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __gnu_CORBA_CDR_gnuValueStream__
