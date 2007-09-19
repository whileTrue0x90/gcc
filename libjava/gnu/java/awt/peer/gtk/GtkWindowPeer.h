
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_peer_gtk_GtkWindowPeer__
#define __gnu_java_awt_peer_gtk_GtkWindowPeer__

#pragma interface

#include <gnu/java/awt/peer/gtk/GtkContainerPeer.h>
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
              class GtkWindowPeer;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class Component;
        class Dialog;
        class Graphics;
        class Rectangle;
        class Window;
    }
  }
}

class gnu::java::awt::peer::gtk::GtkWindowPeer : public ::gnu::java::awt::peer::gtk::GtkContainerPeer
{

public: // actually package-private
  virtual void gtkWindowSetTitle(::java::lang::String *);
  virtual void gtkWindowSetResizable(jboolean);
  virtual void gtkWindowSetModal(jboolean);
  virtual void gtkWindowSetAlwaysOnTop(jboolean);
  virtual jboolean gtkWindowHasFocus();
  virtual void realize();
public:
  virtual void dispose();
public: // actually package-private
  virtual jint getX();
  virtual jint getY();
  virtual jint getWidth();
  virtual jint getHeight();
  virtual void create(jint, jboolean, ::gnu::java::awt::peer::gtk::GtkWindowPeer *);
  virtual void create(jint, jboolean);
  virtual void create();
  virtual void setParent();
  virtual void setVisibleAndEnabled();
public:
  virtual void setVisibleNative(jboolean);
  virtual void setVisibleNativeUnlocked(jboolean);
public: // actually package-private
  virtual void connectSignals();
public:
  GtkWindowPeer(::java::awt::Window *);
  virtual void toBack();
  virtual void toFront();
public: // actually package-private
  virtual void nativeSetBounds(jint, jint, jint, jint);
  virtual void nativeSetBoundsUnlocked(jint, jint, jint, jint);
  virtual void nativeSetLocation(jint, jint);
  virtual void nativeSetLocationUnlocked(jint, jint);
public: // actually protected
  virtual void setLocation(jint, jint);
public:
  virtual void setBounds(jint, jint, jint, jint);
  virtual void setTitle(::java::lang::String *);
public: // actually protected
  virtual void setSize(jint, jint);
public:
  virtual void setResizable(jboolean);
public: // actually protected
  virtual void postInsetsChangedEvent(jint, jint, jint, jint);
  virtual void postConfigureEvent(jint, jint, jint, jint);
public:
  virtual void show();
public: // actually package-private
  virtual void postWindowEvent(jint, ::java::awt::Window *, jint);
public:
  virtual void updateAlwaysOnTop();
public: // actually protected
  virtual void postExposeEvent(jint, jint, jint, jint);
public:
  virtual jboolean requestWindowFocus();
  virtual jboolean requestFocus(::java::awt::Component *, jboolean, jboolean, jlong);
  virtual ::java::awt::Graphics * getGraphics();
public: // actually protected
  virtual void postMouseEvent(jint, jlong, jint, jint, jint, jint, jboolean);
public:
  virtual ::java::awt::Rectangle * getBounds();
  virtual void updateIconImages();
  virtual void updateMinimumSize();
  virtual void setModalBlocked(::java::awt::Dialog *, jboolean);
  virtual void updateFocusableWindowState();
  virtual void setAlwaysOnTop(jboolean);
public: // actually protected
  static const jint GDK_WINDOW_TYPE_HINT_NORMAL = 0;
  static const jint GDK_WINDOW_TYPE_HINT_DIALOG = 1;
  static const jint GDK_WINDOW_TYPE_HINT_MENU = 2;
  static const jint GDK_WINDOW_TYPE_HINT_TOOLBAR = 3;
  static const jint GDK_WINDOW_TYPE_HINT_SPLASHSCREEN = 4;
  static const jint GDK_WINDOW_TYPE_HINT_UTILITY = 5;
  static const jint GDK_WINDOW_TYPE_HINT_DOCK = 6;
  static const jint GDK_WINDOW_TYPE_HINT_DESKTOP = 7;
  jint __attribute__((aligned(__alignof__( ::gnu::java::awt::peer::gtk::GtkContainerPeer)))) windowState;
private:
  jint x;
  jint y;
  jint width;
  jint height;
public: // actually package-private
  static jboolean $assertionsDisabled;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_peer_gtk_GtkWindowPeer__
