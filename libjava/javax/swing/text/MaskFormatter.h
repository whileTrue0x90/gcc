
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_MaskFormatter__
#define __javax_swing_text_MaskFormatter__

#pragma interface

#include <javax/swing/text/DefaultFormatter.h>
extern "Java"
{
  namespace javax
  {
    namespace swing
    {
        class JFormattedTextField;
      namespace text
      {
          class MaskFormatter;
      }
    }
  }
}

class javax::swing::text::MaskFormatter : public ::javax::swing::text::DefaultFormatter
{

public:
  MaskFormatter();
  MaskFormatter(::java::lang::String *);
  virtual ::java::lang::String * getMask();
  virtual ::java::lang::String * getInvalidCharacters();
  virtual void setInvalidCharacters(::java::lang::String *);
  virtual ::java::lang::String * getValidCharacters();
  virtual void setValidCharacters(::java::lang::String *);
  virtual ::java::lang::String * getPlaceholder();
  virtual void setPlaceholder(::java::lang::String *);
  virtual jchar getPlaceholderCharacter();
  virtual void setPlaceholderCharacter(jchar);
  virtual jboolean getValueContainsLiteralCharacters();
  virtual void setValueContainsLiteralCharacters(jboolean);
  virtual void setMask(::java::lang::String *);
  virtual void install(::javax::swing::JFormattedTextField *);
  virtual ::java::lang::Object * stringToValue(::java::lang::String *);
private:
  ::java::lang::String * convertStringToValue(::java::lang::String *);
public:
  virtual ::java::lang::String * valueToString(::java::lang::Object *);
private:
  ::java::lang::String * convertValueToString(::java::lang::String *);
  static const jchar NUM_CHAR = 35;
  static const jchar ESCAPE_CHAR = 39;
  static const jchar UPPERCASE_CHAR = 85;
  static const jchar LOWERCASE_CHAR = 76;
  static const jchar ALPHANUM_CHAR = 65;
  static const jchar LETTER_CHAR = 63;
  static const jchar ANYTHING_CHAR = 42;
  static const jchar HEX_CHAR = 72;
  ::java::lang::String * __attribute__((aligned(__alignof__( ::javax::swing::text::DefaultFormatter)))) mask;
  ::java::lang::String * invalidChars;
  ::java::lang::String * validChars;
  ::java::lang::String * placeHolder;
  jchar placeHolderChar;
  jboolean valueContainsLiteralCharacters;
  static ::java::lang::String * hexString;
public: // actually package-private
  jint maskLength;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_MaskFormatter__
