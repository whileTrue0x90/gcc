
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_concurrent_TimeUnit$7__
#define __java_util_concurrent_TimeUnit$7__

#pragma interface

#include <java/util/concurrent/TimeUnit.h>

class java::util::concurrent::TimeUnit$7 : public ::java::util::concurrent::TimeUnit
{

public: // actually package-private
  TimeUnit$7(::java::lang::String *, jint);
public:
  virtual jlong toNanos(jlong);
  virtual jlong toMicros(jlong);
  virtual jlong toMillis(jlong);
  virtual jlong toSeconds(jlong);
  virtual jlong toMinutes(jlong);
  virtual jlong toHours(jlong);
  virtual jlong toDays(jlong);
  virtual jlong convert(jlong, ::java::util::concurrent::TimeUnit *);
public: // actually package-private
  virtual jint excessNanos(jlong, jlong);
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_concurrent_TimeUnit$7__
