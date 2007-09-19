
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_accessibility_AccessibleEditableText__
#define __javax_accessibility_AccessibleEditableText__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
        class Point;
        class Rectangle;
    }
  }
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleEditableText;
    }
    namespace swing
    {
      namespace text
      {
          class AttributeSet;
      }
    }
  }
}

class javax::accessibility::AccessibleEditableText : public ::java::lang::Object
{

public:
  virtual void setTextContents(::java::lang::String *) = 0;
  virtual void insertTextAtIndex(jint, ::java::lang::String *) = 0;
  virtual ::java::lang::String * getTextRange(jint, jint) = 0;
  virtual void delete$(jint, jint) = 0;
  virtual void cut(jint, jint) = 0;
  virtual void paste(jint) = 0;
  virtual void replaceText(jint, jint, ::java::lang::String *) = 0;
  virtual void selectText(jint, jint) = 0;
  virtual void setAttributes(jint, jint, ::javax::swing::text::AttributeSet *) = 0;
  virtual jint getIndexAtPoint(::java::awt::Point *) = 0;
  virtual ::java::awt::Rectangle * getCharacterBounds(jint) = 0;
  virtual jint getCharCount() = 0;
  virtual jint getCaretPosition() = 0;
  virtual ::java::lang::String * getAtIndex(jint, jint) = 0;
  virtual ::java::lang::String * getAfterIndex(jint, jint) = 0;
  virtual ::java::lang::String * getBeforeIndex(jint, jint) = 0;
  virtual ::javax::swing::text::AttributeSet * getCharacterAttribute(jint) = 0;
  virtual jint getSelectionStart() = 0;
  virtual jint getSelectionEnd() = 0;
  virtual ::java::lang::String * getSelectedText() = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_accessibility_AccessibleEditableText__
