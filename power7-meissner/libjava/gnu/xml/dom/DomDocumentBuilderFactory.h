
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_dom_DomDocumentBuilderFactory__
#define __gnu_xml_dom_DomDocumentBuilderFactory__

#pragma interface

#include <javax/xml/parsers/DocumentBuilderFactory.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace dom
      {
          class DomDocumentBuilderFactory;
      }
    }
  }
  namespace javax
  {
    namespace xml
    {
      namespace parsers
      {
          class DocumentBuilder;
      }
    }
  }
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
          class DOMConfiguration;
          class DOMImplementation;
        namespace ls
        {
            class DOMImplementationLS;
        }
      }
    }
  }
}

class gnu::xml::dom::DomDocumentBuilderFactory : public ::javax::xml::parsers::DocumentBuilderFactory
{

public:
  DomDocumentBuilderFactory();
  virtual ::javax::xml::parsers::DocumentBuilder * newDocumentBuilder();
public: // actually package-private
  virtual void setParameter(::org::w3c::dom::DOMConfiguration *, ::java::lang::String *, ::java::lang::Object *);
public:
  virtual ::java::lang::Object * getAttribute(::java::lang::String *);
  virtual void setAttribute(::java::lang::String *, ::java::lang::Object *);
  virtual void setFeature(::java::lang::String *, jboolean);
  virtual jboolean getFeature(::java::lang::String *);
public: // actually package-private
  ::org::w3c::dom::DOMImplementation * __attribute__((aligned(__alignof__( ::javax::xml::parsers::DocumentBuilderFactory)))) impl;
  ::org::w3c::dom::ls::DOMImplementationLS * ls;
private:
  jboolean secureProcessing;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_dom_DomDocumentBuilderFactory__
