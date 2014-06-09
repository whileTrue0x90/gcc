
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_io_DataOutputStream__
#define __java_io_DataOutputStream__

#pragma interface

#include <java/io/FilterOutputStream.h>
#include <gcj/array.h>


class java::io::DataOutputStream : public ::java::io::FilterOutputStream
{

public:
  DataOutputStream(::java::io::OutputStream *);
  virtual void flush();
  virtual jint size();
  virtual void write(jint);
  virtual void write(JArray< jbyte > *, jint, jint);
  virtual void writeBoolean(jboolean);
  virtual void writeByte(jint);
  virtual void writeShort(jint);
  virtual void writeChar(jint);
  virtual void writeInt(jint);
  virtual void writeLong(jlong);
  virtual void writeFloat(jfloat);
  virtual void writeDouble(jdouble);
  virtual void writeBytes(::java::lang::String *);
  virtual void writeChars(::java::lang::String *);
public: // actually package-private
  virtual jlong getUTFlength(::java::lang::String *, jint, jlong);
public:
  virtual void writeUTF(::java::lang::String *);
public: // actually package-private
  virtual void writeUTFShort(::java::lang::String *, jint);
  virtual void writeUTFLong(::java::lang::String *, jlong);
private:
  void writeUTFBytes(::java::lang::String *);
public: // actually protected
  jint __attribute__((aligned(__alignof__( ::java::io::FilterOutputStream)))) written;
private:
  JArray< jbyte > * buf;
public:
  static ::java::lang::Class class$;
};

#endif // __java_io_DataOutputStream__
