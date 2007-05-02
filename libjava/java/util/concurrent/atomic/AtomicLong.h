
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_concurrent_atomic_AtomicLong__
#define __java_util_concurrent_atomic_AtomicLong__

#pragma interface

#include <java/lang/Number.h>
extern "Java"
{
  namespace sun
  {
    namespace misc
    {
        class Unsafe;
    }
  }
}

class java::util::concurrent::atomic::AtomicLong : public ::java::lang::Number
{

  static jboolean VMSupportsCS8();
public:
  AtomicLong(jlong);
  AtomicLong();
  virtual jlong get();
  virtual void set(jlong);
  virtual void lazySet(jlong);
  virtual jlong getAndSet(jlong);
  virtual jboolean compareAndSet(jlong, jlong);
  virtual jboolean weakCompareAndSet(jlong, jlong);
  virtual jlong getAndIncrement();
  virtual jlong getAndDecrement();
  virtual jlong getAndAdd(jlong);
  virtual jlong incrementAndGet();
  virtual jlong decrementAndGet();
  virtual jlong addAndGet(jlong);
  virtual ::java::lang::String * toString();
  virtual jint intValue();
  virtual jlong longValue();
  virtual jfloat floatValue();
  virtual jdouble doubleValue();
private:
  static const jlong serialVersionUID = 1927816293512124184LL;
  static ::sun::misc::Unsafe * unsafe;
  static jlong valueOffset;
public: // actually package-private
  static jboolean VM_SUPPORTS_LONG_CAS;
private:
  jlong volatile __attribute__((aligned(__alignof__( ::java::lang::Number)))) value;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_concurrent_atomic_AtomicLong__
