
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_util_ZoneInfo__
#define __gnu_java_util_ZoneInfo__

#pragma interface

#include <java/util/TimeZone.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace util
      {
          class ZoneInfo;
      }
    }
  }
}

class gnu::java::util::ZoneInfo : public ::java::util::TimeZone
{

public:
  ZoneInfo(jint, ::java::lang::String *, JArray< jlong > *, ::java::util::SimpleTimeZone *);
  virtual jint getOffset(jint, jint, jint, jint, jint, jint);
private:
  jlong findTransition(jlong);
public:
  virtual jint getOffset(jlong);
  virtual jint getRawOffset();
  virtual void setRawOffset(jint);
private:
  void computeDSTSavings();
public:
  virtual jint getDSTSavings();
  virtual jboolean useDaylightTime();
  virtual jboolean inDaylightTime(::java::util::Date *);
  virtual jint hashCode();
  virtual jboolean equals(::java::lang::Object *);
  virtual jboolean hasSameRules(::java::util::TimeZone *);
  virtual ::java::lang::String * toString();
  static ::java::util::TimeZone * readTZFile(::java::lang::String *, ::java::lang::String *);
private:
  static void skipFully(::java::io::InputStream *, jlong);
  static ::java::util::SimpleTimeZone * createLastRule(::java::lang::String *);
  static JArray< jint > * getDateParams(::java::lang::String *);
  static jint parseTime(::java::lang::String *);
  static const jint SECS_SHIFT = 22;
  static const jlong OFFSET_MASK = 2097151LL;
  static const jint OFFSET_SHIFT = 43;
  static const jlong IS_DST = 2097152LL;
  jint __attribute__((aligned(__alignof__( ::java::util::TimeZone)))) rawOffset;
  jint dstSavings;
  jboolean useDaylight;
  JArray< jlong > * transitions;
  ::java::util::SimpleTimeZone * lastRule;
  static ::java::util::SimpleTimeZone * gmtZone;
public: // actually package-private
  static const jlong serialVersionUID = -3740626706860383657LL;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_util_ZoneInfo__
