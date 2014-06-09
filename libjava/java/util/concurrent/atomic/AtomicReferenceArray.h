
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_concurrent_atomic_AtomicReferenceArray__
#define __java_util_concurrent_atomic_AtomicReferenceArray__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

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

class java::util::concurrent::atomic::AtomicReferenceArray : public ::java::lang::Object
{

  jlong rawIndex(jint);
public:
  AtomicReferenceArray(jint);
  AtomicReferenceArray(JArray< ::java::lang::Object * > *);
  virtual jint length();
  virtual ::java::lang::Object * get(jint);
  virtual void set(jint, ::java::lang::Object *);
  virtual void lazySet(jint, ::java::lang::Object *);
  virtual ::java::lang::Object * getAndSet(jint, ::java::lang::Object *);
  virtual jboolean compareAndSet(jint, ::java::lang::Object *, ::java::lang::Object *);
  virtual jboolean weakCompareAndSet(jint, ::java::lang::Object *, ::java::lang::Object *);
  virtual ::java::lang::String * toString();
private:
  static const jlong serialVersionUID = -6209656149925076980LL;
  static ::sun::misc::Unsafe * unsafe;
  static jint base;
  static jint scale;
  JArray< ::java::lang::Object * > * __attribute__((aligned(__alignof__( ::java::lang::Object)))) array;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_concurrent_atomic_AtomicReferenceArray__
