
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_imageio_bmp_DecodeRLE8__
#define __gnu_javax_imageio_bmp_DecodeRLE8__

#pragma interface

#include <gnu/javax/imageio/bmp/BMPDecoder.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace imageio
      {
        namespace bmp
        {
            class BMPFileHeader;
            class BMPInfoHeader;
            class DecodeRLE8;
        }
      }
    }
  }
  namespace java
  {
    namespace awt
    {
      namespace image
      {
          class BufferedImage;
      }
    }
  }
  namespace javax
  {
    namespace imageio
    {
      namespace stream
      {
          class ImageInputStream;
      }
    }
  }
}

class gnu::javax::imageio::bmp::DecodeRLE8 : public ::gnu::javax::imageio::bmp::BMPDecoder
{

public:
  DecodeRLE8(::gnu::javax::imageio::bmp::BMPFileHeader *, ::gnu::javax::imageio::bmp::BMPInfoHeader *);
  virtual ::java::awt::image::BufferedImage * decode(::javax::imageio::stream::ImageInputStream *);
private:
  JArray< jbyte > * uncompress(jint, jint, ::javax::imageio::stream::ImageInputStream *);
  static const jbyte ESCAPE = 0;
  static const jbyte EOL = 0;
  static const jbyte EOB = 1;
  static const jbyte DELTA = 2;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_imageio_bmp_DecodeRLE8__
