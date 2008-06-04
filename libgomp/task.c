/* Copyright (C) 2007, 2008 Free Software Foundation, Inc.
   Contributed by Richard Henderson <rth@redhat.com>.

   This file is part of the GNU OpenMP Library (libgomp).

   Libgomp is free software; you can redistribute it and/or modify it
   under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   Libgomp is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
   more details.

   You should have received a copy of the GNU Lesser General Public License 
   along with libgomp; see the file COPYING.LIB.  If not, write to the
   Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
   MA 02110-1301, USA.  */

/* As a special exception, if you link this library with other files, some
   of which are compiled with GCC, to produce an executable, this library
   does not by itself cause the resulting executable to be covered by the
   GNU General Public License.  This exception does not however invalidate
   any other reasons why the executable file might be covered by the GNU
   General Public License.  */

/* This file handles the maintainence of tasks in response to task
   creation and termination.  */

#include "libgomp.h"
#include <stdlib.h>
#include <string.h>


/* Create a new task data structure.  */

void
gomp_init_task (struct gomp_task *task, struct gomp_task *parent_task,
		struct gomp_task_icv *prev_icv)
{
  task->parent = parent_task;
  task->icv = *prev_icv;
  task->kind = GOMP_TASK_IMPLICIT;
  task->in_taskwait = false;
  task->children = NULL;
  gomp_sem_init (&task->taskwait_sem, 0);
}

/* Clean up a task, after completing it.  */

void
gomp_end_task (void)
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_task *task = thr->task;

  gomp_finish_task (task);
  thr->task = task->parent;
}

static inline void
gomp_clear_parent (struct gomp_task *children)
{
  struct gomp_task *task = children;

  if (task)
    do
      {
	task->parent = NULL;
	task = task->next_child;
      }
    while (task != children);
}

/* Called when encountering an explicit task directive.  If IF_CLAUSE is
   false, then we must not delay in executing the task.  If UNTIED is true,
   then the task may be executed by any member of the team.  */

void
GOMP_task (void (*fn) (void *), void *data, void (*cpyfn) (void *, void *),
	   long arg_size, long arg_align, bool if_clause,
	   unsigned flags __attribute__((unused)))
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_team *team = thr->ts.team;

#ifdef HAVE_BROKEN_POSIX_SEMAPHORES
  /* If pthread_mutex_* is used for omp_*lock*, then each task must be
     tied to one thread all the time.  This means UNTIED tasks must be
     tied and if CPYFN is non-NULL IF(0) must be forced, as CPYFN
     might be running on different thread than FN.  */
  if (cpyfn)
    if_clause = false;
  if (flags & 1)
    flags &= ~1;
#endif

  if (!if_clause || team == NULL
      || team->task_count > 64 * team->nthreads)
    {
      struct gomp_task task;

      gomp_init_task (&task, thr->task, gomp_icv (false));
      task.kind = GOMP_TASK_IFFALSE;
      thr->task = &task;
      if (__builtin_expect (cpyfn != NULL, 0))
	{
	  char buf[arg_size + arg_align - 1];
	  char *arg = (char *) (((uintptr_t) buf + arg_align - 1)
				& ~(uintptr_t) (arg_align - 1));
	  cpyfn (arg, data);
	  fn (arg);
	}
      else
	fn (data);
      if (task.children)
	{
	  gomp_mutex_lock (&team->task_lock);
	  gomp_clear_parent (task.children);
	  gomp_mutex_unlock (&team->task_lock);
	}
      gomp_end_task ();
    }
  else
    {
      struct gomp_task *task;
      struct gomp_task *parent = thr->task;
      char *arg;
      bool do_wake;

      task = gomp_malloc (sizeof (*task) + arg_size + arg_align - 1);
      arg = (char *) (((uintptr_t) (task + 1) + arg_align - 1)
		      & ~(uintptr_t) (arg_align - 1));
      gomp_init_task (task, parent, gomp_icv (false));
      task->kind = GOMP_TASK_IFFALSE;
      thr->task = task;
      if (cpyfn)
	cpyfn (arg, data);
      else
	memcpy (arg, data, arg_size);
      thr->task = parent;
      task->kind = GOMP_TASK_WAITING;
      task->fn = fn;
      task->fn_data = arg;
      gomp_mutex_lock (&team->task_lock);
      if (parent->children)
	{
	  task->next_child = parent->children;
	  task->prev_child = parent->children->prev_child;
	  task->next_child->prev_child = task;
	  task->prev_child->next_child = task;
	}
      else
	{
	  task->next_child = task;
	  task->prev_child = task;
	}
      parent->children = task;
      if (team->task_queue)
	{
	  task->next_queue = team->task_queue;
	  task->prev_queue = team->task_queue->prev_queue;
	  task->next_queue->prev_queue = task;
	  task->prev_queue->next_queue = task;
	}
      else
	{
	  task->next_queue = task;
	  task->prev_queue = task;
	  team->task_queue = task;
	}
      if (team->task_count++ == 0)
	gomp_team_barrier_set_task_pending (&team->barrier);
      do_wake = team->task_running_count < team->nthreads;
      gomp_mutex_unlock (&team->task_lock);
      if (do_wake)
	gomp_team_barrier_wake (&team->barrier, 1);
    }
}

