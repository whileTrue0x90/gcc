
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_classpath_jdwp_JdwpConstants$ClassStatus__
#define __gnu_classpath_jdwp_JdwpConstants$ClassStatus__

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
          class JdwpConstants$ClassStatus;
      }
    }
  }
}

class gnu::classpath::jdwp::JdwpConstants$ClassStatus : public ::java::lang::Object
{

public:
  JdwpConstants$ClassStatus();
  static const jint VERIFIED = 1;
  static const jint PREPARED = 2;
  static const jint INITIALIZED = 4;
  static const jint ERROR = 8;
  static ::java::lang::Class class$;
};

#endif // __gnu_classpath_jdwp_JdwpConstants$ClassStatus__
