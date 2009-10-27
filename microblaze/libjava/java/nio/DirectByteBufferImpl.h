
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_nio_DirectByteBufferImpl__
#define __java_nio_DirectByteBufferImpl__

#pragma interface

#include <java/nio/ByteBuffer.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace gcj
    {
        class RawData;
    }
  }
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
        class CharBuffer;
        class DirectByteBufferImpl;
        class DoubleBuffer;
        class FloatBuffer;
        class IntBuffer;
        class LongBuffer;
        class ShortBuffer;
    }
  }
}

class java::nio::DirectByteBufferImpl : public ::java::nio::ByteBuffer
{

public: // actually package-private
  DirectByteBufferImpl(jint);
  DirectByteBufferImpl(::gnu::gcj::RawData *, jint);
  DirectByteBufferImpl(::java::lang::Object *, ::gnu::gcj::RawData *, jint, jint, jint);
public:
  static ::java::nio::ByteBuffer * allocate(jint);
public: // actually protected
  virtual void finalize();
public:
  virtual jbyte get();
  virtual jbyte get(jint);
  virtual ::java::nio::ByteBuffer * get(JArray< jbyte > *, jint, jint);
  virtual ::java::nio::ByteBuffer * put(jbyte);
  virtual ::java::nio::ByteBuffer * put(jint, jbyte);
public: // actually package-private
  virtual void shiftDown(jint, jint, jint);
public:
  virtual ::java::nio::ByteBuffer * compact();
  virtual ::java::nio::ByteBuffer * slice();
private:
  ::java::nio::ByteBuffer * duplicate(jboolean);
public:
  virtual ::java::nio::ByteBuffer * duplicate();
  virtual ::java::nio::ByteBuffer * asReadOnlyBuffer();
  virtual jboolean isDirect();
  virtual ::java::nio::CharBuffer * asCharBuffer();
  virtual ::java::nio::ShortBuffer * asShortBuffer();
  virtual ::java::nio::IntBuffer * asIntBuffer();
  virtual ::java::nio::LongBuffer * asLongBuffer();
  virtual ::java::nio::FloatBuffer * asFloatBuffer();
  virtual ::java::nio::DoubleBuffer * asDoubleBuffer();
  virtual jchar getChar();
  virtual ::java::nio::ByteBuffer * putChar(jchar);
  virtual jchar getChar(jint);
  virtual ::java::nio::ByteBuffer * putChar(jint, jchar);
  virtual jshort getShort();
  virtual ::java::nio::ByteBuffer * putShort(jshort);
  virtual jshort getShort(jint);
  virtual ::java::nio::ByteBuffer * putShort(jint, jshort);
  virtual jint getInt();
  virtual ::java::nio::ByteBuffer * putInt(jint);
  virtual jint getInt(jint);
  virtual ::java::nio::ByteBuffer * putInt(jint, jint);
  virtual jlong getLong();
  virtual ::java::nio::ByteBuffer * putLong(jlong);
  virtual jlong getLong(jint);
  virtual ::java::nio::ByteBuffer * putLong(jint, jlong);
  virtual jfloat getFloat();
  virtual ::java::nio::ByteBuffer * putFloat(jfloat);
  virtual jfloat getFloat(jint);
  virtual ::java::nio::ByteBuffer * putFloat(jint, jfloat);
  virtual jdouble getDouble();
  virtual ::java::nio::ByteBuffer * putDouble(jdouble);
  virtual jdouble getDouble(jint);
  virtual ::java::nio::ByteBuffer * putDouble(jint, jdouble);
private:
  ::java::lang::Object * __attribute__((aligned(__alignof__( ::java::nio::ByteBuffer)))) owner;
public:
  static ::java::lang::Class class$;
};

#endif // __java_nio_DirectByteBufferImpl__
