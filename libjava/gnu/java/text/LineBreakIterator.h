
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_text_LineBreakIterator__
#define __gnu_java_text_LineBreakIterator__

#pragma interface

#include <gnu/java/text/BaseBreakIterator.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace text
      {
          class LineBreakIterator;
      }
    }
  }
}

class gnu::java::text::LineBreakIterator : public ::gnu::java::text::BaseBreakIterator
{

public:
  virtual ::java::lang::Object * clone();
  LineBreakIterator();
private:
  LineBreakIterator(::gnu::java::text::LineBreakIterator *);
  jboolean isNb(jchar);
  jboolean isClose(jint);
  jboolean isIdeo(jchar);
public:
  virtual jint next();
  virtual jint previous();
  static ::java::lang::Class class$;
};

#endif // __gnu_java_text_LineBreakIterator__
