
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_text_spi_NumberFormatProvider__
#define __java_text_spi_NumberFormatProvider__

#pragma interface

#include <java/util/spi/LocaleServiceProvider.h>
extern "Java"
{
  namespace java
  {
    namespace text
    {
        class NumberFormat;
      namespace spi
      {
          class NumberFormatProvider;
      }
    }
  }
}

class java::text::spi::NumberFormatProvider : public ::java::util::spi::LocaleServiceProvider
{

public: // actually protected
  NumberFormatProvider();
public:
  virtual ::java::text::NumberFormat * getCurrencyInstance(::java::util::Locale *) = 0;
  virtual ::java::text::NumberFormat * getIntegerInstance(::java::util::Locale *) = 0;
  virtual ::java::text::NumberFormat * getNumberInstance(::java::util::Locale *) = 0;
  virtual ::java::text::NumberFormat * getPercentInstance(::java::util::Locale *) = 0;
  static ::java::lang::Class class$;
};

#endif // __java_text_spi_NumberFormatProvider__
