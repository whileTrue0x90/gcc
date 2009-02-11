
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_GlyphView__
#define __javax_swing_text_GlyphView__

#pragma interface

#include <javax/swing/text/View.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Color;
        class Font;
        class Graphics;
        class Shape;
    }
  }
  namespace javax
  {
    namespace swing
    {
      namespace event
      {
          class DocumentEvent;
      }
      namespace text
      {
          class Element;
          class GlyphView;
          class GlyphView$GlyphPainter;
          class Position$Bias;
          class Segment;
          class TabExpander;
          class View;
          class ViewFactory;
      }
    }
  }
}

class javax::swing::text::GlyphView : public ::javax::swing::text::View
{

public:
  GlyphView(::javax::swing::text::Element *);
  virtual ::javax::swing::text::GlyphView$GlyphPainter * getGlyphPainter();
  virtual void setGlyphPainter(::javax::swing::text::GlyphView$GlyphPainter *);
public: // actually protected
  virtual void checkPainter();
public:
  virtual void paint(::java::awt::Graphics *, ::java::awt::Shape *);
  virtual jfloat getPreferredSpan(jint);
  virtual ::java::awt::Shape * modelToView(jint, ::java::awt::Shape *, ::javax::swing::text::Position$Bias *);
  virtual jint viewToModel(jfloat, jfloat, ::java::awt::Shape *, JArray< ::javax::swing::text::Position$Bias * > *);
  virtual ::javax::swing::text::TabExpander * getTabExpander();
  virtual jfloat getTabbedSpan(jfloat, ::javax::swing::text::TabExpander *);
  virtual jfloat getPartialSpan(jint, jint);
  virtual jint getStartOffset();
  virtual jint getEndOffset();
  virtual ::javax::swing::text::Segment * getText(jint, jint);
  virtual ::java::awt::Font * getFont();
  virtual ::java::awt::Color * getForeground();
  virtual ::java::awt::Color * getBackground();
  virtual jboolean isStrikeThrough();
  virtual jboolean isSubscript();
  virtual jboolean isSuperscript();
  virtual jboolean isUnderline();
public: // actually protected
  virtual ::java::lang::Object * clone();
public:
  virtual ::javax::swing::text::View * breakView(jint, jint, jfloat, jfloat);
  virtual jint getBreakWeight(jint, jfloat, jfloat);
private:
  jint getBreakLocation(jint, jint);
public:
  virtual void changedUpdate(::javax::swing::event::DocumentEvent *, ::java::awt::Shape *, ::javax::swing::text::ViewFactory *);
  virtual void insertUpdate(::javax::swing::event::DocumentEvent *, ::java::awt::Shape *, ::javax::swing::text::ViewFactory *);
  virtual void removeUpdate(::javax::swing::event::DocumentEvent *, ::java::awt::Shape *, ::javax::swing::text::ViewFactory *);
  virtual ::javax::swing::text::View * createFragment(jint, jint);
  virtual jfloat getAlignment(jint);
  virtual jint getNextVisualPositionFrom(jint, ::javax::swing::text::Position$Bias *, ::java::awt::Shape *, jint, JArray< ::javax::swing::text::Position$Bias * > *);
public: // actually package-private
  ::javax::swing::text::GlyphView$GlyphPainter * __attribute__((aligned(__alignof__( ::javax::swing::text::View)))) glyphPainter;
private:
  jint offset;
  jint length;
  jfloat tabX;
  ::javax::swing::text::TabExpander * tabExpander;
  ::javax::swing::text::Segment * cached;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_GlyphView__
