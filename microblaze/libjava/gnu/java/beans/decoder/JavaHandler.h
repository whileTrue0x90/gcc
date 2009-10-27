
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_beans_decoder_JavaHandler__
#define __gnu_java_beans_decoder_JavaHandler__

#pragma interface

#include <gnu/java/beans/decoder/AbstractElementHandler.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace beans
      {
        namespace decoder
        {
            class Context;
            class DummyHandler;
            class JavaHandler;
        }
      }
    }
  }
  namespace java
  {
    namespace beans
    {
        class ExceptionListener;
    }
  }
  namespace org
  {
    namespace xml
    {
      namespace sax
      {
          class Attributes;
      }
    }
  }
}

class gnu::java::beans::decoder::JavaHandler : public ::gnu::java::beans::decoder::AbstractElementHandler
{

public: // actually package-private
  JavaHandler(::gnu::java::beans::decoder::DummyHandler *, ::gnu::java::beans::decoder::Context *, ::java::lang::ClassLoader *);
public: // actually protected
  virtual ::gnu::java::beans::decoder::Context * startElement(::org::xml::sax::Attributes *, ::java::beans::ExceptionListener *);
public:
  virtual ::java::lang::Object * getObject(::java::lang::String *);
  virtual void putObject(::java::lang::String *, ::java::lang::Object *);
  virtual ::java::lang::Class * instantiateClass(::java::lang::String *);
private:
  ::gnu::java::beans::decoder::Context * __attribute__((aligned(__alignof__( ::gnu::java::beans::decoder::AbstractElementHandler)))) context;
  ::java::util::HashMap * objectMap;
  ::java::lang::ClassLoader * classLoader;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_beans_decoder_JavaHandler__
