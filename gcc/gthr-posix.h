/* Threads compatibility routines for libgcc2 and libobjc.  */
/* Compile this one with gcc.  */
/* Copyright (C) 1997, 1999, 2000 Free Software Foundation, Inc.

This file is part of GNU CC.

GNU CC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU CC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU CC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

#ifndef __gthr_posix_h
#define __gthr_posix_h

/* POSIX threads specific definitions.
   Easy, since the interface is just one-to-one mapping. */

#define __GTHREADS 1

#include <pthread.h>

typedef pthread_key_t __gthread_key_t;
typedef pthread_once_t __gthread_once_t;
typedef pthread_mutex_t __gthread_mutex_t;

#define __GTHREAD_MUTEX_INIT PTHREAD_MUTEX_INITIALIZER
#define __GTHREAD_ONCE_INIT PTHREAD_ONCE_INIT

#if SUPPORTS_WEAK && GTHREAD_USE_WEAK

#pragma weak pthread_once
#pragma weak pthread_key_create
#pragma weak pthread_key_delete
#pragma weak pthread_getspecific
#pragma weak pthread_setspecific
#pragma weak pthread_create

#pragma weak pthread_mutex_lock 
#pragma weak pthread_mutex_trylock 
#pragma weak pthread_mutex_unlock 

#ifdef _LIBOBJC
/* Objective C. */
#pragma weak pthread_cond_broadcast
#pragma weak pthread_cond_destroy
#pragma weak pthread_cond_init
#pragma weak pthread_cond_signal
#pragma weak pthread_cond_wait
#pragma weak pthread_exit
#pragma weak pthread_mutex_init
#pragma weak pthread_mutex_destroy
#pragma weak pthread_self
#pragma weak sched_yield
#endif

static void *__gthread_active_ptr = &pthread_create;

static inline int
__gthread_active_p (void)
{
  return __gthread_active_ptr != 0;
}

#else /* not SUPPORTS_WEAK */

static inline int
__gthread_active_p (void)
{
  return 1;
}

#endif /* SUPPORTS_WEAK */

#ifdef _LIBOBJC

/* Key structure for maintaining thread specific storage */
static pthread_key_t _objc_thread_storage;

/* Thread local storage for a single thread */
static void *thread_local_storage = NULL;

/* Backend initialization functions */

/* Initialize the threads subsystem. */
static inline int
__gthread_objc_init_thread_system(void)
{
  if (__gthread_active_p ())
    /* Initialize the thread storage key */
    return pthread_key_create(&_objc_thread_storage, NULL);
  else
    return -1;
}

/* Close the threads subsystem. */
static inline int
__gthread_objc_close_thread_system(void)
{
  if (__gthread_active_p ())
    return 0;
  else
    return -1;
}

/* Backend thread functions */

/* Create a new thread of execution. */
static inline objc_thread_t
__gthread_objc_thread_detach(void (*func)(void *), void *arg)
{
  objc_thread_t thread_id;
  pthread_t new_thread_handle;

  if (!__gthread_active_p ())
    return NULL;
 
  if ( !(pthread_create(&new_thread_handle, NULL, (void *)func, arg)) )
    thread_id = *(objc_thread_t *)&new_thread_handle;
  else
    thread_id = NULL;
  
  return thread_id;
}

/* Set the current thread's priority. */
static inline int
__gthread_objc_thread_set_priority(int priority)
{
  /* Not implemented yet */
  return -1;
}

/* Return the current thread's priority. */
static inline int
__gthread_objc_thread_get_priority(void)
{
  if (__gthread_active_p ())
    /* Not implemented yet */
    return -1;
  else
    return OBJC_THREAD_INTERACTIVE_PRIORITY;
}

/* Yield our process time to another thread. */
static inline void
__gthread_objc_thread_yield(void)
{
  if (__gthread_active_p ())
    sched_yield();
}

/* Terminate the current thread. */
static inline int
__gthread_objc_thread_exit(void)
{
  if (__gthread_active_p ())
    /* exit the thread */
    pthread_exit(&__objc_thread_exit_status);

  /* Failed if we reached here */
  return -1;
}

/* Returns an integer value which uniquely describes a thread. */
static inline objc_thread_t
__gthread_objc_thread_id(void)
{
  if (__gthread_active_p ())
    {
      pthread_t self = pthread_self();

      return *(objc_thread_t *)&self;
    }
  else
    return (objc_thread_t)1;
}

/* Sets the thread's local storage pointer. */
static inline int
__gthread_objc_thread_set_data(void *value)
{
  if (__gthread_active_p ())
    return pthread_setspecific(_objc_thread_storage, value);
  else
    {
      thread_local_storage = value;
      return 0;
    }
}

/* Returns the thread's local storage pointer. */
static inline void *
__gthread_objc_thread_get_data(void)
{
  if (__gthread_active_p ())
    return pthread_getspecific(_objc_thread_storage);
  else
    return thread_local_storage;
}

