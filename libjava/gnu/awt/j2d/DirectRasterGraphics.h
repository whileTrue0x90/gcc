
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_awt_j2d_DirectRasterGraphics__
#define __gnu_awt_j2d_DirectRasterGraphics__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace awt
    {
      namespace j2d
      {
          class DirectRasterGraphics;
          class MappedRaster;
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class Color;
        class Font;
        class FontMetrics;
        class Image;
        class Rectangle;
        class Shape;
      namespace image
      {
          class ImageObserver;
      }
    }
  }
}

class gnu::awt::j2d::DirectRasterGraphics : public ::java::lang::Object
{

public:
  virtual void dispose() = 0;
  virtual void setColor(::java::awt::Color *) = 0;
  virtual void setPaintMode() = 0;
  virtual void setXORMode(::java::awt::Color *) = 0;
  virtual void setFont(::java::awt::Font *) = 0;
  virtual ::java::awt::FontMetrics * getFontMetrics(::java::awt::Font *) = 0;
  virtual void setClip(::java::awt::Shape *) = 0;
  virtual void copyArea(jint, jint, jint, jint, jint, jint) = 0;
  virtual void drawLine(jint, jint, jint, jint) = 0;
  virtual void drawRect(jint, jint, jint, jint) = 0;
  virtual void fillRect(jint, jint, jint, jint) = 0;
  virtual void drawArc(jint, jint, jint, jint, jint, jint) = 0;
  virtual void fillArc(jint, jint, jint, jint, jint, jint) = 0;
  virtual void drawPolyline(JArray< jint > *, JArray< jint > *, jint) = 0;
  virtual void drawPolygon(JArray< jint > *, JArray< jint > *, jint) = 0;
  virtual void fillPolygon(JArray< jint > *, JArray< jint > *, jint, jint, jint) = 0;
  virtual void drawString(::java::lang::String *, jint, jint) = 0;
  virtual jboolean drawImage(::java::awt::Image *, jint, jint, ::java::awt::image::ImageObserver *) = 0;
  virtual ::gnu::awt::j2d::MappedRaster * mapRaster(::java::awt::Rectangle *) = 0;
  virtual void unmapRaster(::gnu::awt::j2d::MappedRaster *) = 0;
  virtual ::java::lang::Object * clone() = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __gnu_awt_j2d_DirectRasterGraphics__
