
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_SwingUtilities__
#define __javax_swing_SwingUtilities__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Component;
        class Container;
        class FontMetrics;
        class Graphics;
        class Point;
        class Rectangle;
        class Window;
      namespace event
      {
          class KeyEvent;
          class MouseEvent;
      }
    }
  }
  namespace javax
  {
    namespace accessibility
    {
        class Accessible;
        class AccessibleStateSet;
    }
    namespace swing
    {
        class Action;
        class ActionMap;
        class Icon;
        class InputMap;
        class JComponent;
        class JRootPane;
        class KeyStroke;
        class SwingUtilities;
        class SwingUtilities$OwnerFrame;
    }
  }
}

class javax::swing::SwingUtilities : public ::java::lang::Object
{

  SwingUtilities();
public:
  static ::java::awt::Rectangle * calculateInnerArea(::javax::swing::JComponent *, ::java::awt::Rectangle *);
  static ::java::awt::Component * findFocusOwner(::java::awt::Component *);
  static ::javax::accessibility::Accessible * getAccessibleAt(::java::awt::Component *, ::java::awt::Point *);
  static ::javax::accessibility::Accessible * getAccessibleChild(::java::awt::Component *, jint);
  static jint getAccessibleChildrenCount(::java::awt::Component *);
  static jint getAccessibleIndexInParent(::java::awt::Component *);
  static ::javax::accessibility::AccessibleStateSet * getAccessibleStateSet(::java::awt::Component *);
  static ::java::awt::Rectangle * getLocalBounds(::java::awt::Component *);
  static ::javax::swing::JRootPane * getRootPane(::java::awt::Component *);
  static ::java::awt::Container * getAncestorNamed(::java::lang::String *, ::java::awt::Component *);
  static ::java::awt::Container * getAncestorOfClass(::java::lang::Class *, ::java::awt::Component *);
  static ::java::awt::Window * getWindowAncestor(::java::awt::Component *);
  static ::java::awt::Window * windowForComponent(::java::awt::Component *);
  static ::java::awt::Component * getRoot(::java::awt::Component *);
  static jboolean isDescendingFrom(::java::awt::Component *, ::java::awt::Component *);
  static ::java::awt::Component * getDeepestComponentAt(::java::awt::Component *, jint, jint);
  static void convertPointToScreen(::java::awt::Point *, ::java::awt::Component *);
  static void convertPointFromScreen(::java::awt::Point *, ::java::awt::Component *);
  static ::java::awt::Point * convertPoint(::java::awt::Component *, jint, jint, ::java::awt::Component *);
  static ::java::awt::Point * convertPoint(::java::awt::Component *, ::java::awt::Point *, ::java::awt::Component *);
  static ::java::awt::Rectangle * convertRectangle(::java::awt::Component *, ::java::awt::Rectangle *, ::java::awt::Component *);
  static ::java::awt::event::MouseEvent * convertMouseEvent(::java::awt::Component *, ::java::awt::event::MouseEvent *, ::java::awt::Component *);
  static void updateComponentTreeUI(::java::awt::Component *);
private:
  static void updateComponentTreeUIImpl(::java::awt::Component *);
public:
  static ::java::lang::String * layoutCompoundLabel(::javax::swing::JComponent *, ::java::awt::FontMetrics *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, ::java::awt::Rectangle *, ::java::awt::Rectangle *, ::java::awt::Rectangle *, jint);
  static ::java::lang::String * layoutCompoundLabel(::java::awt::FontMetrics *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, ::java::awt::Rectangle *, ::java::awt::Rectangle *, ::java::awt::Rectangle *, jint);
private:
  static ::java::lang::String * layoutCompoundLabelImpl(::javax::swing::JComponent *, ::java::awt::FontMetrics *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, ::java::awt::Rectangle *, ::java::awt::Rectangle *, ::java::awt::Rectangle *, jint);
  static ::java::lang::String * clipString(::javax::swing::JComponent *, ::java::awt::FontMetrics *, ::java::lang::String *, jint);
public:
  static void invokeLater(::java::lang::Runnable *);
  static void invokeAndWait(::java::lang::Runnable *);
  static jboolean isEventDispatchThread();
  static void paintComponent(::java::awt::Graphics *, ::java::awt::Component *, ::java::awt::Container *, jint, jint, jint, jint);
  static void paintComponent(::java::awt::Graphics *, ::java::awt::Component *, ::java::awt::Container *, ::java::awt::Rectangle *);
public: // actually package-private
  static ::java::awt::Window * getOwnerFrame(::java::awt::Window *);
public:
  static jboolean isLeftMouseButton(::java::awt::event::MouseEvent *);
  static jboolean isMiddleMouseButton(::java::awt::event::MouseEvent *);
  static jboolean isRightMouseButton(::java::awt::event::MouseEvent *);
  static jboolean notifyAction(::javax::swing::Action *, ::javax::swing::KeyStroke *, ::java::awt::event::KeyEvent *, ::java::lang::Object *, jint);
  static void replaceUIActionMap(::javax::swing::JComponent *, ::javax::swing::ActionMap *);
  static void replaceUIInputMap(::javax::swing::JComponent *, jint, ::javax::swing::InputMap *);
  static JArray< ::java::awt::Rectangle * > * computeDifference(::java::awt::Rectangle *, ::java::awt::Rectangle *);
  static ::java::awt::Rectangle * computeIntersection(jint, jint, jint, jint, ::java::awt::Rectangle *);
  static jint computeStringWidth(::java::awt::FontMetrics *, ::java::lang::String *);
  static ::java::awt::Rectangle * computeUnion(jint, jint, jint, jint, ::java::awt::Rectangle *);
  static jboolean isRectangleContainingRectangle(::java::awt::Rectangle *, ::java::awt::Rectangle *);
  static ::javax::swing::InputMap * getUIInputMap(::javax::swing::JComponent *, jint);
  static ::javax::swing::ActionMap * getUIActionMap(::javax::swing::JComponent *);
  static jboolean processKeyBindings(::java::awt::event::KeyEvent *);
public: // actually package-private
  static ::java::lang::String * convertHorizontalAlignmentCodeToString(jint);
  static ::java::lang::String * convertVerticalAlignmentCodeToString(jint);
  static ::java::lang::String * convertWindowConstantToString(jint);
  static void convertRectangleToAncestor(::java::awt::Component *, ::java::awt::Rectangle *, ::java::awt::Component *);
private:
  static ::javax::swing::SwingUtilities$OwnerFrame * ownerFrame;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_SwingUtilities__
