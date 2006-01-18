#include <stdio.h>

static struct sss{
  float f;
  char * snd;
} sss;

#define _offsetof(st,f) ((char *)&((st *) 16)->f - (char *) 16)

int main (void) {
  printf ("+++Struct float-pointer:\n");
  printf ("size=%d,align=%d,offset-float=%d,offset-pointer=%d,\nalign-float=%d,align-pointer=%d\n",
          sizeof (sss), __alignof__ (sss),
          _offsetof (struct sss, f), _offsetof (struct sss, snd),
          __alignof__ (sss.f), __alignof__ (sss.snd));
  return 0;
}
