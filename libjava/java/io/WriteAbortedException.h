
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_io_WriteAbortedException__
#define __java_io_WriteAbortedException__

#pragma interface

#include <java/io/ObjectStreamException.h>

class java::io::WriteAbortedException : public ::java::io::ObjectStreamException
{

public:
  WriteAbortedException(::java::lang::String *, ::java::lang::Exception *);
  virtual ::java::lang::String * getMessage();
  virtual ::java::lang::Throwable * getCause();
private:
  static const jlong serialVersionUID = -3326426625597282442LL;
public:
  ::java::lang::Exception * __attribute__((aligned(__alignof__( ::java::io::ObjectStreamException)))) detail;
  static ::java::lang::Class class$;
};

#endif // __java_io_WriteAbortedException__
