
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_lang_reflect_TypeImpl__
#define __gnu_java_lang_reflect_TypeImpl__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace lang
      {
        namespace reflect
        {
            class TypeImpl;
        }
      }
    }
  }
}

class gnu::java::lang::reflect::TypeImpl : public ::java::lang::Object
{

public: // actually package-private
  TypeImpl();
  virtual ::java::lang::reflect::Type * resolve() = 0;
  static void resolve(JArray< ::java::lang::reflect::Type * > *);
  static ::java::lang::reflect::Type * resolve(::java::lang::reflect::Type *);
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_lang_reflect_TypeImpl__
