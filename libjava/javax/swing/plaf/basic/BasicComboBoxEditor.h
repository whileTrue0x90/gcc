
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_basic_BasicComboBoxEditor__
#define __javax_swing_plaf_basic_BasicComboBoxEditor__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Component;
      namespace event
      {
          class ActionListener;
          class FocusEvent;
      }
    }
  }
  namespace javax
  {
    namespace swing
    {
        class JTextField;
      namespace plaf
      {
        namespace basic
        {
            class BasicComboBoxEditor;
        }
      }
    }
  }
}

class javax::swing::plaf::basic::BasicComboBoxEditor : public ::java::lang::Object
{

public:
  BasicComboBoxEditor();
  virtual ::java::awt::Component * getEditorComponent();
  virtual void setItem(::java::lang::Object *);
  virtual ::java::lang::Object * getItem();
  virtual void selectAll();
  virtual void focusGained(::java::awt::event::FocusEvent *);
  virtual void focusLost(::java::awt::event::FocusEvent *);
  virtual void addActionListener(::java::awt::event::ActionListener *);
  virtual void removeActionListener(::java::awt::event::ActionListener *);
public: // actually protected
  ::javax::swing::JTextField * __attribute__((aligned(__alignof__( ::java::lang::Object)))) editor;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_basic_BasicComboBoxEditor__
