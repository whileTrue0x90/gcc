
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_lang_reflect_VMProxy__
#define __java_lang_reflect_VMProxy__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>


class java::lang::reflect::VMProxy : public ::java::lang::Object
{

public: // actually package-private
  VMProxy();
  static ::java::lang::Class * getProxyClass(::java::lang::ClassLoader *, JArray< ::java::lang::Class * > *);
  static ::java::lang::reflect::Proxy$ProxyData * getProxyData(::java::lang::ClassLoader *, JArray< ::java::lang::Class * > *);
  static ::java::lang::Class * generateProxyClass(::java::lang::ClassLoader *, ::java::lang::reflect::Proxy$ProxyData *);
  static jboolean HAVE_NATIVE_GET_PROXY_CLASS;
  static jboolean HAVE_NATIVE_GET_PROXY_DATA;
  static jboolean HAVE_NATIVE_GENERATE_PROXY_CLASS;
public:
  static ::java::lang::Class class$;
};

#endif // __java_lang_reflect_VMProxy__
