
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_lang_reflect_Field__
#define __java_lang_reflect_Field__

#pragma interface

#include <java/lang/reflect/AccessibleObject.h>
#include <gcj/array.h>


jfieldID _Jv_FromReflectedField (java::lang::reflect::Field *);
jobject JNICALL _Jv_JNI_ToReflectedField (_Jv_JNIEnv*, jclass, jfieldID, jboolean);
jobject _Jv_getFieldInternal (java::lang::reflect::Field *f, jclass c, jobject o);

class java::lang::reflect::Field : public ::java::lang::reflect::AccessibleObject
{

public: // actually package-private
  Field();
public:
  ::java::lang::Class * getDeclaringClass();
  ::java::lang::String * getName();
private:
  jint getModifiersInternal();
public:
  jint getModifiers();
  jboolean isSynthetic();
  jboolean isEnumConstant();
  ::java::lang::Class * getType();
  jboolean equals(::java::lang::Object *);
  jint hashCode();
  ::java::lang::String * toString();
  ::java::lang::String * toGenericString();
  ::java::lang::Object * get(::java::lang::Object *);
  jboolean getBoolean(::java::lang::Object *);
  jbyte getByte(::java::lang::Object *);
  jchar getChar(::java::lang::Object *);
  jshort getShort(::java::lang::Object *);
  jint getInt(::java::lang::Object *);
  jlong getLong(::java::lang::Object *);
  jfloat getFloat(::java::lang::Object *);
  jdouble getDouble(::java::lang::Object *);
private:
  jboolean getBoolean(::java::lang::Class *, ::java::lang::Object *);
  jchar getChar(::java::lang::Class *, ::java::lang::Object *);
  jbyte getByte(::java::lang::Class *, ::java::lang::Object *);
  jshort getShort(::java::lang::Class *, ::java::lang::Object *);
  jint getInt(::java::lang::Class *, ::java::lang::Object *);
  jlong getLong(::java::lang::Class *, ::java::lang::Object *);
  jfloat getFloat(::java::lang::Class *, ::java::lang::Object *);
  jdouble getDouble(::java::lang::Class *, ::java::lang::Object *);
  ::java::lang::Object * get(::java::lang::Class *, ::java::lang::Object *);
public:
  void set(::java::lang::Object *, ::java::lang::Object *);
  void setBoolean(::java::lang::Object *, jboolean);
  void setByte(::java::lang::Object *, jbyte);
  void setChar(::java::lang::Object *, jchar);
  void setShort(::java::lang::Object *, jshort);
  void setInt(::java::lang::Object *, jint);
  void setLong(::java::lang::Object *, jlong);
  void setFloat(::java::lang::Object *, jfloat);
  void setDouble(::java::lang::Object *, jdouble);
  ::java::lang::reflect::Type * getGenericType();
  ::java::lang::annotation::Annotation * getAnnotation(::java::lang::Class *);
  JArray< ::java::lang::annotation::Annotation * > * getDeclaredAnnotations();
private:
  JArray< ::java::lang::annotation::Annotation * > * getDeclaredAnnotationsInternal();
  ::java::lang::String * getSignature();
public: // actually package-private
  void setByte(::java::lang::Class *, ::java::lang::Object *, jbyte, jboolean);
  void setShort(::java::lang::Class *, ::java::lang::Object *, jshort, jboolean);
  void setInt(::java::lang::Class *, ::java::lang::Object *, jint, jboolean);
  void setLong(::java::lang::Class *, ::java::lang::Object *, jlong, jboolean);
  void setFloat(::java::lang::Class *, ::java::lang::Object *, jfloat, jboolean);
  void setDouble(::java::lang::Class *, ::java::lang::Object *, jdouble, jboolean);
  void setChar(::java::lang::Class *, ::java::lang::Object *, jchar, jboolean);
  void setBoolean(::java::lang::Class *, ::java::lang::Object *, jboolean, jboolean);
  void set(::java::lang::Class *, ::java::lang::Object *, ::java::lang::Object *, ::java::lang::Class *, jboolean);
private:
  void set(::java::lang::Class *, ::java::lang::Object *, ::java::lang::Object *);
  ::java::lang::Class * __attribute__((aligned(__alignof__( ::java::lang::reflect::AccessibleObject)))) declaringClass;
  ::java::lang::String * name;
  jint offset;
  ::java::lang::Class * type;
public: // actually package-private
  static const jint FIELD_MODIFIERS = 223;
public:
  static ::java::lang::Class class$;

  friend jfieldID (::_Jv_FromReflectedField) (java::lang::reflect::Field *);
  friend jobject JNICALL (::_Jv_JNI_ToReflectedField) (_Jv_JNIEnv*, jclass, jfieldID, jboolean);
  friend class java::lang::Class;
  friend jobject (::_Jv_getFieldInternal) (java::lang::reflect::Field *f, jclass c, jobject o);
};

#endif // __java_lang_reflect_Field__
