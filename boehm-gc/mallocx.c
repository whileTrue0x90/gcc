/*
 * Copyright 1988, 1989 Hans-J. Boehm, Alan J. Demers
 * Copyright (c) 1991-1994 by Xerox Corporation.  All rights reserved.
 * Copyright (c) 1996 by Silicon Graphics.  All rights reserved.
 * Copyright (c) 2000 by Hewlett-Packard Company.  All rights reserved.
 *
 * THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 * OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 * Permission is hereby granted to use or copy this program
 * for any purpose,  provided the above notices are retained on all copies.
 * Permission to modify the code and to distribute modified code is granted,
 * provided the above notices are retained, and a notice that the code was
 * modified is included with the above copyright notice.
 */

/*
 * These are extra allocation routines which are likely to be less
 * frequently used than those in malloc.c.  They are separate in the
 * hope that the .o file will be excluded from statically linked
 * executables.  We should probably break this up further.
 */

#include <stdio.h>
#include "private/gc_priv.h"

extern ptr_t GC_clear_stack();  /* in misc.c, behaves like identity */
void GC_extend_size_map();      /* in misc.c. */
GC_bool GC_alloc_reclaim_list();	/* in malloc.c */

/* Some externally visible but unadvertised variables to allow access to */
/* free lists from inlined allocators without including gc_priv.h	 */
/* or introducing dependencies on internal data structure layouts.	 */
ptr_t * GC_CONST GC_objfreelist_ptr = GC_objfreelist;
ptr_t * GC_CONST GC_aobjfreelist_ptr = GC_aobjfreelist;
ptr_t * GC_CONST GC_uobjfreelist_ptr = GC_uobjfreelist;
# ifdef ATOMIC_UNCOLLECTABLE
    ptr_t * GC_CONST GC_auobjfreelist_ptr = GC_auobjfreelist;
# endif


GC_PTR GC_generic_or_special_malloc(lb,knd)
word lb;
int knd;
{
    switch(knd) {
#     ifdef STUBBORN_ALLOC
	case STUBBORN:
	    return(GC_malloc_stubborn((size_t)lb));
#     endif
	case PTRFREE:
	    return(GC_malloc_atomic((size_t)lb));
	case NORMAL:
	    return(GC_malloc((size_t)lb));
	case UNCOLLECTABLE:
	    return(GC_malloc_uncollectable((size_t)lb));
#       ifdef ATOMIC_UNCOLLECTABLE
	  case AUNCOLLECTABLE:
	    return(GC_malloc_atomic_uncollectable((size_t)lb));
#	endif /* ATOMIC_UNCOLLECTABLE */
	default:
	    return(GC_generic_malloc(lb,knd));
    }
}


/* Change the size of the block pointed to by p to contain at least   */
/* lb bytes.  The object may be (and quite likely will be) moved.     */
/* The kind (e.g. atomic) is the same as that of the old.	      */
/* Shrinking of large blocks is not implemented well.                 */
# ifdef __STDC__
    GC_PTR GC_realloc(GC_PTR p, size_t lb)
# else
    GC_PTR GC_realloc(p,lb)
    GC_PTR p;
    size_t lb;
