
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_StyleContext$NamedStyle__
#define __javax_swing_text_StyleContext$NamedStyle__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace swing
    {
      namespace event
      {
          class ChangeEvent;
          class ChangeListener;
          class EventListenerList;
      }
      namespace text
      {
          class AttributeSet;
          class Style;
          class StyleContext;
          class StyleContext$NamedStyle;
      }
    }
  }
}

class javax::swing::text::StyleContext$NamedStyle : public ::java::lang::Object
{

public:
  StyleContext$NamedStyle(::javax::swing::text::StyleContext *);
  StyleContext$NamedStyle(::javax::swing::text::StyleContext *, ::javax::swing::text::Style *);
  StyleContext$NamedStyle(::javax::swing::text::StyleContext *, ::java::lang::String *, ::javax::swing::text::Style *);
  virtual ::java::lang::String * getName();
  virtual void setName(::java::lang::String *);
  virtual void addChangeListener(::javax::swing::event::ChangeListener *);
  virtual void removeChangeListener(::javax::swing::event::ChangeListener *);
  virtual JArray< ::java::util::EventListener * > * getListeners(::java::lang::Class *);
  virtual JArray< ::javax::swing::event::ChangeListener * > * getChangeListeners();
public: // actually protected
  virtual void fireStateChanged();
public:
  virtual void addAttribute(::java::lang::Object *, ::java::lang::Object *);
  virtual void addAttributes(::javax::swing::text::AttributeSet *);
  virtual jboolean containsAttribute(::java::lang::Object *, ::java::lang::Object *);
  virtual jboolean containsAttributes(::javax::swing::text::AttributeSet *);
  virtual ::javax::swing::text::AttributeSet * copyAttributes();
  virtual ::java::lang::Object * getAttribute(::java::lang::Object *);
  virtual jint getAttributeCount();
  virtual ::java::util::Enumeration * getAttributeNames();
  virtual jboolean isDefined(::java::lang::Object *);
  virtual jboolean isEqual(::javax::swing::text::AttributeSet *);
  virtual void removeAttribute(::java::lang::Object *);
  virtual void removeAttributes(::javax::swing::text::AttributeSet *);
  virtual void removeAttributes(::java::util::Enumeration *);
  virtual ::javax::swing::text::AttributeSet * getResolveParent();
  virtual void setResolveParent(::javax::swing::text::AttributeSet *);
  virtual ::java::lang::String * toString();
private:
  void writeObject(::java::io::ObjectOutputStream *);
  void readObject(::java::io::ObjectInputStream *);
  static const jlong serialVersionUID = -6690628971806226374LL;
public: // actually protected
  ::javax::swing::event::ChangeEvent * __attribute__((aligned(__alignof__( ::java::lang::Object)))) changeEvent;
  ::javax::swing::event::EventListenerList * listenerList;
private:
  ::javax::swing::text::AttributeSet * attributes;
public: // actually package-private
  ::javax::swing::text::StyleContext * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_StyleContext$NamedStyle__
