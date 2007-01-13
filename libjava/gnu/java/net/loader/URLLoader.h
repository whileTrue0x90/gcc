
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_net_loader_URLLoader__
#define __gnu_java_net_loader_URLLoader__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace net
      {
        namespace loader
        {
            class Resource;
            class URLLoader;
            class URLStreamHandlerCache;
        }
      }
    }
  }
  namespace java
  {
    namespace net
    {
        class URL;
        class URLClassLoader;
        class URLStreamHandlerFactory;
    }
    namespace security
    {
        class CodeSource;
    }
  }
}

class gnu::java::net::loader::URLLoader : public ::java::lang::Object
{

public:
  URLLoader(::java::net::URLClassLoader *, ::gnu::java::net::loader::URLStreamHandlerCache *, ::java::net::URLStreamHandlerFactory *, ::java::net::URL *);
  URLLoader(::java::net::URLClassLoader *, ::gnu::java::net::loader::URLStreamHandlerCache *, ::java::net::URLStreamHandlerFactory *, ::java::net::URL *, ::java::net::URL *);
  virtual ::java::net::URL * getBaseURL();
  virtual ::java::lang::Class * getClass(::java::lang::String *);
  virtual ::gnu::java::net::loader::Resource * getResource(::java::lang::String *) = 0;
  virtual ::java::util::jar::Manifest * getManifest();
  virtual ::java::util::ArrayList * getClassPath();
public: // actually package-private
  ::java::net::URLClassLoader * __attribute__((aligned(__alignof__( ::java::lang::Object)))) classloader;
  ::java::net::URL * baseURL;
  ::java::net::URLStreamHandlerFactory * factory;
  ::gnu::java::net::loader::URLStreamHandlerCache * cache;
  ::java::security::CodeSource * noCertCodeSource;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_net_loader_URLLoader__
