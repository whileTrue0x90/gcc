
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_nio_IntViewBufferImpl__
#define __java_nio_IntViewBufferImpl__

#pragma interface

#include <java/nio/IntBuffer.h>
extern "Java"
{
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
        class ByteOrder;
        class IntBuffer;
        class IntViewBufferImpl;
    }
  }
}

class java::nio::IntViewBufferImpl : public ::java::nio::IntBuffer
{

public: // actually package-private
  IntViewBufferImpl(::java::nio::ByteBuffer *, jint);
public:
  IntViewBufferImpl(::java::nio::ByteBuffer *, jint, jint, jint, jint, jint, jboolean, ::java::nio::ByteOrder *);
  jint get();
  jint get(jint);
  ::java::nio::IntBuffer * put(jint);
  ::java::nio::IntBuffer * put(jint, jint);
  ::java::nio::IntBuffer * compact();
  ::java::nio::IntBuffer * slice();
public: // actually package-private
  ::java::nio::IntBuffer * duplicate(jboolean);
public:
  ::java::nio::IntBuffer * duplicate();
  ::java::nio::IntBuffer * asReadOnlyBuffer();
  jboolean isReadOnly();
  jboolean isDirect();
  ::java::nio::ByteOrder * order();
private:
  jint __attribute__((aligned(__alignof__( ::java::nio::IntBuffer)))) offset;
  ::java::nio::ByteBuffer * bb;
  jboolean readOnly;
  ::java::nio::ByteOrder * endian;
public:
  static ::java::lang::Class class$;
};

#endif // __java_nio_IntViewBufferImpl__