# endif
{
register struct hblk * h;
register hdr * hhdr;
register word sz;	 /* Current size in bytes	*/
register word orig_sz;	 /* Original sz in bytes	*/
int obj_kind;

    if (p == 0) return(GC_malloc(lb));	/* Required by ANSI */
    h = HBLKPTR(p);
    hhdr = HDR(h);
    sz = hhdr -> hb_sz;
    obj_kind = hhdr -> hb_obj_kind;
    sz = WORDS_TO_BYTES(sz);
    orig_sz = sz;

    if (sz > MAXOBJBYTES) {
	/* Round it up to the next whole heap block */
	  register word descr;
	  
	  sz = (sz+HBLKSIZE-1) & (~HBLKMASK);
	  hhdr -> hb_sz = BYTES_TO_WORDS(sz);
	  descr = GC_obj_kinds[obj_kind].ok_descriptor;
          if (GC_obj_kinds[obj_kind].ok_relocate_descr) descr += sz;
          hhdr -> hb_descr = descr;
	  if (IS_UNCOLLECTABLE(obj_kind)) GC_non_gc_bytes += (sz - orig_sz);
	  /* Extra area is already cleared by GC_alloc_large_and_clear. */
    }
    if (ADD_SLOP(lb) <= sz) {
	if (lb >= (sz >> 1)) {
#	    ifdef STUBBORN_ALLOC
	        if (obj_kind == STUBBORN) GC_change_stubborn(p);
#	    endif
	    if (orig_sz > lb) {
	      /* Clear unneeded part of object to avoid bogus pointer */
	      /* tracing.					      */
	      /* Safe for stubborn objects.			      */
	        BZERO(((ptr_t)p) + lb, orig_sz - lb);
	    }
	    return(p);
	} else {
	    /* shrink */
	      GC_PTR result =
	      		GC_generic_or_special_malloc((word)lb, obj_kind);

	      if (result == 0) return(0);
	          /* Could also return original object.  But this 	*/
	          /* gives the client warning of imminent disaster.	*/
	      BCOPY(p, result, lb);
#	      ifndef IGNORE_FREE
	        GC_free(p);
#	      endif
	      return(result);
	}
    } else {
	/* grow */
	  GC_PTR result =
	  	GC_generic_or_special_malloc((word)lb, obj_kind);

	  if (result == 0) return(0);
	  BCOPY(p, result, sz);
#	  ifndef IGNORE_FREE
	    GC_free(p);
#	  endif
	  return(result);
    }
}

# if defined(REDIRECT_MALLOC) && !defined(REDIRECT_REALLOC)
#   define REDIRECT_REALLOC GC_realloc
# endif

# ifdef REDIRECT_REALLOC

/* As with malloc, avoid two levels of extra calls here.	*/
# ifdef GC_ADD_CALLER
#   define RA GC_RETURN_ADDR,
# else
#   define RA
# endif
# define GC_debug_realloc_replacement(p, lb) \
	GC_debug_realloc(p, lb, RA "unknown", 0)

# ifdef __STDC__
    GC_PTR realloc(GC_PTR p, size_t lb)
# else
    GC_PTR realloc(p,lb)
    GC_PTR p;
    size_t lb;
# endif
  {
    return(REDIRECT_REALLOC(p, lb));
  }

# undef GC_debug_realloc_replacement
# endif /* REDIRECT_REALLOC */


/* The same thing, except caller does not hold allocation lock.	*/
/* We avoid holding allocation lock while we clear memory.	*/
ptr_t GC_generic_malloc_ignore_off_page(lb, k)
register size_t lb;
register int k;
{
    register ptr_t result;
    word lw;
    word n_blocks;
    GC_bool init;
    DCL_LOCK_STATE;
    
    if (SMALL_OBJ(lb))
        return(GC_generic_malloc((word)lb, k));
    lw = ROUNDED_UP_WORDS(lb);
    n_blocks = OBJ_SZ_TO_BLOCKS(lw);
    init = GC_obj_kinds[k].ok_init;
    if (GC_have_errors) GC_print_all_errors();
    GC_INVOKE_FINALIZERS();
    DISABLE_SIGNALS();
    LOCK();
    result = (ptr_t)GC_alloc_large(lw, k, IGNORE_OFF_PAGE);
    if (0 != result) {
        if (GC_debugging_started) {
	    BZERO(result, n_blocks * HBLKSIZE);
        } else {
#           ifdef THREADS
	      /* Clear any memory that might be used for GC descriptors */
	      /* before we release the lock.			      */
	        ((word *)result)[0] = 0;
	        ((word *)result)[1] = 0;
	        ((word *)result)[lw-1] = 0;
	        ((word *)result)[lw-2] = 0;
#	    endif
        }
    }
    GC_words_allocd += lw;
    UNLOCK();
    ENABLE_SIGNALS();
    if (0 == result) {
        return((*GC_oom_fn)(lb));
    } else {
    	if (init && !GC_debugging_started) {
	    BZERO(result, n_blocks * HBLKSIZE);
        }
        return(result);
    }
}

# if defined(__STDC__) || defined(__cplusplus)
  void * GC_malloc_ignore_off_page(size_t lb)
# else
  char * GC_malloc_ignore_off_page(lb)
  register size_t lb;
# endif
{
    return((GC_PTR)GC_generic_malloc_ignore_off_page(lb, NORMAL));
}

