
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __java_util_prefs_AbstractPreferences__
#define __java_util_prefs_AbstractPreferences__

#pragma interface

#include <java/util/prefs/Preferences.h>
#include <gcj/array.h>


class java::util::prefs::AbstractPreferences : public ::java::util::prefs::Preferences
{

public: // actually protected
  AbstractPreferences(::java::util::prefs::AbstractPreferences *, ::java::lang::String *);
public:
  virtual ::java::lang::String * absolutePath();
private:
  ::java::lang::String * path();
public:
  virtual jboolean isUserNode();
  virtual ::java::lang::String * name();
  virtual ::java::lang::String * toString();
public: // actually protected
  virtual JArray< ::java::util::prefs::AbstractPreferences * > * cachedChildren();
public:
  virtual JArray< ::java::lang::String * > * childrenNames();
  virtual ::java::util::prefs::Preferences * node(::java::lang::String *);
private:
  ::java::util::prefs::Preferences * getNode(::java::lang::String *);
public:
  virtual jboolean nodeExists(::java::lang::String *);
private:
  jboolean existsNode(::java::lang::String *);
public: // actually protected
  virtual ::java::util::prefs::AbstractPreferences * getChild(::java::lang::String *);
  virtual jboolean isRemoved();
public:
  virtual ::java::util::prefs::Preferences * parent();
  virtual void exportNode(::java::io::OutputStream *);
  virtual void exportSubtree(::java::io::OutputStream *);
  virtual JArray< ::java::lang::String * > * keys();
  virtual ::java::lang::String * get(::java::lang::String *, ::java::lang::String *);
  virtual jboolean getBoolean(::java::lang::String *, jboolean);
  virtual JArray< jbyte > * getByteArray(::java::lang::String *, JArray< jbyte > *);
private:
  static JArray< jbyte > * decode64(::java::lang::String *);
public:
  virtual jdouble getDouble(::java::lang::String *, jdouble);
  virtual jfloat getFloat(::java::lang::String *, jfloat);
  virtual jint getInt(::java::lang::String *, jint);
  virtual jlong getLong(::java::lang::String *, jlong);
  virtual void put(::java::lang::String *, ::java::lang::String *);
  virtual void putBoolean(::java::lang::String *, jboolean);
  virtual void putByteArray(::java::lang::String *, JArray< jbyte > *);
private:
  static ::java::lang::String * encode64(JArray< jbyte > *);
public:
  virtual void putDouble(::java::lang::String *, jdouble);
  virtual void putFloat(::java::lang::String *, jfloat);
  virtual void putInt(::java::lang::String *, jint);
  virtual void putLong(::java::lang::String *, jlong);
  virtual void remove(::java::lang::String *);
  virtual void clear();
  virtual void flush();
  virtual void sync();
private:
  void flushNode(jboolean);
public:
  virtual void removeNode();
private:
  void purge();
public:
  virtual void addNodeChangeListener(::java::util::prefs::NodeChangeListener *);
  virtual void addPreferenceChangeListener(::java::util::prefs::PreferenceChangeListener *);
  virtual void removeNodeChangeListener(::java::util::prefs::NodeChangeListener *);
  virtual void removePreferenceChangeListener(::java::util::prefs::PreferenceChangeListener *);
private:
  void fire(::java::util::prefs::PreferenceChangeEvent *);
  void fire(::java::util::prefs::NodeChangeEvent *, jboolean);
public: // actually protected
  virtual JArray< ::java::lang::String * > * childrenNamesSpi() = 0;
  virtual ::java::util::prefs::AbstractPreferences * childSpi(::java::lang::String *) = 0;
  virtual JArray< ::java::lang::String * > * keysSpi() = 0;
  virtual ::java::lang::String * getSpi(::java::lang::String *) = 0;
  virtual void putSpi(::java::lang::String *, ::java::lang::String *) = 0;
  virtual void removeSpi(::java::lang::String *) = 0;
  virtual void flushSpi() = 0;
  virtual void syncSpi() = 0;
  virtual void removeNodeSpi() = 0;
  ::java::lang::Object * __attribute__((aligned(__alignof__( ::java::util::prefs::Preferences)))) lock;
  jboolean newNode;
private:
  ::java::util::prefs::AbstractPreferences * parent__;
  ::java::lang::String * name__;
  jboolean removed;
  ::java::util::HashMap * childCache;
  ::java::util::ArrayList * nodeListeners;
  ::java::util::ArrayList * preferenceListeners;
public:
  static ::java::lang::Class class$;
};

#endif // __java_util_prefs_AbstractPreferences__
