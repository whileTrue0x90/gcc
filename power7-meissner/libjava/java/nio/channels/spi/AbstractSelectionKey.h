
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_nio_channels_spi_AbstractSelectionKey__
#define __java_nio_channels_spi_AbstractSelectionKey__

#pragma interface

#include <java/nio/channels/SelectionKey.h>
extern "Java"
{
  namespace java
  {
    namespace nio
    {
      namespace channels
      {
        namespace spi
        {
            class AbstractSelectionKey;
        }
      }
    }
  }
}

class java::nio::channels::spi::AbstractSelectionKey : public ::java::nio::channels::SelectionKey
{

public: // actually protected
  AbstractSelectionKey();
public:
  virtual void cancel();
  virtual jboolean isValid();
private:
  jboolean __attribute__((aligned(__alignof__( ::java::nio::channels::SelectionKey)))) cancelled;
public:
  static ::java::lang::Class class$;
};

#endif // __java_nio_channels_spi_AbstractSelectionKey__
