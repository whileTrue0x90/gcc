
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_naming_directory_BasicAttributes$BasicAttributesEnumeration__
#define __javax_naming_directory_BasicAttributes$BasicAttributesEnumeration__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace naming
    {
      namespace directory
      {
          class Attribute;
          class BasicAttributes;
          class BasicAttributes$BasicAttributesEnumeration;
      }
    }
  }
}

class javax::naming::directory::BasicAttributes$BasicAttributesEnumeration : public ::java::lang::Object
{

public:
  BasicAttributes$BasicAttributesEnumeration(::javax::naming::directory::BasicAttributes *);
  virtual void close();
  virtual jboolean hasMore();
  virtual ::javax::naming::directory::Attribute * BasicAttributes$BasicAttributesEnumeration$next();
  virtual jboolean hasMoreElements();
  virtual ::javax::naming::directory::Attribute * BasicAttributes$BasicAttributesEnumeration$nextElement();
  virtual ::java::lang::Object * next();
  virtual ::java::lang::Object * nextElement();
public: // actually package-private
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) where;
  ::javax::naming::directory::BasicAttributes * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_naming_directory_BasicAttributes$BasicAttributesEnumeration__