/* Backend mutex functions */

/* Allocate a mutex. */
static inline int
__gthread_objc_mutex_allocate(objc_mutex_t mutex)
{
  if (__gthread_active_p ())
    {
      mutex->backend = objc_malloc(sizeof(pthread_mutex_t));

      if (pthread_mutex_init((pthread_mutex_t *)mutex->backend, NULL))
	{
	  objc_free(mutex->backend);
	  mutex->backend = NULL;
	  return -1;
	}
    }

  return 0;
}

/* Deallocate a mutex. */
static inline int
__gthread_objc_mutex_deallocate(objc_mutex_t mutex)
{
  if (__gthread_active_p ())
    {
      int count;

      /*
       * Posix Threads specifically require that the thread be unlocked
       * for pthread_mutex_destroy to work.
       */

      do
	{
	  count = pthread_mutex_unlock((pthread_mutex_t *)mutex->backend);
	  if (count < 0)
	    return -1;
	}
      while (count);

      if (pthread_mutex_destroy((pthread_mutex_t *)mutex->backend))
	return -1;

      objc_free(mutex->backend);
      mutex->backend = NULL;
    }
  return 0;
}

/* Grab a lock on a mutex. */
static inline int
__gthread_objc_mutex_lock(objc_mutex_t mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_lock((pthread_mutex_t *)mutex->backend);
  else
    return 0;
}

/* Try to grab a lock on a mutex. */
static inline int
__gthread_objc_mutex_trylock(objc_mutex_t mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_trylock((pthread_mutex_t *)mutex->backend);
  else
    return 0;
}

/* Unlock the mutex */
static inline int
__gthread_objc_mutex_unlock(objc_mutex_t mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_unlock((pthread_mutex_t *)mutex->backend);
  else
    return 0;
}

/* Backend condition mutex functions */

/* Allocate a condition. */
static inline int
__gthread_objc_condition_allocate(objc_condition_t condition)
{
  if (__gthread_active_p ())
    {
      condition->backend = objc_malloc(sizeof(pthread_cond_t));

      if (pthread_cond_init((pthread_cond_t *)condition->backend, NULL))
	{
	  objc_free(condition->backend);
	  condition->backend = NULL;
	  return -1;
	}
    }

  return 0;
}

/* Deallocate a condition. */
static inline int
__gthread_objc_condition_deallocate(objc_condition_t condition)
{
  if (__gthread_active_p ())
    {
      if (pthread_cond_destroy((pthread_cond_t *)condition->backend))
	return -1;

      objc_free(condition->backend);
      condition->backend = NULL;
    }
  return 0;
}

/* Wait on the condition */
static inline int
__gthread_objc_condition_wait(objc_condition_t condition, objc_mutex_t mutex)
{
  if (__gthread_active_p ())
    return pthread_cond_wait((pthread_cond_t *)condition->backend,
			   (pthread_mutex_t *)mutex->backend);
  else
    return 0;
}

/* Wake up all threads waiting on this condition. */
static inline int
__gthread_objc_condition_broadcast(objc_condition_t condition)
{
  if (__gthread_active_p ())
    return pthread_cond_broadcast((pthread_cond_t *)condition->backend);
  else
    return 0;
}

/* Wake up one thread waiting on this condition. */
static inline int
__gthread_objc_condition_signal(objc_condition_t condition)
{
  if (__gthread_active_p ())
    return pthread_cond_signal((pthread_cond_t *)condition->backend);
  else
    return 0;
}

#else /* _LIBOBJC */

static inline int
__gthread_once (__gthread_once_t *once, void (*func) (void))
{
  if (__gthread_active_p ())
    return pthread_once (once, func);
  else
    return -1;
}

static inline int
__gthread_key_create (__gthread_key_t *key, void (*dtor) (void *))
{
  return pthread_key_create (key, dtor);
}

static inline int
__gthread_key_dtor (__gthread_key_t key, void *ptr)
{
  /* Just reset the key value to zero. */
  if (ptr)
    return pthread_setspecific (key, 0);
  else
    return 0;
}

static inline int
__gthread_key_delete (__gthread_key_t key)
{
  return pthread_key_delete (key);
}

static inline void *
__gthread_getspecific (__gthread_key_t key)
{
  return pthread_getspecific (key);
}

static inline int
__gthread_setspecific (__gthread_key_t key, const void *ptr)
{
  return pthread_setspecific (key, ptr);
}

static inline int
__gthread_mutex_lock (__gthread_mutex_t *mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_lock (mutex);
  else
    return 0;
}

static inline int
__gthread_mutex_trylock (__gthread_mutex_t *mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_trylock (mutex);
  else
    return 0;
}

static inline int
__gthread_mutex_unlock (__gthread_mutex_t *mutex)
{
  if (__gthread_active_p ())
    return pthread_mutex_unlock (mutex);
  else
    return 0;
}

#endif /* _LIBOBJC */

#endif /* not __gthr_posix_h */
