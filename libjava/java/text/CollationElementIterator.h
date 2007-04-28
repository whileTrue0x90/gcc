
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_text_CollationElementIterator__
#define __java_text_CollationElementIterator__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace java
  {
    namespace text
    {
        class CharacterIterator;
        class CollationElementIterator;
        class RuleBasedCollator;
        class RuleBasedCollator$CollationElement;
    }
  }
}

class java::text::CollationElementIterator : public ::java::lang::Object
{

public: // actually package-private
  CollationElementIterator(::java::text::RuleBasedCollator *, ::java::lang::String *);
  ::java::text::RuleBasedCollator$CollationElement * nextBlock();
  ::java::text::RuleBasedCollator$CollationElement * previousBlock();
public:
  jint next();
  jint previous();
  static jint primaryOrder(jint);
  void reset();
  static jshort secondaryOrder(jint);
  static jshort tertiaryOrder(jint);
  void setText(::java::lang::String *);
  void setText(::java::text::CharacterIterator *);
  jint getOffset();
  void setOffset(jint);
  jint getMaxExpansion(jint);
  static const jint NULLORDER = -1;
public: // actually package-private
  ::java::text::RuleBasedCollator * __attribute__((aligned(__alignof__( ::java::lang::Object)))) collator;
  ::java::lang::String * text;
  jint index;
  jint textIndex;
private:
  JArray< ::java::text::RuleBasedCollator$CollationElement * > * text_decomposition;
  JArray< jint > * text_indexes;
public:
  static ::java::lang::Class class$;
};

#endif // __java_text_CollationElementIterator__
