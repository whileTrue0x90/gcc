
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_swing_ProgressMonitor$1__
#define __javax_swing_ProgressMonitor$1__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace java
  {
    namespace awt
    {
      namespace event
      {
          class ActionEvent;
      }
    }
  }
  namespace javax
  {
    namespace swing
    {
        class ProgressMonitor;
        class ProgressMonitor$1;
    }
  }
}

class javax::swing::ProgressMonitor$1 : public ::java::lang::Object
{

public: // actually package-private
  ProgressMonitor$1(::javax::swing::ProgressMonitor *);
public:
  virtual void actionPerformed(::java::awt::event::ActionEvent *);
public: // actually package-private
  ::javax::swing::ProgressMonitor * __attribute__((aligned(__alignof__( ::java::lang::Object)))) this$0;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_swing_ProgressMonitor$1__
