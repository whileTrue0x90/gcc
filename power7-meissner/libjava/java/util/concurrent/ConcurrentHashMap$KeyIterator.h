
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_concurrent_ConcurrentHashMap$KeyIterator__
#define __java_util_concurrent_ConcurrentHashMap$KeyIterator__

#pragma interface

#include <java/util/concurrent/ConcurrentHashMap$HashIterator.h>

class java::util::concurrent::ConcurrentHashMap$KeyIterator : public ::java::util::concurrent::ConcurrentHashMap$HashIterator
{

public: // actually package-private
  ConcurrentHashMap$KeyIterator(::java::util::concurrent::ConcurrentHashMap *);
public:
  ::java::lang::Object * next();
  ::java::lang::Object * nextElement();
public: // actually package-private
  ::java::util::concurrent::ConcurrentHashMap * __attribute__((aligned(__alignof__( ::java::util::concurrent::ConcurrentHashMap$HashIterator)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_concurrent_ConcurrentHashMap$KeyIterator__
