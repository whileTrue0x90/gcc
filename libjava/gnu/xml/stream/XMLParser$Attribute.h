
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_stream_XMLParser$Attribute__
#define __gnu_xml_stream_XMLParser$Attribute__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace stream
      {
          class XMLParser;
          class XMLParser$Attribute;
      }
    }
  }
}

class gnu::xml::stream::XMLParser$Attribute : public ::java::lang::Object
{

public: // actually package-private
  XMLParser$Attribute(::gnu::xml::stream::XMLParser *, ::java::lang::String *, ::java::lang::String *, jboolean, ::java::lang::String *);
public:
  virtual jboolean equals(::java::lang::Object *);
public: // actually package-private
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) name;
  ::java::lang::String * type;
  jboolean specified;
  ::java::lang::String * value;
  ::java::lang::String * prefix;
  ::java::lang::String * localName;
  ::gnu::xml::stream::XMLParser * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_stream_XMLParser$Attribute__
