
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_naming_jndi_url_rmi_ListEnumeration__
#define __gnu_javax_naming_jndi_url_rmi_ListEnumeration__

#pragma interface

#include <gnu/javax/naming/jndi/url/rmi/RmiNamingEnumeration.h>
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
                class ListEnumeration;
            }
          }
        }
      }
    }
  }
}

class gnu::javax::naming::jndi::url::rmi::ListEnumeration : public ::gnu::javax::naming::jndi::url::rmi::RmiNamingEnumeration
{

public:
  ListEnumeration(JArray< ::java::lang::String * > *);
  virtual ::java::lang::Object * convert(::java::lang::String *);
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_naming_jndi_url_rmi_ListEnumeration__
