
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_accessibility_AccessibleHyperlink__
#define __javax_accessibility_AccessibleHyperlink__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleHyperlink;
    }
  }
}

class javax::accessibility::AccessibleHyperlink : public ::java::lang::Object
{

public:
  AccessibleHyperlink();
  virtual jboolean isValid() = 0;
  virtual jint getAccessibleActionCount() = 0;
  virtual jboolean doAccessibleAction(jint) = 0;
  virtual ::java::lang::String * getAccessibleActionDescription(jint) = 0;
  virtual ::java::lang::Object * getAccessibleActionObject(jint) = 0;
  virtual ::java::lang::Object * getAccessibleActionAnchor(jint) = 0;
  virtual jint getStartIndex() = 0;
  virtual jint getEndIndex() = 0;
  static ::java::lang::Class class$;
};

#endif // __javax_accessibility_AccessibleHyperlink__
