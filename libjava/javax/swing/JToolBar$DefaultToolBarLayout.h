
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_JToolBar$DefaultToolBarLayout__
#define __javax_swing_JToolBar$DefaultToolBarLayout__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Component;
        class Container;
        class Dimension;
    }
  }
  namespace javax
  {
    namespace swing
    {
        class JToolBar;
        class JToolBar$DefaultToolBarLayout;
    }
  }
}

class javax::swing::JToolBar$DefaultToolBarLayout : public ::java::lang::Object
{

  JToolBar$DefaultToolBarLayout(::javax::swing::JToolBar *);
public:
  virtual void addLayoutComponent(::java::lang::String *, ::java::awt::Component *);
  virtual void layoutContainer(::java::awt::Container *);
  virtual ::java::awt::Dimension * minimumLayoutSize(::java::awt::Container *);
  virtual ::java::awt::Dimension * preferredLayoutSize(::java::awt::Container *);
  virtual void removeLayoutComponent(::java::awt::Component *);
public: // actually package-private
  JToolBar$DefaultToolBarLayout(::javax::swing::JToolBar *, ::javax::swing::JToolBar$DefaultToolBarLayout *);
  ::javax::swing::JToolBar * __attribute__((aligned(__alignof__( ::java::lang::Object)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_JToolBar$DefaultToolBarLayout__
