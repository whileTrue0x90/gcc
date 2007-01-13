
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_lang_StringBuffer__
#define __java_lang_StringBuffer__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>


class java::lang::StringBuffer : public ::java::lang::Object
{

public:
  StringBuffer();
  StringBuffer(jint);
  StringBuffer(::java::lang::String *);
  StringBuffer(::java::lang::CharSequence *);
  jint length();
  jint capacity();
  void ensureCapacity(jint);
  void setLength(jint);
  jchar charAt(jint);
  jint codePointAt(jint);
  jint codePointBefore(jint);
  void getChars(jint, jint, JArray< jchar > *, jint);
  void setCharAt(jint, jchar);
  ::java::lang::StringBuffer * append(::java::lang::Object *);
  ::java::lang::StringBuffer * append(::java::lang::String *);
  ::java::lang::StringBuffer * append(::java::lang::StringBuffer *);
  ::java::lang::StringBuffer * append(::java::lang::CharSequence *);
  ::java::lang::StringBuffer * append(::java::lang::CharSequence *, jint, jint);
  ::java::lang::StringBuffer * append(JArray< jchar > *);
  ::java::lang::StringBuffer * append(JArray< jchar > *, jint, jint);
  ::java::lang::StringBuffer * append(jboolean);
  ::java::lang::StringBuffer * append(jchar);
  ::java::lang::StringBuffer * appendCodePoint(jint);
  ::java::lang::StringBuffer * append(jint);
  ::java::lang::StringBuffer * append(jlong);
  ::java::lang::StringBuffer * append(jfloat);
  ::java::lang::StringBuffer * append(jdouble);
  ::java::lang::StringBuffer * delete$(jint, jint);
  ::java::lang::StringBuffer * deleteCharAt(jint);
  ::java::lang::StringBuffer * replace(jint, jint, ::java::lang::String *);
  ::java::lang::String * substring(jint);
  ::java::lang::CharSequence * subSequence(jint, jint);
  ::java::lang::String * substring(jint, jint);
  ::java::lang::StringBuffer * insert(jint, JArray< jchar > *, jint, jint);
  ::java::lang::StringBuffer * insert(jint, ::java::lang::Object *);
  ::java::lang::StringBuffer * insert(jint, ::java::lang::String *);
  ::java::lang::StringBuffer * insert(jint, ::java::lang::CharSequence *);
  ::java::lang::StringBuffer * insert(jint, ::java::lang::CharSequence *, jint, jint);
  ::java::lang::StringBuffer * insert(jint, JArray< jchar > *);
  ::java::lang::StringBuffer * insert(jint, jboolean);
  ::java::lang::StringBuffer * insert(jint, jchar);
  ::java::lang::StringBuffer * insert(jint, jint);
  ::java::lang::StringBuffer * insert(jint, jlong);
  ::java::lang::StringBuffer * insert(jint, jfloat);
  ::java::lang::StringBuffer * insert(jint, jdouble);
  jint indexOf(::java::lang::String *);
  jint indexOf(::java::lang::String *, jint);
  jint lastIndexOf(::java::lang::String *);
  jint lastIndexOf(::java::lang::String *, jint);
  ::java::lang::StringBuffer * reverse();
  ::java::lang::String * toString();
  void trimToSize();
  jint codePointCount(jint, jint);
  jint offsetByCodePoints(jint, jint);
private:
  void ensureCapacity_unsynchronized(jint);
  jboolean regionMatches(jint, ::java::lang::String *);
  static const jlong serialVersionUID = 3388685877147921107LL;
public: // actually package-private
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) count;
  JArray< jchar > * value;
  jboolean shared;
private:
  static const jint DEFAULT_CAPACITY = 16;
public:
  static ::java::lang::Class class$;
};

#endif // __java_lang_StringBuffer__
