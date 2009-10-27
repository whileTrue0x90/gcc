
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_imageio_gif_GIFFile__
#define __gnu_javax_imageio_gif_GIFFile__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace imageio
      {
        namespace gif
        {
            class GIFFile;
        }
      }
    }
  }
}

class gnu::javax::imageio::gif::GIFFile : public ::java::lang::Object
{

public:
  GIFFile(::java::io::InputStream *);
private:
  GIFFile(::gnu::javax::imageio::gif::GIFFile *, ::java::io::InputStream *, jint);
public:
  static jboolean readSignature(::java::io::InputStream *);
private:
  void loadImage(::java::io::InputStream *);
  void packPixels();
public:
  virtual jint getWidth();
  virtual jint getHeight();
  virtual jint getNColors();
  virtual jboolean hasTransparency();
  virtual jint getTransparentIndex();
  virtual ::java::lang::String * getComment();
  virtual jint getDuration();
private:
  void deinterlace();
  void readLocal(::java::io::InputStream *);
public:
  virtual JArray< jbyte > * getRawPalette();
  virtual ::gnu::javax::imageio::gif::GIFFile * getImage(jint);
  virtual JArray< jbyte > * getRawImage();
  virtual jint nImages();
private:
  void readExtension(::java::io::InputStream *);
  JArray< jbyte > * readData(::java::io::InputStream *);
  void decodeRaster(::java::io::InputStream *);
  jint getBits(jint);
  static JArray< jbyte > * nsBlock;
  static const jint EXTENSION = 33;
  static const jint LOCAL = 44;
  static const jint TERMINATOR = 59;
  static const jint EXTENSION_COMMENT = 254;
  static const jint EXTENSION_GCONTROL = 249;
  static const jint EXTENSION_APPLICATION = 255;
  static const jint UNDRAW_OVERWRITE = 1;
  static const jint UNDRAW_RESTORE_BACKGROUND = 2;
  static const jint UNDRAW_RESTORE_PREVIOUS = 3;
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) x;
  jint y;
  jint width;
  jint height;
  jint globalWidth;
  jint globalHeight;
  jbyte bgIndex;
  jint nColors;
  JArray< jbyte > * globalPalette;
  jboolean hasGlobalColorMap;
  JArray< jbyte > * localPalette;
  jboolean interlaced;
  jboolean hasTransparency__;
  jint undraw;
  jint transparentIndex;
  JArray< jbyte > * raster;
  JArray< jbyte > * compressedData;
  jint duration;
  jint dataBlockIndex;
  ::java::lang::String * comment;
  jint remainingBits;
  jint currentBits;
  jboolean isLooped;
  jint loops;
  ::java::util::Vector * animationFrames;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_imageio_gif_GIFFile__
