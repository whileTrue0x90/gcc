
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_font_opentype_MacResourceFork$Resource__
#define __gnu_java_awt_font_opentype_MacResourceFork$Resource__

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
        namespace font
        {
          namespace opentype
          {
              class MacResourceFork$Resource;
          }
        }
      }
    }
  }
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
    }
  }
}

class gnu::java::awt::font::opentype::MacResourceFork$Resource : public ::java::lang::Object
{

  MacResourceFork$Resource(::java::nio::ByteBuffer *, jint, jshort, jint, jint);
public:
  jint getType();
  jshort getID();
  ::java::nio::ByteBuffer * getContent();
  jint getLength();
public: // actually package-private
  MacResourceFork$Resource(::java::nio::ByteBuffer *, jint, jshort, jint, jint, ::gnu::java::awt::font::opentype::MacResourceFork$Resource *);
  jint __attribute__((aligned(__alignof__( ::java::lang::Object)))) type;
  jshort id;
  jbyte attribute;
  jint nameOffset;
  jint dataOffset;
  ::java::nio::ByteBuffer * buf;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_font_opentype_MacResourceFork$Resource__
