
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_awt_peer_GnomeDesktopPeer__
#define __gnu_java_awt_peer_GnomeDesktopPeer__

#pragma interface

#include <gnu/java/awt/peer/ClasspathDesktopPeer.h>
extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace awt
      {
        namespace peer
        {
            class GnomeDesktopPeer;
        }
      }
    }
  }
  namespace java
  {
    namespace net
    {
        class URI;
    }
  }
}

class gnu::java::awt::peer::GnomeDesktopPeer : public ::gnu::java::awt::peer::ClasspathDesktopPeer
{

public:
  GnomeDesktopPeer();
public: // actually protected
  virtual ::java::lang::String * getCommand(::java::lang::String *);
public:
  virtual void browse(::java::net::URI *);
public: // actually protected
  virtual jboolean supportCommand(::java::lang::String *);
public:
  virtual void mail();
public: // actually protected
  virtual ::java::lang::String * execQuery(::java::lang::String *);
private:
  static ::java::lang::String * BROWSER_QUERY_GNOME;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_awt_peer_GnomeDesktopPeer__
