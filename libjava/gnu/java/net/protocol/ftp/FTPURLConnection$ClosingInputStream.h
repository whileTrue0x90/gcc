
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_net_protocol_ftp_FTPURLConnection$ClosingInputStream__
#define __gnu_java_net_protocol_ftp_FTPURLConnection$ClosingInputStream__

#pragma interface

#include <java/io/FilterInputStream.h>
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
              class FTPURLConnection;
              class FTPURLConnection$ClosingInputStream;
          }
        }
      }
    }
  }
}

class gnu::java::net::protocol::ftp::FTPURLConnection$ClosingInputStream : public ::java::io::FilterInputStream
{

public: // actually package-private
  FTPURLConnection$ClosingInputStream(::gnu::java::net::protocol::ftp::FTPURLConnection *, ::java::io::InputStream *);
public:
  virtual void close();
public: // actually package-private
  ::gnu::java::net::protocol::ftp::FTPURLConnection * __attribute__((aligned(__alignof__( ::java::io::FilterInputStream)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_net_protocol_ftp_FTPURLConnection$ClosingInputStream__
