
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_AbstractList$2__
#define __java_util_AbstractList$2__

#pragma interface

#include <java/lang/Object.h>

class java::util::AbstractList$2 : public ::java::lang::Object
{

public: // actually package-private
  AbstractList$2(::java::util::AbstractList *);
private:
  void checkMod();
public:
  virtual jboolean hasNext();
  virtual ::java::lang::Object * next();
  virtual void remove();
private:
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) pos;
  jint size;
  jint last;
  jint knownMod;
public: // actually package-private
  ::java::util::AbstractList * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_AbstractList$2__
