
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_TextAction__
#define __javax_swing_text_TextAction__

#pragma interface

#include <javax/swing/AbstractAction.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace event
      {
          class ActionEvent;
      }
    }
  }
  namespace javax
  {
    namespace swing
    {
        class Action;
      namespace text
      {
          class JTextComponent;
          class TextAction;
      }
    }
  }
}

class javax::swing::text::TextAction : public ::javax::swing::AbstractAction
{

public:
  TextAction(::java::lang::String *);
public: // actually protected
  virtual ::javax::swing::text::JTextComponent * getTextComponent(::java::awt::event::ActionEvent *);
public:
  static JArray< ::javax::swing::Action * > * augmentList(JArray< ::javax::swing::Action * > *, JArray< ::javax::swing::Action * > *);
public: // actually protected
  virtual ::javax::swing::text::JTextComponent * getFocusedComponent();
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_TextAction__
