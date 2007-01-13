
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_security_hash_RipeMD160__
#define __gnu_java_security_hash_RipeMD160__

#pragma interface

#include <gnu/java/security/hash/BaseHash.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace security
      {
        namespace hash
        {
            class RipeMD160;
        }
      }
    }
  }
}

class gnu::java::security::hash::RipeMD160 : public ::gnu::java::security::hash::BaseHash
{

public:
  RipeMD160();
private:
  RipeMD160(::gnu::java::security::hash::RipeMD160 *);
public:
  virtual ::java::lang::Object * clone();
public: // actually protected
  virtual void transform(JArray< jbyte > *, jint);
  virtual JArray< jbyte > * padBuffer();
  virtual JArray< jbyte > * getResult();
  virtual void resetContext();
public:
  virtual jboolean selfTest();
private:
  static const jint BLOCK_SIZE = 64;
  static ::java::lang::String * DIGEST0;
  static JArray< jint > * R;
  static JArray< jint > * Rp;
  static JArray< jint > * S;
  static JArray< jint > * Sp;
  static ::java::lang::Boolean * valid;
  jint __attribute__((aligned(__alignof__( ::gnu::java::security::hash::BaseHash)))) h0;
  jint h1;
  jint h2;
  jint h3;
  jint h4;
  JArray< jint > * X;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_security_hash_RipeMD160__
