
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_StringContent$StickyPosition__
#define __javax_swing_text_StringContent$StickyPosition__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace javax
  {
    namespace swing
    {
      namespace text
      {
          class StringContent;
          class StringContent$Mark;
          class StringContent$StickyPosition;
      }
    }
  }
}

class javax::swing::text::StringContent$StickyPosition : public ::java::lang::Object
{

public:
  StringContent$StickyPosition(::javax::swing::text::StringContent *, jint);
  virtual jint getOffset();
public: // actually package-private
  ::javax::swing::text::StringContent$Mark * __attribute__((aligned(__alignof__( ::java::lang::Object)))) mark;
  ::javax::swing::text::StringContent * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_StringContent$StickyPosition__
