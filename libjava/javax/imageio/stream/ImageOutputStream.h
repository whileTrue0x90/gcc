
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_imageio_stream_ImageOutputStream__
#define __javax_imageio_stream_ImageOutputStream__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace nio
    {
        class ByteOrder;
    }
  }
  namespace javax
  {
    namespace imageio
    {
      namespace stream
      {
          class IIOByteBuffer;
          class ImageOutputStream;
      }
    }
  }
}

class javax::imageio::stream::ImageOutputStream : public ::java::lang::Object
{

public:
  virtual void flushBefore(jlong) = 0;
  virtual void write(JArray< jbyte > *) = 0;
  virtual void write(JArray< jbyte > *, jint, jint) = 0;
  virtual void write(jint) = 0;
  virtual void writeBit(jint) = 0;
  virtual void writeBits(jlong, jint) = 0;
  virtual void writeBoolean(jboolean) = 0;
  virtual void writeByte(jint) = 0;
  virtual void writeBytes(::java::lang::String *) = 0;
  virtual void writeChar(jint) = 0;
  virtual void writeChars(JArray< jchar > *, jint, jint) = 0;
  virtual void writeChars(::java::lang::String *) = 0;
  virtual void writeDouble(jdouble) = 0;
  virtual void writeDoubles(JArray< jdouble > *, jint, jint) = 0;
  virtual void writeFloat(jfloat) = 0;
  virtual void writeFloats(JArray< jfloat > *, jint, jint) = 0;
  virtual void writeInt(jint) = 0;
  virtual void writeInts(JArray< jint > *, jint, jint) = 0;
  virtual void writeLong(jlong) = 0;
  virtual void writeLongs(JArray< jlong > *, jint, jint) = 0;
  virtual void writeShort(jint) = 0;
  virtual void writeShorts(JArray< jshort > *, jint, jint) = 0;
  virtual void writeUTF(::java::lang::String *) = 0;
  virtual void setByteOrder(::java::nio::ByteOrder *) = 0;
  virtual ::java::nio::ByteOrder * getByteOrder() = 0;
  virtual jint read() = 0;
  virtual jint read(JArray< jbyte > *) = 0;
  virtual jint read(JArray< jbyte > *, jint, jint) = 0;
  virtual void readBytes(::javax::imageio::stream::IIOByteBuffer *, jint) = 0;
  virtual jboolean readBoolean() = 0;
  virtual jbyte readByte() = 0;
  virtual jint readUnsignedByte() = 0;
  virtual jshort readShort() = 0;
  virtual jint readUnsignedShort() = 0;
  virtual jchar readChar() = 0;
  virtual jint readInt() = 0;
  virtual jlong readUnsignedInt() = 0;
  virtual jlong readLong() = 0;
  virtual jfloat readFloat() = 0;
  virtual jdouble readDouble() = 0;
  virtual ::java::lang::String * readLine() = 0;
  virtual ::java::lang::String * readUTF() = 0;
  virtual void readFully(JArray< jbyte > *, jint, jint) = 0;
  virtual void readFully(JArray< jbyte > *) = 0;
  virtual void readFully(JArray< jshort > *, jint, jint) = 0;
  virtual void readFully(JArray< jchar > *, jint, jint) = 0;
  virtual void readFully(JArray< jint > *, jint, jint) = 0;
  virtual void readFully(JArray< jlong > *, jint, jint) = 0;
  virtual void readFully(JArray< jfloat > *, jint, jint) = 0;
  virtual void readFully(JArray< jdouble > *, jint, jint) = 0;
  virtual jlong getStreamPosition() = 0;
  virtual jint getBitOffset() = 0;
  virtual void setBitOffset(jint) = 0;
  virtual jint readBit() = 0;
  virtual jlong readBits(jint) = 0;
  virtual jlong length() = 0;
  virtual jint skipBytes(jint) = 0;
  virtual jlong skipBytes(jlong) = 0;
  virtual void seek(jlong) = 0;
  virtual void mark() = 0;
  virtual void reset() = 0;
  virtual void flush() = 0;
  virtual jlong getFlushedPosition() = 0;
  virtual jboolean isCached() = 0;
  virtual jboolean isCachedMemory() = 0;
  virtual jboolean isCachedFile() = 0;
  virtual void close() = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_imageio_stream_ImageOutputStream__
