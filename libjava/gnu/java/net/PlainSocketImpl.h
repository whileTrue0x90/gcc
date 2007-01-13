
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_net_PlainSocketImpl__
#define __gnu_java_net_PlainSocketImpl__

#pragma interface

#include <java/net/SocketImpl.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace net
      {
          class PlainSocketImpl;
      }
    }
  }
  namespace java
  {
    namespace net
    {
        class InetAddress;
        class SocketAddress;
        class SocketImpl;
    }
  }
}

class gnu::java::net::PlainSocketImpl : public ::java::net::SocketImpl
{

public:
  jboolean isInChannelOperation();
  void setInChannelOperation(jboolean);
  PlainSocketImpl();
public: // actually protected
  void finalize();
public:
  jint getNativeFD();
  void setOption(jint, ::java::lang::Object *);
  ::java::lang::Object * getOption(jint);
  void shutdownInput();
  void shutdownOutput();
public: // actually protected
  void create(jboolean);
  void connect(::java::lang::String *, jint);
  void connect(::java::net::InetAddress *, jint);
  void connect(::java::net::SocketAddress *, jint);
  void bind(::java::net::InetAddress *, jint);
  void listen(jint);
  void accept(::java::net::SocketImpl *);
private:
  void accept(::gnu::java::net::PlainSocketImpl *);
public: // actually protected
  jint available();
  void close();
  void sendUrgentData(jint);
  ::java::io::InputStream * getInputStream();
  ::java::io::OutputStream * getOutputStream();
public: // actually package-private
  static const jint _Jv_TCP_NODELAY_ = 1;
  static const jint _Jv_SO_BINDADDR_ = 15;
  static const jint _Jv_SO_REUSEADDR_ = 4;
  static const jint _Jv_SO_BROADCAST_ = 32;
  static const jint _Jv_SO_OOBINLINE_ = 4099;
  static const jint _Jv_IP_MULTICAST_IF_ = 16;
  static const jint _Jv_IP_MULTICAST_IF2_ = 31;
  static const jint _Jv_IP_MULTICAST_LOOP_ = 18;
  static const jint _Jv_IP_TOS_ = 3;
  static const jint _Jv_SO_LINGER_ = 128;
  static const jint _Jv_SO_TIMEOUT_ = 4102;
  static const jint _Jv_SO_SNDBUF_ = 4097;
  static const jint _Jv_SO_RCVBUF_ = 4098;
  static const jint _Jv_SO_KEEPALIVE_ = 8;
  jint __attribute__((aligned(__alignof__( ::java::net::SocketImpl)))) native_fd;
  jint timeout;
  ::java::net::InetAddress * localAddress;
private:
  ::java::io::InputStream * in;
  ::java::io::OutputStream * out;
  jboolean inChannelOperation;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_net_PlainSocketImpl__
