
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_image_DataBufferInt__
#define __java_awt_image_DataBufferInt__

#pragma interface

#include <java/awt/image/DataBuffer.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace image
      {
          class DataBufferInt;
      }
    }
  }
}

class java::awt::image::DataBufferInt : public ::java::awt::image::DataBuffer
{

public:
  DataBufferInt(jint);
  DataBufferInt(jint, jint);
  DataBufferInt(JArray< jint > *, jint);
  DataBufferInt(JArray< jint > *, jint, jint);
  DataBufferInt(JArray< JArray< jint > * > *, jint);
  DataBufferInt(JArray< JArray< jint > * > *, jint, JArray< jint > *);
  JArray< jint > * getData();
  JArray< jint > * getData(jint);
  JArray< JArray< jint > * > * getBankData();
  jint getElem(jint);
  jint getElem(jint, jint);
  void setElem(jint, jint);
  void setElem(jint, jint, jint);
private:
  JArray< jint > * __attribute__((aligned(__alignof__( ::java::awt::image::DataBuffer)))) data;
  JArray< JArray< jint > * > * bankData;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_image_DataBufferInt__
