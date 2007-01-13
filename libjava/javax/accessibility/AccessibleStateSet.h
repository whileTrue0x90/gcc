
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_accessibility_AccessibleStateSet__
#define __javax_accessibility_AccessibleStateSet__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleState;
        class AccessibleStateSet;
    }
  }
}

class javax::accessibility::AccessibleStateSet : public ::java::lang::Object
{

public:
  AccessibleStateSet();
  AccessibleStateSet(JArray< ::javax::accessibility::AccessibleState * > *);
  virtual jboolean add(::javax::accessibility::AccessibleState *);
  virtual void addAll(JArray< ::javax::accessibility::AccessibleState * > *);
  virtual jboolean remove(::javax::accessibility::AccessibleState *);
  virtual void clear();
  virtual jboolean contains(::javax::accessibility::AccessibleState *);
  virtual JArray< ::javax::accessibility::AccessibleState * > * toArray();
  virtual ::java::lang::String * toString();
public: // actually protected
  ::java::util::Vector * __attribute__((aligned(__alignof__( ::java::lang::Object)))) states;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_accessibility_AccessibleStateSet__
