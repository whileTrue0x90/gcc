
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_nio_charset_ByteEncodeLoopHelper__
#define __gnu_java_nio_charset_ByteEncodeLoopHelper__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace nio
      {
        namespace charset
        {
            class ByteEncodeLoopHelper;
        }
      }
    }
  }
  namespace java
  {
    namespace nio
    {
        class ByteBuffer;
        class CharBuffer;
      namespace charset
      {
          class CoderResult;
      }
    }
  }
}

class gnu::java::nio::charset::ByteEncodeLoopHelper : public ::java::lang::Object
{

public:
  ByteEncodeLoopHelper();
public: // actually protected
  virtual jboolean isMappable(jchar) = 0;
  virtual jbyte mapToByte(jchar) = 0;
public: // actually package-private
  virtual ::java::nio::charset::CoderResult * encodeLoop(::java::nio::CharBuffer *, ::java::nio::ByteBuffer *);
private:
  ::java::nio::charset::CoderResult * normalEncodeLoop(::java::nio::CharBuffer *, ::java::nio::ByteBuffer *);
  ::java::nio::charset::CoderResult * arrayEncodeLoop(::java::nio::CharBuffer *, ::java::nio::ByteBuffer *);
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_nio_charset_ByteEncodeLoopHelper__
