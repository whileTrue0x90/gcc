
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_net_protocol_ftp_StreamOutputStream__
#define __gnu_java_net_protocol_ftp_StreamOutputStream__

#pragma interface

#include <gnu/java/net/protocol/ftp/DTPOutputStream.h>
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
          namespace ftp
          {
              class DTP;
              class StreamOutputStream;
          }
        }
      }
    }
  }
}

class gnu::java::net::protocol::ftp::StreamOutputStream : public ::gnu::java::net::protocol::ftp::DTPOutputStream
{

public: // actually package-private
  StreamOutputStream(::gnu::java::net::protocol::ftp::DTP *, ::java::io::OutputStream *);
public:
  virtual void write(jint);
  virtual void write(JArray< jbyte > *);
  virtual void write(JArray< jbyte > *, jint, jint);
  static ::java::lang::Class class$;
};

#endif // __gnu_java_net_protocol_ftp_StreamOutputStream__
