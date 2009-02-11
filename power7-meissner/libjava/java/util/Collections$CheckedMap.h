
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_Collections$CheckedMap__
#define __java_util_Collections$CheckedMap__

#pragma interface

#include <java/lang/Object.h>

class java::util::Collections$CheckedMap : public ::java::lang::Object
{

public: // actually package-private
  Collections$CheckedMap(::java::util::Map *, ::java::lang::Class *, ::java::lang::Class *);
public:
  virtual void clear();
  virtual jboolean containsKey(::java::lang::Object *);
  virtual jboolean containsValue(::java::lang::Object *);
  virtual ::java::util::Set * entrySet();
  virtual jboolean equals(::java::lang::Object *);
  virtual ::java::lang::Object * get(::java::lang::Object *);
  virtual ::java::lang::Object * put(::java::lang::Object *, ::java::lang::Object *);
  virtual jint hashCode();
  virtual jboolean isEmpty();
  virtual ::java::util::Set * keySet();
  virtual void putAll(::java::util::Map *);
  virtual ::java::lang::Object * remove(::java::lang::Object *);
  virtual jint size();
  virtual ::java::lang::String * toString();
  virtual ::java::util::Collection * values();
private:
  static const jlong serialVersionUID = 5742860141034234728LL;
  ::java::util::Map * __attribute__((aligned(__alignof__( ::java::lang::Object)))) m;
public: // actually package-private
  ::java::lang::Class * keyType;
  ::java::lang::Class * valueType;
private:
  ::java::util::Set * entries;
  ::java::util::Set * keys;
  ::java::util::Collection * values__;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_Collections$CheckedMap__
