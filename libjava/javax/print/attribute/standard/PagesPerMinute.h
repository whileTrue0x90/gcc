
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_print_attribute_standard_PagesPerMinute__
#define __javax_print_attribute_standard_PagesPerMinute__

#pragma interface

#include <javax/print/attribute/IntegerSyntax.h>
extern "Java"
{
  namespace javax
  {
    namespace print
    {
      namespace attribute
      {
        namespace standard
        {
            class PagesPerMinute;
        }
      }
    }
  }
}

class javax::print::attribute::standard::PagesPerMinute : public ::javax::print::attribute::IntegerSyntax
{

public:
  PagesPerMinute(jint);
  jboolean equals(::java::lang::Object *);
  ::java::lang::Class * getCategory();
  ::java::lang::String * getName();
private:
  static const jlong serialVersionUID = -6366403993072862015LL;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_print_attribute_standard_PagesPerMinute__
