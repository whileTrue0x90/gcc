
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_sql_SQLException__
#define __java_sql_SQLException__

#pragma interface

#include <java/lang/Exception.h>
extern "Java"
{
  namespace java
  {
    namespace sql
    {
        class SQLException;
    }
  }
}

class java::sql::SQLException : public ::java::lang::Exception
{

public:
  SQLException(::java::lang::String *, ::java::lang::String *, jint);
  SQLException(::java::lang::String *, ::java::lang::String *);
  SQLException(::java::lang::String *);
  SQLException();
  virtual ::java::lang::String * getSQLState();
  virtual jint getErrorCode();
  virtual ::java::sql::SQLException * getNextException();
  virtual void setNextException(::java::sql::SQLException *);
public: // actually package-private
  static const jlong serialVersionUID = 2135244094396331484LL;
private:
  ::java::sql::SQLException * __attribute__((aligned(__alignof__( ::java::lang::Exception)))) next;
  ::java::lang::String * SQLState;
  jint vendorCode;
public:
  static ::java::lang::Class class$;
};

#endif // __java_sql_SQLException__
