
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __org_w3c_dom_traversal_NodeFilter__
#define __org_w3c_dom_traversal_NodeFilter__

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
        namespace traversal
        {
            class NodeFilter;
        }
      }
    }
  }
}

class org::w3c::dom::traversal::NodeFilter : public ::java::lang::Object
{

public:
  virtual jshort acceptNode(::org::w3c::dom::Node *) = 0;
  static const jshort FILTER_ACCEPT = 1;
  static const jshort FILTER_REJECT = 2;
  static const jshort FILTER_SKIP = 3;
  static const jint SHOW_ALL = -1;
  static const jint SHOW_ELEMENT = 1;
  static const jint SHOW_ATTRIBUTE = 2;
  static const jint SHOW_TEXT = 4;
  static const jint SHOW_CDATA_SECTION = 8;
  static const jint SHOW_ENTITY_REFERENCE = 16;
  static const jint SHOW_ENTITY = 32;
  static const jint SHOW_PROCESSING_INSTRUCTION = 64;
  static const jint SHOW_COMMENT = 128;
  static const jint SHOW_DOCUMENT = 256;
  static const jint SHOW_DOCUMENT_TYPE = 512;
  static const jint SHOW_DOCUMENT_FRAGMENT = 1024;
  static const jint SHOW_NOTATION = 2048;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __org_w3c_dom_traversal_NodeFilter__
