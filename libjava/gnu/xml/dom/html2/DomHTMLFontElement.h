
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_dom_html2_DomHTMLFontElement__
#define __gnu_xml_dom_html2_DomHTMLFontElement__

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
            class DomHTMLFontElement;
        }
      }
    }
  }
}

class gnu::xml::dom::html2::DomHTMLFontElement : public ::gnu::xml::dom::html2::DomHTMLElement
{

public: // actually protected
  DomHTMLFontElement(::gnu::xml::dom::html2::DomHTMLDocument *, ::java::lang::String *, ::java::lang::String *);
public:
  virtual ::java::lang::String * getColor();
  virtual void setColor(::java::lang::String *);
  virtual ::java::lang::String * getFace();
  virtual void setFace(::java::lang::String *);
  virtual ::java::lang::String * getSize();
  virtual void setSize(::java::lang::String *);
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_dom_html2_DomHTMLFontElement__
