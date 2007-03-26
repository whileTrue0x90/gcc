/* TLS emulation.
   Copyright (C) 2006 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs,
and to distribute those combinations without any restriction coming
from the use of this file.  (The General Public License restrictions
do apply in other respects; for example, they cover modification of
the file, and distribution when not linked into a combine
executable.)

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#include "tconfig.h"
#include "tsystem.h"
#include "coretypes.h"
#include "tm.h"
#include "gthr.h"

typedef unsigned int word __attribute__((mode(word)));
typedef unsigned int pointer __attribute__((mode(pointer)));

struct __emutls_object
{
  word size;
  word align;
  union {
    pointer offset;
    void *ptr;
  } loc;
  void *templ;
};

#ifdef __GTHREADS
#ifdef __GTHREAD_MUTEX_INIT
static __gthread_mutex_t emutls_mutex = __GTHREAD_MUTEX_INIT;
#else
static __gthread_mutex_t emutls_mutex;
#endif
static __gthread_key_t emutls_key;
static pointer emutls_size;

static void
emutls_destroy (void *ptr)
{
  void ***arr = (void ***) ptr;
  unsigned long int size = (unsigned long int) arr[0];
  ++arr;
  while (--size)
    {
      if (*arr)
	free ((*arr)[-1]);
      ++arr;
    }
  free (ptr);
}

static void
emutls_init (void)
{
#ifndef __GTHREAD_MUTEX_INIT
  __GTHREAD_MUTEX_INIT_FUNCTION (&emutls_mutex);
#endif
  if (__gthread_key_create (&emutls_key, emutls_destroy) != 0)
    abort ();
}
#endif

static void *
emutls_alloc (struct __emutls_object *obj)
{
  void *ptr;
  void *ret;

  /* We could use here posix_memalign if available and adjust
     emutls_destroy accordingly.  */
  if (obj->align <= sizeof (void *))
    {
      ptr = malloc (obj->size + sizeof (void *));
      if (ptr == NULL)
	abort ();
      ((void **) ptr)[0] = ptr;
      ret = ptr + sizeof (void *);
    }
  else
    {
      ptr = malloc (obj->size + sizeof (void *) + obj->align - 1);
      if (ptr == NULL)
	abort ();
      ret = (void *) (((pointer) (ptr + sizeof (void *) + obj->align - 1))
		      & ~(pointer)(obj->align - 1));
      ((void **) ret)[-1] = ptr;
    }

  if (obj->templ)
    memcpy (ret, obj->templ, obj->size);
  else
    memset (ret, 0, obj->size);

  return ret;
}

void *
__emutls_get_address (struct __emutls_object *obj)
{
  if (! __gthread_active_p ())
    {
      if (__builtin_expect (obj->loc.ptr == NULL, 0))
	obj->loc.ptr = emutls_alloc (obj);
      return obj->loc.ptr;
    }

#ifndef __GTHREADS
  abort ();
#else
  pointer offset;

  if (__builtin_expect (obj->loc.offset == 0, 0))
    {
      static __gthread_once_t once = __GTHREAD_ONCE_INIT;
      __gthread_once (&once, emutls_init);
      __gthread_mutex_lock (&emutls_mutex);
      offset = ++emutls_size;
      obj->loc.offset = offset;
      __gthread_mutex_unlock (&emutls_mutex);
    }
  else
    offset = obj->loc.offset;

  void **arr = (void **) __gthread_getspecific (emutls_key);
  if (__builtin_expect (arr == NULL, 0))
    {
      pointer size = offset + 32;
      arr = calloc (size, sizeof (void *));
      if (arr == NULL)
	abort ();
      arr[0] = (void *) size;
      __gthread_setspecific (emutls_key, (void *) arr);
    }
  else if (__builtin_expect (offset >= (pointer) arr[0], 0))
    {
      pointer orig_size = (pointer) arr[0];
      pointer size = orig_size * 2;
      if (offset >= size)
	size = offset + 32;
      arr = realloc (arr, size * sizeof (void *));
      if (arr == NULL)
	abort ();
      memset (arr + orig_size, 0, (size - orig_size) * sizeof (void *));
      __gthread_setspecific (emutls_key, (void *) arr);
    }

  void *ret = arr[offset];
  if (__builtin_expect (ret == NULL, 0))
    {
      ret = emutls_alloc (obj);
      arr[offset] = ret;
    }
  return ret;
#endif
}

void
__emutls_register_common (struct __emutls_object *obj,
			  word size, word align, void *templ)
{
  if (obj->size < size)
    {
      obj->size = size;
      obj->templ = NULL;
    }
  if (obj->align < align)
    obj->align = align;
  if (templ && size == obj->size)
    obj->templ = templ;
}
