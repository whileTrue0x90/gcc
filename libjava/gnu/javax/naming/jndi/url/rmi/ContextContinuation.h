
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_naming_jndi_url_rmi_ContextContinuation__
#define __gnu_javax_naming_jndi_url_rmi_ContextContinuation__

#pragma interface

#include <java/lang/Object.h>
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
                class ContextContinuation;
            }
          }
        }
      }
    }
  }
  namespace java
  {
    namespace rmi
    {
      namespace registry
      {
          class Registry;
      }
    }
  }
  namespace javax
  {
    namespace naming
    {
        class Context;
        class Name;
        class NameParser;
        class NamingEnumeration;
    }
  }
}

class gnu::javax::naming::jndi::url::rmi::ContextContinuation : public ::java::lang::Object
{

public:
  virtual ::java::lang::Object * addToEnvironment(::java::lang::String *, ::java::lang::Object *);
  virtual ::java::util::Hashtable * getEnvironment();
  virtual ::java::lang::Object * removeFromEnvironment(::java::lang::String *);
  virtual void removeRegistry();
  virtual ::java::rmi::registry::Registry * getRegistry();
  ContextContinuation(::java::util::Map *, ::java::rmi::registry::Registry *);
  virtual void bind(::javax::naming::Name *, ::java::lang::Object *);
  virtual void bind(::java::lang::String *, ::java::lang::Object *);
  virtual ::javax::naming::Name * composeName(::javax::naming::Name *, ::javax::naming::Name *);
  virtual ::java::lang::String * composeName(::java::lang::String *, ::java::lang::String *);
  virtual ::javax::naming::Context * createSubcontext(::javax::naming::Name *);
  virtual ::javax::naming::Context * createSubcontext(::java::lang::String *);
  virtual void destroySubcontext(::javax::naming::Name *);
  virtual void destroySubcontext(::java::lang::String *);
  virtual ::java::lang::String * getNameInNamespace();
  virtual ::javax::naming::NameParser * getNameParser(::javax::naming::Name *);
  virtual ::javax::naming::NameParser * getNameParser(::java::lang::String *);
  virtual ::javax::naming::NamingEnumeration * list(::javax::naming::Name *);
  virtual ::javax::naming::NamingEnumeration * list(::java::lang::String *);
  virtual ::javax::naming::NamingEnumeration * listBindings(::javax::naming::Name *);
  virtual ::javax::naming::NamingEnumeration * listBindings(::java::lang::String *);
  virtual ::java::lang::Object * lookupLink(::javax::naming::Name *);
  virtual ::java::lang::Object * lookupLink(::java::lang::String *);
  virtual void rebind(::javax::naming::Name *, ::java::lang::Object *);
  virtual void rebind(::java::lang::String *, ::java::lang::Object *);
  virtual void rename(::javax::naming::Name *, ::javax::naming::Name *);
  virtual void rename(::java::lang::String *, ::java::lang::String *);
  virtual void unbind(::javax::naming::Name *);
  virtual void unbind(::java::lang::String *);
  virtual void close();
  virtual ::java::lang::Object * lookup(::javax::naming::Name *);
  virtual ::java::lang::Object * lookup(::java::lang::String *);
  static ::java::lang::String * DEFAULT_REGISTRY_LOCATION;
public: // actually package-private
  ::java::rmi::registry::Registry * __attribute__((aligned(__alignof__( ::java::lang::Object)))) registry;
  ::java::util::Properties * properties;
  jboolean lookupCalled;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_naming_jndi_url_rmi_ContextContinuation__
