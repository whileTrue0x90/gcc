
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_ToolTipManager$stillInsideTimerAction__
#define __javax_swing_ToolTipManager$stillInsideTimerAction__

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
          class ActionEvent;
      }
    }
  }
  namespace javax
  {
    namespace swing
    {
        class ToolTipManager;
        class ToolTipManager$stillInsideTimerAction;
    }
  }
}

class javax::swing::ToolTipManager$stillInsideTimerAction : public ::java::lang::Object
{

public: // actually protected
  ToolTipManager$stillInsideTimerAction(::javax::swing::ToolTipManager *);
public:
  virtual void actionPerformed(::java::awt::event::ActionEvent *);
public: // actually package-private
  ::javax::swing::ToolTipManager * __attribute__((aligned(__alignof__( ::java::lang::Object)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_ToolTipManager$stillInsideTimerAction__
