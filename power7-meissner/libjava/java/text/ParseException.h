
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_text_ParseException__
#define __java_text_ParseException__

#pragma interface

#include <java/lang/Exception.h>
extern "Java"
{
  namespace java
  {
    namespace text
    {
        class ParseException;
    }
  }
}

class java::text::ParseException : public ::java::lang::Exception
{

public:
  ParseException(::java::lang::String *, jint);
  virtual jint getErrorOffset();
private:
  static const jlong serialVersionUID = 2703218443322787634LL;
  jint __attribute__((aligned(__alignof__( ::java::lang::Exception)))) errorOffset;
public:
  static ::java::lang::Class class$;
};

#endif // __java_text_ParseException__
