
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_event_ContainerListener__
#define __java_awt_event_ContainerListener__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace event
      {
          class ContainerEvent;
          class ContainerListener;
      }
    }
  }
}

class java::awt::event::ContainerListener : public ::java::lang::Object
{

public:
  virtual void componentAdded(::java::awt::event::ContainerEvent *) = 0;
  virtual void componentRemoved(::java::awt::event::ContainerEvent *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __java_awt_event_ContainerListener__
