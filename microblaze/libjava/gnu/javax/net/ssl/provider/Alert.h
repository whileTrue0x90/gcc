
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_net_ssl_provider_Alert__
#define __gnu_javax_net_ssl_provider_Alert__

#pragma interface

#include <java/lang/Object.h>
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
          namespace provider
          {
              class Alert;
              class Alert$Description;
              class Alert$Level;
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

class gnu::javax::net::ssl::provider::Alert : public ::java::lang::Object
{

public:
  Alert(::java::nio::ByteBuffer *);
  Alert(::gnu::javax::net::ssl::provider::Alert$Level *, ::gnu::javax::net::ssl::provider::Alert$Description *);
  jint length();
public: // actually package-private
  JArray< jbyte > * getEncoded();
public:
  ::gnu::javax::net::ssl::provider::Alert$Level * level();
  ::gnu::javax::net::ssl::provider::Alert$Description * description();
  void setLevel(::gnu::javax::net::ssl::provider::Alert$Level *);
  void setDescription(::gnu::javax::net::ssl::provider::Alert$Description *);
  jboolean equals(::java::lang::Object *);
  jint hashCode();
  ::java::lang::String * toString();
  ::java::lang::String * toString(::java::lang::String *);
private:
  ::java::nio::ByteBuffer * __attribute__((aligned(__alignof__( ::java::lang::Object)))) buffer;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_net_ssl_provider_Alert__
