
// DO NOT EDIT THIS FILE - it is machine generated -*- c++ -*-

#ifndef __gnu_java_lang_management_ThreadMXBeanImpl__
#define __gnu_java_lang_management_ThreadMXBeanImpl__

#pragma interface

#include <gnu/java/lang/management/BeanImpl.h>
#include <gcj/array.h>

extern "Java"
{
  namespace gnu
  {
    namespace java
    {
      namespace lang
      {
        namespace management
        {
            class ThreadMXBeanImpl;
        }
      }
    }
  }
}

class gnu::java::lang::management::ThreadMXBeanImpl : public ::gnu::java::lang::management::BeanImpl
{

public:
  ThreadMXBeanImpl();
  JArray< ::java::lang::management::ThreadInfo * > * dumpAllThreads(jboolean, jboolean);
  JArray< jlong > * findDeadlockedThreads();
  JArray< jlong > * findMonitorDeadlockedThreads();
  JArray< jlong > * getAllThreadIds();
  jlong getCurrentThreadCpuTime();
  jlong getCurrentThreadUserTime();
  jint getDaemonThreadCount();
  jint getPeakThreadCount();
  jint getThreadCount();
  jlong getThreadCpuTime(jlong);
  ::java::lang::management::ThreadInfo * getThreadInfo(jlong);
  JArray< ::java::lang::management::ThreadInfo * > * getThreadInfo(JArray< jlong > *);
  ::java::lang::management::ThreadInfo * getThreadInfo(jlong, jint);
  JArray< ::java::lang::management::ThreadInfo * > * getThreadInfo(JArray< jlong > *, jint);
  JArray< ::java::lang::management::ThreadInfo * > * getThreadInfo(JArray< jlong > *, jboolean, jboolean);
  jlong getThreadUserTime(jlong);
  jlong getTotalStartedThreadCount();
  jboolean isCurrentThreadCpuTimeSupported();
  jboolean isObjectMonitorUsageSupported();
  jboolean isSynchronizerUsageSupported();
  jboolean isThreadContentionMonitoringEnabled();
  jboolean isThreadContentionMonitoringSupported();
  jboolean isThreadCpuTimeEnabled();
  jboolean isThreadCpuTimeSupported();
  void resetPeakThreadCount();
  void setThreadContentionMonitoringEnabled(jboolean);
  void setThreadCpuTimeEnabled(jboolean);
private:
  static ::java::lang::String * CURRENT_THREAD_TIME_SUPPORT;
  static ::java::lang::String * THREAD_TIME_SUPPORT;
  static ::java::lang::String * CONTENTION_SUPPORT;
  static ::java::lang::String * TIME_ENABLED;
  static ::java::lang::String * MONITOR_SUPPORT;
  static ::java::lang::String * SYNCHRONIZER_SUPPORT;
  jboolean __attribute__((aligned(__alignof__( ::gnu::java::lang::management::BeanImpl)))) timeEnabled;
  jboolean contentionEnabled;
public:
  static ::java::lang::Class class$;
};

#endif // __gnu_java_lang_management_ThreadMXBeanImpl__
