
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_rmi_dgc_DGCImpl_Skel__
#define __gnu_java_rmi_dgc_DGCImpl_Skel__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace rmi
      {
        namespace dgc
        {
            class DGCImpl_Skel;
        }
      }
    }
  }
  namespace java
  {
    namespace rmi
    {
        class Remote;
      namespace server
      {
          class Operation;
          class RemoteCall;
      }
    }
  }
}

class gnu::java::rmi::dgc::DGCImpl_Skel : public ::java::lang::Object
{

public:
  DGCImpl_Skel();
  JArray< ::java::rmi::server::Operation * > * getOperations();
  void dispatch(::java::rmi::Remote *, ::java::rmi::server::RemoteCall *, jint, jlong);
private:
  static const jlong interfaceHash = -669196253586618813LL;
  static JArray< ::java::rmi::server::Operation * > * operations;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_rmi_dgc_DGCImpl_Skel__
