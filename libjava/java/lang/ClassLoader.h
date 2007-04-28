
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_lang_ClassLoader__
#define __java_lang_ClassLoader__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace net
    {
        class URL;
    }
    namespace nio
    {
        class ByteBuffer;
    }
    namespace security
    {
        class ProtectionDomain;
    }
  }
}

jclass _Jv_FindClass (_Jv_Utf8Const *name, java::lang::ClassLoader *loader);
void _Jv_RunMain (jclass klass, const char *name, int argc, const char **argv, bool is_jar);

class java::lang::ClassLoader : public ::java::lang::Object
{

public: // actually package-private
  virtual JArray< ::java::lang::Object * > * getDeclaredAnnotations(::java::lang::Class *, jint, jint, jint);
  virtual JArray< ::java::lang::Object * > * putDeclaredAnnotations(::java::lang::Class *, jint, jint, jint, JArray< ::java::lang::Object * > *);
public: // actually protected
  ClassLoader();
  ClassLoader(::java::lang::ClassLoader *);
public:
  virtual ::java::lang::Class * loadClass(::java::lang::String *);
private:
  ::java::lang::Class * loadClassFromSig(::java::lang::String *);
public: // actually protected
  virtual ::java::lang::Class * loadClass(::java::lang::String *, jboolean);
  virtual ::java::lang::Class * findClass(::java::lang::String *);
  virtual ::java::lang::Class * defineClass(JArray< jbyte > *, jint, jint);
  virtual ::java::lang::Class * defineClass(::java::lang::String *, JArray< jbyte > *, jint, jint);
  virtual ::java::lang::Class * defineClass(::java::lang::String *, JArray< jbyte > *, jint, jint, ::java::security::ProtectionDomain *);
  virtual ::java::lang::Class * defineClass(::java::lang::String *, ::java::nio::ByteBuffer *, ::java::security::ProtectionDomain *);
  virtual void resolveClass(::java::lang::Class *);
  virtual ::java::lang::Class * findSystemClass(::java::lang::String *);
public:
  virtual ::java::lang::ClassLoader * getParent();
public: // actually protected
  virtual void setSigners(::java::lang::Class *, JArray< ::java::lang::Object * > *);
  virtual ::java::lang::Class * findLoadedClass(::java::lang::String *);
public:
  virtual ::java::net::URL * getResource(::java::lang::String *);
  virtual ::java::util::Enumeration * getResources(::java::lang::String *);
public: // actually protected
  virtual ::java::util::Enumeration * findResources(::java::lang::String *);
  virtual ::java::net::URL * findResource(::java::lang::String *);
public:
  static ::java::net::URL * getSystemResource(::java::lang::String *);
  static ::java::util::Enumeration * getSystemResources(::java::lang::String *);
  virtual ::java::io::InputStream * getResourceAsStream(::java::lang::String *);
  static ::java::io::InputStream * getSystemResourceAsStream(::java::lang::String *);
  static ::java::lang::ClassLoader * getSystemClassLoader();
public: // actually protected
  virtual ::java::lang::Package * definePackage(::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::net::URL *);
  virtual ::java::lang::Package * getPackage(::java::lang::String *);
  virtual JArray< ::java::lang::Package * > * getPackages();
  virtual ::java::lang::String * findLibrary(::java::lang::String *);
public:
  virtual void setDefaultAssertionStatus(jboolean);
  virtual void setPackageAssertionStatus(::java::lang::String *, jboolean);
  virtual void setClassAssertionStatus(::java::lang::String *, jboolean);
  virtual void clearAssertionStatus();
public: // actually package-private
  virtual jboolean isAncestorOf(::java::lang::ClassLoader *);
private:
  void checkInitialized();
public: // actually package-private
  ::java::util::HashMap * __attribute__((aligned(__alignof__( ::java::lang::Object)))) loadedClasses;
  ::java::util::HashMap * definedPackages;
private:
  ::java::lang::ClassLoader * parent;
  jboolean initialized;
public: // actually package-private
  static ::java::lang::ClassLoader * systemClassLoader;
private:
  ::java::util::concurrent::ConcurrentHashMap * declaredAnnotations;
public: // actually package-private
  static ::java::security::ProtectionDomain * defaultProtectionDomain;
  jboolean defaultAssertionStatus;
  static ::java::util::Map * systemPackageAssertionStatus;
  ::java::util::Map * packageAssertionStatus;
  static ::java::util::Map * systemClassAssertionStatus;
  ::java::util::Map * classAssertionStatus;
public:
  static ::java::lang::Class class$;

  friend jclass (::_Jv_FindClass) (_Jv_Utf8Const *name, java::lang::ClassLoader *loader);
  friend void ::_Jv_RunMain (jclass klass, const char *name, int argc, const char **argv, bool is_jar);
};

#endif // __java_lang_ClassLoader__