# if defined(__STDC__) || defined(__cplusplus)
  void * GC_malloc_atomic_ignore_off_page(size_t lb)
# else
  char * GC_malloc_atomic_ignore_off_page(lb)
  register size_t lb;
# endif
{
    return((GC_PTR)GC_generic_malloc_ignore_off_page(lb, PTRFREE));
}

/* Increment GC_words_allocd from code that doesn't have direct access 	*/
/* to GC_arrays.							*/
# ifdef __STDC__
void GC_incr_words_allocd(size_t n)
{
    GC_words_allocd += n;
}

/* The same for GC_mem_freed.				*/
void GC_incr_mem_freed(size_t n)
{
    GC_mem_freed += n;
}
# endif /* __STDC__ */

/* Analogous to the above, but assumes a small object size, and 	*/
/* bypasses MERGE_SIZES mechanism.  Used by gc_inline.h.		*/
ptr_t GC_generic_malloc_words_small_inner(lw, k)
register word lw;
register int k;
{
register ptr_t op;
register ptr_t *opp;
register struct obj_kind * kind = GC_obj_kinds + k;

    opp = &(kind -> ok_freelist[lw]);
    if( (op = *opp) == 0 ) {
        if (!GC_is_initialized) {
            GC_init_inner();
        }
	if (kind -> ok_reclaim_list != 0 || GC_alloc_reclaim_list(kind)) {
	    op = GC_clear_stack(GC_allocobj((word)lw, k));
	}
	if (op == 0) {
	    UNLOCK();
	    ENABLE_SIGNALS();
	    return ((*GC_oom_fn)(WORDS_TO_BYTES(lw)));
	}
    }
    *opp = obj_link(op);
    obj_link(op) = 0;
    GC_words_allocd += lw;
    return((ptr_t)op);
}

/* Analogous to the above, but assumes a small object size, and 	*/
/* bypasses MERGE_SIZES mechanism.  Used by gc_inline.h.		*/
#ifdef __STDC__
     ptr_t GC_generic_malloc_words_small(size_t lw, int k)
#else 
     ptr_t GC_generic_malloc_words_small(lw, k)
     register word lw;
     register int k;
#endif
{
register ptr_t op;
DCL_LOCK_STATE;

    if (GC_have_errors) GC_print_all_errors();
    GC_INVOKE_FINALIZERS();
    DISABLE_SIGNALS();
    LOCK();
    op = GC_generic_malloc_words_small_inner(lw, k);
    UNLOCK();
    ENABLE_SIGNALS();
    return((ptr_t)op);
}

#if defined(THREADS) && !defined(SRC_M3)

extern signed_word GC_mem_found;   /* Protected by GC lock.  */

#ifdef PARALLEL_MARK
volatile signed_word GC_words_allocd_tmp = 0;
                        /* Number of words of memory allocated since    */
                        /* we released the GC lock.  Instead of         */
                        /* reacquiring the GC lock just to add this in, */
                        /* we add it in the next time we reacquire      */
                        /* the lock.  (Atomically adding it doesn't     */
                        /* work, since we would have to atomically      */
                        /* update it in GC_malloc, which is too         */
                        /* expensive.                                   */
#endif /* PARALLEL_MARK */

/* See reclaim.c: */
extern ptr_t GC_reclaim_generic();

