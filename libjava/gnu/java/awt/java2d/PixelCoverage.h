
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_java2d_PixelCoverage__
#define __gnu_java_awt_java2d_PixelCoverage__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace awt
      {
        namespace java2d
        {
            class PixelCoverage;
            class PixelCoverage$Bucket;
        }
      }
    }
  }
}

class gnu::java::awt::java2d::PixelCoverage : public ::java::lang::Object
{

public: // actually package-private
  PixelCoverage();
  void rewind();
  void clear();
  void add(jint, jint, jint);
private:
  ::gnu::java::awt::java2d::PixelCoverage$Bucket * findOrInsert(jint);
  ::gnu::java::awt::java2d::PixelCoverage$Bucket * __attribute__((aligned(__alignof__( ::java::lang::Object)))) head;
  ::gnu::java::awt::java2d::PixelCoverage$Bucket * current;
  ::gnu::java::awt::java2d::PixelCoverage$Bucket * last;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_java2d_PixelCoverage__
