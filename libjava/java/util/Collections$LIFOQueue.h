
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_Collections$LIFOQueue__
#define __java_util_Collections$LIFOQueue__

#pragma interface

#include <java/util/AbstractQueue.h>

class java::util::Collections$LIFOQueue : public ::java::util::AbstractQueue
{

public:
  Collections$LIFOQueue(::java::util::Deque *);
  virtual jboolean add(::java::lang::Object *);
  virtual jboolean addAll(::java::util::Collection *);
  virtual void clear();
  virtual jboolean isEmpty();
  virtual ::java::util::Iterator * iterator();
  virtual jboolean offer(::java::lang::Object *);
  virtual ::java::lang::Object * peek();
  virtual ::java::lang::Object * poll();
  virtual jint size();
private:
  ::java::util::Deque * __attribute__((aligned(__alignof__( ::java::util::AbstractQueue)))) deque;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_Collections$LIFOQueue__
