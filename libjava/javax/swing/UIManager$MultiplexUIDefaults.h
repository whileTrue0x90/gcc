
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_UIManager$MultiplexUIDefaults__
#define __javax_swing_UIManager$MultiplexUIDefaults__

#pragma interface

#include <javax/swing/UIDefaults.h>
extern "Java"
{
  namespace javax
  {
    namespace swing
    {
        class UIDefaults;
        class UIManager$MultiplexUIDefaults;
    }
  }
}

class javax::swing::UIManager$MultiplexUIDefaults : public ::javax::swing::UIDefaults
{

public: // actually package-private
  UIManager$MultiplexUIDefaults(::javax::swing::UIDefaults *);
public:
  virtual ::java::lang::Object * get(::java::lang::Object *);
  virtual ::java::lang::Object * get(::java::lang::Object *, ::java::util::Locale *);
  virtual ::java::lang::Object * remove(::java::lang::Object *);
  virtual jint size();
  virtual ::java::util::Enumeration * keys();
  virtual ::java::util::Enumeration * elements();
public: // actually package-private
  ::javax::swing::UIDefaults * __attribute__((aligned(__alignof__( ::javax::swing::UIDefaults)))) fallback;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_UIManager$MultiplexUIDefaults__
