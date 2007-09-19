
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_LightweightDispatcher__
#define __java_awt_LightweightDispatcher__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class AWTEvent;
        class Component;
        class Container;
        class LightweightDispatcher;
      namespace event
      {
          class MouseEvent;
      }
    }
  }
}

class java::awt::LightweightDispatcher : public ::java::lang::Object
{

public: // actually package-private
  static ::java::awt::LightweightDispatcher * getInstance();
private:
  LightweightDispatcher();
public:
  jboolean dispatchEvent(::java::awt::AWTEvent *);
private:
  jboolean handleMouseEvent(::java::awt::event::MouseEvent *);
  ::java::awt::Component * findTarget(::java::awt::Container *, jint, jint);
  jboolean isMouseListening(::java::awt::Component *);
  void trackEnterExit(::java::awt::Component *, ::java::awt::event::MouseEvent *);
  void redispatch(::java::awt::event::MouseEvent *, ::java::awt::Component *, jint);
  jboolean isDragging(::java::awt::event::MouseEvent *);
  static ::java::util::WeakHashMap * instances;
  ::java::awt::Component * __attribute__((aligned(__alignof__( ::java::lang::Object)))) lastTarget;
  ::java::awt::Component * mouseEventTarget;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_LightweightDispatcher__
