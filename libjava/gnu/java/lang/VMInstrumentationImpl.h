
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_lang_VMInstrumentationImpl__
#define __gnu_java_lang_VMInstrumentationImpl__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace lang
      {
          class VMInstrumentationImpl;
      }
    }
  }
}

class gnu::java::lang::VMInstrumentationImpl : public ::java::lang::Object
{

public: // actually package-private
  VMInstrumentationImpl();
  static jboolean isRedefineClassesSupported();
  static void redefineClasses(::java::lang::instrument::Instrumentation *, JArray< ::java::lang::instrument::ClassDefinition * > *);
  static JArray< ::java::lang::Class * > * getAllLoadedClasses();
  static JArray< ::java::lang::Class * > * getInitiatedClasses(::java::lang::ClassLoader *);
  static jlong getObjectSize(::java::lang::Object *);
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_lang_VMInstrumentationImpl__
