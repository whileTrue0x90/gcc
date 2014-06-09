
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_CORBA_Poa_gnuPOAManager__
#define __gnu_CORBA_Poa_gnuPOAManager__

#pragma interface

#include <org/omg/CORBA/LocalObject.h>
extern "Java"
{
  namespace gnu
  {
    namespace CORBA
    {
      namespace Poa
      {
          class gnuPOA;
          class gnuPOAManager;
      }
    }
  }
  namespace org
  {
    namespace omg
    {
      namespace PortableServer
      {
        namespace POAManagerPackage
        {
            class State;
        }
      }
    }
  }
}

class gnu::CORBA::Poa::gnuPOAManager : public ::org::omg::CORBA::LocalObject
{

public:
  gnuPOAManager();
  virtual ::org::omg::PortableServer::POAManagerPackage::State * get_state();
  virtual void activate();
  virtual void hold_requests(jboolean);
  virtual void deactivate(jboolean, jboolean);
  virtual void discard_requests(jboolean);
  virtual void waitForIdle();
  virtual void addPoa(::gnu::CORBA::Poa::gnuPOA *);
  virtual void removePOA(::gnu::CORBA::Poa::gnuPOA *);
  virtual void poaDestroyed(::gnu::CORBA::Poa::gnuPOA *);
  virtual void notifyInterceptors(jint);
private:
  static const jlong serialVersionUID = 1LL;
  ::java::util::HashSet * __attribute__((aligned(__alignof__( ::org::omg::CORBA::LocalObject)))) POAs;
public: // actually package-private
  ::org::omg::PortableServer::POAManagerPackage::State * state;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_CORBA_Poa_gnuPOAManager__
