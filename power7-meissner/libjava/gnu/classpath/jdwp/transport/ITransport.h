
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_classpath_jdwp_transport_ITransport__
#define __gnu_classpath_jdwp_transport_ITransport__

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
        namespace transport
        {
            class ITransport;
        }
      }
    }
  }
}

class gnu::classpath::jdwp::transport::ITransport : public ::java::lang::Object
{

public:
  virtual void configure(::java::util::HashMap *) = 0;
  virtual void initialize() = 0;
  virtual void shutdown() = 0;
  virtual ::java::io::InputStream * getInputStream() = 0;
  virtual ::java::io::OutputStream * getOutputStream() = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __gnu_classpath_jdwp_transport_ITransport__
