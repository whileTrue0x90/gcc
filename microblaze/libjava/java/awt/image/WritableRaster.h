
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_image_WritableRaster__
#define __java_awt_image_WritableRaster__

#pragma interface

#include <java/awt/image/Raster.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Point;
        class Rectangle;
      namespace image
      {
          class DataBuffer;
          class Raster;
          class SampleModel;
          class WritableRaster;
      }
    }
  }
}

class java::awt::image::WritableRaster : public ::java::awt::image::Raster
{

public: // actually protected
  WritableRaster(::java::awt::image::SampleModel *, ::java::awt::Point *);
  WritableRaster(::java::awt::image::SampleModel *, ::java::awt::image::DataBuffer *, ::java::awt::Point *);
  WritableRaster(::java::awt::image::SampleModel *, ::java::awt::image::DataBuffer *, ::java::awt::Rectangle *, ::java::awt::Point *, ::java::awt::image::WritableRaster *);
public:
  virtual ::java::awt::image::WritableRaster * getWritableParent();
  virtual ::java::awt::image::WritableRaster * createWritableTranslatedChild(jint, jint);
  virtual ::java::awt::image::WritableRaster * createWritableChild(jint, jint, jint, jint, jint, jint, JArray< jint > *);
  virtual ::java::awt::image::Raster * createChild(jint, jint, jint, jint, jint, jint, JArray< jint > *);
  virtual void setDataElements(jint, jint, ::java::lang::Object *);
  virtual void setDataElements(jint, jint, ::java::awt::image::Raster *);
  virtual void setDataElements(jint, jint, jint, jint, ::java::lang::Object *);
  virtual void setRect(::java::awt::image::Raster *);
  virtual void setRect(jint, jint, ::java::awt::image::Raster *);
  virtual void setPixel(jint, jint, JArray< jint > *);
  virtual void setPixel(jint, jint, JArray< jfloat > *);
  virtual void setPixel(jint, jint, JArray< jdouble > *);
  virtual void setPixels(jint, jint, jint, jint, JArray< jint > *);
  virtual void setPixels(jint, jint, jint, jint, JArray< jfloat > *);
  virtual void setPixels(jint, jint, jint, jint, JArray< jdouble > *);
  virtual void setSample(jint, jint, jint, jint);
  virtual void setSample(jint, jint, jint, jfloat);
  virtual void setSample(jint, jint, jint, jdouble);
  virtual void setSamples(jint, jint, jint, jint, jint, JArray< jint > *);
  virtual void setSamples(jint, jint, jint, jint, jint, JArray< jfloat > *);
  virtual void setSamples(jint, jint, jint, jint, jint, JArray< jdouble > *);
  static ::java::lang::Class class$;
};

#endif // __java_awt_image_WritableRaster__
