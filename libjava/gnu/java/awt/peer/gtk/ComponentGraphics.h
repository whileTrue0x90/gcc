
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_peer_gtk_ComponentGraphics__
#define __gnu_java_awt_peer_gtk_ComponentGraphics__

#pragma interface

#include <gnu/java/awt/peer/gtk/CairoGraphics2D.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace classpath
    {
        class Pointer;
    }
    namespace java
    {
      namespace awt
      {
        namespace peer
        {
          namespace gtk
          {
              class CairoSurface;
              class ComponentGraphics;
              class GtkComponentPeer;
              class GtkImage;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class Color;
        class Graphics;
        class Graphics2D;
        class GraphicsConfiguration;
        class Image;
        class Shape;
      namespace font
      {
          class GlyphVector;
      }
      namespace geom
      {
          class AffineTransform;
          class Rectangle2D;
      }
      namespace image
      {
          class BufferedImage;
          class ColorModel;
          class ImageObserver;
          class RenderedImage;
      }
    }
  }
}

class gnu::java::awt::peer::gtk::ComponentGraphics : public ::gnu::java::awt::peer::gtk::CairoGraphics2D
{

public: // actually package-private
  ComponentGraphics();
private:
  ComponentGraphics(::gnu::java::awt::peer::gtk::GtkComponentPeer *);
  ComponentGraphics(::gnu::java::awt::peer::gtk::ComponentGraphics *);
  jlong initState(::gnu::java::awt::peer::gtk::GtkComponentPeer *);
  void lock();
  void unlock();
public: // actually protected
  virtual jlong initFromVolatile(jlong);
private:
  void start_gdk_drawing();
  void end_gdk_drawing();
public:
  static jboolean hasXRender();
private:
  static ::gnu::classpath::Pointer * nativeGrab(::gnu::java::awt::peer::gtk::GtkComponentPeer *);
  void copyAreaNative(::gnu::java::awt::peer::gtk::GtkComponentPeer *, jint, jint, jint, jint, jint, jint);
  void drawVolatile(::gnu::java::awt::peer::gtk::GtkComponentPeer *, jlong, jint, jint, jint, jint, jint, jint, jint, jint);
public:
  static ::gnu::java::awt::peer::gtk::GtkImage * grab(::gnu::java::awt::peer::gtk::GtkComponentPeer *);
  static ::java::awt::Graphics2D * getComponentGraphics(::gnu::java::awt::peer::gtk::GtkComponentPeer *);
  virtual ::java::awt::GraphicsConfiguration * getDeviceConfiguration();
  virtual ::java::awt::Graphics * create();
public: // actually protected
  virtual ::java::awt::geom::Rectangle2D * getRealBounds();
public:
  virtual void copyAreaImpl(jint, jint, jint, jint, jint, jint);
  virtual void draw(::java::awt::Shape *);
  virtual void fill(::java::awt::Shape *);
  virtual void drawRenderedImage(::java::awt::image::RenderedImage *, ::java::awt::geom::AffineTransform *);
public: // actually protected
  virtual jboolean drawImage(::java::awt::Image *, ::java::awt::geom::AffineTransform *, ::java::awt::Color *, ::java::awt::image::ImageObserver *);
public:
  virtual void drawGlyphVector(::java::awt::font::GlyphVector *, jfloat, jfloat);
  virtual jboolean drawImage(::java::awt::Image *, jint, jint, ::java::awt::image::ImageObserver *);
  virtual jboolean drawImage(::java::awt::Image *, jint, jint, jint, jint, ::java::awt::image::ImageObserver *);
private:
  jboolean drawComposite(::java::awt::geom::Rectangle2D *, ::java::awt::image::ImageObserver *);
  void createBuffer();
public: // actually protected
  virtual ::java::awt::image::ColorModel * getNativeCM();
  virtual jlong init(jlong);
  virtual void drawPixels(jlong, JArray< jint > *, jint, jint, jint, JArray< jdouble > *, jdouble, jint);
  virtual void setGradient(jlong, jdouble, jdouble, jdouble, jdouble, jint, jint, jint, jint, jint, jint, jint, jint, jboolean);
  virtual void setPaintPixels(jlong, JArray< jint > *, jint, jint, jint, jboolean, jint, jint);
  virtual void cairoSetMatrix(jlong, JArray< jdouble > *);
  virtual void cairoScale(jlong, jdouble, jdouble);
  virtual void cairoSetOperator(jlong, jint);
  virtual void cairoSetRGBAColor(jlong, jdouble, jdouble, jdouble, jdouble);
  virtual void cairoSetFillRule(jlong, jint);
  virtual void cairoSetLine(jlong, jdouble, jint, jint, jdouble);
  virtual void cairoSetDash(jlong, JArray< jdouble > *, jint, jdouble);
  virtual void cairoRectangle(jlong, jdouble, jdouble, jdouble, jdouble);
  virtual void cairoArc(jlong, jdouble, jdouble, jdouble, jdouble, jdouble);
  virtual void cairoSave(jlong);
  virtual void cairoRestore(jlong);
  virtual void cairoNewPath(jlong);
  virtual void cairoClosePath(jlong);
  virtual void cairoMoveTo(jlong, jdouble, jdouble);
  virtual void cairoLineTo(jlong, jdouble, jdouble);
  virtual void cairoCurveTo(jlong, jdouble, jdouble, jdouble, jdouble, jdouble, jdouble);
  virtual void cairoStroke(jlong);
  virtual void cairoFill(jlong, jdouble);
  virtual void cairoClip(jlong);
  virtual void cairoResetClip(jlong);
  virtual void cairoSetAntialias(jlong, jboolean);
  virtual void drawCairoSurface(::gnu::java::awt::peer::gtk::CairoSurface *, ::java::awt::geom::AffineTransform *, jdouble, jint);
private:
  static jboolean hasXRenderExtension;
  ::gnu::java::awt::peer::gtk::GtkComponentPeer * __attribute__((aligned(__alignof__( ::gnu::java::awt::peer::gtk::CairoGraphics2D)))) component;
public: // actually protected
  jlong cairo_t;
private:
  ::java::awt::image::BufferedImage * buffer;
  ::java::awt::image::BufferedImage * componentBuffer;
  static ::java::lang::ThreadLocal * hasLock;
  static ::java::lang::Integer * ONE;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_peer_gtk_ComponentGraphics__
