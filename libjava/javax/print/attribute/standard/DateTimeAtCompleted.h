
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_print_attribute_standard_DateTimeAtCompleted__
#define __javax_print_attribute_standard_DateTimeAtCompleted__

#pragma interface

#include <javax/print/attribute/DateTimeSyntax.h>
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
            class DateTimeAtCompleted;
        }
      }
    }
  }
}

class javax::print::attribute::standard::DateTimeAtCompleted : public ::javax::print::attribute::DateTimeSyntax
{

public:
  DateTimeAtCompleted(::java::util::Date *);
  jboolean equals(::java::lang::Object *);
  ::java::lang::Class * getCategory();
  ::java::lang::String * getName();
private:
  static const jlong serialVersionUID = 6497399708058490000LL;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_print_attribute_standard_DateTimeAtCompleted__
