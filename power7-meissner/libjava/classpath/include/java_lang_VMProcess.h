/* DO NOT EDIT THIS FILE - it is machine generated */

#include <jni.h>

#ifndef __java_lang_VMProcess__
#define __java_lang_VMProcess__

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_java_lang_VMProcess_nativeSpawn (JNIEnv *env, jobject, jobjectArray, jobjectArray, jobject, jboolean);
JNIEXPORT jboolean JNICALL Java_java_lang_VMProcess_nativeReap (JNIEnv *env, jclass);
JNIEXPORT void JNICALL Java_java_lang_VMProcess_nativeKill (JNIEnv *env, jclass, jlong);

#undef java_lang_VMProcess_INITIAL
#define java_lang_VMProcess_INITIAL 0L
#undef java_lang_VMProcess_RUNNING
#define java_lang_VMProcess_RUNNING 1L
#undef java_lang_VMProcess_TERMINATED
#define java_lang_VMProcess_TERMINATED 2L

#ifdef __cplusplus
}
#endif

#endif /* __java_lang_VMProcess__ */
