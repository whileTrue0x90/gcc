
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_aelfred2_XmlReader__
#define __gnu_xml_aelfred2_XmlReader__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace aelfred2
      {
          class SAXDriver;
          class XmlReader;
      }
      namespace pipeline
      {
          class EventFilter;
      }
    }
  }
  namespace org
  {
    namespace xml
    {
      namespace sax
      {
          class ContentHandler;
          class DTDHandler;
          class EntityResolver;
          class ErrorHandler;
          class InputSource;
      }
    }
  }
}

class gnu::xml::aelfred2::XmlReader : public ::java::lang::Object
{

public:
  XmlReader();
  XmlReader(jboolean);
  ::org::xml::sax::ContentHandler * getContentHandler();
  void setContentHandler(::org::xml::sax::ContentHandler *);
  ::org::xml::sax::DTDHandler * getDTDHandler();
  void setDTDHandler(::org::xml::sax::DTDHandler *);
  ::org::xml::sax::EntityResolver * getEntityResolver();
  void setEntityResolver(::org::xml::sax::EntityResolver *);
  ::org::xml::sax::ErrorHandler * getErrorHandler();
  void setErrorHandler(::org::xml::sax::ErrorHandler *);
  void setProperty(::java::lang::String *, ::java::lang::Object *);
  ::java::lang::Object * getProperty(::java::lang::String *);
private:
  void forceValidating();
public:
  void setFeature(::java::lang::String *, jboolean);
  jboolean getFeature(::java::lang::String *);
  void setLocale(::java::util::Locale *);
  void parse(::java::lang::String *);
  void parse(::org::xml::sax::InputSource *);
private:
  ::gnu::xml::aelfred2::SAXDriver * __attribute__((aligned(__alignof__( ::java::lang::Object)))) aelfred2;
  ::gnu::xml::pipeline::EventFilter * filter;
  jboolean isValidating;
  jboolean active;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_aelfred2_XmlReader__
