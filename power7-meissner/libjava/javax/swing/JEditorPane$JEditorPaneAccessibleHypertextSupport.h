
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_JEditorPane$JEditorPaneAccessibleHypertextSupport__
#define __javax_swing_JEditorPane$JEditorPaneAccessibleHypertextSupport__

#pragma interface

#include <javax/swing/JEditorPane$AccessibleJEditorPane.h>
extern "Java"
{
  namespace javax
  {
    namespace accessibility
    {
        class AccessibleHyperlink;
    }
    namespace swing
    {
        class JEditorPane;
        class JEditorPane$JEditorPaneAccessibleHypertextSupport;
    }
  }
}

class javax::swing::JEditorPane$JEditorPaneAccessibleHypertextSupport : public ::javax::swing::JEditorPane$AccessibleJEditorPane
{

public:
  JEditorPane$JEditorPaneAccessibleHypertextSupport(::javax::swing::JEditorPane *);
  virtual jint getLinkCount();
  virtual ::javax::accessibility::AccessibleHyperlink * getLink(jint);
  virtual jint getLinkIndex(jint);
  virtual ::java::lang::String * getLinkText(jint);
public: // actually package-private
  static ::javax::swing::JEditorPane * access$0(::javax::swing::JEditorPane$JEditorPaneAccessibleHypertextSupport *);
  ::javax::swing::JEditorPane * __attribute__((aligned(__alignof__( ::javax::swing::JEditorPane$AccessibleJEditorPane)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_JEditorPane$JEditorPaneAccessibleHypertextSupport__
