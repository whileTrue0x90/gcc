
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_classpath_debug_TeeOutputStream__
#define __gnu_classpath_debug_TeeOutputStream__

#pragma interface

#include <java/io/OutputStream.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace classpath
    {
      namespace debug
      {
          class TeeOutputStream;
      }
    }
  }
}

class gnu::classpath::debug::TeeOutputStream : public ::java::io::OutputStream
{

public:
  TeeOutputStream(::java::io::OutputStream *, ::java::io::OutputStream *);
  virtual void write(jint);
  virtual void write(JArray< jbyte > *, jint, jint);
  virtual void flush();
  virtual void close();
private:
  ::java::io::OutputStream * __attribute__((aligned(__alignof__( ::java::io::OutputStream)))) out;
  ::java::io::OutputStream * sink;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_classpath_debug_TeeOutputStream__
