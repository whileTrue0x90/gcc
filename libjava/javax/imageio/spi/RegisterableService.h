
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_imageio_spi_RegisterableService__
#define __javax_imageio_spi_RegisterableService__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace imageio
    {
      namespace spi
      {
          class RegisterableService;
          class ServiceRegistry;
      }
    }
  }
}

class javax::imageio::spi::RegisterableService : public ::java::lang::Object
{

public:
  virtual void onRegistration(::javax::imageio::spi::ServiceRegistry *, ::java::lang::Class *) = 0;
  virtual void onDeregistration(::javax::imageio::spi::ServiceRegistry *, ::java::lang::Class *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_imageio_spi_RegisterableService__
