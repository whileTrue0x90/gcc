
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_security_DigestOutputStream__
#define __java_security_DigestOutputStream__

#pragma interface

#include <java/io/FilterOutputStream.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace security
    {
        class DigestOutputStream;
        class MessageDigest;
    }
  }
}

class java::security::DigestOutputStream : public ::java::io::FilterOutputStream
{

public:
  DigestOutputStream(::java::io::OutputStream *, ::java::security::MessageDigest *);
  virtual ::java::security::MessageDigest * getMessageDigest();
  virtual void setMessageDigest(::java::security::MessageDigest *);
  virtual void write(jint);
  virtual void write(JArray< jbyte > *, jint, jint);
  virtual void on(jboolean);
  virtual ::java::lang::String * toString();
public: // actually protected
  ::java::security::MessageDigest * __attribute__((aligned(__alignof__( ::java::io::FilterOutputStream)))) digest;
private:
  jboolean state;
public:
  static ::java::lang::Class class$;
};

#endif // __java_security_DigestOutputStream__
