
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_JTextComponent$KeymapActionMap__
#define __javax_swing_text_JTextComponent$KeymapActionMap__

#pragma interface

#include <javax/swing/ActionMap.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace swing
    {
        class Action;
      namespace text
      {
          class JTextComponent;
          class JTextComponent$KeymapActionMap;
          class Keymap;
      }
    }
  }
}

class javax::swing::text::JTextComponent$KeymapActionMap : public ::javax::swing::ActionMap
{

public:
  JTextComponent$KeymapActionMap(::javax::swing::text::JTextComponent *, ::javax::swing::text::Keymap *);
  virtual ::javax::swing::Action * get(::java::lang::Object *);
  virtual jint size();
  virtual JArray< ::java::lang::Object * > * keys();
  virtual JArray< ::java::lang::Object * > * allKeys();
public: // actually package-private
  ::javax::swing::text::Keymap * __attribute__((aligned(__alignof__( ::javax::swing::ActionMap)))) map;
  ::javax::swing::text::JTextComponent * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_JTextComponent$KeymapActionMap__
