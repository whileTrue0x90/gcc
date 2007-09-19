
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_imageio_png_PNGChunk__
#define __gnu_javax_imageio_png_PNGChunk__

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
        namespace png
        {
            class PNGChunk;
        }
      }
    }
  }
}

class gnu::javax::imageio::png::PNGChunk : public ::java::lang::Object
{

public: // actually protected
  PNGChunk(jint, JArray< jbyte > *, jint);
  PNGChunk(jint);
public:
  static ::gnu::javax::imageio::png::PNGChunk * readChunk(::java::io::InputStream *, jboolean);
private:
  static ::gnu::javax::imageio::png::PNGChunk * getChunk(jint, JArray< jbyte > *, jint);
  static jboolean isEssentialChunk(jint);
public:
  virtual jboolean isValidChunk();
  virtual jint getType();
  virtual void writeChunk(::java::io::OutputStream *);
  virtual jboolean isEmpty();
  static JArray< jbyte > * getInt(jint);
private:
  jint calcCRC();
public:
  virtual ::java::lang::String * toString();
private:
  static JArray< jlong > * crcTable;
public:
  static const jint TYPE_HEADER = 1229472850;
  static const jint TYPE_PALETTE = 1347179589;
  static const jint TYPE_DATA = 1229209940;
  static const jint TYPE_TIME = 1950960965;
  static const jint TYPE_END = 1229278788;
  static const jint TYPE_PHYS = 1883789683;
  static const jint TYPE_GAMMA = 1732332865;
  static const jint TYPE_PROFILE = 1766015824;
private:
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) type;
public: // actually protected
  JArray< jbyte > * data;
private:
  jint crc;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_imageio_png_PNGChunk__
