
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_util_regex_RETokenLookBehind__
#define __gnu_java_util_regex_RETokenLookBehind__

#pragma interface

#include <gnu/java/util/regex/REToken.h>
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
            class CharIndexed;
            class REMatch;
            class REToken;
            class RETokenLookBehind;
        }
      }
    }
  }
}

class gnu::java::util::regex::RETokenLookBehind : public ::gnu::java::util::regex::REToken
{

public: // actually package-private
  RETokenLookBehind(::gnu::java::util::regex::REToken *, jboolean);
  jint getMaximumLength();
  ::gnu::java::util::regex::REMatch * matchThis(::gnu::java::util::regex::CharIndexed *, ::gnu::java::util::regex::REMatch *);
  void dump(::java::lang::StringBuffer *);
  ::gnu::java::util::regex::REToken * __attribute__((aligned(__alignof__( ::gnu::java::util::regex::REToken)))) re;
  jboolean negative;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_util_regex_RETokenLookBehind__
