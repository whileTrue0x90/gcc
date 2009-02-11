
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_ComponentUI__
#define __javax_swing_plaf_ComponentUI__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Dimension;
        class Graphics;
    }
  }
  namespace javax
  {
    namespace accessibility
    {
        class Accessible;
    }
    namespace swing
    {
        class JComponent;
      namespace plaf
      {
          class ComponentUI;
      }
    }
  }
}

class javax::swing::plaf::ComponentUI : public ::java::lang::Object
{

public:
  ComponentUI();
  virtual void installUI(::javax::swing::JComponent *);
  virtual void uninstallUI(::javax::swing::JComponent *);
  virtual void paint(::java::awt::Graphics *, ::javax::swing::JComponent *);
  virtual void update(::java::awt::Graphics *, ::javax::swing::JComponent *);
  virtual ::java::awt::Dimension * getPreferredSize(::javax::swing::JComponent *);
  virtual ::java::awt::Dimension * getMinimumSize(::javax::swing::JComponent *);
  virtual ::java::awt::Dimension * getMaximumSize(::javax::swing::JComponent *);
  virtual jboolean contains(::javax::swing::JComponent *, jint, jint);
  static ::javax::swing::plaf::ComponentUI * createUI(::javax::swing::JComponent *);
  virtual jint getAccessibleChildrenCount(::javax::swing::JComponent *);
  virtual ::javax::accessibility::Accessible * getAccessibleChild(::javax::swing::JComponent *, jint);
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_ComponentUI__
