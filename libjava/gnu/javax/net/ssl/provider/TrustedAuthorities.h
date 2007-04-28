
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_TrustedAuthorities__
#define __gnu_javax_net_ssl_provider_TrustedAuthorities__

#pragma interface

#include <gnu/javax/net/ssl/provider/Extension$Value.h>
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
          namespace provider
          {
              class TrustedAuthorities;
              class TrustedAuthorities$TrustedAuthority;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
    }
  }
}

class gnu::javax::net::ssl::provider::TrustedAuthorities : public ::gnu::javax::net::ssl::provider::Extension$Value
{

public:
  TrustedAuthorities(::java::nio::ByteBuffer *);
  virtual jint length();
  virtual ::java::nio::ByteBuffer * buffer();
  virtual jint size();
  virtual ::gnu::javax::net::ssl::provider::TrustedAuthorities$TrustedAuthority * get(jint);
  virtual ::java::lang::String * toString();
  virtual ::java::lang::String * toString(::java::lang::String *);
  virtual ::java::util::Iterator * iterator();
private:
  ::java::nio::ByteBuffer * __attribute__((aligned(__alignof__( ::gnu::javax::net::ssl::provider::Extension$Value)))) buffer__;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_TrustedAuthorities__
