
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_EventQueue__
#define __java_awt_EventQueue__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class AWTEvent;
        class EventDispatchThread;
        class EventQueue;
        class EventQueue$Queue;
    }
  }
}

class java::awt::EventQueue : public ::java::lang::Object
{

  jboolean isShutdown();
public:
  EventQueue();
  virtual ::java::awt::AWTEvent * getNextEvent();
private:
  ::java::awt::AWTEvent * getNextEventImpl(jboolean);
public:
  virtual ::java::awt::AWTEvent * peekEvent();
  virtual ::java::awt::AWTEvent * peekEvent(jint);
  virtual void postEvent(::java::awt::AWTEvent *);
private:
  void postEventImpl(::java::awt::AWTEvent *);
  void postEventImpl(::java::awt::AWTEvent *, jint);
public:
  static void invokeAndWait(::java::lang::Runnable *);
  static void invokeLater(::java::lang::Runnable *);
  static jboolean isDispatchThread();
  static ::java::awt::AWTEvent * getCurrentEvent();
  virtual void push(::java::awt::EventQueue *);
public: // actually protected
  virtual void pop();
  virtual void dispatchEvent(::java::awt::AWTEvent *);
public:
  static jlong getMostRecentEventTime();
private:
  static const jint NORM_PRIORITY = 0;
  static const jint LOW_PRIORITY = 1;
  JArray< ::java::awt::EventQueue$Queue * > * __attribute__((aligned(__alignof__( ::java::lang::Object)))) queues;
  ::java::awt::EventQueue * next;
  ::java::awt::EventQueue * prev;
  ::java::awt::AWTEvent * currentEvent;
  jlong lastWhen;
  ::java::awt::EventDispatchThread * dispatchThread;
  jboolean nativeLoopRunning;
public: // actually package-private
  static jboolean $assertionsDisabled;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_EventQueue__
