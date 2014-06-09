
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_dom_html2_DomHTMLFormElement__
#define __gnu_xml_dom_html2_DomHTMLFormElement__

#pragma interface

#include <gnu/xml/dom/html2/DomHTMLElement.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace dom
      {
        namespace html2
        {
            class DomHTMLDocument;
            class DomHTMLFormElement;
        }
      }
    }
  }
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
        namespace html2
        {
            class HTMLCollection;
        }
      }
    }
  }
}

class gnu::xml::dom::html2::DomHTMLFormElement : public ::gnu::xml::dom::html2::DomHTMLElement
{

public: // actually protected
  DomHTMLFormElement(::gnu::xml::dom::html2::DomHTMLDocument *, ::java::lang::String *, ::java::lang::String *);
public:
  virtual ::org::w3c::dom::html2::HTMLCollection * getElements();
  virtual jint getLength();
  virtual ::java::lang::String * getName();
  virtual void setName(::java::lang::String *);
  virtual ::java::lang::String * getAcceptCharset();
  virtual void setAcceptCharset(::java::lang::String *);
  virtual ::java::lang::String * getAction();
  virtual void setAction(::java::lang::String *);
  virtual ::java::lang::String * getEnctype();
  virtual void setEnctype(::java::lang::String *);
  virtual ::java::lang::String * getMethod();
  virtual void setMethod(::java::lang::String *);
  virtual ::java::lang::String * getTarget();
  virtual void setTarget(::java::lang::String *);
  virtual void submit();
  virtual void reset();
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_dom_html2_DomHTMLFormElement__
