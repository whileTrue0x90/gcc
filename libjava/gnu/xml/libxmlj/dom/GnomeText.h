
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_libxmlj_dom_GnomeText__
#define __gnu_xml_libxmlj_dom_GnomeText__

#pragma interface

#include <gnu/xml/libxmlj/dom/GnomeCharacterData.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace libxmlj
      {
        namespace dom
        {
            class GnomeText;
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
          class Text;
      }
    }
  }
}

class gnu::xml::libxmlj::dom::GnomeText : public ::gnu::xml::libxmlj::dom::GnomeCharacterData
{

public: // actually package-private
  GnomeText(::java::lang::Object *);
public:
  virtual ::org::w3c::dom::Text * splitText(jint);
  virtual jboolean isElementContentWhitespace();
  virtual ::java::lang::String * getWholeText();
  virtual ::org::w3c::dom::Text * replaceWholeText(::java::lang::String *);
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_libxmlj_dom_GnomeText__
