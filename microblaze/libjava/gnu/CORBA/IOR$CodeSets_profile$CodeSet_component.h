
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_IOR$CodeSets_profile$CodeSet_component__
#define __gnu_CORBA_IOR$CodeSets_profile$CodeSet_component__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
        class IOR$CodeSets_profile$CodeSet_component;
    }
  }
  namespace org
  {
    namespace omg
    {
      namespace CORBA
      {
        namespace portable
        {
            class InputStream;
            class OutputStream;
        }
      }
    }
  }
}

class gnu::CORBA::IOR$CodeSets_profile$CodeSet_component : public ::java::lang::Object
{

public:
  IOR$CodeSets_profile$CodeSet_component();
  virtual void read(::org::omg::CORBA::portable::InputStream *);
  virtual ::java::lang::String * toString();
  virtual ::java::lang::String * toStringFormatted();
  virtual void write(::org::omg::CORBA::portable::OutputStream *);
private:
  ::java::lang::String * name(jint);
public:
  JArray< jint > * __attribute__((aligned(__alignof__( ::java::lang::Object)))) conversion;
  jint native_set;
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_IOR$CodeSets_profile$CodeSet_component__
