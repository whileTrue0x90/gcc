
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_dnd_DragSourceDropEvent__
#define __java_awt_dnd_DragSourceDropEvent__

#pragma interface

#include <java/awt/dnd/DragSourceEvent.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace dnd
      {
          class DragSourceContext;
          class DragSourceDropEvent;
      }
    }
  }
}

class java::awt::dnd::DragSourceDropEvent : public ::java::awt::dnd::DragSourceEvent
{

public:
  DragSourceDropEvent(::java::awt::dnd::DragSourceContext *);
  DragSourceDropEvent(::java::awt::dnd::DragSourceContext *, jint, jboolean);
  DragSourceDropEvent(::java::awt::dnd::DragSourceContext *, jint, jboolean, jint, jint);
  virtual jint getDropAction();
  virtual jboolean getDropSuccess();
private:
  static const jlong serialVersionUID = -5571321229470821891LL;
  jint __attribute__((aligned(__alignof__( ::java::awt::dnd::DragSourceEvent)))) dropAction;
  jboolean dropSuccess;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_dnd_DragSourceDropEvent__