/* Return a list of 1 or more objects of the indicated size, linked	*/
/* through the first word in the object.  This has the advantage that	*/
/* it acquires the allocation lock only once, and may greatly reduce	*/
/* time wasted contending for the allocation lock.  Typical usage would */
/* be in a thread that requires many items of the same size.  It would	*/
/* keep its own free list in thread-local storage, and call		*/
/* GC_malloc_many or friends to replenish it.  (We do not round up	*/
/* object sizes, since a call indicates the intention to consume many	*/
/* objects of exactly this size.)					*/
/* We return the free-list by assigning it to *result, since it is	*/
/* not safe to return, e.g. a linked list of pointer-free objects,	*/
/* since the collector would not retain the entire list if it were 	*/
/* invoked just as we were returning.					*/
/* Note that the client should usually clear the link field.		*/
void GC_generic_malloc_many(lb, k, result)
register word lb;
register int k;
ptr_t *result;
{
ptr_t op;
ptr_t p;
ptr_t *opp;
word lw;
word my_words_allocd = 0;
struct obj_kind * ok = &(GC_obj_kinds[k]);
DCL_LOCK_STATE;

#   if defined(GATHERSTATS) || defined(PARALLEL_MARK)
#     define COUNT_ARG , &my_words_allocd
#   else
#     define COUNT_ARG
#     define NEED_TO_COUNT
#   endif
    if (!SMALL_OBJ(lb)) {
        op = GC_generic_malloc(lb, k);
        if(0 != op) obj_link(op) = 0;
	*result = op;
        return;
    }
    lw = ALIGNED_WORDS(lb);
    if (GC_have_errors) GC_print_all_errors();
    GC_INVOKE_FINALIZERS();
    DISABLE_SIGNALS();
    LOCK();
    if (!GC_is_initialized) GC_init_inner();
    /* Do our share of marking work */
      if (GC_incremental && !GC_dont_gc) {
        ENTER_GC();
	GC_collect_a_little_inner(1);
        EXIT_GC();
      }
    /* First see if we can reclaim a page of objects waiting to be */
    /* reclaimed.						   */
    {
	struct hblk ** rlh = ok -> ok_reclaim_list;
	struct hblk * hbp;
	hdr * hhdr;

	rlh += lw;
    	while ((hbp = *rlh) != 0) {
            hhdr = HDR(hbp);
            *rlh = hhdr -> hb_next;
	    hhdr -> hb_last_reclaimed = (unsigned short) GC_gc_no;
#	    ifdef PARALLEL_MARK
		{
		  signed_word my_words_allocd_tmp = GC_words_allocd_tmp;

		  GC_ASSERT(my_words_allocd_tmp >= 0);
		  /* We only decrement it while holding the GC lock.	*/
		  /* Thus we can't accidentally adjust it down in more	*/
		  /* than one thread simultaneously.			*/
		  if (my_words_allocd_tmp != 0) {
		    (void)GC_atomic_add(
				(volatile GC_word *)(&GC_words_allocd_tmp),
				(GC_word)(-my_words_allocd_tmp));
		    GC_words_allocd += my_words_allocd_tmp;
		  }
		}
		GC_acquire_mark_lock();
		++ GC_fl_builder_count;
		UNLOCK();
		ENABLE_SIGNALS();
		GC_release_mark_lock();
#	    endif
	    op = GC_reclaim_generic(hbp, hhdr, lw,
				    ok -> ok_init, 0 COUNT_ARG);
            if (op != 0) {
#	      ifdef NEED_TO_COUNT
		/* We are neither gathering statistics, nor marking in	*/
		/* parallel.  Thus GC_reclaim_generic doesn't count	*/
		/* for us.						*/
    		for (p = op; p != 0; p = obj_link(p)) {
        	  my_words_allocd += lw;
		}
#	      endif
#	      if defined(GATHERSTATS)
	        /* We also reclaimed memory, so we need to adjust 	*/
	        /* that count.						*/
		/* This should be atomic, so the results may be		*/
		/* inaccurate.						*/
		GC_mem_found += my_words_allocd;
#	      endif
#	      ifdef PARALLEL_MARK
		*result = op;
		(void)GC_atomic_add(
				(volatile GC_word *)(&GC_words_allocd_tmp),
				(GC_word)(my_words_allocd));
		GC_acquire_mark_lock();
		-- GC_fl_builder_count;
		if (GC_fl_builder_count == 0) GC_notify_all_builder();
		GC_release_mark_lock();
		(void) GC_clear_stack(0);
		return;
#	      else
	        GC_words_allocd += my_words_allocd;
	        goto out;
#	      endif
	    }
#	    ifdef PARALLEL_MARK
	      GC_acquire_mark_lock();
	      -- GC_fl_builder_count;
	      if (GC_fl_builder_count == 0) GC_notify_all_builder();
	      GC_release_mark_lock();
	      DISABLE_SIGNALS();
	      LOCK();
	      /* GC lock is needed for reclaim list access.	We	*/
	      /* must decrement fl_builder_count before reaquiring GC	*/
	      /* lock.  Hopefully this path is rare.			*/
#	    endif
    	}
    }
    /* Next try to use prefix of global free list if there is one.	*/
    /* We don't refill it, but we need to use it up before allocating	*/
    /* a new block ourselves.						*/
      opp = &(GC_obj_kinds[k].ok_freelist[lw]);
      if ( (op = *opp) != 0 ) {
	*opp = 0;
        my_words_allocd = 0;
        for (p = op; p != 0; p = obj_link(p)) {
          my_words_allocd += lw;
          if (my_words_allocd >= BODY_SZ) {
            *opp = obj_link(p);
            obj_link(p) = 0;
            break;
	  }
        }
	GC_words_allocd += my_words_allocd;
	goto out;
      }
    /* Next try to allocate a new block worth of objects of this size.	*/
    {
	struct hblk *h = GC_allochblk(lw, k, 0);
	if (h != 0) {
	  if (IS_UNCOLLECTABLE(k)) GC_set_hdr_marks(HDR(h));
	  GC_words_allocd += BYTES_TO_WORDS(HBLKSIZE)
			       - BYTES_TO_WORDS(HBLKSIZE) % lw;
#	  ifdef PARALLEL_MARK
	    GC_acquire_mark_lock();
	    ++ GC_fl_builder_count;
	    UNLOCK();
	    ENABLE_SIGNALS();
	    GC_release_mark_lock();
#	  endif

	  op = GC_build_fl(h, lw, ok -> ok_init, 0);
#	  ifdef PARALLEL_MARK
	    *result = op;
	    GC_acquire_mark_lock();
	    -- GC_fl_builder_count;
	    if (GC_fl_builder_count == 0) GC_notify_all_builder();
	    GC_release_mark_lock();
	    (void) GC_clear_stack(0);
	    return;
#	  else
	    goto out;
#	  endif
	}
    }
    
    /* As a last attempt, try allocating a single object.  Note that	*/
    /* this may trigger a collection or expand the heap.		*/
      op = GC_generic_malloc_inner(lb, k);
      if (0 != op) obj_link(op) = 0;
    
  out:
    *result = op;
    UNLOCK();
    ENABLE_SIGNALS();
    (void) GC_clear_stack(0);
}

