
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_transform_ValueOfNode__
#define __gnu_xml_transform_ValueOfNode__

#pragma interface

#include <gnu/xml/transform/TemplateNode.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace transform
      {
          class Stylesheet;
          class TemplateNode;
          class ValueOfNode;
      }
      namespace xpath
      {
          class Expr;
      }
    }
  }
  namespace javax
  {
    namespace xml
    {
      namespace namespace
      {
          class QName;
      }
    }
  }
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
          class Node;
      }
    }
  }
}

class gnu::xml::transform::ValueOfNode : public ::gnu::xml::transform::TemplateNode
{

public: // actually package-private
  ValueOfNode(::gnu::xml::xpath::Expr *, jboolean);
  ::gnu::xml::transform::TemplateNode * clone(::gnu::xml::transform::Stylesheet *);
  void doApply(::gnu::xml::transform::Stylesheet *, ::javax::xml::namespace::QName *, ::org::w3c::dom::Node *, jint, jint, ::org::w3c::dom::Node *, ::org::w3c::dom::Node *);
public:
  jboolean references(::javax::xml::namespace::QName *);
  ::java::lang::String * toString();
public: // actually package-private
  ::gnu::xml::xpath::Expr * __attribute__((aligned(__alignof__( ::gnu::xml::transform::TemplateNode)))) select;
  jboolean disableOutputEscaping;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_transform_ValueOfNode__
