
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_classpath_jdwp_id_ClassObjectId__
#define __gnu_classpath_jdwp_id_ClassObjectId__

#pragma interface

#include <gnu/classpath/jdwp/id/ObjectId.h>
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
            class ClassObjectId;
        }
      }
    }
  }
}

class gnu::classpath::jdwp::id::ClassObjectId : public ::gnu::classpath::jdwp::id::ObjectId
{

public:
  ClassObjectId();
  virtual ::java::lang::Class * getClassObject();
  static ::java::lang::Class * typeClass;
  static ::java::lang::Class class$;
};

#endif // __gnu_classpath_jdwp_id_ClassObjectId__
