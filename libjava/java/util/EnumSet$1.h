
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_EnumSet$1__
#define __java_util_EnumSet$1__

#pragma interface

#include <java/util/EnumSet.h>

class java::util::EnumSet$1 : public ::java::util::EnumSet
{

public: // actually package-private
  EnumSet$1();
public:
  virtual jboolean EnumSet$1$add(::java::lang::Enum *);
  virtual jboolean addAll(::java::util::Collection *);
  virtual void clear();
  virtual jboolean contains(::java::lang::Object *);
  virtual jboolean containsAll(::java::util::Collection *);
  virtual ::java::util::Iterator * iterator();
  virtual jboolean remove(::java::lang::Object *);
  virtual jboolean removeAll(::java::util::Collection *);
  virtual jboolean retainAll(::java::util::Collection *);
  virtual jint size();
  virtual jboolean add(::java::lang::Object *);
  static ::java::lang::Class class$;
};

#endif // __java_util_EnumSet$1__
