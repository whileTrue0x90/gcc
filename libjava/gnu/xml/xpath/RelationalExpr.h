
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_xpath_RelationalExpr__
#define __gnu_xml_xpath_RelationalExpr__

#pragma interface

#include <gnu/xml/xpath/Expr.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace xpath
      {
          class Expr;
          class RelationalExpr;
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

class gnu::xml::xpath::RelationalExpr : public ::gnu::xml::xpath::Expr
{

public: // actually package-private
  RelationalExpr(::gnu::xml::xpath::Expr *, ::gnu::xml::xpath::Expr *, jboolean, jboolean);
public:
  ::java::lang::Object * evaluate(::org::w3c::dom::Node *, jint, jint);
  ::gnu::xml::xpath::Expr * clone(::java::lang::Object *);
  jboolean references(::javax::xml::namespace$::QName *);
  ::java::lang::String * toString();
public: // actually package-private
  ::gnu::xml::xpath::Expr * __attribute__((aligned(__alignof__( ::gnu::xml::xpath::Expr)))) lhs;
  ::gnu::xml::xpath::Expr * rhs;
  jboolean lt;
  jboolean eq;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_xpath_RelationalExpr__
