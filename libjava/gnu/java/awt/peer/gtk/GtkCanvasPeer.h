
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_peer_gtk_GtkCanvasPeer__
#define __gnu_java_awt_peer_gtk_GtkCanvasPeer__

#pragma interface

#include <gnu/java/awt/peer/gtk/GtkComponentPeer.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace awt
      {
        namespace peer
        {
          namespace gtk
          {
              class GtkCanvasPeer;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class Canvas;
        class Dimension;
    }
  }
}

class gnu::java::awt::peer::gtk::GtkCanvasPeer : public ::gnu::java::awt::peer::gtk::GtkComponentPeer
{

public: // actually package-private
  virtual void create();
public:
  GtkCanvasPeer(::java::awt::Canvas *);
  virtual ::java::awt::Dimension * preferredSize();
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_peer_gtk_GtkCanvasPeer__
