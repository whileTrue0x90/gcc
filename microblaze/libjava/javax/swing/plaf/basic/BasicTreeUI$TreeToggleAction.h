
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_basic_BasicTreeUI$TreeToggleAction__
#define __javax_swing_plaf_basic_BasicTreeUI$TreeToggleAction__

#pragma interface

#include <javax/swing/AbstractAction.h>
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
      namespace plaf
      {
        namespace basic
        {
            class BasicTreeUI;
            class BasicTreeUI$TreeToggleAction;
        }
      }
    }
  }
}

class javax::swing::plaf::basic::BasicTreeUI$TreeToggleAction : public ::javax::swing::AbstractAction
{

public:
  BasicTreeUI$TreeToggleAction(::javax::swing::plaf::basic::BasicTreeUI *, ::java::lang::String *);
  virtual void actionPerformed(::java::awt::event::ActionEvent *);
  virtual jboolean isEnabled();
public: // actually package-private
  ::javax::swing::plaf::basic::BasicTreeUI * __attribute__((aligned(__alignof__( ::javax::swing::AbstractAction)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_basic_BasicTreeUI$TreeToggleAction__
