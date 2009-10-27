
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_regex_Pattern__
#define __java_util_regex_Pattern__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace util
      {
        namespace regex
        {
            class RE;
        }
      }
    }
  }
}

class java::util::regex::Pattern : public ::java::lang::Object
{

  Pattern(::java::lang::String *, jint);
public: // actually package-private
  ::gnu::java::util::regex::RE * getRE();
public:
  static ::java::util::regex::Pattern * compile(::java::lang::String *);
  static ::java::util::regex::Pattern * compile(::java::lang::String *, jint);
  jint flags();
  static jboolean matches(::java::lang::String *, ::java::lang::CharSequence *);
  ::java::util::regex::Matcher * matcher(::java::lang::CharSequence *);
  JArray< ::java::lang::String * > * split(::java::lang::CharSequence *);
  JArray< ::java::lang::String * > * split(::java::lang::CharSequence *, jint);
  ::java::lang::String * pattern();
  ::java::lang::String * toString();
private:
  static const jlong serialVersionUID = 5073258162644648461LL;
public:
  static const jint CANON_EQ = 128;
  static const jint CASE_INSENSITIVE = 2;
  static const jint COMMENTS = 4;
  static const jint DOTALL = 32;
  static const jint MULTILINE = 8;
  static const jint UNICODE_CASE = 64;
  static const jint UNIX_LINES = 1;
private:
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) regex;
  jint flags__;
  ::gnu::java::util::regex::RE * re;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_regex_Pattern__
