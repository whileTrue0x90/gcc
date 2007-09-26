
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_awt_xlib_XCanvasPeer__
#define __gnu_awt_xlib_XCanvasPeer__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace awt
    {
      namespace xlib
      {
          class XCanvasPeer;
          class XGraphicsConfiguration;
          class XToolkit;
      }
    }
    namespace gcj
    {
      namespace xlib
      {
          class Window;
          class WindowAttributes;
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class AWTEvent;
        class BufferCapabilities;
        class BufferCapabilities$FlipContents;
        class Color;
        class Component;
        class Cursor;
        class Dimension;
        class Font;
        class FontMetrics;
        class Graphics;
        class GraphicsConfiguration;
        class Image;
        class Point;
        class Rectangle;
        class Toolkit;
      namespace event
      {
          class PaintEvent;
      }
      namespace image
      {
          class ColorModel;
          class ImageObserver;
          class ImageProducer;
          class VolatileImage;
      }
      namespace peer
      {
          class ContainerPeer;
      }
    }
  }
  namespace sun
  {
    namespace awt
    {
        class CausedFocusEvent$Cause;
    }
  }
}

class gnu::awt::xlib::XCanvasPeer : public ::java::lang::Object
{

public:
  XCanvasPeer(::java::awt::Component *);
public: // actually package-private
  virtual ::gnu::gcj::xlib::Window * locateParentWindow(::java::awt::Rectangle *);
  virtual void initWindowProperties();
  virtual ::gnu::awt::xlib::XToolkit * getXToolkit();
public: // actually protected
  virtual void ensureFlush();
public:
  virtual ::java::awt::Component * getComponent();
public: // actually package-private
  virtual jlong getBasicEventMask();
public:
  virtual jint checkImage(::java::awt::Image *, jint, jint, ::java::awt::image::ImageObserver *);
  virtual ::java::awt::Image * createImage(::java::awt::image::ImageProducer *);
  virtual ::java::awt::Image * createImage(jint, jint);
  virtual void dispose();
  virtual ::java::awt::GraphicsConfiguration * getGraphicsConfiguration();
  virtual ::java::awt::FontMetrics * getFontMetrics(::java::awt::Font *);
  virtual ::java::awt::image::ColorModel * getColorModel();
  virtual ::java::awt::Graphics * getGraphics();
  virtual ::java::awt::Point * getLocationOnScreen();
  virtual ::java::awt::Dimension * getMinimumSize();
  virtual ::java::awt::Dimension * minimumSize();
  virtual ::java::awt::Dimension * getPreferredSize();
  virtual ::java::awt::Dimension * preferredSize();
  virtual ::java::awt::Toolkit * getToolkit();
  virtual void handleEvent(::java::awt::AWTEvent *);
  virtual jboolean isFocusTraversable();
  virtual void paint(::java::awt::Graphics *);
  virtual jboolean prepareImage(::java::awt::Image *, jint, jint, ::java::awt::image::ImageObserver *);
  virtual void print(::java::awt::Graphics *);
  virtual void repaint(jlong, jint, jint, jint, jint);
  virtual void requestFocus();
  virtual void setBackground(::java::awt::Color *);
  virtual void setBounds(jint, jint, jint, jint);
  virtual void reshape(jint, jint, jint, jint);
  virtual void setCursor(::java::awt::Cursor *);
  virtual void setEnabled(jboolean);
  virtual void enable();
  virtual void disable();
  virtual void setEventMask(jlong);
  virtual void setFont(::java::awt::Font *);
  virtual void setForeground(::java::awt::Color *);
  virtual void setVisible(jboolean);
  virtual void show();
  virtual void hide();
  virtual jboolean isFocusable();
  virtual jboolean requestFocus(::java::awt::Component *, jboolean, jboolean, jlong);
  virtual jboolean requestFocus(::java::awt::Component *, jboolean, jboolean, jlong, ::sun::awt::CausedFocusEvent$Cause *);
  virtual jboolean isObscured();
  virtual jboolean canDetermineObscurity();
  virtual void coalescePaintEvent(::java::awt::event::PaintEvent *);
  virtual void updateCursorImmediately();
  virtual ::java::awt::image::VolatileImage * createVolatileImage(jint, jint);
  virtual jboolean handlesWheelScrolling();
  virtual void createBuffers(jint, ::java::awt::BufferCapabilities *);
  virtual ::java::awt::Image * getBackBuffer();
  virtual void flip(::java::awt::BufferCapabilities$FlipContents *);
  virtual void destroyBuffers();
  virtual jboolean isRestackSupported();
  virtual void cancelPendingPaint(jint, jint, jint, jint);
  virtual void restack();
  virtual ::java::awt::Rectangle * getBounds();
  virtual void reparent(::java::awt::peer::ContainerPeer *);
  virtual void setBounds(jint, jint, jint, jint, jint);
  virtual jboolean isReparentSupported();
  virtual void layout();
public: // actually package-private
  static ::java::awt::Dimension * MIN_SIZE;
public:
  ::gnu::gcj::xlib::Window * __attribute__((aligned(__alignof__( ::java::lang::Object)))) window;
public: // actually package-private
  ::gnu::gcj::xlib::Window * parent;
  ::java::awt::Component * component;
  ::gnu::awt::xlib::XGraphicsConfiguration * config;
private:
  ::gnu::gcj::xlib::WindowAttributes * attributes;
  jlong eventMask;
  ::java::awt::Rectangle * locationBounds;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_awt_xlib_XCanvasPeer__
