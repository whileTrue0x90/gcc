
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_basic_BasicPopupMenuUI$PopupMenuHandler__
#define __javax_swing_plaf_basic_BasicPopupMenuUI$PopupMenuHandler__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace swing
    {
      namespace event
      {
          class PopupMenuEvent;
      }
      namespace plaf
      {
        namespace basic
        {
            class BasicPopupMenuUI;
            class BasicPopupMenuUI$PopupMenuHandler;
        }
      }
    }
  }
}

class javax::swing::plaf::basic::BasicPopupMenuUI$PopupMenuHandler : public ::java::lang::Object
{

  BasicPopupMenuUI$PopupMenuHandler(::javax::swing::plaf::basic::BasicPopupMenuUI *);
public:
  virtual void popupMenuCanceled(::javax::swing::event::PopupMenuEvent *);
  virtual void popupMenuWillBecomeInvisible(::javax::swing::event::PopupMenuEvent *);
  virtual void popupMenuWillBecomeVisible(::javax::swing::event::PopupMenuEvent *);
public: // actually package-private
  BasicPopupMenuUI$PopupMenuHandler(::javax::swing::plaf::basic::BasicPopupMenuUI *, ::javax::swing::plaf::basic::BasicPopupMenuUI$PopupMenuHandler *);
  ::javax::swing::plaf::basic::BasicPopupMenuUI * __attribute__((aligned(__alignof__( ::java::lang::Object)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_basic_BasicPopupMenuUI$PopupMenuHandler__
