
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_awt_xlib_XEventLoop__
#define __gnu_awt_xlib_XEventLoop__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace awt
    {
        class LightweightRedirector;
      namespace xlib
      {
          class XEventLoop;
      }
    }
    namespace gcj
    {
      namespace xlib
      {
          class Display;
          class XAnyEvent;
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class AWTEvent;
        class Component;
        class EventQueue;
    }
  }
}

class gnu::awt::xlib::XEventLoop : public ::java::lang::Object
{

public:
  XEventLoop(::gnu::gcj::xlib::Display *, ::java::awt::EventQueue *);
  virtual void run();
public: // actually package-private
  virtual jboolean postNextEvent(jboolean);
public:
  virtual ::java::awt::AWTEvent * getNextEvent(jboolean);
public: // actually package-private
  virtual jboolean loadNextEvent(jboolean);
  virtual ::java::awt::AWTEvent * createEvent();
  virtual ::java::awt::AWTEvent * createPaintEvent(::java::awt::Component *);
  virtual ::java::awt::AWTEvent * createMouseEvent(jint, ::java::awt::Component *);
  virtual void configureNotify(::java::lang::Object *);
public:
  virtual void flushIfIdle();
public: // actually package-private
  virtual void setIdle(jboolean);
  virtual jboolean isIdle();
  ::gnu::gcj::xlib::Display * __attribute__((aligned(__alignof__( ::java::lang::Object)))) display;
  ::java::awt::EventQueue * queue;
  ::gnu::gcj::xlib::XAnyEvent * anyEvent;
private:
  ::java::lang::Thread * eventLoopThread;
public: // actually package-private
  ::gnu::awt::LightweightRedirector * lightweightRedirector;
  volatile jboolean idle;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_awt_xlib_XEventLoop__