void
gomp_barrier_handle_tasks (gomp_barrier_state_t state)
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_team *team = thr->ts.team;
  struct gomp_task *task = thr->task;
  struct gomp_task *child_task = NULL;
  struct gomp_task *to_free = NULL;

  gomp_mutex_lock (&team->task_lock);
  if (gomp_barrier_last_thread (state))
    {
      if (team->task_count == 0)
	{
	  gomp_team_barrier_done (&team->barrier, state);
	  gomp_mutex_unlock (&team->task_lock);
	  gomp_team_barrier_wake (&team->barrier, 0);
	  return;
	}
      gomp_team_barrier_set_waiting_for_tasks (&team->barrier);
    }

  while (1)
    {
      if (team->task_queue != NULL)
	{
	  struct gomp_task *parent;

	  child_task = team->task_queue;
	  parent = child_task->parent;
	  if (parent && parent->children == child_task)
	    parent->children = child_task->next_child;
	  child_task->prev_queue->next_queue = child_task->next_queue;
	  child_task->next_queue->prev_queue = child_task->prev_queue;
	  if (child_task->next_queue != child_task)
	    team->task_queue = child_task->next_queue;
	  else
	    team->task_queue = NULL;
	  child_task->kind = GOMP_TASK_TIED;
	  team->task_running_count++;
	  if (team->task_count == team->task_running_count)
	    gomp_team_barrier_clear_task_pending (&team->barrier);
	}
      gomp_mutex_unlock (&team->task_lock);
      if (to_free)
	{
	  gomp_finish_task (to_free);
	  free (to_free);
	  to_free = NULL;
	}
      if (child_task)
	{
	  thr->task = child_task;
	  child_task->fn (child_task->fn_data);
	  thr->task = task;
	}
      else
	return;
      gomp_mutex_lock (&team->task_lock);
      if (child_task)
	{
	  struct gomp_task *parent = child_task->parent;
	  if (parent)
	    {
	      child_task->prev_child->next_child = child_task->next_child;
	      child_task->next_child->prev_child = child_task->prev_child;
	      if (parent->children == child_task)
		{
		  if (child_task->next_child != child_task)
		    parent->children = child_task->next_child;
		  else
		    {
		      parent->children = NULL;
		      if (parent->in_taskwait)
			gomp_sem_post (&parent->taskwait_sem);
		    }
		}
	    }
	  gomp_clear_parent (child_task->children);
	  to_free = child_task;
	  child_task = NULL;
	  team->task_running_count--;
	  if (--team->task_count == 0
	      && gomp_team_barrier_waiting_for_tasks (&team->barrier))
	    {
	      gomp_team_barrier_done (&team->barrier, state);
	      gomp_mutex_unlock (&team->task_lock);
	      gomp_team_barrier_wake (&team->barrier, 0);
	    }
	}
    }
}

/* Called when encountering a taskwait directive.  */

void
GOMP_taskwait (void)
{
  struct gomp_thread *thr = gomp_thread ();
  struct gomp_team *team = thr->ts.team;
  struct gomp_task *task = thr->task;
  struct gomp_task *child_task = NULL;
  struct gomp_task *to_free = NULL;

  if (task == NULL || task->children == NULL)
    return;
  gomp_mutex_lock (&team->task_lock);
  while (1)
    {
      if (task->children == NULL)
	{
	  gomp_mutex_unlock (&team->task_lock);
	  if (to_free)
	    {
	      gomp_finish_task (to_free);
	      free (to_free);
	    }
	  return;
	}
      if (task->children->kind == GOMP_TASK_WAITING)
	{
	  child_task = task->children;
	  task->children = child_task->next_child;
	  child_task->prev_queue->next_queue = child_task->next_queue;
	  child_task->next_queue->prev_queue = child_task->prev_queue;
	  if (team->task_queue == child_task)
	    {
	      if (child_task->next_queue != child_task)
		team->task_queue = child_task->next_queue;
	      else
		team->task_queue = NULL;
	    }
	  child_task->kind = GOMP_TASK_TIED;
	  team->task_running_count++;
	  if (team->task_count == team->task_running_count)
	    gomp_team_barrier_clear_task_pending (&team->barrier);
	}
      else
	/* All tasks we are waiting for are already running
	   in other threads.  Wait for them.  */
	task->in_taskwait = true;
      gomp_mutex_unlock (&team->task_lock);
      if (to_free)
	{
	  gomp_finish_task (to_free);
	  free (to_free);
	  to_free = NULL;
	}
      if (child_task)
	{
	  thr->task = child_task;
	  child_task->fn (child_task->fn_data);
	  thr->task = task;
	}
      else
	{
	  gomp_sem_wait (&task->taskwait_sem);
	  task->in_taskwait = false;
	  return;
	}
      gomp_mutex_lock (&team->task_lock);
      if (child_task)
	{
	  child_task->prev_child->next_child = child_task->next_child;
	  child_task->next_child->prev_child = child_task->prev_child;
	  if (task->children == child_task)
	    {
	      if (child_task->next_child != child_task)
		task->children = child_task->next_child;
	      else
		task->children = NULL;
	    }
	  gomp_clear_parent (child_task->children);
	  to_free = child_task;
	  child_task = NULL;
	  team->task_count--;
	  team->task_running_count--;
	}
    }
}
