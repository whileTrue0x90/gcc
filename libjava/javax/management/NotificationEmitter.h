
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_management_NotificationEmitter__
#define __javax_management_NotificationEmitter__

#pragma interface

#include <java/lang/Object.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace management
    {
        class MBeanNotificationInfo;
        class NotificationEmitter;
        class NotificationFilter;
        class NotificationListener;
    }
  }
}

class javax::management::NotificationEmitter : public ::java::lang::Object
{

public:
  virtual void removeNotificationListener(::javax::management::NotificationListener *, ::javax::management::NotificationFilter *, ::java::lang::Object *) = 0;
  virtual void addNotificationListener(::javax::management::NotificationListener *, ::javax::management::NotificationFilter *, ::java::lang::Object *) = 0;
  virtual JArray< ::javax::management::MBeanNotificationInfo * > * getNotificationInfo() = 0;
  virtual void removeNotificationListener(::javax::management::NotificationListener *) = 0;
  static ::java::lang::Class class$;
} __attribute__ ((java_interface));

#endif // __javax_management_NotificationEmitter__
