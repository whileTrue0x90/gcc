
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_validation_datatype_Type__
#define __gnu_xml_validation_datatype_Type__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace validation
      {
        namespace datatype
        {
            class Type;
        }
      }
    }
  }
  namespace javax
  {
    namespace xml
    {
      namespace namespace$
      {
          class QName;
      }
    }
  }
}

class gnu::xml::validation::datatype::Type : public ::java::lang::Object
{

public:
  Type(::javax::xml::namespace$::QName *);
  static ::gnu::xml::validation::datatype::Type * ANY_TYPE;
  ::javax::xml::namespace$::QName * __attribute__((aligned(__alignof__( ::java::lang::Object)))) name;
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_validation_datatype_Type__
