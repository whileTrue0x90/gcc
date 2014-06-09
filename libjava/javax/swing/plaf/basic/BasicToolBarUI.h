
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_basic_BasicToolBarUI__
#define __javax_swing_plaf_basic_BasicToolBarUI__

#pragma interface

#include <javax/swing/plaf/ToolBarUI.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Color;
        class Component;
        class Container;
        class Dimension;
        class Point;
        class Window;
      namespace event
      {
          class ContainerListener;
          class FocusListener;
          class WindowListener;
      }
    }
    namespace beans
    {
        class PropertyChangeListener;
    }
  }
  namespace javax
  {
    namespace swing
    {
        class ActionMap;
        class JComponent;
        class JFrame;
        class JToolBar;
        class KeyStroke;
        class RootPaneContainer;
      namespace border
      {
          class Border;
      }
      namespace event
      {
          class MouseInputListener;
      }
      namespace plaf
      {
          class ComponentUI;
        namespace basic
        {
            class BasicToolBarUI;
            class BasicToolBarUI$DragWindow;
        }
      }
    }
  }
}

class javax::swing::plaf::basic::BasicToolBarUI : public ::javax::swing::plaf::ToolBarUI
{

public:
  BasicToolBarUI();
  virtual jboolean canDock(::java::awt::Component *, ::java::awt::Point *);
private:
  jint areaOfClick(::java::awt::Component *, ::java::awt::Point *);
public: // actually protected
  virtual ::javax::swing::event::MouseInputListener * createDockingListener();
  virtual ::javax::swing::plaf::basic::BasicToolBarUI$DragWindow * createDragWindow(::javax::swing::JToolBar *);
  virtual ::javax::swing::JFrame * createFloatingFrame(::javax::swing::JToolBar *);
  virtual ::javax::swing::RootPaneContainer * createFloatingWindow(::javax::swing::JToolBar *);
  virtual ::java::awt::event::WindowListener * createFrameListener();
  virtual ::javax::swing::border::Border * createNonRolloverBorder();
  virtual ::java::beans::PropertyChangeListener * createPropertyListener();
  virtual ::javax::swing::border::Border * createRolloverBorder();
  virtual ::java::awt::event::ContainerListener * createToolBarContListener();
  virtual ::java::awt::event::FocusListener * createToolBarFocusListener();
public:
  static ::javax::swing::plaf::ComponentUI * createUI(::javax::swing::JComponent *);
public: // actually protected
  virtual void dragTo(::java::awt::Point *, ::java::awt::Point *);
  virtual void floatAt(::java::awt::Point *, ::java::awt::Point *);
public:
  virtual ::java::awt::Color * getDockingColor();
  virtual ::java::awt::Color * getFloatingColor();
  virtual ::java::awt::Dimension * getMaximumSize(::javax::swing::JComponent *);
  virtual ::java::awt::Dimension * getMinimumSize(::javax::swing::JComponent *);
public: // actually protected
  virtual void installComponents();
  virtual void installDefaults();
  virtual void installKeyboardActions();
private:
  ::javax::swing::ActionMap * getActionMap();
  ::javax::swing::ActionMap * createDefaultActions();
public: // actually protected
  virtual void installListeners();
  virtual void installNonRolloverBorders(::javax::swing::JComponent *);
  virtual void installNormalBorders(::javax::swing::JComponent *);
  virtual void installRolloverBorders(::javax::swing::JComponent *);
private:
  void fillHashtable();
public:
  virtual void installUI(::javax::swing::JComponent *);
  virtual jboolean isFloating();
  virtual jboolean isRolloverBorders();
public: // actually protected
  virtual void navigateFocusedComp(jint);
  virtual void setBorderToNonRollover(::java::awt::Component *);
  virtual void setBorderToNormal(::java::awt::Component *);
  virtual void setBorderToRollover(::java::awt::Component *);
public:
  virtual void setDockingColor(::java::awt::Color *);
  virtual void setFloating(jboolean, ::java::awt::Point *);
  virtual void setFloatingColor(::java::awt::Color *);
  virtual void setFloatingLocation(jint, jint);
  virtual void setOrientation(jint);
  virtual void setRolloverBorders(jboolean);
public: // actually protected
  virtual void uninstallComponents();
  virtual void uninstallDefaults();
  virtual void uninstallKeyboardActions();
  virtual void uninstallListeners();
public:
  virtual void uninstallUI(::javax::swing::JComponent *);
public: // actually package-private
  static ::javax::swing::JFrame * owner;
private:
  static ::javax::swing::border::Border * nonRolloverBorder;
  static ::javax::swing::border::Border * rolloverBorder;
public: // actually protected
  ::java::lang::String * __attribute__((aligned(__alignof__( ::javax::swing::plaf::ToolBarUI)))) constraintBeforeFloating;
public: // actually package-private
  jint lastGoodOrientation;
public: // actually protected
  ::java::awt::Color * dockingBorderColor;
  ::java::awt::Color * dockingColor;
  ::javax::swing::event::MouseInputListener * dockingListener;
  ::javax::swing::plaf::basic::BasicToolBarUI$DragWindow * dragWindow;
  ::java::awt::Color * floatingBorderColor;
  ::java::awt::Color * floatingColor;
  jint focusedCompIndex;
  ::java::beans::PropertyChangeListener * propertyListener;
  ::javax::swing::JToolBar * toolBar;
  ::java::awt::event::ContainerListener * toolBarContListener;
  ::java::awt::event::FocusListener * toolBarFocusListener;
  ::javax::swing::KeyStroke * leftKey;
  ::javax::swing::KeyStroke * rightKey;
  ::javax::swing::KeyStroke * upKey;
  ::javax::swing::KeyStroke * downKey;
private:
  ::java::awt::Window * floatFrame;
public: // actually package-private
  ::java::awt::Container * origParent;
  ::java::util::Hashtable * borders;
private:
  ::java::awt::event::WindowListener * windowListener;
public: // actually package-private
  ::java::awt::Dimension * cachedBounds;
  jint cachedOrientation;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_basic_BasicToolBarUI__
