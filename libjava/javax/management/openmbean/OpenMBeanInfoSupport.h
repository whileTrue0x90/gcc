
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __javax_management_openmbean_OpenMBeanInfoSupport__
#define __javax_management_openmbean_OpenMBeanInfoSupport__

#pragma interface

#include <javax/management/MBeanInfo.h>
#include <gcj/array.h>

extern "Java"
{
  namespace javax
  {
    namespace management
    {
        class MBeanNotificationInfo;
      namespace openmbean
      {
          class OpenMBeanAttributeInfo;
          class OpenMBeanConstructorInfo;
          class OpenMBeanInfoSupport;
          class OpenMBeanOperationInfo;
      }
    }
  }
}

class javax::management::openmbean::OpenMBeanInfoSupport : public ::javax::management::MBeanInfo
{

public:
  OpenMBeanInfoSupport(::java::lang::String *, ::java::lang::String *, JArray< ::javax::management::openmbean::OpenMBeanAttributeInfo * > *, JArray< ::javax::management::openmbean::OpenMBeanConstructorInfo * > *, JArray< ::javax::management::openmbean::OpenMBeanOperationInfo * > *, JArray< ::javax::management::MBeanNotificationInfo * > *);
  virtual jboolean equals(::java::lang::Object *);
  virtual jint hashCode();
  virtual ::java::lang::String * toString();
private:
  static const jlong serialVersionUID = 4349395935420511492LL;
  ::java::lang::Integer * __attribute__((aligned(__alignof__( ::javax::management::MBeanInfo)))) hashCode__;
  ::java::lang::String * string;
public:
  static ::java::lang::Class class$;
};

#endif // __javax_management_openmbean_OpenMBeanInfoSupport__
