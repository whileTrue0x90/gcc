
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_print_attribute_AttributeSetUtilities$UnmodifiableAttributeSet__
#define __javax_print_attribute_AttributeSetUtilities$UnmodifiableAttributeSet__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace print
    {
      namespace attribute
      {
          class Attribute;
          class AttributeSet;
          class AttributeSetUtilities$UnmodifiableAttributeSet;
      }
    }
  }
}

class javax::print::attribute::AttributeSetUtilities$UnmodifiableAttributeSet : public ::java::lang::Object
{

public:
  AttributeSetUtilities$UnmodifiableAttributeSet(::javax::print::attribute::AttributeSet *);
  virtual jboolean add(::javax::print::attribute::Attribute *);
  virtual jboolean addAll(::javax::print::attribute::AttributeSet *);
  virtual void clear();
  virtual jboolean containsKey(::java::lang::Class *);
  virtual jboolean containsValue(::javax::print::attribute::Attribute *);
  virtual jboolean equals(::java::lang::Object *);
  virtual ::javax::print::attribute::Attribute * get(::java::lang::Class *);
  virtual jint hashCode();
  virtual jboolean isEmpty();
  virtual jboolean remove(::java::lang::Class *);
  virtual jboolean remove(::javax::print::attribute::Attribute *);
  virtual jint size();
  virtual JArray< ::javax::print::attribute::Attribute * > * toArray();
private:
  ::javax::print::attribute::AttributeSet * __attribute__((aligned(__alignof__( ::java::lang::Object)))) attrset;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_print_attribute_AttributeSetUtilities$UnmodifiableAttributeSet__
