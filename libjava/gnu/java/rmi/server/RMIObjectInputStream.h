
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_rmi_server_RMIObjectInputStream__
#define __gnu_java_rmi_server_RMIObjectInputStream__

#pragma interface

#include <java/io/ObjectInputStream.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace rmi
      {
        namespace server
        {
            class RMIObjectInputStream;
        }
      }
    }
  }
}

class gnu::java::rmi::server::RMIObjectInputStream : public ::java::io::ObjectInputStream
{

public:
  RMIObjectInputStream(::java::io::InputStream *);
public: // actually protected
  virtual ::java::lang::Class * resolveClass(::java::io::ObjectStreamClass *);
  virtual ::java::lang::Object * getAnnotation();
  virtual ::java::lang::Class * resolveProxyClass(JArray< ::java::lang::String * > *);
  virtual ::java::lang::Object * readValue(::java::lang::Class *);
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_rmi_server_RMIObjectInputStream__
