
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_WCharHolder__
#define __gnu_CORBA_WCharHolder__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
        class WCharHolder;
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
    }
  }
}

class gnu::CORBA::WCharHolder : public ::java::lang::Object
{

public:
  WCharHolder();
  WCharHolder(jchar);
  void _read(::org::omg::CORBA::portable::InputStream *);
  ::org::omg::CORBA::TypeCode * _type();
  void _write(::org::omg::CORBA::portable::OutputStream *);
private:
  static ::org::omg::CORBA::TypeCode * t_char;
public:
  jchar __attribute__((aligned(__alignof__( ::java::lang::Object)))) value;
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_WCharHolder__
