
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_util_EmptyEnumeration__
#define __gnu_java_util_EmptyEnumeration__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace util
      {
          class EmptyEnumeration;
      }
    }
  }
}

class gnu::java::util::EmptyEnumeration : public ::java::lang::Object
{

  EmptyEnumeration();
public:
  static ::gnu::java::util::EmptyEnumeration * getInstance();
  jboolean hasMoreElements();
  ::java::lang::Object * nextElement();
private:
  static ::gnu::java::util::EmptyEnumeration * instance;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_util_EmptyEnumeration__
