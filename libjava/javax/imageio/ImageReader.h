
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_imageio_ImageReader__
#define __javax_imageio_ImageReader__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Rectangle;
      namespace image
      {
          class BufferedImage;
          class Raster;
          class RenderedImage;
      }
    }
  }
  namespace javax
  {
    namespace imageio
    {
        class IIOImage;
        class ImageReadParam;
        class ImageReader;
        class ImageTypeSpecifier;
      namespace event
      {
          class IIOReadProgressListener;
          class IIOReadUpdateListener;
          class IIOReadWarningListener;
      }
      namespace metadata
      {
          class IIOMetadata;
      }
      namespace spi
      {
          class ImageReaderSpi;
      }
    }
  }
}

class javax::imageio::ImageReader : public ::java::lang::Object
{

public: // actually protected
  ImageReader(::javax::imageio::spi::ImageReaderSpi *);
public:
  virtual void abort();
public: // actually protected
  virtual jboolean abortRequested();
public:
  virtual void addIIOReadProgressListener(::javax::imageio::event::IIOReadProgressListener *);
  virtual void addIIOReadUpdateListener(::javax::imageio::event::IIOReadUpdateListener *);
  virtual void addIIOReadWarningListener(::javax::imageio::event::IIOReadWarningListener *);
  virtual jboolean canReadRaster();
public: // actually protected
  virtual void clearAbortRequest();
public:
  virtual void dispose();
  virtual jfloat getAspectRatio(jint);
  virtual JArray< ::java::util::Locale * > * getAvailableLocales();
  virtual ::javax::imageio::ImageReadParam * getDefaultReadParam();
  virtual ::java::lang::String * getFormatName();
  virtual jint getHeight(jint) = 0;
  virtual ::javax::imageio::metadata::IIOMetadata * getImageMetadata(jint) = 0;
  virtual ::java::util::Iterator * getImageTypes(jint) = 0;
  virtual void setInput(::java::lang::Object *, jboolean, jboolean);
  virtual void setInput(::java::lang::Object *, jboolean);
  virtual void setInput(::java::lang::Object *);
  virtual ::java::lang::Object * getInput();
  virtual ::java::util::Locale * getLocale();
  virtual jint getNumImages(jboolean) = 0;
  virtual jint getNumThumbnails(jint);
  virtual ::javax::imageio::spi::ImageReaderSpi * getOriginatingProvider();
  virtual ::javax::imageio::metadata::IIOMetadata * getStreamMetadata() = 0;
  virtual jint getThumbnailHeight(jint, jint);
  virtual jint getThumbnailWidth(jint, jint);
  virtual jint getTileGridXOffset(jint);
  virtual jint getTileGridYOffset(jint);
  virtual jint getTileHeight(jint);
  virtual jint getTileWidth(jint);
  virtual jint getWidth(jint) = 0;
  virtual jboolean hasThumbnails(jint);
  virtual jboolean isIgnoringMetadata();
  virtual jboolean isImageTiled(jint);
  virtual jboolean isRandomAccessEasy(jint);
  virtual jboolean isSeekForwardOnly();
public: // actually protected
  virtual void processImageComplete();
  virtual void processImageProgress(jfloat);
  virtual void processImageStarted(jint);
  virtual void processImageUpdate(::java::awt::image::BufferedImage *, jint, jint, jint, jint, jint, jint, JArray< jint > *);
  virtual void processPassComplete(::java::awt::image::BufferedImage *);
  virtual void processPassStarted(::java::awt::image::BufferedImage *, jint, jint, jint, jint, jint, jint, jint, JArray< jint > *);
  virtual void processReadAborted();
  virtual void processSequenceComplete();
  virtual void processSequenceStarted(jint);
  virtual void processThumbnailComplete();
  virtual void processThumbnailPassComplete(::java::awt::image::BufferedImage *);
  virtual void processThumbnailPassStarted(::java::awt::image::BufferedImage *, jint, jint, jint, jint, jint, jint, jint, JArray< jint > *);
  virtual void processThumbnailProgress(jfloat);
  virtual void processThumbnailStarted(jint, jint);
  virtual void processThumbnailUpdate(::java::awt::image::BufferedImage *, jint, jint, jint, jint, jint, jint, JArray< jint > *);
  virtual void processWarningOccurred(::java::lang::String *);
  virtual void processWarningOccurred(::java::lang::String *, ::java::lang::String *);
public:
  virtual ::java::awt::image::BufferedImage * read(jint, ::javax::imageio::ImageReadParam *) = 0;
  virtual jboolean readerSupportsThumbnails();
  virtual ::java::awt::image::Raster * readRaster(jint, ::javax::imageio::ImageReadParam *);
  virtual ::java::awt::image::BufferedImage * readThumbnail(jint, jint);
  virtual void removeAllIIOReadProgressListeners();
  virtual void removeAllIIOReadUpdateListeners();
  virtual void removeAllIIOReadWarningListeners();
  virtual void removeIIOReadProgressListener(::javax::imageio::event::IIOReadProgressListener *);
  virtual void removeIIOReadUpdateListener(::javax::imageio::event::IIOReadUpdateListener *);
  virtual void removeIIOReadWarningListener(::javax::imageio::event::IIOReadWarningListener *);
  virtual void setLocale(::java::util::Locale *);
public: // actually protected
  static void checkReadParamBandSettings(::javax::imageio::ImageReadParam *, jint, jint);
  static void computeRegions(::javax::imageio::ImageReadParam *, jint, jint, ::java::awt::image::BufferedImage *, ::java::awt::Rectangle *, ::java::awt::Rectangle *);
  static ::java::awt::image::BufferedImage * getDestination(::javax::imageio::ImageReadParam *, ::java::util::Iterator *, jint, jint);
public:
  virtual ::javax::imageio::metadata::IIOMetadata * getImageMetadata(jint, ::java::lang::String *, ::java::util::Set *);
  virtual jint getMinIndex();
  virtual ::javax::imageio::ImageTypeSpecifier * getRawImageType(jint);
public: // actually protected
  static ::java::awt::Rectangle * getSourceRegion(::javax::imageio::ImageReadParam *, jint, jint);
public:
  virtual ::javax::imageio::metadata::IIOMetadata * getStreamMetadata(::java::lang::String *, ::java::util::Set *);
  virtual ::java::awt::image::BufferedImage * read(jint);
  virtual ::javax::imageio::IIOImage * readAll(jint, ::javax::imageio::ImageReadParam *);
  virtual ::java::util::Iterator * readAll(::java::util::Iterator *);
  virtual ::java::awt::image::RenderedImage * readAsRenderedImage(jint, ::javax::imageio::ImageReadParam *);
  virtual ::java::awt::image::BufferedImage * readTile(jint, jint, jint);
  virtual ::java::awt::image::Raster * readTileRaster(jint, jint, jint);
  virtual void reset();
private:
  jboolean __attribute__((aligned(__alignof__( ::java::lang::Object)))) aborted;
public: // actually protected
  JArray< ::java::util::Locale * > * availableLocales;
  jboolean ignoreMetadata;
  ::java::lang::Object * input;
  ::java::util::Locale * locale;
  jint minIndex;
  ::javax::imageio::spi::ImageReaderSpi * originatingProvider;
  ::java::util::List * progressListeners;
  jboolean seekForwardOnly;
  ::java::util::List * updateListeners;
  ::java::util::List * warningListeners;
  ::java::util::List * warningLocales;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_imageio_ImageReader__
