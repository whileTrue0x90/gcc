#include <config.h>

#include <gcj/cni.h>
#include <jvm.h>

#include <gnu/java/nio/LongBufferImpl.h>

JArray<jlong>*
gnu::java::nio::LongBufferImpl::nio_cast(JArray<jbyte>*)
{
  return NULL;
}

void
gnu::java::nio::LongBufferImpl::nio_put_Byte(gnu::java::nio::LongBufferImpl*, jint, jint, jbyte)
{
}

jbyte
gnu::java::nio::LongBufferImpl::nio_get_Byte(gnu::java::nio::LongBufferImpl*, jint, jint)
{
  return 0;
}
