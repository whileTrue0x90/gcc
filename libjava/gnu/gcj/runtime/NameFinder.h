
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_gcj_runtime_NameFinder__
#define __gnu_gcj_runtime_NameFinder__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace gcj
    {
      namespace runtime
      {
          class NameFinder;
      }
    }
  }
}

class gnu::gcj::runtime::NameFinder : public ::java::lang::Object
{

public: // actually package-private
  static jboolean showRaw();
  static jboolean removeUnknown();
public:
  NameFinder();
  virtual ::java::lang::String * getSourceFile();
  virtual jint getLineNum();
  virtual void lookup(::java::lang::String *, jlong);
  static ::java::lang::String * demangleInterpreterMethod(::java::lang::String *, ::java::lang::String *);
  virtual void close();
private:
  ::java::lang::String * __attribute__((aligned(__alignof__( ::java::lang::Object)))) binaryFile;
  ::java::lang::String * sourceFile;
  jint lineNum;
  ::java::util::HashMap * procs;
  static ::java::util::Set * blacklist;
  static jboolean use_addr2line;
  static jboolean show_raw;
  static jboolean remove_unknown;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_gcj_runtime_NameFinder__
