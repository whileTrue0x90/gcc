
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_locale_LocaleInformation_de__
#define __gnu_java_locale_LocaleInformation_de__

#pragma interface

#include <java/util/ListResourceBundle.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace locale
      {
          class LocaleInformation_de;
      }
    }
  }
}

class gnu::java::locale::LocaleInformation_de : public ::java::util::ListResourceBundle
{

public:
  LocaleInformation_de();
  virtual JArray< JArray< ::java::lang::Object * > * > * getContents();
private:
  static ::java::lang::String * collation_rules;
  static JArray< ::java::lang::String * > * months;
  static JArray< ::java::lang::String * > * shortMonths;
  static JArray< ::java::lang::String * > * weekdays;
  static JArray< ::java::lang::String * > * shortWeekdays;
  static JArray< ::java::lang::String * > * eras;
  static JArray< JArray< ::java::lang::String * > * > * zoneStrings;
  static ::java::lang::String * shortDateFormat;
  static ::java::lang::String * mediumDateFormat;
  static ::java::lang::String * longDateFormat;
  static ::java::lang::String * fullDateFormat;
  static ::java::lang::String * defaultDateFormat;
  static ::java::lang::String * shortTimeFormat;
  static ::java::lang::String * mediumTimeFormat;
  static ::java::lang::String * longTimeFormat;
  static ::java::lang::String * fullTimeFormat;
  static ::java::lang::String * defaultTimeFormat;
  static ::java::lang::String * currencySymbol;
  static ::java::lang::String * intlCurrencySymbol;
  static ::java::lang::String * decimalSeparator;
  static ::java::lang::String * monetarySeparator;
  static JArray< JArray< ::java::lang::Object * > * > * contents;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_locale_LocaleInformation_de__
