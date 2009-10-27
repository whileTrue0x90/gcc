
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_plaf_synth_SynthGraphicsUtils__
#define __javax_swing_plaf_synth_SynthGraphicsUtils__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Dimension;
        class Font;
        class FontMetrics;
        class Graphics;
        class Rectangle;
    }
  }
  namespace javax
  {
    namespace swing
    {
        class Icon;
      namespace plaf
      {
        namespace synth
        {
            class SynthContext;
            class SynthGraphicsUtils;
        }
      }
    }
  }
}

class javax::swing::plaf::synth::SynthGraphicsUtils : public ::java::lang::Object
{

public:
  SynthGraphicsUtils();
  virtual void drawLine(::javax::swing::plaf::synth::SynthContext *, ::java::lang::Object *, ::java::awt::Graphics *, jint, jint, jint, jint);
  virtual ::java::lang::String * layoutText(::javax::swing::plaf::synth::SynthContext *, ::java::awt::FontMetrics *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, ::java::awt::Rectangle *, ::java::awt::Rectangle *, ::java::awt::Rectangle *, jint);
  virtual jint computeStringWidth(::javax::swing::plaf::synth::SynthContext *, ::java::awt::Font *, ::java::awt::FontMetrics *, ::java::lang::String *);
  virtual ::java::awt::Dimension * getMinimumSize(::javax::swing::plaf::synth::SynthContext *, ::java::awt::Font *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, jint, jint);
  virtual ::java::awt::Dimension * getPreferredSize(::javax::swing::plaf::synth::SynthContext *, ::java::awt::Font *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, jint, jint);
  virtual ::java::awt::Dimension * getMaximumSize(::javax::swing::plaf::synth::SynthContext *, ::java::awt::Font *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, jint, jint);
  virtual jint getMaximumCharHeight(::javax::swing::plaf::synth::SynthContext *);
  virtual void paintText(::javax::swing::plaf::synth::SynthContext *, ::java::awt::Graphics *, ::java::lang::String *, ::java::awt::Rectangle *, jint);
  virtual void paintText(::javax::swing::plaf::synth::SynthContext *, ::java::awt::Graphics *, ::java::lang::String *, jint, jint, jint);
  virtual void paintText(::javax::swing::plaf::synth::SynthContext *, ::java::awt::Graphics *, ::java::lang::String *, ::javax::swing::Icon *, jint, jint, jint, jint, jint, jint, jint);
  static ::java::lang::Class class$;
};

#endif // __javax_swing_plaf_synth_SynthGraphicsUtils__
