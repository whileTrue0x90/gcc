
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_security_util_ExpirableObject$Destroyer__
#define __gnu_java_security_util_ExpirableObject$Destroyer__

#pragma interface

#include <java/util/TimerTask.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace security
      {
        namespace util
        {
            class ExpirableObject;
            class ExpirableObject$Destroyer;
        }
      }
    }
  }
}

class gnu::java::security::util::ExpirableObject$Destroyer : public ::java::util::TimerTask
{

public: // actually package-private
  ExpirableObject$Destroyer(::gnu::java::security::util::ExpirableObject *, ::gnu::java::security::util::ExpirableObject *);
public:
  void run();
private:
  ::gnu::java::security::util::ExpirableObject * __attribute__((aligned(__alignof__( ::java::util::TimerTask)))) target;
public: // actually package-private
  ::gnu::java::security::util::ExpirableObject * this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_security_util_ExpirableObject$Destroyer__
