
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_Hashtable$ValueIterator__
#define __java_util_Hashtable$ValueIterator__

#pragma interface

#include <java/lang/Object.h>

class java::util::Hashtable$ValueIterator : public ::java::lang::Object
{

public: // actually package-private
  Hashtable$ValueIterator(::java::util::Hashtable *);
public:
  virtual jboolean hasNext();
  virtual ::java::lang::Object * next();
  virtual void remove();
private:
  ::java::util::Hashtable$EntryIterator * __attribute__((aligned(__alignof__( ::java::lang::Object)))) iterator;
public: // actually package-private
  ::java::util::Hashtable * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_Hashtable$ValueIterator__
