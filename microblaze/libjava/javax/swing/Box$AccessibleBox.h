
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_Box$AccessibleBox__
#define __javax_swing_Box$AccessibleBox__

#pragma interface

#include <java/awt/Container$AccessibleAWTContainer.h>
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
        class Box;
        class Box$AccessibleBox;
    }
  }
}

class javax::swing::Box$AccessibleBox : public ::java::awt::Container$AccessibleAWTContainer
{

public: // actually protected
  Box$AccessibleBox(::javax::swing::Box *);
public:
  virtual ::javax::accessibility::AccessibleRole * getAccessibleRole();
private:
  static const jlong serialVersionUID = -7775079816389931944LL;
public: // actually package-private
  ::javax::swing::Box * __attribute__((aligned(__alignof__( ::java::awt::Container$AccessibleAWTContainer)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_Box$AccessibleBox__
