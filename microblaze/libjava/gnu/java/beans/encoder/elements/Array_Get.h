
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_beans_encoder_elements_Array_Get__
#define __gnu_java_beans_encoder_elements_Array_Get__

#pragma interface

#include <gnu/java/beans/encoder/elements/Element.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace beans
      {
        namespace encoder
        {
            class Writer;
          namespace elements
          {
              class Array_Get;
          }
        }
      }
    }
  }
}

class gnu::java::beans::encoder::elements::Array_Get : public ::gnu::java::beans::encoder::elements::Element
{

public:
  Array_Get(::java::lang::String *);
  virtual void writeStart(::gnu::java::beans::encoder::Writer *);
public: // actually package-private
  ::java::lang::String * __attribute__((aligned(__alignof__( ::gnu::java::beans::encoder::elements::Element)))) indexAsString;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_beans_encoder_elements_Array_Get__
