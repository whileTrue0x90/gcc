
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_JEditorPane$PageStream__
#define __javax_swing_JEditorPane$PageStream__

#pragma interface

#include <java/io/FilterInputStream.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace swing
    {
        class JEditorPane;
        class JEditorPane$PageStream;
    }
  }
}

class javax::swing::JEditorPane$PageStream : public ::java::io::FilterInputStream
{

public: // actually protected
  JEditorPane$PageStream(::javax::swing::JEditorPane *, ::java::io::InputStream *);
private:
  void checkCancelled();
public: // actually package-private
  virtual void cancel();
public:
  virtual jint read();
  virtual jint read(JArray< jbyte > *, jint, jint);
  virtual jlong skip(jlong);
  virtual jint available();
  virtual void reset();
private:
  jboolean __attribute__((aligned(__alignof__( ::java::io::FilterInputStream)))) cancelled;
public: // actually package-private
  ::javax::swing::JEditorPane * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_JEditorPane$PageStream__
