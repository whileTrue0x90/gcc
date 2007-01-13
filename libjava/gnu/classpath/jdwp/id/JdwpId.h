
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_classpath_jdwp_id_JdwpId__
#define __gnu_classpath_jdwp_id_JdwpId__

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
        namespace id
        {
            class JdwpId;
        }
      }
    }
  }
}

class gnu::classpath::jdwp::id::JdwpId : public ::java::lang::Object
{

public:
  JdwpId(jbyte);
  virtual void setId(jlong);
  virtual jlong getId();
  virtual ::java::lang::ref::SoftReference * getReference();
  virtual void setReference(::java::lang::ref::SoftReference *);
  virtual jboolean equals(::gnu::classpath::jdwp::id::JdwpId *);
  virtual void write(::java::io::DataOutputStream *) = 0;
  virtual void writeTagged(::java::io::DataOutputStream *);
  static const jint SIZE = 8;
public: // actually protected
  jlong __attribute__((aligned(__alignof__( ::java::lang::Object)))) _id;
private:
  jbyte _tag;
public: // actually protected
  ::java::lang::ref::SoftReference * _reference;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_classpath_jdwp_id_JdwpId__
