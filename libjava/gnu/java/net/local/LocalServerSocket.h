
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_net_local_LocalServerSocket__
#define __gnu_java_net_local_LocalServerSocket__

#pragma interface

#include <java/net/ServerSocket.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace net
      {
        namespace local
        {
            class LocalServerSocket;
            class LocalSocketImpl;
        }
      }
    }
  }
  namespace java
  {
    namespace net
    {
        class InetAddress;
        class Socket;
        class SocketAddress;
    }
  }
}

class gnu::java::net::local::LocalServerSocket : public ::java::net::ServerSocket
{

public:
  LocalServerSocket();
  LocalServerSocket(::java::net::SocketAddress *);
  void bind(::java::net::SocketAddress *);
  void bind(::java::net::SocketAddress *, jint);
  ::java::net::InetAddress * getInetAddress();
  jint getLocalPort();
  ::java::net::SocketAddress * getLocalSocketAddress();
  ::java::net::Socket * accept();
  void close();
  jboolean isBound();
  jboolean isClosed();
  void setSoTimeout(jint);
  jint getSoTimeout();
  void setReuseAddress(jboolean);
  jboolean getReuseAddress();
  ::java::lang::String * toString();
  void setReceiveBufferSize(jint);
  jint getReceiveBufferSize();
  void setSendBufferSize(jint);
  jint getSendBufferSize();
private:
  ::gnu::java::net::local::LocalSocketImpl * __attribute__((aligned(__alignof__( ::java::net::ServerSocket)))) myImpl;
  jboolean closed;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_net_local_LocalServerSocket__
