
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_FocusManager$WrappingFocusManager__
#define __javax_swing_FocusManager$WrappingFocusManager__

#pragma interface

#include <javax/swing/FocusManager.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class AWTEvent;
        class Component;
        class Container;
        class FocusTraversalPolicy;
        class KeyEventDispatcher;
        class KeyEventPostProcessor;
        class KeyboardFocusManager;
        class Window;
      namespace event
      {
          class KeyEvent;
      }
    }
    namespace beans
    {
        class PropertyChangeListener;
        class VetoableChangeListener;
    }
  }
  namespace javax
  {
    namespace swing
    {
        class FocusManager$WrappingFocusManager;
    }
  }
}

class javax::swing::FocusManager$WrappingFocusManager : public ::javax::swing::FocusManager
{

public: // actually package-private
  FocusManager$WrappingFocusManager(::java::awt::KeyboardFocusManager *);
public:
  virtual jboolean dispatchEvent(::java::awt::AWTEvent *);
  virtual jboolean dispatchKeyEvent(::java::awt::event::KeyEvent *);
  virtual void downFocusCycle(::java::awt::Container *);
  virtual void upFocusCycle(::java::awt::Container *);
  virtual void focusNextComponent(::java::awt::Component *);
  virtual void focusPreviousComponent(::java::awt::Component *);
  virtual jboolean postProcessKeyEvent(::java::awt::event::KeyEvent *);
  virtual void processKeyEvent(::java::awt::Component *, ::java::awt::event::KeyEvent *);
  virtual void addKeyEventDispatcher(::java::awt::KeyEventDispatcher *);
  virtual void addKeyEventPostProcessor(::java::awt::KeyEventPostProcessor *);
  virtual void addPropertyChangeListener(::java::beans::PropertyChangeListener *);
  virtual void addPropertyChangeListener(::java::lang::String *, ::java::beans::PropertyChangeListener *);
  virtual void addVetoableChangeListener(::java::lang::String *, ::java::beans::VetoableChangeListener *);
  virtual void addVetoableChangeListener(::java::beans::VetoableChangeListener *);
  virtual void clearGlobalFocusOwner();
  virtual ::java::awt::Window * getActiveWindow();
  virtual ::java::awt::Container * getCurrentFocusCycleRoot();
  virtual ::java::util::Set * getDefaultFocusTraversalKeys(jint);
  virtual ::java::awt::FocusTraversalPolicy * getDefaultFocusTraversalPolicy();
  virtual ::java::awt::Window * getFocusedWindow();
  virtual ::java::awt::Component * getFocusOwner();
  virtual ::java::awt::Component * getPermanentFocusOwner();
  virtual JArray< ::java::beans::PropertyChangeListener * > * getPropertyChangeListeners();
  virtual JArray< ::java::beans::PropertyChangeListener * > * getPropertyChangeListeners(::java::lang::String *);
  virtual JArray< ::java::beans::VetoableChangeListener * > * getVetoableChangeListeners();
  virtual JArray< ::java::beans::VetoableChangeListener * > * getVetoableChangeListeners(::java::lang::String *);
  virtual void removeKeyEventDispatcher(::java::awt::KeyEventDispatcher *);
  virtual void removeKeyEventPostProcessor(::java::awt::KeyEventPostProcessor *);
  virtual void removePropertyChangeListener(::java::beans::PropertyChangeListener *);
  virtual void removePropertyChangeListener(::java::lang::String *, ::java::beans::PropertyChangeListener *);
  virtual void removeVetoableChangeListener(::java::beans::VetoableChangeListener *);
  virtual void removeVetoableChangeListener(::java::lang::String *, ::java::beans::VetoableChangeListener *);
  virtual void setDefaultFocusTraversalKeys(jint, ::java::util::Set *);
  virtual void setDefaultFocusTraversalPolicy(::java::awt::FocusTraversalPolicy *);
  virtual void setGlobalCurrentFocusCycleRoot(::java::awt::Container *);
private:
  ::java::awt::KeyboardFocusManager * __attribute__((aligned(__alignof__( ::javax::swing::FocusManager)))) wrapped;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_FocusManager$WrappingFocusManager__
