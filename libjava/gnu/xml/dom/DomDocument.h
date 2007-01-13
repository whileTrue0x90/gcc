
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_xml_dom_DomDocument__
#define __gnu_xml_dom_DomDocument__

#pragma interface

#include <gnu/xml/dom/DomNode.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace xml
    {
      namespace dom
      {
          class DomDocument;
          class DomDocumentConfiguration;
          class DomNode;
      }
    }
  }
  namespace org
  {
    namespace w3c
    {
      namespace dom
      {
          class Attr;
          class CDATASection;
          class Comment;
          class DOMConfiguration;
          class DOMImplementation;
          class DocumentFragment;
          class DocumentType;
          class Element;
          class EntityReference;
          class Node;
          class ProcessingInstruction;
          class Text;
        namespace traversal
        {
            class NodeFilter;
            class NodeIterator;
            class TreeWalker;
        }
        namespace xpath
        {
            class XPathExpression;
            class XPathNSResolver;
        }
      }
    }
  }
}

class gnu::xml::dom::DomDocument : public ::gnu::xml::dom::DomNode
{

public:
  DomDocument();
public: // actually protected
  DomDocument(::org::w3c::dom::DOMImplementation *);
public:
  virtual void setBuilding(jboolean);
  virtual void setCheckWellformedness(jboolean);
  virtual void setCheckingCharacters(jboolean);
  virtual ::java::lang::String * getNodeName();
  virtual ::org::w3c::dom::Element * getDocumentElement();
  virtual ::org::w3c::dom::DocumentType * getDoctype();
  virtual ::org::w3c::dom::DOMImplementation * getImplementation();
  virtual ::org::w3c::dom::Element * getElementById(::java::lang::String *);
private:
  void checkNewChild(::org::w3c::dom::Node *);
public:
  virtual ::org::w3c::dom::Node * appendChild(::org::w3c::dom::Node *);
  virtual ::org::w3c::dom::Node * insertBefore(::org::w3c::dom::Node *, ::org::w3c::dom::Node *);
  virtual ::org::w3c::dom::Node * replaceChild(::org::w3c::dom::Node *, ::org::w3c::dom::Node *);
  static void verifyXmlName(::java::lang::String *);
public: // actually package-private
  static void checkName(::java::lang::String *, jboolean);
  static void checkNCName(::java::lang::String *, jboolean);
  static void checkChar(::java::lang::String *, jboolean);
  static void checkChar(JArray< jchar > *, jint, jint, jboolean);
public:
  virtual ::org::w3c::dom::Element * createElement(::java::lang::String *);
  virtual ::org::w3c::dom::Element * createElementNS(::java::lang::String *, ::java::lang::String *);
private:
  void defaultAttributes(::org::w3c::dom::Element *, ::java::lang::String *);
public:
  virtual ::org::w3c::dom::DocumentFragment * createDocumentFragment();
  virtual ::org::w3c::dom::Text * createTextNode(::java::lang::String *);
  virtual ::org::w3c::dom::Text * createTextNode(JArray< jchar > *, jint, jint);
  virtual ::org::w3c::dom::Comment * createComment(::java::lang::String *);
  virtual ::org::w3c::dom::CDATASection * createCDATASection(::java::lang::String *);
  virtual ::org::w3c::dom::CDATASection * createCDATASection(JArray< jchar > *, jint, jint);
  virtual ::org::w3c::dom::ProcessingInstruction * createProcessingInstruction(::java::lang::String *, ::java::lang::String *);
  virtual ::org::w3c::dom::Attr * createAttribute(::java::lang::String *);
  virtual ::org::w3c::dom::Attr * createAttributeNS(::java::lang::String *, ::java::lang::String *);
  virtual ::org::w3c::dom::EntityReference * createEntityReference(::java::lang::String *);
  virtual ::org::w3c::dom::Node * importNode(::org::w3c::dom::Node *, jboolean);
  virtual ::org::w3c::dom::traversal::NodeIterator * createNodeIterator(::org::w3c::dom::Node *, jint, ::org::w3c::dom::traversal::NodeFilter *, jboolean);
  virtual ::org::w3c::dom::traversal::TreeWalker * createTreeWalker(::org::w3c::dom::Node *, jint, ::org::w3c::dom::traversal::NodeFilter *, jboolean);
  virtual ::java::lang::String * getInputEncoding();
  virtual void setInputEncoding(::java::lang::String *);
  virtual ::java::lang::String * getXmlEncoding();
  virtual void setXmlEncoding(::java::lang::String *);
  virtual jboolean getXmlStandalone();
  virtual void setXmlStandalone(jboolean);
  virtual ::java::lang::String * getXmlVersion();
  virtual void setXmlVersion(::java::lang::String *);
  virtual jboolean getStrictErrorChecking();
  virtual void setStrictErrorChecking(jboolean);
  virtual ::java::lang::String * lookupPrefix(::java::lang::String *);
  virtual jboolean isDefaultNamespace(::java::lang::String *);
  virtual ::java::lang::String * lookupNamespaceURI(::java::lang::String *);
  virtual ::java::lang::String * getBaseURI();
  virtual ::java::lang::String * getDocumentURI();
  virtual void setDocumentURI(::java::lang::String *);
  virtual ::org::w3c::dom::Node * adoptNode(::org::w3c::dom::Node *);
public: // actually package-private
  virtual void adoptChildren(::org::w3c::dom::Node *, ::org::w3c::dom::Node *);
  virtual void adoptAttributes(::org::w3c::dom::Node *, ::org::w3c::dom::Node *);
public:
  virtual ::org::w3c::dom::DOMConfiguration * getDomConfig();
  virtual jboolean isEqualNode(::org::w3c::dom::Node *);
  virtual void normalizeDocument();
public: // actually package-private
  virtual void normalizeNode(::gnu::xml::dom::DomNode *);
public:
  virtual ::org::w3c::dom::Node * renameNode(::org::w3c::dom::Node *, ::java::lang::String *, ::java::lang::String *);
  virtual ::org::w3c::dom::xpath::XPathExpression * createExpression(::java::lang::String *, ::org::w3c::dom::xpath::XPathNSResolver *);
  virtual ::org::w3c::dom::xpath::XPathNSResolver * createNSResolver(::org::w3c::dom::Node *);
  virtual ::java::lang::Object * evaluate(::java::lang::String *, ::org::w3c::dom::Node *, ::org::w3c::dom::xpath::XPathNSResolver *, jshort, ::java::lang::Object *);
private:
  ::org::w3c::dom::DOMImplementation * __attribute__((aligned(__alignof__( ::gnu::xml::dom::DomNode)))) implementation;
  jboolean checkingCharacters;
public: // actually package-private
  jboolean checkingWellformedness;
  jboolean building;
  ::gnu::xml::dom::DomDocumentConfiguration * config;
  ::java::lang::String * inputEncoding;
  ::java::lang::String * encoding;
  ::java::lang::String * version;
  jboolean standalone;
  ::java::lang::String * systemId;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_xml_dom_DomDocument__
