
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_GlyphView$DefaultGlyphPainter__
#define __javax_swing_text_GlyphView$DefaultGlyphPainter__

#pragma interface

#include <javax/swing/text/GlyphView$GlyphPainter.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class FontMetrics;
        class Graphics;
        class Shape;
    }
  }
  namespace javax
  {
    namespace swing
    {
      namespace text
      {
          class GlyphView;
          class GlyphView$DefaultGlyphPainter;
          class Position$Bias;
          class TabExpander;
      }
    }
  }
}

class javax::swing::text::GlyphView$DefaultGlyphPainter : public ::javax::swing::text::GlyphView$GlyphPainter
{

public: // actually package-private
  GlyphView$DefaultGlyphPainter();
public:
  virtual jfloat getHeight(::javax::swing::text::GlyphView *);
  virtual void paint(::javax::swing::text::GlyphView *, ::java::awt::Graphics *, ::java::awt::Shape *, jint, jint);
  virtual ::java::awt::Shape * modelToView(::javax::swing::text::GlyphView *, jint, ::javax::swing::text::Position$Bias *, ::java::awt::Shape *);
  virtual jfloat getSpan(::javax::swing::text::GlyphView *, jint, jint, ::javax::swing::text::TabExpander *, jfloat);
  virtual jfloat getAscent(::javax::swing::text::GlyphView *);
  virtual jfloat getDescent(::javax::swing::text::GlyphView *);
  virtual jint getBoundedPosition(::javax::swing::text::GlyphView *, jint, jfloat, jfloat);
  virtual jint viewToModel(::javax::swing::text::GlyphView *, jfloat, jfloat, ::java::awt::Shape *, JArray< ::javax::swing::text::Position$Bias * > *);
private:
  void updateFontMetrics(::javax::swing::text::GlyphView *);
public: // actually package-private
  ::java::awt::FontMetrics * __attribute__((aligned(__alignof__( ::javax::swing::text::GlyphView$GlyphPainter)))) fontMetrics;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_GlyphView$DefaultGlyphPainter__
