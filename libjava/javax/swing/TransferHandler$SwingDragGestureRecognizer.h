
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_TransferHandler$SwingDragGestureRecognizer__
#define __javax_swing_TransferHandler$SwingDragGestureRecognizer__

#pragma interface

#include <java/awt/dnd/DragGestureRecognizer.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace dnd
      {
          class DragGestureListener;
      }
      namespace event
      {
          class MouseEvent;
      }
    }
  }
  namespace javax
  {
    namespace swing
    {
        class JComponent;
        class TransferHandler$SwingDragGestureRecognizer;
    }
  }
}

class javax::swing::TransferHandler$SwingDragGestureRecognizer : public ::java::awt::dnd::DragGestureRecognizer
{

public: // actually protected
  TransferHandler$SwingDragGestureRecognizer(::java::awt::dnd::DragGestureListener *);
public: // actually package-private
  virtual void gesture(::javax::swing::JComponent *, ::java::awt::event::MouseEvent *, jint, jint);
public: // actually protected
  virtual void registerListeners();
  virtual void unregisterListeners();
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_TransferHandler$SwingDragGestureRecognizer__
