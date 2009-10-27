
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_imageio_jpeg_JPEGImageReaderSpi__
#define __gnu_javax_imageio_jpeg_JPEGImageReaderSpi__

#pragma interface

#include <javax/imageio/spi/ImageReaderSpi.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace imageio
      {
        namespace jpeg
        {
            class JPEGImageReaderSpi;
        }
      }
    }
  }
  namespace javax
  {
    namespace imageio
    {
        class ImageReader;
      namespace spi
      {
          class IIORegistry;
      }
    }
  }
}

class gnu::javax::imageio::jpeg::JPEGImageReaderSpi : public ::javax::imageio::spi::ImageReaderSpi
{

public:
  JPEGImageReaderSpi();
  virtual ::java::lang::String * getDescription(::java::util::Locale *);
  virtual jboolean canDecodeInput(::java::lang::Object *);
  virtual ::javax::imageio::ImageReader * createReaderInstance(::java::lang::Object *);
  static void registerSpis(::javax::imageio::spi::IIORegistry *);
  static ::gnu::javax::imageio::jpeg::JPEGImageReaderSpi * getReaderSpi();
public: // actually package-private
  static ::java::lang::String * vendorName;
  static ::java::lang::String * version;
  static ::java::lang::String * readerClassName;
  static JArray< ::java::lang::String * > * names;
  static JArray< ::java::lang::String * > * suffixes;
  static JArray< ::java::lang::String * > * MIMETypes;
  static JArray< ::java::lang::String * > * writerSpiNames;
  static const jboolean supportsStandardStreamMetadataFormat = 0;
  static ::java::lang::String * nativeStreamMetadataFormatName;
  static ::java::lang::String * nativeStreamMetadataFormatClassName;
  static JArray< ::java::lang::String * > * extraStreamMetadataFormatNames;
  static JArray< ::java::lang::String * > * extraStreamMetadataFormatClassNames;
  static const jboolean supportsStandardImageMetadataFormat = 0;
  static ::java::lang::String * nativeImageMetadataFormatName;
  static ::java::lang::String * nativeImageMetadataFormatClassName;
  static JArray< ::java::lang::String * > * extraImageMetadataFormatNames;
  static JArray< ::java::lang::String * > * extraImageMetadataFormatClassNames;
private:
  static ::gnu::javax::imageio::jpeg::JPEGImageReaderSpi * readerSpi;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_imageio_jpeg_JPEGImageReaderSpi__
