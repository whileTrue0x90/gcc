
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_xml_transform_sax_TransformerHandler__
#define __javax_xml_transform_sax_TransformerHandler__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace xml
    {
      namespace transform
      {
          class Result;
          class Transformer;
        namespace sax
        {
            class TransformerHandler;
        }
      }
    }
  }
  namespace org
  {
    namespace xml
    {
      namespace sax
      {
          class Attributes;
          class Locator;
      }
    }
  }
}

class javax::xml::transform::sax::TransformerHandler : public ::java::lang::Object
{

public:
  virtual void setResult(::javax::xml::transform::Result *) = 0;
  virtual void setSystemId(::java::lang::String *) = 0;
  virtual ::java::lang::String * getSystemId() = 0;
  virtual ::javax::xml::transform::Transformer * getTransformer() = 0;
  virtual void setDocumentLocator(::org::xml::sax::Locator *) = 0;
  virtual void startDocument() = 0;
  virtual void endDocument() = 0;
  virtual void startPrefixMapping(::java::lang::String *, ::java::lang::String *) = 0;
  virtual void endPrefixMapping(::java::lang::String *) = 0;
  virtual void startElement(::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::org::xml::sax::Attributes *) = 0;
  virtual void endElement(::java::lang::String *, ::java::lang::String *, ::java::lang::String *) = 0;
  virtual void characters(JArray< jchar > *, jint, jint) = 0;
  virtual void ignorableWhitespace(JArray< jchar > *, jint, jint) = 0;
  virtual void processingInstruction(::java::lang::String *, ::java::lang::String *) = 0;
  virtual void skippedEntity(::java::lang::String *) = 0;
  virtual void startDTD(::java::lang::String *, ::java::lang::String *, ::java::lang::String *) = 0;
  virtual void endDTD() = 0;
  virtual void startEntity(::java::lang::String *) = 0;
  virtual void endEntity(::java::lang::String *) = 0;
  virtual void startCDATA() = 0;
  virtual void endCDATA() = 0;
  virtual void comment(JArray< jchar > *, jint, jint) = 0;
  virtual void notationDecl(::java::lang::String *, ::java::lang::String *, ::java::lang::String *) = 0;
  virtual void unparsedEntityDecl(::java::lang::String *, ::java::lang::String *, ::java::lang::String *, ::java::lang::String *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_xml_transform_sax_TransformerHandler__
