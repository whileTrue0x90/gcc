
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_xml_stream_XMLStreamException__
#define __javax_xml_stream_XMLStreamException__

#pragma interface

#include <java/lang/Exception.h>
extern "Java"
{
  namespace javax
  {
    namespace xml
    {
      namespace stream
      {
          class Location;
          class XMLStreamException;
      }
    }
  }
}

class javax::xml::stream::XMLStreamException : public ::java::lang::Exception
{

public:
  XMLStreamException();
  XMLStreamException(::java::lang::String *);
  XMLStreamException(::java::lang::Throwable *);
  XMLStreamException(::java::lang::String *, ::java::lang::Throwable *);
  XMLStreamException(::java::lang::String *, ::javax::xml::stream::Location *, ::java::lang::Throwable *);
  XMLStreamException(::java::lang::String *, ::javax::xml::stream::Location *);
  virtual ::java::lang::Throwable * getNestedException();
  virtual ::javax::xml::stream::Location * getLocation();
public: // actually protected
  ::javax::xml::stream::Location * __attribute__((aligned(__alignof__( ::java::lang::Exception)))) location;
  ::java::lang::Throwable * nested;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_xml_stream_XMLStreamException__
