
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_JRootPane$AccessibleJRootPane__
#define __javax_swing_JRootPane$AccessibleJRootPane__

#pragma interface

#include <javax/swing/JComponent$AccessibleJComponent.h>
extern "Java"
{
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleRole;
    }
    namespace swing
    {
        class JRootPane;
        class JRootPane$AccessibleJRootPane;
    }
  }
}

class javax::swing::JRootPane$AccessibleJRootPane : public ::javax::swing::JComponent$AccessibleJComponent
{

public: // actually protected
  JRootPane$AccessibleJRootPane(::javax::swing::JRootPane *);
public:
  virtual ::javax::accessibility::AccessibleRole * getAccessibleRole();
private:
  static const jlong serialVersionUID = 1082432482784468088LL;
public: // actually package-private
  ::javax::swing::JRootPane * __attribute__((aligned(__alignof__( ::javax::swing::JComponent$AccessibleJComponent)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_JRootPane$AccessibleJRootPane__
