
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_lang_EnumConstantNotPresentException__
#define __java_lang_EnumConstantNotPresentException__

#pragma interface

#include <java/lang/RuntimeException.h>

class java::lang::EnumConstantNotPresentException : public ::java::lang::RuntimeException
{

public:
  EnumConstantNotPresentException(::java::lang::Class *, ::java::lang::String *);
  virtual ::java::lang::String * constantName();
  virtual ::java::lang::Class * enumType();
private:
  static const jlong serialVersionUID = -6046998521960521108LL;
  ::java::lang::Class * __attribute__((aligned(__alignof__( ::java::lang::RuntimeException)))) enumType__;
  ::java::lang::String * constantName__;
public:
  static ::java::lang::Class class$;
};

#endif // __java_lang_EnumConstantNotPresentException__
