
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_prefs_BackingStoreException__
#define __java_util_prefs_BackingStoreException__

#pragma interface

#include <java/lang/Exception.h>

class java::util::prefs::BackingStoreException : public ::java::lang::Exception
{

public:
  BackingStoreException(::java::lang::String *);
  BackingStoreException(::java::lang::Throwable *);
private:
  void writeObject(::java::io::ObjectOutputStream *);
  void readObject(::java::io::ObjectInputStream *);
public: // actually package-private
  static const jlong serialVersionUID = 859796500401108469LL;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_prefs_BackingStoreException__
