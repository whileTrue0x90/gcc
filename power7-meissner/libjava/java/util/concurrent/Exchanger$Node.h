
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_concurrent_Exchanger$Node__
#define __java_util_concurrent_Exchanger$Node__

#pragma interface

#include <java/util/concurrent/atomic/AtomicReference.h>

class java::util::concurrent::Exchanger$Node : public ::java::util::concurrent::atomic::AtomicReference
{

public:
  Exchanger$Node(::java::lang::Object *);
  ::java::lang::Object * __attribute__((aligned(__alignof__( ::java::util::concurrent::atomic::AtomicReference)))) item;
  ::java::lang::Thread * volatile waiter;
  static ::java::lang::Class class$;
};

#endif // __java_util_concurrent_Exchanger$Node__
