/* DO NOT EDIT THIS FILE - it is machine generated */

#include <jni.h>

#ifndef __gnu_java_awt_peer_qt_QtImage__
#define __gnu_java_awt_peer_qt_QtImage__

#ifdef __cplusplus
extern "C"
{
#endif

JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_clear (JNIEnv *env, jobject);
JNIEXPORT jintArray JNICALL Java_gnu_java_awt_peer_qt_QtImage_getPixels (JNIEnv *env, jobject);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_setPixels (JNIEnv *env, jobject, jintArray);
JNIEXPORT jboolean JNICALL Java_gnu_java_awt_peer_qt_QtImage_loadImage (JNIEnv *env, jobject, jstring);
JNIEXPORT jboolean JNICALL Java_gnu_java_awt_peer_qt_QtImage_loadImageFromData (JNIEnv *env, jobject, jbyteArray);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_createImage (JNIEnv *env, jobject);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_freeImage (JNIEnv *env, jobject);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_createScaledImage (JNIEnv *env, jobject, jobject, jint);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_drawPixels (JNIEnv *env, jobject, jobject, jint, jint, jint, jint, jint, jboolean);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_drawPixelsScaled (JNIEnv *env, jobject, jobject, jint, jint, jint, jint, jint, jint, jint, jboolean);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_drawPixelsTransformed (JNIEnv *env, jobject, jobject, jobject);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_drawPixelsScaledFlipped (JNIEnv *env, jobject, jobject, jint, jint, jint, jboolean, jboolean, jint, jint, jint, jint, jint, jint, jint, jint, jboolean);
JNIEXPORT void JNICALL Java_gnu_java_awt_peer_qt_QtImage_copyArea (JNIEnv *env, jobject, jint, jint, jint, jint, jint, jint);

#undef java_awt_Image_SCALE_DEFAULT
#define java_awt_Image_SCALE_DEFAULT 1L
#undef java_awt_Image_SCALE_FAST
#define java_awt_Image_SCALE_FAST 2L
#undef java_awt_Image_SCALE_SMOOTH
#define java_awt_Image_SCALE_SMOOTH 4L
#undef java_awt_Image_SCALE_REPLICATE
#define java_awt_Image_SCALE_REPLICATE 8L
#undef java_awt_Image_SCALE_AREA_AVERAGING
#define java_awt_Image_SCALE_AREA_AVERAGING 16L

#ifdef __cplusplus
}
#endif

#endif /* __gnu_java_awt_peer_qt_QtImage__ */
