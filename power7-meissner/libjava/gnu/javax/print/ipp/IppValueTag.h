
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_print_ipp_IppValueTag__
#define __gnu_javax_print_ipp_IppValueTag__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace print
      {
        namespace ipp
        {
            class IppValueTag;
        }
      }
    }
  }
}

class gnu::javax::print::ipp::IppValueTag : public ::java::lang::Object
{

  IppValueTag();
public:
  static jboolean isValueTag(jbyte);
  static const jbyte UNSUPPORTED = 16;
  static const jbyte UNKNOWN = 18;
  static const jbyte NO_VALUE = 19;
  static const jbyte INTEGER = 33;
  static const jbyte BOOLEAN = 34;
  static const jbyte ENUM = 35;
  static const jbyte OCTECTSTRING_UNSPECIFIED = 48;
  static const jbyte DATETIME = 49;
  static const jbyte RESOLUTION = 50;
  static const jbyte RANGEOFINTEGER = 51;
  static const jbyte TEXT_WITH_LANGUAGE = 53;
  static const jbyte NAME_WITH_LANGUAGE = 54;
  static const jbyte TEXT_WITHOUT_LANGUAGE = 65;
  static const jbyte NAME_WITHOUT_LANGUAGE = 66;
  static const jbyte KEYWORD = 68;
  static const jbyte URI = 69;
  static const jbyte URI_SCHEME = 70;
  static const jbyte CHARSET = 71;
  static const jbyte NATURAL_LANGUAGE = 72;
  static const jbyte MIME_MEDIA_TYPE = 73;
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_print_ipp_IppValueTag__
