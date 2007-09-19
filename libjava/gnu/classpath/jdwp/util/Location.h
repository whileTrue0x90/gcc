
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_classpath_jdwp_util_Location__
#define __gnu_classpath_jdwp_util_Location__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace classpath
    {
      namespace jdwp
      {
          class VMMethod;
        namespace util
        {
            class Location;
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

class gnu::classpath::jdwp::util::Location : public ::java::lang::Object
{

public:
  Location(::gnu::classpath::jdwp::VMMethod *, jlong);
  Location(::java::nio::ByteBuffer *);
  virtual void write(::java::io::DataOutputStream *);
  static ::gnu::classpath::jdwp::util::Location * getEmptyLocation();
  virtual ::gnu::classpath::jdwp::VMMethod * getMethod();
  virtual jlong getIndex();
  virtual ::java::lang::String * toString();
  virtual jboolean equals(::java::lang::Object *);
private:
  ::gnu::classpath::jdwp::VMMethod * __attribute__((aligned(__alignof__( ::java::lang::Object)))) method;
  jlong index;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_classpath_jdwp_util_Location__
