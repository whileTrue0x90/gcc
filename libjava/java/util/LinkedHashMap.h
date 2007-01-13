
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_LinkedHashMap__
#define __java_util_LinkedHashMap__

#pragma interface

#include <java/util/HashMap.h>

class java::util::LinkedHashMap : public ::java::util::HashMap
{

public:
  LinkedHashMap();
  LinkedHashMap(::java::util::Map *);
  LinkedHashMap(jint);
  LinkedHashMap(jint, jfloat);
  LinkedHashMap(jint, jfloat, jboolean);
  virtual void clear();
  virtual jboolean containsValue(::java::lang::Object *);
  virtual ::java::lang::Object * get(::java::lang::Object *);
public: // actually protected
  virtual jboolean removeEldestEntry(::java::util::Map$Entry *);
public: // actually package-private
  virtual void addEntry(::java::lang::Object *, ::java::lang::Object *, jint, jboolean);
  virtual void putAllInternal(::java::util::Map *);
  virtual ::java::util::Iterator * iterator(jint);
private:
  static const jlong serialVersionUID = 3801124242820219131LL;
public: // actually package-private
  ::java::util::LinkedHashMap$LinkedHashEntry * __attribute__((aligned(__alignof__( ::java::util::HashMap)))) root;
  jboolean accessOrder;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_LinkedHashMap__
