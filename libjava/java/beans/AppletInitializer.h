
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_beans_AppletInitializer__
#define __java_beans_AppletInitializer__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace applet
    {
        class Applet;
    }
    namespace beans
    {
        class AppletInitializer;
      namespace beancontext
      {
          class BeanContext;
      }
    }
  }
}

class java::beans::AppletInitializer : public ::java::lang::Object
{

public:
  virtual void activate(::java::applet::Applet *) = 0;
  virtual void initialize(::java::applet::Applet *, ::java::beans::beancontext::BeanContext *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __java_beans_AppletInitializer__
