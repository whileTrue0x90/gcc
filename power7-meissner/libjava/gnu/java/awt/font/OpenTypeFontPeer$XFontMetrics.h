
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_font_OpenTypeFontPeer$XFontMetrics__
#define __gnu_java_awt_font_OpenTypeFontPeer$XFontMetrics__

#pragma interface

#include <java/awt/FontMetrics.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace awt
      {
        namespace font
        {
            class OpenTypeFontPeer;
            class OpenTypeFontPeer$XFontMetrics;
        }
      }
    }
  }
  namespace java
  {
    namespace awt
    {
        class Font;
      namespace geom
      {
          class Point2D;
      }
    }
  }
}

class gnu::java::awt::font::OpenTypeFontPeer$XFontMetrics : public ::java::awt::FontMetrics
{

public: // actually package-private
  OpenTypeFontPeer$XFontMetrics(::gnu::java::awt::font::OpenTypeFontPeer *, ::java::awt::Font *);
public:
  virtual jint getAscent();
  virtual jint getDescent();
  virtual jint getHeight();
  virtual jint charWidth(jchar);
  virtual jint charsWidth(JArray< jchar > *, jint, jint);
  virtual jint stringWidth(::java::lang::String *);
private:
  ::java::awt::geom::Point2D * __attribute__((aligned(__alignof__( ::java::awt::FontMetrics)))) cachedPoint;
public: // actually package-private
  ::gnu::java::awt::font::OpenTypeFontPeer * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_font_OpenTypeFontPeer$XFontMetrics__
