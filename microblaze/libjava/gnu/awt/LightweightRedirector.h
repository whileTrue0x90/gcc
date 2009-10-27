
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_awt_LightweightRedirector__
#define __gnu_awt_LightweightRedirector__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace awt
    {
        class LightweightRedirector;
    }
  }
  namespace java
  {
    namespace awt
    {
        class AWTEvent;
        class Component;
      namespace event
      {
          class InputEvent;
          class MouseEvent;
      }
    }
  }
}

class gnu::awt::LightweightRedirector : public ::java::lang::Object
{

public:
  LightweightRedirector();
  virtual ::java::awt::AWTEvent * redirect(::java::awt::AWTEvent *);
public: // actually package-private
  virtual ::java::awt::event::MouseEvent * redirectMouse(::java::awt::event::MouseEvent *);
  virtual jint getButtonNumber(::java::awt::event::InputEvent *);
  static const jint LAST_BUTTON_NUMBER = 3;
  JArray< ::java::awt::Component * > * __attribute__((aligned(__alignof__( ::java::lang::Object)))) releaseTargets;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_awt_LightweightRedirector__
