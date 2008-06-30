
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_io_DataInputStream__
#define __java_io_DataInputStream__

#pragma interface

#include <java/io/FilterInputStream.h>
#include <gcj/array.h>


class java::io::DataInputStream : public ::java::io::FilterInputStream
{

public:
  DataInputStream(::java::io::InputStream *);
  virtual jint read(JArray< jbyte > *);
  virtual jint read(JArray< jbyte > *, jint, jint);
  virtual jboolean readBoolean();
  virtual jbyte readByte();
  virtual jchar readChar();
  virtual jdouble readDouble();
  virtual jfloat readFloat();
  virtual void readFully(JArray< jbyte > *);
  virtual void readFully(JArray< jbyte > *, jint, jint);
  virtual jint readInt();
  virtual ::java::lang::String * readLine();
  virtual jlong readLong();
  virtual jshort readShort();
  virtual jint readUnsignedByte();
  virtual jint readUnsignedShort();
  virtual ::java::lang::String * readUTF();
  static ::java::lang::String * readUTF(::java::io::DataInput *);
public: // actually package-private
  virtual ::java::lang::String * readUTFLong();
private:
  static ::java::lang::String * readUTF(::java::io::DataInput *, jint);
public:
  virtual jint skipBytes(jint);
public: // actually package-private
  static jboolean convertToBoolean(jint);
  static jbyte convertToByte(jint);
  static jint convertToUnsignedByte(jint);
  static jchar convertToChar(JArray< jbyte > *);
  static jshort convertToShort(JArray< jbyte > *);
  static jint convertToUnsignedShort(JArray< jbyte > *);
  static jint convertToInt(JArray< jbyte > *);
  static jlong convertToLong(JArray< jbyte > *);
  static ::java::lang::String * convertFromUTF(JArray< jbyte > *);
  JArray< jbyte > * __attribute__((aligned(__alignof__( ::java::io::FilterInputStream)))) buf;
public:
  static ::java::lang::Class class$;
};

#endif // __java_io_DataInputStream__
