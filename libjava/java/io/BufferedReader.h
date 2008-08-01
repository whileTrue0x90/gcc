
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_io_BufferedReader__
#define __java_io_BufferedReader__

#pragma interface

#include <java/io/Reader.h>
#include <gcj/array.h>


class java::io::BufferedReader : public ::java::io::Reader
{

public:
  BufferedReader(::java::io::Reader *);
  BufferedReader(::java::io::Reader *, jint);
  virtual void close();
  virtual jboolean markSupported();
  virtual void mark(jint);
  virtual void reset();
  virtual jboolean ready();
  virtual jint read(JArray< jchar > *, jint, jint);
private:
  jint fill();
public:
  virtual jint read();
private:
  jint lineEnd(jint);
public:
  virtual ::java::lang::String * readLine();
  virtual jlong skip(jlong);
private:
  void checkStatus();
public: // actually package-private
  ::java::io::Reader * __attribute__((aligned(__alignof__( ::java::io::Reader)))) in;
  JArray< jchar > * buffer;
  jint pos;
  jint limit;
  jint markPos;
  static const jint DEFAULT_BUFFER_SIZE = 8192;
public:
  static ::java::lang::Class class$;
};

#endif // __java_io_BufferedReader__
