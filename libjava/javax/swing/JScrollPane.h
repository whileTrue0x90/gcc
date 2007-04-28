
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_JScrollPane__
#define __javax_swing_JScrollPane__

#pragma interface

#include <javax/swing/JComponent.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Component;
        class ComponentOrientation;
        class LayoutManager;
        class Rectangle;
    }
  }
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleContext;
    }
    namespace swing
    {
        class JScrollBar;
        class JScrollPane;
        class JViewport;
      namespace border
      {
          class Border;
      }
      namespace plaf
      {
          class ScrollPaneUI;
      }
    }
  }
}

class javax::swing::JScrollPane : public ::javax::swing::JComponent
{

public:
  virtual ::javax::swing::JViewport * getColumnHeader();
  virtual ::java::awt::Component * getCorner(::java::lang::String *);
  virtual ::javax::swing::JScrollBar * getHorizontalScrollBar();
  virtual jint getHorizontalScrollBarPolicy();
  virtual ::javax::swing::JViewport * getRowHeader();
  virtual ::javax::swing::JScrollBar * getVerticalScrollBar();
  virtual jint getVerticalScrollBarPolicy();
  virtual ::javax::swing::JViewport * getViewport();
  virtual ::javax::swing::border::Border * getViewportBorder();
  virtual ::java::awt::Rectangle * getViewportBorderBounds();
  virtual jboolean isWheelScrollingEnabled();
private:
  void sync();
  void removeNonNull(::java::awt::Component *);
  void addNonNull(::java::awt::Component *, ::java::lang::Object *);
public:
  virtual void setComponentOrientation(::java::awt::ComponentOrientation *);
  virtual void setColumnHeader(::javax::swing::JViewport *);
  virtual void setColumnHeaderView(::java::awt::Component *);
  virtual void setCorner(::java::lang::String *, ::java::awt::Component *);
  virtual void setHorizontalScrollBar(::javax::swing::JScrollBar *);
  virtual void setHorizontalScrollBarPolicy(jint);
  virtual void setLayout(::java::awt::LayoutManager *);
  virtual void setRowHeader(::javax::swing::JViewport *);
  virtual void setRowHeaderView(::java::awt::Component *);
  virtual void setVerticalScrollBar(::javax::swing::JScrollBar *);
  virtual void setVerticalScrollBarPolicy(jint);
  virtual void setWheelScrollingEnabled(jboolean);
  virtual void setViewport(::javax::swing::JViewport *);
  virtual void setViewportBorder(::javax::swing::border::Border *);
  virtual void setViewportView(::java::awt::Component *);
  virtual jboolean isValidateRoot();
  JScrollPane();
  JScrollPane(::java::awt::Component *);
  JScrollPane(jint, jint);
  JScrollPane(::java::awt::Component *, jint, jint);
  virtual ::javax::swing::JScrollBar * createHorizontalScrollBar();
  virtual ::javax::swing::JScrollBar * createVerticalScrollBar();
public: // actually protected
  virtual ::javax::swing::JViewport * createViewport();
public:
  virtual ::java::lang::String * getUIClassID();
  virtual void updateUI();
  virtual ::javax::swing::plaf::ScrollPaneUI * getUI();
  virtual void setUI(::javax::swing::plaf::ScrollPaneUI *);
  virtual ::javax::accessibility::AccessibleContext * getAccessibleContext();
private:
  static const jlong serialVersionUID = 5203525440012340014LL;
public: // actually protected
  ::javax::swing::JViewport * __attribute__((aligned(__alignof__( ::javax::swing::JComponent)))) columnHeader;
  ::javax::swing::JViewport * rowHeader;
  ::java::awt::Component * lowerLeft;
  ::java::awt::Component * lowerRight;
  ::java::awt::Component * upperLeft;
  ::java::awt::Component * upperRight;
  ::javax::swing::JScrollBar * horizontalScrollBar;
  jint horizontalScrollBarPolicy;
  ::javax::swing::JScrollBar * verticalScrollBar;
  jint verticalScrollBarPolicy;
  ::javax::swing::JViewport * viewport;
private:
  ::javax::swing::border::Border * viewportBorder;
  jboolean wheelScrollingEnabled;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_JScrollPane__
