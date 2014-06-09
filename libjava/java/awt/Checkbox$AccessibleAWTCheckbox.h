
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_awt_Checkbox$AccessibleAWTCheckbox__
#define __java_awt_Checkbox$AccessibleAWTCheckbox__

#pragma interface

#include <java/awt/Component$AccessibleAWTComponent.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Checkbox;
        class Checkbox$AccessibleAWTCheckbox;
      namespace event
      {
          class ItemEvent;
      }
    }
  }
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleAction;
        class AccessibleRole;
        class AccessibleStateSet;
        class AccessibleValue;
    }
  }
}

class java::awt::Checkbox$AccessibleAWTCheckbox : public ::java::awt::Component$AccessibleAWTComponent
{

public:
  Checkbox$AccessibleAWTCheckbox(::java::awt::Checkbox *);
  virtual void itemStateChanged(::java::awt::event::ItemEvent *);
  virtual ::javax::accessibility::AccessibleAction * getAccessibleAction();
  virtual ::javax::accessibility::AccessibleValue * getAccessibleValue();
  virtual jint getAccessibleActionCount();
  virtual ::java::lang::String * getAccessibleActionDescription(jint);
  virtual jboolean doAccessibleAction(jint);
  virtual ::java::lang::Number * getCurrentAccessibleValue();
  virtual jboolean setCurrentAccessibleValue(::java::lang::Number *);
  virtual ::java::lang::Number * getMinimumAccessibleValue();
  virtual ::java::lang::Number * getMaximumAccessibleValue();
  virtual ::javax::accessibility::AccessibleRole * getAccessibleRole();
  virtual ::javax::accessibility::AccessibleStateSet * getAccessibleStateSet();
private:
  static const jlong serialVersionUID = 7881579233144754107LL;
public: // actually package-private
  ::java::awt::Checkbox * __attribute__((aligned(__alignof__( ::java::awt::Component$AccessibleAWTComponent)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __java_awt_Checkbox$AccessibleAWTCheckbox__
