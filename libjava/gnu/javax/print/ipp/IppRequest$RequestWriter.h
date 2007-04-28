
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_print_ipp_IppRequest$RequestWriter__
#define __gnu_javax_print_ipp_IppRequest$RequestWriter__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace print
      {
        namespace ipp
        {
            class IppRequest;
            class IppRequest$RequestWriter;
          namespace attribute
          {
              class CharsetSyntax;
              class NaturalLanguageSyntax;
              class RequestedAttributes;
          }
        }
      }
    }
  }
  namespace javax
  {
    namespace print
    {
      namespace attribute
      {
          class AttributeSet;
          class DateTimeSyntax;
          class EnumSyntax;
          class IntegerSyntax;
          class ResolutionSyntax;
          class SetOfIntegerSyntax;
          class TextSyntax;
          class URISyntax;
      }
    }
  }
}

class gnu::javax::print::ipp::IppRequest$RequestWriter : public ::java::lang::Object
{

public: // actually package-private
  IppRequest$RequestWriter(::gnu::javax::print::ipp::IppRequest *, ::java::io::DataOutputStream *);
private:
  void write(::javax::print::attribute::IntegerSyntax *);
  void write(::javax::print::attribute::EnumSyntax *);
  void write(::javax::print::attribute::SetOfIntegerSyntax *);
  void write(::javax::print::attribute::ResolutionSyntax *);
  void write(::javax::print::attribute::DateTimeSyntax *);
  void write(::javax::print::attribute::TextSyntax *);
  void write(::javax::print::attribute::URISyntax *);
  void write(::gnu::javax::print::ipp::attribute::CharsetSyntax *);
  void write(::gnu::javax::print::ipp::attribute::NaturalLanguageSyntax *);
  void write(::gnu::javax::print::ipp::attribute::RequestedAttributes *);
public:
  virtual void writeOperationAttributes(::javax::print::attribute::AttributeSet *);
  virtual void writeAttributes(::javax::print::attribute::AttributeSet *);
private:
  ::java::io::DataOutputStream * __attribute__((aligned(__alignof__( ::java::lang::Object)))) out;
public: // actually package-private
  ::gnu::javax::print::ipp::IppRequest * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_print_ipp_IppRequest$RequestWriter__
