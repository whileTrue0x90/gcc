
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_javax_security_auth_callback_SwingCallbackHandler$4__
#define __gnu_javax_security_auth_callback_SwingCallbackHandler$4__

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
              class SwingCallbackHandler$4;
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
            class NameCallback;
        }
      }
    }
    namespace swing
    {
        class JDialog;
        class JTextField;
    }
  }
}

class gnu::javax::security::auth::callback::SwingCallbackHandler$4 : public ::java::lang::Object
{

public: // actually package-private
  SwingCallbackHandler$4(::gnu::javax::security::auth::callback::SwingCallbackHandler *, ::javax::security::auth::callback::NameCallback *, ::javax::swing::JTextField *, ::javax::swing::JDialog *);
public:
  void actionPerformed(::java::awt::event::ActionEvent *);
public: // actually package-private
  ::gnu::javax::security::auth::callback::SwingCallbackHandler * __attribute__((aligned(__alignof__( ::java::lang::Object)))) this$0;
private:
  ::javax::security::auth::callback::NameCallback * val$callback;
  ::javax::swing::JTextField * val$name;
  ::javax::swing::JDialog * val$dialog;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_javax_security_auth_callback_SwingCallbackHandler$4__
