
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_basic_BasicInternalFrameTitlePane$IconifyAction__
#define __javax_swing_plaf_basic_BasicInternalFrameTitlePane$IconifyAction__

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
            class BasicInternalFrameTitlePane;
            class BasicInternalFrameTitlePane$IconifyAction;
        }
      }
    }
  }
}

class javax::swing::plaf::basic::BasicInternalFrameTitlePane$IconifyAction : public ::javax::swing::AbstractAction
{

public:
  BasicInternalFrameTitlePane$IconifyAction(::javax::swing::plaf::basic::BasicInternalFrameTitlePane *);
  virtual void actionPerformed(::java::awt::event::ActionEvent *);
public: // actually package-private
  ::javax::swing::plaf::basic::BasicInternalFrameTitlePane * __attribute__((aligned(__alignof__( ::javax::swing::AbstractAction)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_basic_BasicInternalFrameTitlePane$IconifyAction__
