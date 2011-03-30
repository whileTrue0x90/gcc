/* go-reflect-chan.c -- channel reflection support for Go.

   Copyright 2009 The Go Authors. All rights reserved.
   Use of this source code is governed by a BSD-style
   license that can be found in the LICENSE file.  */

#include <stdlib.h>
#include <stdint.h>

#include "config.h"
#include "go-type.h"
#include "channel.h"

/* This file implements support for reflection on channels.  These
   functions are called from reflect/value.go.  */

extern unsigned char *makechan (const struct __go_type_descriptor *, uint32_t)
  asm ("libgo_reflect.reflect.makechan");

unsigned char *
makechan (const struct __go_type_descriptor *typ, uint32_t size)
{
  return (unsigned char *) __go_new_channel (typ->__size, size);
}

extern void chansend (unsigned char *, unsigned char *, _Bool *)
  asm ("libgo_reflect.reflect.chansend");

void
chansend (unsigned char *ch, unsigned char *val, _Bool *selected)
{
  struct __go_channel *channel = (struct __go_channel *) ch;

  if (channel->element_size <= sizeof (uint64_t))
    {
      union
      {
	char b[sizeof (uint64_t)];
	uint64_t v;
      } u;

      __builtin_memset (u.b, 0, sizeof (uint64_t));
#ifndef WORDS_BIGENDIAN
      __builtin_memcpy (u.b, val, channel->element_size);
#else
      __builtin_memcpy (u.b + sizeof (uint64_t) - channel->element_size, val,
			channel->element_size);
#endif
      if (selected == NULL)
	__go_send_small (channel, u.v, 0);
      else
	*selected = __go_send_nonblocking_small (channel, u.v);
    }
  else
    {
      if (selected == NULL)
	__go_send_big (channel, val, 0);
      else
	*selected = __go_send_nonblocking_big (channel, val);
    }
}

extern void chanrecv (unsigned char *, unsigned char *, _Bool *, _Bool *)
  asm ("libgo_reflect.reflect.chanrecv");

void
chanrecv (unsigned char *ch, unsigned char *val, _Bool *selected,
	  _Bool *received)
{
  struct __go_channel *channel = (struct __go_channel *) ch;

  if (channel->element_size <= sizeof (uint64_t))
    {
      union
      {
	char b[sizeof (uint64_t)];
	uint64_t v;
      } u;

      if (selected == NULL)
	u.v = __go_receive_small_closed (channel, 0, received);
      else
	{
	  struct __go_receive_nonblocking_small s;

	  s = __go_receive_nonblocking_small (channel);
	  *selected = s.__success || s.__closed;
	  if (received != NULL)
	    *received = s.__success;
	  u.v = s.__val;
	}

#ifndef WORDS_BIGENDIAN
      __builtin_memcpy (val, u.b, channel->element_size);
#else
      __builtin_memcpy (val, u.b + sizeof (uint64_t) - channel->element_size,
			channel->element_size);
#endif
    }
  else
    {
      if (selected == NULL)
	{
	  _Bool success;

	  success = __go_receive_big (channel, val, 0);
	  if (received != NULL)
	    *received = success;
	}
      else
	{
	  _Bool got;
	  _Bool closed;

	  got = __go_receive_nonblocking_big (channel, val, &closed);
	  *selected = got || closed;
	  if (received != NULL)
	    *received = got;
	}
    }
}

extern void chanclose (unsigned char *)
  asm ("libgo_reflect.reflect.chanclose");

void
chanclose (unsigned char *ch)
{
  struct __go_channel *channel = (struct __go_channel *) ch;

  __go_builtin_close (channel);
}

extern int32_t chanlen (unsigned char *) asm ("libgo_reflect.reflect.chanlen");

int32_t
chanlen (unsigned char *ch)
{
  struct __go_channel *channel = (struct __go_channel *) ch;

  return (int32_t) __go_chan_len (channel);
}

extern int32_t chancap (unsigned char *) asm ("libgo_reflect.reflect.chancap");

int32_t
chancap (unsigned char *ch)
{
  struct __go_channel *channel = (struct __go_channel *) ch;

  return (int32_t) __go_chan_cap (channel);
}
