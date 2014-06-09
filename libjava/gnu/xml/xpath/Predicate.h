
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_xpath_Predicate__
#define __gnu_xml_xpath_Predicate__

#pragma interface

#include <gnu/xml/xpath/Test.h>
extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace xpath
      {
          class Expr;
          class Predicate;
          class Test;
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

class gnu::xml::xpath::Predicate : public ::gnu::xml::xpath::Test
{

public: // actually package-private
  Predicate(::gnu::xml::xpath::Expr *);
public:
  virtual jboolean matches(::org::w3c::dom::Node *, jint, jint);
  virtual ::gnu::xml::xpath::Test * clone(::java::lang::Object *);
  virtual jboolean references(::javax::xml::namespace$::QName *);
  virtual ::java::lang::String * toString();
public: // actually package-private
  ::gnu::xml::xpath::Expr * __attribute__((aligned(__alignof__( ::gnu::xml::xpath::Test)))) expr;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_xpath_Predicate__
