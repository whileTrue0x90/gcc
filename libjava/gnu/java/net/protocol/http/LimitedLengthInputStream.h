
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_net_protocol_http_LimitedLengthInputStream__
#define __gnu_java_net_protocol_http_LimitedLengthInputStream__

#pragma interface

#include <java/io/InputStream.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace net
      {
        namespace protocol
        {
          namespace http
          {
              class HTTPConnection;
              class LimitedLengthInputStream;
          }
        }
      }
    }
  }
}

class gnu::java::net::protocol::http::LimitedLengthInputStream : public ::java::io::InputStream
{

  void handleClose();
public: // actually package-private
  LimitedLengthInputStream(::java::io::InputStream *, jlong, jboolean, ::gnu::java::net::protocol::http::HTTPConnection *, jboolean);
public:
  virtual jint read();
  virtual jint read(JArray< jbyte > *);
  virtual jint read(JArray< jbyte > *, jint, jint);
  virtual jlong skip(jlong);
  virtual jint available();
  virtual void close();
private:
  jlong __attribute__((aligned(__alignof__( ::java::io::InputStream)))) remainingLen;
  jboolean restrictLen;
  ::gnu::java::net::protocol::http::HTTPConnection * connection;
  jboolean eof;
  ::java::io::InputStream * in;
  jboolean doClose;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_net_protocol_http_LimitedLengthInputStream__
