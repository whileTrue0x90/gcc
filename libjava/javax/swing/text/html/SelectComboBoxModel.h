
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_text_html_SelectComboBoxModel__
#define __javax_swing_text_html_SelectComboBoxModel__

#pragma interface

#include <javax/swing/DefaultComboBoxModel.h>
extern "Java"
{
  namespace javax
  {
    namespace swing
    {
      namespace text
      {
        namespace html
        {
            class Option;
            class SelectComboBoxModel;
        }
      }
    }
  }
}

class javax::swing::text::html::SelectComboBoxModel : public ::javax::swing::DefaultComboBoxModel
{

public: // actually package-private
  SelectComboBoxModel();
  virtual void setInitialSelection(::javax::swing::text::html::Option *);
  virtual ::javax::swing::text::html::Option * getInitialSelection();
public:
  virtual void reset();
private:
  ::javax::swing::text::html::Option * __attribute__((aligned(__alignof__( ::javax::swing::DefaultComboBoxModel)))) initial;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_text_html_SelectComboBoxModel__
