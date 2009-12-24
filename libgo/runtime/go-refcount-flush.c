/* go-refcount-flush.c -- flush reference count queue.

   Copyright 2009 The Go Authors. All rights reserved.
   Use of this source code is governed by a BSD-style
   license that can be found in the LICENSE file.  */

#include <assert.h>
#include <stdlib.h>

#include "go-alloc.h"
#include "go-refcount.h"
#include "runtime.h"
#include "malloc.h"

/* The head of the list of reference count queues.  This variable is
   maintained by the code generated by the compiler.  It exists in
   order to initialize the __caller field of struct __go_refcount.  */

__thread struct __go_refcount *__go_refcount_head;

void
__go_refcount_flush_queue (struct __go_refcount *queue)
{
  unsigned int allocated;
  struct __go_refcount_msg *msg;
  struct __go_refcount *prc;
  unsigned int increment_count;
  unsigned int count;
  unsigned int offset;
  unsigned int i;
  unsigned int decrement_count;

  /* Gather all the increments.  We gather up all the increments in
     this function and in all the callers.  */
  allocated = 0;
  msg = NULL;
  increment_count = 0;
  for (prc = queue; prc != NULL; prc = prc->__caller)
    {
      count = prc->__increment_copy_count;
      if (count > 0)
	{
	  offset = (prc->__decrement_new_count
		    + prc->__decrement_computed_count
		    + prc->__decrement_old_count);
	  for (i = offset; i < offset + count; ++i)
	    {
	      struct __go_refcount_entry *pre;
	      void *value;
	      struct __go_refcount *qrc;
	      _Bool found;
	      unsigned int up_count;
	      const unsigned int up_count_limit = 20;

	      pre = &prc->__entries[i];
	      value = pre->__value;
	      if (value == NULL)
		continue;
	      pre->__value = NULL;

	      /* If we find an increment for something which has a
		 decrement queued up, we can discard both the
		 increment and the decrement.  Don't look up more than
		 20 stack frames, so that this doesn't take too
		 long.  */
	      found = 0;
	      for (qrc = queue, up_count = 0;
		   qrc != NULL && up_count < up_count_limit && !found;
		   qrc = qrc->__caller, ++up_count)
		{
		  unsigned int c;
		  unsigned int j;

		  c = (qrc->__decrement_new_count
		       + qrc->__decrement_computed_count
		       + qrc->__decrement_old_count);
		  for (j = 0; j < c; ++j)
		    {
		      struct __go_refcount_entry *qre;

		      qre = &qrc->__entries[j];
		      if (qre->__value == value)
			{
			  qre->__value = NULL;
			  found = 1;
			  break;
			}
		    }
		}

	      if (!found)
		{
		  if (qrc != NULL)
		    qrc->__did_not_scan_decrements = 1;
		  if (increment_count >= allocated)
		    {
		      unsigned int new_allocated;
		      struct __go_refcount_msg *new_msg;

		      new_allocated = allocated == 0 ? 32 : allocated * 2;
		      new_msg = __go_alloc (sizeof (struct __go_refcount_msg)
					    + new_allocated * sizeof (void *));
		      if (allocated > 0)
			{
			  __builtin_memcpy (new_msg, msg,
					    (sizeof (struct __go_refcount_msg)
					     + allocated * sizeof (void *)));
			  __go_free (msg);
			}
		      allocated = new_allocated;
		      msg = new_msg;
		    }
		  msg->__pointers[increment_count] = value;
		  ++increment_count;
		}
	    }
	}

      /* If we've already scanned the callers for increments, we can
	 stop now.  */
      if (prc->__callers_were_scanned)
	break;

      /* We are going to scan all the callers of PRC for increments;
	 mark it now to avoid future scanning.  */
      prc->__callers_were_scanned = 1;
    }

  /* Gather up the decrements.  We can only send the decrements from
     the current function.  That is because callers may have queued up
     decrements for temporary objects they created in order to call
     this function.  If we apply those decrements now, we will discard
     objects that we might still be using.  */
  decrement_count = 0;

  if (queue->__did_not_scan_decrements)
    {
      if (queue->__caller != NULL)
	queue->__caller->__did_not_scan_decrements = 1;
      count = (queue->__decrement_new_count
	       + queue->__decrement_computed_count
	       + queue->__decrement_old_count);
      offset = 0;
    }
  else
    {
      /* Any decrements in __decrement_new_count can be freed
	 immediately.  We did not see any increments for these
	 objects; if we did, the increment would have cleared the
	 decrement in the loop above.  */
      count = queue->__decrement_new_count;
      for (i = 0; i < count; ++i)
	{
	  struct __go_refcount_entry *pre;
	  void *value;
	  size_t size;

	  pre = &queue->__entries[i];
	  value = pre->__value;
	  if (value == NULL)
	    continue;
	  pre->__value = NULL;

	  /* FIXME: For debugging.  We can't just free a slice because
	     it has an embedded pointer.  */
	  if (pre->__descriptor->__code != GO_SLICE
	      && mlookup (value, NULL, NULL, NULL))
	    {
	      size = pre->__descriptor->__size;

#if 0
	      /* FIXME: This is wrong if the value has embedded pointers.  */
	      __builtin_memset (value, 0xa5, size);
	      __go_free (value);
#endif
	    }
	}

      count = queue->__decrement_computed_count + queue->__decrement_old_count;
      offset = queue->__decrement_new_count;
    }

  for (i = offset; i < offset + count; ++i)
    {
      struct __go_refcount_entry *pre;
      void *value;

      pre = &queue->__entries[i];
      value = pre->__value;
      if (value == NULL)
	continue;
      pre->__value = NULL;

      if (increment_count + decrement_count >= allocated)
	{
	  unsigned int new_allocated;
	  struct __go_refcount_msg *new_msg;

	  new_allocated = allocated == 0 ? 32 : allocated * 2;
	  new_msg = __go_alloc (sizeof (struct __go_refcount_msg)
				+ new_allocated * sizeof (void *));
	  if (allocated > 0)
	    {
	      __builtin_memcpy (new_msg, msg,
				(sizeof (struct __go_refcount_msg)
				 + allocated * sizeof (void *)));
	      __go_free (msg);
	    }
	  allocated = new_allocated;
	  msg = new_msg;
	}
      msg->__pointers[increment_count + decrement_count] = value;
      ++decrement_count;
    }

  if (increment_count == 0 && decrement_count == 0)
    assert (msg == NULL);
  else
    {
      msg->__increments = increment_count;
      msg->__decrements = decrement_count;
      __go_send_refcount_msg (msg);
    }
}
