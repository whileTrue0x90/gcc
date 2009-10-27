
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_BigDecimalHelper__
#define __gnu_CORBA_BigDecimalHelper__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
        class BigDecimalHelper;
    }
  }
  namespace java
  {
    namespace math
    {
        class BigDecimal;
    }
  }
}

class gnu::CORBA::BigDecimalHelper : public ::java::lang::Object
{

public:
  BigDecimalHelper();
  static void main(JArray< ::java::lang::String * > *);
  static ::java::math::BigDecimal * read(::java::io::InputStream *, jint);
  static void write(::java::io::OutputStream *, ::java::math::BigDecimal *);
private:
  static ::java::math::BigDecimal * createFixed(jint, JArray< jbyte > *);
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_BigDecimalHelper__
