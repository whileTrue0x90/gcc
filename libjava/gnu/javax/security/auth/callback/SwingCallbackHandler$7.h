
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_security_auth_callback_SwingCallbackHandler$7__
#define __gnu_javax_security_auth_callback_SwingCallbackHandler$7__

#pragma interface

#include <java/lang/Object.h>
extern "Java"
{
  namespace gnu
  {
    namespace javax
    {
      namespace security
      {
        namespace auth
        {
          namespace callback
          {
              class SwingCallbackHandler;
              class SwingCallbackHandler$7;
          }
        }
      }
    }
  }
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
    namespace security
    {
      namespace auth
      {
        namespace callback
        {
            class TextOutputCallback;
        }
      }
    }
    namespace swing
    {
        class JDialog;
    }
  }
}

class gnu::javax::security::auth::callback::SwingCallbackHandler$7 : public ::java::lang::Object
{

public: // actually package-private
  SwingCallbackHandler$7(::gnu::javax::security::auth::callback::SwingCallbackHandler *, ::javax::swing::JDialog *, ::javax::security::auth::callback::TextOutputCallback *);
public:
  virtual void actionPerformed(::java::awt::event::ActionEvent *);
public: // actually package-private
  ::gnu::javax::security::auth::callback::SwingCallbackHandler * __attribute__((aligned(__alignof__( ::java::lang::Object)))) this$0;
private:
  ::javax::swing::JDialog * val$dialog;
  ::javax::security::auth::callback::TextOutputCallback * val$callback;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_security_auth_callback_SwingCallbackHandler$7__
