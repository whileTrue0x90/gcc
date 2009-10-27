
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __sun_misc_Unsafe__
#define __sun_misc_Unsafe__

#pragma interface

#include <java/lang/Object.h>
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

class sun::misc::Unsafe : public ::java::lang::Object
{

  Unsafe();
public:
  static ::sun::misc::Unsafe * getUnsafe();
  virtual jlong objectFieldOffset(::java::lang::reflect::Field *);
  virtual jboolean compareAndSwapInt(::java::lang::Object *, jlong, jint, jint);
  virtual jboolean compareAndSwapLong(::java::lang::Object *, jlong, jlong, jlong);
  virtual jboolean compareAndSwapObject(::java::lang::Object *, jlong, ::java::lang::Object *, ::java::lang::Object *);
  virtual void putOrderedInt(::java::lang::Object *, jlong, jint);
  virtual void putOrderedLong(::java::lang::Object *, jlong, jlong);
  virtual void putOrderedObject(::java::lang::Object *, jlong, ::java::lang::Object *);
  virtual void putIntVolatile(::java::lang::Object *, jlong, jint);
  virtual jint getIntVolatile(::java::lang::Object *, jlong);
  virtual void putLongVolatile(::java::lang::Object *, jlong, jlong);
  virtual void putLong(::java::lang::Object *, jlong, jlong);
  virtual jlong getLongVolatile(::java::lang::Object *, jlong);
  virtual jlong getLong(::java::lang::Object *, jlong);
  virtual void putObjectVolatile(::java::lang::Object *, jlong, ::java::lang::Object *);
  virtual void putObject(::java::lang::Object *, jlong, ::java::lang::Object *);
  virtual ::java::lang::Object * getObjectVolatile(::java::lang::Object *, jlong);
  virtual jint arrayBaseOffset(::java::lang::Class *);
  virtual jint arrayIndexScale(::java::lang::Class *);
  virtual void unpark(::java::lang::Thread *);
  virtual void park(jboolean, jlong);
private:
  static ::sun::misc::Unsafe * unsafe;
public:
  static ::java::lang::Class class$;
};

#endif // __sun_misc_Unsafe__
