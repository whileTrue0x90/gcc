
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_basic_BasicMenuUI$SelectMenuAction__
#define __javax_swing_plaf_basic_BasicMenuUI$SelectMenuAction__

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
            class BasicMenuUI;
            class BasicMenuUI$SelectMenuAction;
        }
      }
    }
  }
}

class javax::swing::plaf::basic::BasicMenuUI$SelectMenuAction : public ::javax::swing::AbstractAction
{

public: // actually package-private
  BasicMenuUI$SelectMenuAction(::javax::swing::plaf::basic::BasicMenuUI *);
public:
  virtual void actionPerformed(::java::awt::event::ActionEvent *);
public: // actually package-private
  ::javax::swing::plaf::basic::BasicMenuUI * __attribute__((aligned(__alignof__( ::javax::swing::AbstractAction)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_basic_BasicMenuUI$SelectMenuAction__
