
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_net_URLClassLoader$JarURLLoader__
#define __java_net_URLClassLoader$JarURLLoader__

#pragma interface

#include <java/net/URLClassLoader$URLLoader.h>
extern "Java"
{
  namespace java
  {
    namespace net
    {
        class URL;
        class URLClassLoader;
        class URLClassLoader$JarURLLoader;
        class URLClassLoader$Resource;
    }
  }
}

class java::net::URLClassLoader$JarURLLoader : public ::java::net::URLClassLoader$URLLoader
{

public:
  URLClassLoader$JarURLLoader(::java::net::URLClassLoader *, ::java::net::URL *, ::java::net::URL *);
public: // actually package-private
  ::java::net::URLClassLoader$Resource * getResource(::java::lang::String *);
  ::java::util::jar::Manifest * getManifest();
  ::java::util::Vector * getClassPath();
  ::java::util::jar::JarFile * __attribute__((aligned(__alignof__( ::java::net::URLClassLoader$URLLoader)))) jarfile;
  ::java::net::URL * baseJarURL;
  ::java::util::Vector * classPath;
public:
  static ::java::lang::Class class$;
};

#endif // __java_net_URLClassLoader$JarURLLoader__
