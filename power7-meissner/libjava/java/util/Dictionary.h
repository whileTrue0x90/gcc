
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_Dictionary__
#define __java_util_Dictionary__

#pragma interface

#include <java/lang/Object.h>

class java::util::Dictionary : public ::java::lang::Object
{

public:
  Dictionary();
  virtual ::java::util::Enumeration * elements() = 0;
  virtual ::java::lang::Object * get(::java::lang::Object *) = 0;
  virtual jboolean isEmpty() = 0;
  virtual ::java::util::Enumeration * keys() = 0;
  virtual ::java::lang::Object * put(::java::lang::Object *, ::java::lang::Object *) = 0;
  virtual ::java::lang::Object * remove(::java::lang::Object *) = 0;
  virtual jint size() = 0;
  static ::java::lang::Class class$;
};

#endif // __java_util_Dictionary__
