
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_naming_jndi_url_rmi_RmiNamingEnumeration__
#define __gnu_javax_naming_jndi_url_rmi_RmiNamingEnumeration__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace naming
      {
        namespace jndi
        {
          namespace url
          {
            namespace rmi
            {
                class RmiNamingEnumeration;
            }
          }
        }
      }
    }
  }
}

class gnu::javax::naming::jndi::url::rmi::RmiNamingEnumeration : public ::java::lang::Object
{

public: // actually package-private
  RmiNamingEnumeration(JArray< ::java::lang::String * > *);
public:
  virtual ::java::lang::Object * convert(::java::lang::String *) = 0;
  virtual jboolean hasMore();
  virtual ::java::lang::Object * next();
  virtual jboolean hasMoreElements();
  virtual ::java::lang::Object * nextElement();
  virtual void close();
public: // actually package-private
  JArray< ::java::lang::String * > * __attribute__((aligned(__alignof__( ::java::lang::Object)))) list;
  jint p;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_naming_jndi_url_rmi_RmiNamingEnumeration__
