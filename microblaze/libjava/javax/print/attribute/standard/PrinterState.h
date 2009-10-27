
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_print_attribute_standard_PrinterState__
#define __javax_print_attribute_standard_PrinterState__

#pragma interface

#include <javax/print/attribute/EnumSyntax.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace print
    {
      namespace attribute
      {
          class EnumSyntax;
        namespace standard
        {
            class PrinterState;
        }
      }
    }
  }
}

class javax::print::attribute::standard::PrinterState : public ::javax::print::attribute::EnumSyntax
{

public: // actually protected
  PrinterState(jint);
public:
  ::java::lang::Class * getCategory();
  ::java::lang::String * getName();
public: // actually protected
  JArray< ::java::lang::String * > * getStringTable();
  JArray< ::javax::print::attribute::EnumSyntax * > * getEnumValueTable();
private:
  static const jlong serialVersionUID = -649578618346507718LL;
public:
  static ::javax::print::attribute::standard::PrinterState * UNKNOWN;
  static ::javax::print::attribute::standard::PrinterState * IDLE;
  static ::javax::print::attribute::standard::PrinterState * PROCESSING;
  static ::javax::print::attribute::standard::PrinterState * STOPPED;
private:
  static JArray< ::java::lang::String * > * stringTable;
  static JArray< ::javax::print::attribute::standard::PrinterState * > * enumValueTable;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_print_attribute_standard_PrinterState__
