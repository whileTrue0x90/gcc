
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_w3c_dom_xpath_XPathResult__
#define __org_w3c_dom_xpath_XPathResult__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
          class Node;
        namespace xpath
        {
            class XPathResult;
        }
      }
    }
  }
}

class org::w3c::dom::xpath::XPathResult : public ::java::lang::Object
{

public:
  virtual jshort getResultType() = 0;
  virtual jdouble getNumberValue() = 0;
  virtual ::java::lang::String * getStringValue() = 0;
  virtual jboolean getBooleanValue() = 0;
  virtual ::org::w3c::dom::Node * getSingleNodeValue() = 0;
  virtual jboolean getInvalidIteratorState() = 0;
  virtual jint getSnapshotLength() = 0;
  virtual ::org::w3c::dom::Node * iterateNext() = 0;
  virtual ::org::w3c::dom::Node * snapshotItem(jint) = 0;
  static const jshort ANY_TYPE = 0;
  static const jshort NUMBER_TYPE = 1;
  static const jshort STRING_TYPE = 2;
  static const jshort BOOLEAN_TYPE = 3;
  static const jshort UNORDERED_NODE_ITERATOR_TYPE = 4;
  static const jshort ORDERED_NODE_ITERATOR_TYPE = 5;
  static const jshort UNORDERED_NODE_SNAPSHOT_TYPE = 6;
  static const jshort ORDERED_NODE_SNAPSHOT_TYPE = 7;
  static const jshort ANY_UNORDERED_NODE_TYPE = 8;
  static const jshort FIRST_ORDERED_NODE_TYPE = 9;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __org_w3c_dom_xpath_XPathResult__