GC_PTR GC_malloc_many(size_t lb)
{
    ptr_t result;
    GC_generic_malloc_many(lb, NORMAL, &result);
    return result;
}

/* Note that the "atomic" version of this would be unsafe, since the	*/
/* links would not be seen by the collector.				*/
# endif

/* Allocate lb bytes of pointerful, traced, but not collectable data */
# ifdef __STDC__
    GC_PTR GC_malloc_uncollectable(size_t lb)
# else
    GC_PTR GC_malloc_uncollectable(lb)
    size_t lb;
# endif
{
register ptr_t op;
register ptr_t *opp;
register word lw;
DCL_LOCK_STATE;

    if( SMALL_OBJ(lb) ) {
#       ifdef MERGE_SIZES
	  if (EXTRA_BYTES != 0 && lb != 0) lb--;
	    	  /* We don't need the extra byte, since this won't be	*/
	    	  /* collected anyway.					*/
	  lw = GC_size_map[lb];
#	else
	  lw = ALIGNED_WORDS(lb);
#       endif
	opp = &(GC_uobjfreelist[lw]);
	FASTLOCK();
        if( FASTLOCK_SUCCEEDED() && (op = *opp) != 0 ) {
            /* See above comment on signals.	*/
            *opp = obj_link(op);
            obj_link(op) = 0;
            GC_words_allocd += lw;
            /* Mark bit ws already set on free list.  It will be	*/
	    /* cleared only temporarily during a collection, as a 	*/
	    /* result of the normal free list mark bit clearing.	*/
            GC_non_gc_bytes += WORDS_TO_BYTES(lw);
            FASTUNLOCK();
            return((GC_PTR) op);
        }
        FASTUNLOCK();
        op = (ptr_t)GC_generic_malloc((word)lb, UNCOLLECTABLE);
    } else {
	op = (ptr_t)GC_generic_malloc((word)lb, UNCOLLECTABLE);
    }
    if (0 == op) return(0);
    /* We don't need the lock here, since we have an undisguised 	*/
    /* pointer.  We do need to hold the lock while we adjust		*/
    /* mark bits.							*/
    {
	register struct hblk * h;
	
	h = HBLKPTR(op);
	lw = HDR(h) -> hb_sz;
	
	DISABLE_SIGNALS();
	LOCK();
	GC_set_mark_bit(op);
	GC_non_gc_bytes += WORDS_TO_BYTES(lw);
	UNLOCK();
	ENABLE_SIGNALS();
	return((GC_PTR) op);
    }
}

