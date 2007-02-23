
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_gcj_jvmti_ExceptionEvent__
#define __gnu_gcj_jvmti_ExceptionEvent__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace gcj
    {
      namespace jvmti
      {
          class ExceptionEvent;
      }
    }
  }
}

class gnu::gcj::jvmti::ExceptionEvent : public ::java::lang::Object
{

  ExceptionEvent(::java::lang::Thread *, jlong, jlong, ::java::lang::Throwable *, jlong, jlong);
public:
  static void postExceptionEvent(::java::lang::Thread *, jlong, jlong, ::java::lang::Throwable *, jlong, jlong);
  virtual void sendEvent();
  virtual void checkCatch();
private:
  jlong __attribute__((aligned(__alignof__( ::java::lang::Object)))) _throwMeth;
  jlong _throwLoc;
  jlong _catchMeth;
  jlong _catchLoc;
  ::java::lang::Thread * _thread;
  ::java::lang::Throwable * _ex;
  static ::java::util::WeakHashMap * _exMap;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_gcj_jvmti_ExceptionEvent__
