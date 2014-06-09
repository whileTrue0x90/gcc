
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_SSLProtocolVersion__
#define __gnu_javax_net_ssl_SSLProtocolVersion__

#pragma interface

#include <java/lang/Enum.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace net
      {
        namespace ssl
        {
            class SSLProtocolVersion;
        }
      }
    }
  }
}

class gnu::javax::net::ssl::SSLProtocolVersion : public ::java::lang::Enum
{

  SSLProtocolVersion(::java::lang::String *, jint, jint, jint);
public:
  static JArray< ::gnu::javax::net::ssl::SSLProtocolVersion * > * values();
  static ::gnu::javax::net::ssl::SSLProtocolVersion * valueOf(::java::lang::String *);
  static ::gnu::javax::net::ssl::SSLProtocolVersion * SSLv3;
  static ::gnu::javax::net::ssl::SSLProtocolVersion * TLSv1;
  jint __attribute__((aligned(__alignof__( ::java::lang::Enum)))) major;
  jint minor;
private:
  static JArray< ::gnu::javax::net::ssl::SSLProtocolVersion * > * ENUM$VALUES;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_SSLProtocolVersion__