#ifdef __STDC__
/* Not well tested nor integrated.	*/
/* Debug version is tricky and currently missing.	*/
#include <limits.h>

GC_PTR GC_memalign(size_t align, size_t lb) 
{ 
    size_t new_lb;
    size_t offset;
    ptr_t result;

#   ifdef ALIGN_DOUBLE
	if (align <= WORDS_TO_BYTES(2) && lb > align) return GC_malloc(lb);
#   endif
    if (align <= WORDS_TO_BYTES(1)) return GC_malloc(lb);
    if (align >= HBLKSIZE/2 || lb >= HBLKSIZE/2) {
        if (align > HBLKSIZE) return GC_oom_fn(LONG_MAX-1024) /* Fail */;
	return GC_malloc(lb <= HBLKSIZE? HBLKSIZE : lb);
	    /* Will be HBLKSIZE aligned.	*/
    }
    /* We could also try to make sure that the real rounded-up object size */
    /* is a multiple of align.  That would be correct up to HBLKSIZE.	   */
    new_lb = lb + align - 1;
    result = GC_malloc(new_lb);
    offset = (word)result % align;
    if (offset != 0) {
	offset = align - offset;
        if (!GC_all_interior_pointers) {
	    if (offset >= VALID_OFFSET_SZ) return GC_malloc(HBLKSIZE);
	    GC_register_displacement(offset);
	}
    }
    result = (GC_PTR) ((ptr_t)result + offset);
    GC_ASSERT((word)result % align == 0);
    return result;
}
#endif 

# ifdef ATOMIC_UNCOLLECTABLE
/* Allocate lb bytes of pointerfree, untraced, uncollectable data 	*/
/* This is normally roughly equivalent to the system malloc.		*/
/* But it may be useful if malloc is redefined.				*/
# ifdef __STDC__
    GC_PTR GC_malloc_atomic_uncollectable(size_t lb)
# else
    GC_PTR GC_malloc_atomic_uncollectable(lb)
    size_t lb;
# endif
{
register ptr_t op;
register ptr_t *opp;
register word lw;
DCL_LOCK_STATE;

    if( SMALL_OBJ(lb) ) {
#       ifdef MERGE_SIZES
	  if (EXTRA_BYTES != 0 && lb != 0) lb--;
	    	  /* We don't need the extra byte, since this won't be	*/
	    	  /* collected anyway.					*/
	  lw = GC_size_map[lb];
#	else
	  lw = ALIGNED_WORDS(lb);
#       endif
	opp = &(GC_auobjfreelist[lw]);
	FASTLOCK();
        if( FASTLOCK_SUCCEEDED() && (op = *opp) != 0 ) {
            /* See above comment on signals.	*/
            *opp = obj_link(op);
            obj_link(op) = 0;
            GC_words_allocd += lw;
	    /* Mark bit was already set while object was on free list. */
            GC_non_gc_bytes += WORDS_TO_BYTES(lw);
            FASTUNLOCK();
            return((GC_PTR) op);
        }
        FASTUNLOCK();
        op = (ptr_t)GC_generic_malloc((word)lb, AUNCOLLECTABLE);
    } else {
	op = (ptr_t)GC_generic_malloc((word)lb, AUNCOLLECTABLE);
    }
    if (0 == op) return(0);
    /* We don't need the lock here, since we have an undisguised 	*/
    /* pointer.  We do need to hold the lock while we adjust		*/
    /* mark bits.							*/
    {
	register struct hblk * h;
	
	h = HBLKPTR(op);
	lw = HDR(h) -> hb_sz;
	
	DISABLE_SIGNALS();
	LOCK();
	GC_set_mark_bit(op);
	GC_non_gc_bytes += WORDS_TO_BYTES(lw);
	UNLOCK();
	ENABLE_SIGNALS();
	return((GC_PTR) op);
    }
}

#endif /* ATOMIC_UNCOLLECTABLE */
