/* Standard problems for dataflow support routines.
   Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007
   Free Software Foundation, Inc.
   Originally contributed by Michael P. Hayes 
             (m.hayes@elec.canterbury.ac.nz, mhayes@redhat.com)
   Major rewrite contributed by Danny Berlin (dberlin@dberlin.org)
             and Kenneth Zadeck (zadeck@naturalbridge.com).

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "tm_p.h"
#include "insn-config.h"
#include "recog.h"
#include "function.h"
#include "regs.h"
#include "output.h"
#include "alloc-pool.h"
#include "flags.h"
#include "hard-reg-set.h"
#include "basic-block.h"
#include "sbitmap.h"
#include "bitmap.h"
#include "timevar.h"
#include "df.h"
#include "except.h"
#include "dce.h"
#include "vecprim.h"

#if 0
#define REG_DEAD_DEBUGGING
#endif

#define DF_SPARSE_THRESHOLD 32

static bitmap seen_in_block = NULL;
static bitmap seen_in_insn = NULL;


/*----------------------------------------------------------------------------
   Public functions access functions for the dataflow problems.
----------------------------------------------------------------------------*/
/* Get the live at out set for BB no matter what problem happens to be
   defined.  This function is used by the register allocators who
   choose different dataflow problems depending on the optimization
   level.  */

bitmap
df_get_live_out (basic_block bb)
{
  gcc_assert (df_lr);

  if (df_urec)
    return DF_RA_LIVE_OUT (bb);
  else if (df_live)
    return DF_LIVE_OUT (bb);
  else 
    return DF_LR_OUT (bb);
}

/* Get the live at in set for BB no matter what problem happens to be
   defined.  This function is used by the register allocators who
   choose different dataflow problems depending on the optimization
   level.  */

bitmap
df_get_live_in (basic_block bb)
{
  gcc_assert (df_lr);

  if (df_urec)
    return DF_RA_LIVE_IN (bb);
  else if (df_live)
    return DF_LIVE_IN (bb);
  else 
    return DF_LR_IN (bb);
}

/* Get the live at top set for BB no matter what problem happens to be
   defined.  This function is used by the register allocators who
   choose different dataflow problems depending on the optimization
   level.  */

bitmap
df_get_live_top (basic_block bb)
{
  gcc_assert (df_lr);

  if (df_urec)
    return DF_RA_LIVE_TOP (bb);
  else 
    return DF_LR_TOP (bb);
}


/*----------------------------------------------------------------------------
   Utility functions.
----------------------------------------------------------------------------*/

/* Generic versions to get the void* version of the block info.  Only
   used inside the problem instance vectors.  */

/* Grow the bb_info array.  */

void
df_grow_bb_info (struct dataflow *dflow)
{
  unsigned int new_size = last_basic_block + 1;
  if (dflow->block_info_size < new_size)
    {
      new_size += new_size / 4;
      dflow->block_info = xrealloc (dflow->block_info, 
				    new_size *sizeof (void*));
      memset (dflow->block_info + dflow->block_info_size, 0,
	      (new_size - dflow->block_info_size) *sizeof (void *));
      dflow->block_info_size = new_size;
    }
}

/* Dump a def-use or use-def chain for REF to FILE.  */

void
df_chain_dump (struct df_link *link, FILE *file)
{
  fprintf (file, "{ ");
  for (; link; link = link->next)
    {
      fprintf (file, "%c%d(bb %d insn %d) ",
	       DF_REF_REG_DEF_P (link->ref) ? 'd' : 'u',
	       DF_REF_ID (link->ref),
	       DF_REF_BBNO (link->ref),
	       DF_REF_INSN (link->ref) ? DF_REF_INSN_UID (link->ref) : -1);
    }
  fprintf (file, "}");
}


/* Print some basic block info as part of df_dump.  */

void 
df_print_bb_index (basic_block bb, FILE *file)
{
  edge e;
  edge_iterator ei;

  fprintf (file, "\n( ");
    FOR_EACH_EDGE (e, ei, bb->preds)
    {
      basic_block pred = e->src;
      fprintf (file, "%d ", pred->index);
    } 
  fprintf (file, ")->[%d]->( ", bb->index);
  FOR_EACH_EDGE (e, ei, bb->succs)
    {
      basic_block succ = e->dest;
      fprintf (file, "%d ", succ->index);
    } 
  fprintf (file, ")\n");
}



/* Make sure that the seen_in_insn and seen_in_block sbitmaps are set
   up correctly. */

static void
df_set_seen (void)
{
  seen_in_block = BITMAP_ALLOC (&df_bitmap_obstack);
  seen_in_insn = BITMAP_ALLOC (&df_bitmap_obstack);
}


static void
df_unset_seen (void)
{
  BITMAP_FREE (seen_in_block);
  BITMAP_FREE (seen_in_insn);
}



/*----------------------------------------------------------------------------
   REACHING USES

   Find the locations in the function where each use site for a pseudo
   can reach backwards.  In and out bitvectors are built for each basic
   block.  The id field in the ref is used to index into these sets.
   See df.h for details.

----------------------------------------------------------------------------*/

/* This problem plays a large number of games for the sake of
   efficiency.  
   
   1) The order of the bits in the bitvectors.  After the scanning
   phase, all of the uses are sorted.  All of the uses for the reg 0
   are first, followed by all uses for reg 1 and so on.
   
   2) There are two kill sets, one if the number of uses is less or
   equal to DF_SPARSE_THRESHOLD and another if it is greater.

   <= : Data is built directly in the kill set.

   > : One level of indirection is used to keep from generating long
   strings of 1 bits in the kill sets.  Bitvectors that are indexed
   by the regnum are used to represent that there is a killing def
   for the register.  The confluence and transfer functions use
   these along with the bitmap_clear_range call to remove ranges of
   bits without actually generating a knockout vector.

   The kill and sparse_kill and the dense_invalidated_by_call and
   sparse_invalidated_by_call both play this game.  */

/* Private data used to compute the solution for this problem.  These
   data structures are not accessible outside of this module.  */
struct df_ru_problem_data
{
  /* The set of defs to regs invalidated by call.  */
  bitmap sparse_invalidated_by_call;  
  /* The set of defs to regs invalidated by call for ru.  */  
  bitmap dense_invalidated_by_call;
  /* An obstack for the bitmaps we need for this problem.  */
  bitmap_obstack ru_bitmaps;
};

/* Set basic block info.  */

static void
df_ru_set_bb_info (unsigned int index, struct df_ru_bb_info *bb_info)
{
  gcc_assert (df_ru);
  gcc_assert (index < df_ru->block_info_size);
  df_ru->block_info[index] = bb_info;
}


/* Free basic block info.  */

static void
df_ru_free_bb_info (basic_block bb ATTRIBUTE_UNUSED, 
		    void *vbb_info)
{
  struct df_ru_bb_info *bb_info = (struct df_ru_bb_info *) vbb_info;
  if (bb_info)
    {
      BITMAP_FREE (bb_info->kill);
      BITMAP_FREE (bb_info->sparse_kill);
      BITMAP_FREE (bb_info->gen);
      BITMAP_FREE (bb_info->in);
      BITMAP_FREE (bb_info->out);
      pool_free (df_ru->block_pool, bb_info);
    }
}


/* Allocate or reset bitmaps for DF_RU blocks. The solution bits are
   not touched unless the block is new.  */

static void 
df_ru_alloc (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
  struct df_ru_problem_data *problem_data;

  if (!df_ru->block_pool)
    df_ru->block_pool = create_alloc_pool ("df_ru_block pool", 
					   sizeof (struct df_ru_bb_info), 50);

  if (df_ru->problem_data)
    {
      problem_data = (struct df_ru_problem_data *) df_ru->problem_data;
      bitmap_clear (problem_data->sparse_invalidated_by_call);
      bitmap_clear (problem_data->dense_invalidated_by_call);
    }
  else 
    {
      problem_data = XNEW (struct df_ru_problem_data);
      df_ru->problem_data = problem_data;

      bitmap_obstack_initialize (&problem_data->ru_bitmaps);
      problem_data->sparse_invalidated_by_call
	= BITMAP_ALLOC (&problem_data->ru_bitmaps);
      problem_data->dense_invalidated_by_call
	= BITMAP_ALLOC (&problem_data->ru_bitmaps);
    }

  df_grow_bb_info (df_ru);

  /* Because of the clustering of all def sites for the same pseudo,
     we have to process all of the blocks before doing the
     analysis.  */

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_ru_bb_info *bb_info = df_ru_get_bb_info (bb_index);
      if (bb_info)
	{ 
	  bitmap_clear (bb_info->kill);
	  bitmap_clear (bb_info->sparse_kill);
	  bitmap_clear (bb_info->gen);
	}
      else
	{ 
	  bb_info = (struct df_ru_bb_info *) pool_alloc (df_ru->block_pool);
	  df_ru_set_bb_info (bb_index, bb_info);
	  bb_info->kill = BITMAP_ALLOC (&problem_data->ru_bitmaps);
	  bb_info->sparse_kill = BITMAP_ALLOC (&problem_data->ru_bitmaps);
	  bb_info->gen = BITMAP_ALLOC (&problem_data->ru_bitmaps);
	  bb_info->in = BITMAP_ALLOC (&problem_data->ru_bitmaps);
	  bb_info->out = BITMAP_ALLOC (&problem_data->ru_bitmaps);
	}
    }
}


/* Process a list of DEFs for df_ru_bb_local_compute.  */

static void
df_ru_bb_local_compute_process_def (struct df_ru_bb_info *bb_info, 
				    struct df_ref **def_rec,
				    enum df_ref_flags top_flag)
{
  while (*def_rec)
    {
      struct df_ref *def = *def_rec;
      if ((top_flag == (DF_REF_FLAGS (def) & DF_REF_AT_TOP))
	  /* If the def is to only part of the reg, it is as if it did
	     not happen, since some of the bits may get thru.  */
	  && (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL))))
	{
	  unsigned int regno = DF_REF_REGNO (def);
	  unsigned int begin = DF_USES_BEGIN (regno);
	  unsigned int n_uses = DF_USES_COUNT (regno);

	  if (!bitmap_bit_p (seen_in_block, regno))
	    {
	      /* The first def for regno in the insn, causes the kill
		 info to be generated.  Do not modify the gen set
		 because the only values in it are the uses from here
		 to the top of the block and this def does not effect
		 them.  */
	      if (!bitmap_bit_p (seen_in_insn, regno))
		{
		  if (n_uses > DF_SPARSE_THRESHOLD)
		    bitmap_set_bit (bb_info->sparse_kill, regno);
		  else
		    bitmap_set_range (bb_info->kill, begin, n_uses);
		}
	      bitmap_set_bit (seen_in_insn, regno);
	    }
	}
      def_rec++;
    }
}


/* Process a list of USEs for df_ru_bb_local_compute.  */

static void
df_ru_bb_local_compute_process_use (struct df_ru_bb_info *bb_info, 
				    struct df_ref **use_rec,
				    enum df_ref_flags top_flag)
{
  while (*use_rec)
    {
      struct df_ref *use = *use_rec;
      if (top_flag == (DF_REF_FLAGS (use) & DF_REF_AT_TOP))
	{
	  /* Add use to set of gens in this BB unless we have seen a
	     def in a previous instruction.  */
	  unsigned int regno = DF_REF_REGNO (use);
	  if (!bitmap_bit_p (seen_in_block, regno))
	    bitmap_set_bit (bb_info->gen, DF_REF_ID (use));
	}
      use_rec++;
    }
}

/* Compute local reaching use (upward exposed use) info for basic
   block BB.  USE_INFO->REGS[R] caches the set of uses for register R.  */
static void
df_ru_bb_local_compute (unsigned int bb_index)
{
  basic_block bb = BASIC_BLOCK (bb_index);
  struct df_ru_bb_info *bb_info = df_ru_get_bb_info (bb_index);
  rtx insn;

  /* Set when a def for regno is seen.  */
  bitmap_clear (seen_in_block);
  bitmap_clear (seen_in_insn);

#ifdef EH_USES
  /* Variables defined in the prolog that are used by the exception
     handler.  */
  df_ru_bb_local_compute_process_use (bb_info, 
				      df_get_artificial_uses (bb_index),
				      DF_REF_AT_TOP);
#endif
  df_ru_bb_local_compute_process_def (bb_info, 
				      df_get_artificial_defs (bb_index),
				      DF_REF_AT_TOP);

  FOR_BB_INSNS (bb, insn)
    {
      unsigned int uid = INSN_UID (insn);
      if (!INSN_P (insn))
	continue;

      df_ru_bb_local_compute_process_use (bb_info, 
					  DF_INSN_UID_USES (uid), 0);

      if (df->changeable_flags & DF_EQ_NOTES)
	df_ru_bb_local_compute_process_use (bb_info, 
					    DF_INSN_UID_EQ_USES (uid), 0);

      df_ru_bb_local_compute_process_def (bb_info, 
					  DF_INSN_UID_DEFS (uid), 0);

      bitmap_ior_into (seen_in_block, seen_in_insn);
      bitmap_clear (seen_in_insn);
    }

  /* Process the hardware registers that are always live.  */
  df_ru_bb_local_compute_process_use (bb_info, 
				      df_get_artificial_uses (bb_index), 0);

  df_ru_bb_local_compute_process_def (bb_info, 
				      df_get_artificial_defs (bb_index), 0);
}


/* Compute local reaching use (upward exposed use) info for each basic
   block within BLOCKS.  */
static void
df_ru_local_compute (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
  unsigned int regno;
  struct df_ru_problem_data *problem_data
    = (struct df_ru_problem_data *) df_ru->problem_data;
  bitmap sparse_invalidated = problem_data->sparse_invalidated_by_call;
  bitmap dense_invalidated = problem_data->dense_invalidated_by_call;

  df_set_seen ();

  df_maybe_reorganize_use_refs (df->changeable_flags & DF_EQ_NOTES ? 
				DF_REF_ORDER_BY_REG_WITH_NOTES : DF_REF_ORDER_BY_REG);

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      df_ru_bb_local_compute (bb_index);
    }
  
  /* Set up the knockout bit vectors to be applied across EH_EDGES.  */
  EXECUTE_IF_SET_IN_BITMAP (df_invalidated_by_call, 0, regno, bi)
    {
      if (DF_USES_COUNT (regno) > DF_SPARSE_THRESHOLD)
	bitmap_set_bit (sparse_invalidated, regno);
      else
	bitmap_set_range (dense_invalidated,
			  DF_USES_BEGIN (regno), 
			  DF_USES_COUNT (regno));
    }

  df_unset_seen ();
}


/* Initialize the solution bit vectors for problem.  */

static void 
df_ru_init_solution (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_ru_bb_info *bb_info = df_ru_get_bb_info (bb_index);
      bitmap_copy (bb_info->in, bb_info->gen);
      bitmap_clear (bb_info->out);
    }
}


/* Out of target gets or of in of source.  */

static void
df_ru_confluence_n (edge e)
{
  bitmap op1 = df_ru_get_bb_info (e->src->index)->out;
  bitmap op2 = df_ru_get_bb_info (e->dest->index)->in;

  if (e->flags & EDGE_EH)
    {
      struct df_ru_problem_data *problem_data
	= (struct df_ru_problem_data *) df_ru->problem_data;
      bitmap sparse_invalidated = problem_data->sparse_invalidated_by_call;
      bitmap dense_invalidated = problem_data->dense_invalidated_by_call;
      bitmap_iterator bi;
      unsigned int regno;
      bitmap tmp = BITMAP_ALLOC (&df_bitmap_obstack);

      bitmap_copy (tmp, op2);
      bitmap_and_compl_into (tmp, dense_invalidated);

      EXECUTE_IF_SET_IN_BITMAP (sparse_invalidated, 0, regno, bi)
	{
 	  bitmap_clear_range (tmp, 
 			      DF_USES_BEGIN (regno), 
 			      DF_USES_COUNT (regno));
	}
      bitmap_ior_into (op1, tmp);
      BITMAP_FREE (tmp);
    }
  else
    bitmap_ior_into (op1, op2);
}


/* Transfer function.  */

static bool
df_ru_transfer_function (int bb_index)
{
  struct df_ru_bb_info *bb_info = df_ru_get_bb_info (bb_index);
  unsigned int regno;
  bitmap_iterator bi;
  bitmap in = bb_info->in;
  bitmap out = bb_info->out;
  bitmap gen = bb_info->gen;
  bitmap kill = bb_info->kill;
  bitmap sparse_kill = bb_info->sparse_kill;

  if (bitmap_empty_p (sparse_kill))
    return  bitmap_ior_and_compl (in, gen, out, kill);
  else 
    {
      struct df_ru_problem_data *problem_data;
      bitmap tmp;
      bool changed = false;

      /* Note that TMP is _not_ a temporary bitmap if we end up replacing
	 IN with TMP.  Therefore, allocate TMP in the RU bitmaps obstack.  */
      problem_data = (struct df_ru_problem_data *) df_ru->problem_data;
      tmp = BITMAP_ALLOC (&problem_data->ru_bitmaps);

      bitmap_copy (tmp, out);
      EXECUTE_IF_SET_IN_BITMAP (sparse_kill, 0, regno, bi)
	{
	  bitmap_clear_range (tmp, 
 			      DF_USES_BEGIN (regno), 
 			      DF_USES_COUNT (regno));
	}
      bitmap_and_compl_into (tmp, kill);
      bitmap_ior_into (tmp, gen);
      changed = !bitmap_equal_p (tmp, in);
      if (changed)
	{
	  BITMAP_FREE (in);
	  bb_info->in = tmp;
	}
      else 
	BITMAP_FREE (tmp);
      return changed;
    }
}


/* Free all storage associated with the problem.  */

static void
df_ru_free (void)
{
  unsigned int i;
  struct df_ru_problem_data *problem_data
    = (struct df_ru_problem_data *) df_ru->problem_data;

  if (problem_data)
    {
      for (i = 0; i < df_ru->block_info_size; i++)
	{
	  struct df_ru_bb_info *bb_info = df_ru_get_bb_info (i);
	  if (bb_info)
	    {
	      BITMAP_FREE (bb_info->kill);
	      BITMAP_FREE (bb_info->sparse_kill);
	      BITMAP_FREE (bb_info->gen);
	      BITMAP_FREE (bb_info->in);
	      BITMAP_FREE (bb_info->out);
	    }
	}
      
      free_alloc_pool (df_ru->block_pool);
      BITMAP_FREE (problem_data->sparse_invalidated_by_call);
      BITMAP_FREE (problem_data->dense_invalidated_by_call);
      bitmap_obstack_release (&problem_data->ru_bitmaps);
      
      df_ru->block_info_size = 0;
      free (df_ru->block_info);
      free (df_ru->problem_data);
    }
  free (df_ru);
}


/* Debugging info.  */

static void
df_ru_start_dump (FILE *file)
{
  struct df_ru_problem_data *problem_data
    = (struct df_ru_problem_data *) df_ru->problem_data;
  unsigned int m = DF_REG_SIZE(df);
  unsigned int regno;
  
  if (!df_ru->block_info) 
    return;

  fprintf (file, ";; Reaching uses:\n");

  fprintf (file, ";;   sparse invalidated \t");
  dump_bitmap (file, problem_data->sparse_invalidated_by_call);
  fprintf (file, " dense invalidated \t");
  dump_bitmap (file, problem_data->dense_invalidated_by_call);
  
  for (regno = 0; regno < m; regno++)
    if (DF_USES_COUNT (regno))
      fprintf (file, "%d[%d,%d] ", regno, 
	       DF_USES_BEGIN (regno), 
	       DF_USES_COUNT (regno));
  fprintf (file, "\n");
}


/* Debugging info at top of bb.  */

static void
df_ru_top_dump (basic_block bb, FILE *file)
{
  struct df_ru_bb_info *bb_info = df_ru_get_bb_info (bb->index);
  if (!bb_info || !bb_info->in)
    return;
  
  fprintf (file, ";; ru  in  \t(%d)\n", (int) bitmap_count_bits (bb_info->in));
  dump_bitmap (file, bb_info->in);
  fprintf (file, ";; ru  gen \t(%d)\n", (int) bitmap_count_bits (bb_info->gen));
  dump_bitmap (file, bb_info->gen);
  fprintf (file, ";; ru  kill\t(%d)\n", (int) bitmap_count_bits (bb_info->kill));
  dump_bitmap (file, bb_info->kill);
}  


/* Debugging info at bottom of bb.  */

static void
df_ru_bottom_dump (basic_block bb, FILE *file)
{
  struct df_ru_bb_info *bb_info = df_ru_get_bb_info (bb->index);
  if (!bb_info || !bb_info->out)
    return;
  
  fprintf (file, ";; ru  out \t(%d)\n", (int) bitmap_count_bits (bb_info->out));
  dump_bitmap (file, bb_info->out);
}  


/* All of the information associated with every instance of the problem.  */

static struct df_problem problem_RU =
{
  DF_RU,                      /* Problem id.  */
  DF_BACKWARD,                /* Direction.  */
  df_ru_alloc,                /* Allocate the problem specific data.  */
  NULL,                       /* Reset global information.  */
  df_ru_free_bb_info,         /* Free basic block info.  */
  df_ru_local_compute,        /* Local compute function.  */
  df_ru_init_solution,        /* Init the solution specific data.  */
  df_worklist_dataflow,       /* Worklist solver.  */
  NULL,                       /* Confluence operator 0.  */ 
  df_ru_confluence_n,         /* Confluence operator n.  */ 
  df_ru_transfer_function,    /* Transfer function.  */
  NULL,                       /* Finalize function.  */
  df_ru_free,                 /* Free all of the problem information.  */
  df_ru_free,                 /* Remove this problem from the stack of dataflow problems.  */
  df_ru_start_dump,           /* Debugging.  */
  df_ru_top_dump,             /* Debugging start block.  */
  df_ru_bottom_dump,          /* Debugging end block.  */
  NULL,                       /* Incremental solution verify start.  */
  NULL,                       /* Incremental solution verfiy end.  */
  NULL,                       /* Dependent problem.  */
  TV_DF_RU                    /* Timing variable.  */ 
};



/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_ru_add_problem (void)
{
  df_add_problem (&problem_RU);
}


/*----------------------------------------------------------------------------
   REACHING DEFINITIONS

   Find the locations in the function where each definition site for a
   pseudo reaches.  In and out bitvectors are built for each basic
   block.  The id field in the ref is used to index into these sets.
   See df.h for details.
   ----------------------------------------------------------------------------*/

/* See the comment at the top of the Reaching Uses problem for how the
   uses are represented in the kill sets. The same games are played
   here for the defs.  */

/* Private data used to compute the solution for this problem.  These
   data structures are not accessible outside of this module.  */
struct df_rd_problem_data
{
  /* The set of defs to regs invalidated by call.  */
  bitmap sparse_invalidated_by_call;  
  /* The set of defs to regs invalidate by call for rd.  */  
  bitmap dense_invalidated_by_call;
  /* An obstack for the bitmaps we need for this problem.  */
  bitmap_obstack rd_bitmaps;
};

/* Set basic block info.  */

static void
df_rd_set_bb_info (unsigned int index, 
		   struct df_rd_bb_info *bb_info)
{
  gcc_assert (df_rd);
  gcc_assert (index < df_rd->block_info_size);
  df_rd->block_info[index] = bb_info;
}


/* Free basic block info.  */

static void
df_rd_free_bb_info (basic_block bb ATTRIBUTE_UNUSED, 
		    void *vbb_info)
{
  struct df_rd_bb_info *bb_info = (struct df_rd_bb_info *) vbb_info;
  if (bb_info)
    {
      BITMAP_FREE (bb_info->kill);
      BITMAP_FREE (bb_info->sparse_kill);
      BITMAP_FREE (bb_info->gen);
      BITMAP_FREE (bb_info->in);
      BITMAP_FREE (bb_info->out);
      pool_free (df_rd->block_pool, bb_info);
    }
}


/* Allocate or reset bitmaps for DF_RD blocks. The solution bits are
   not touched unless the block is new.  */

static void 
df_rd_alloc (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
  struct df_rd_problem_data *problem_data;

  if (!df_rd->block_pool)
    df_rd->block_pool = create_alloc_pool ("df_rd_block pool", 
					   sizeof (struct df_rd_bb_info), 50);

  if (df_rd->problem_data)
    {
      problem_data = (struct df_rd_problem_data *) df_rd->problem_data;
      bitmap_clear (problem_data->sparse_invalidated_by_call);
      bitmap_clear (problem_data->dense_invalidated_by_call);
    }
  else 
    {
      problem_data = XNEW (struct df_rd_problem_data);
      df_rd->problem_data = problem_data;

      bitmap_obstack_initialize (&problem_data->rd_bitmaps);
      problem_data->sparse_invalidated_by_call
	= BITMAP_ALLOC (&problem_data->rd_bitmaps);
      problem_data->dense_invalidated_by_call
	= BITMAP_ALLOC (&problem_data->rd_bitmaps);
    }

  df_grow_bb_info (df_rd);

  /* Because of the clustering of all use sites for the same pseudo,
     we have to process all of the blocks before doing the
     analysis.  */

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_rd_bb_info *bb_info = df_rd_get_bb_info (bb_index);
      if (bb_info)
	{ 
	  bitmap_clear (bb_info->kill);
	  bitmap_clear (bb_info->sparse_kill);
	  bitmap_clear (bb_info->gen);
	}
      else
	{ 
	  bb_info = (struct df_rd_bb_info *) pool_alloc (df_rd->block_pool);
	  df_rd_set_bb_info (bb_index, bb_info);
	  bb_info->kill = BITMAP_ALLOC (&problem_data->rd_bitmaps);
	  bb_info->sparse_kill = BITMAP_ALLOC (&problem_data->rd_bitmaps);
	  bb_info->gen = BITMAP_ALLOC (&problem_data->rd_bitmaps);
	  bb_info->in = BITMAP_ALLOC (&problem_data->rd_bitmaps);
	  bb_info->out = BITMAP_ALLOC (&problem_data->rd_bitmaps);
	}
    }
}


/* Process a list of DEFs for df_rd_bb_local_compute.  */

static void
df_rd_bb_local_compute_process_def (struct df_rd_bb_info *bb_info, 
				    struct df_ref **def_rec,
				    enum df_ref_flags top_flag)
{
  while (*def_rec)
    {
      struct df_ref *def = *def_rec;
      if (top_flag == (DF_REF_FLAGS (def) & DF_REF_AT_TOP))
	{
	  unsigned int regno = DF_REF_REGNO (def);
	  unsigned int begin = DF_DEFS_BEGIN (regno);
	  unsigned int n_defs = DF_DEFS_COUNT (regno);
	  
	  if ((!(df->changeable_flags & DF_NO_HARD_REGS))
	      || (regno >= FIRST_PSEUDO_REGISTER))
	    {
	      /* Only the last def(s) for a regno in the block has any
		 effect.  */ 
	      if (!bitmap_bit_p (seen_in_block, regno))
		{
		  /* The first def for regno in insn gets to knock out the
		     defs from other instructions.  */
		  if ((!bitmap_bit_p (seen_in_insn, regno))
		      /* If the def is to only part of the reg, it does
			 not kill the other defs that reach here.  */
		      && (!(DF_REF_FLAGS (def) & 
			    (DF_REF_PARTIAL | DF_REF_CONDITIONAL | DF_REF_MAY_CLOBBER))))
		    {
		      if (n_defs > DF_SPARSE_THRESHOLD)
			{
			  bitmap_set_bit (bb_info->sparse_kill, regno);
			  bitmap_clear_range(bb_info->gen, begin, n_defs);
			}
		      else
			{
			  bitmap_set_range (bb_info->kill, begin, n_defs);
			  bitmap_clear_range (bb_info->gen, begin, n_defs);
			}
		    }
		  
		  bitmap_set_bit (seen_in_insn, regno);
		  /* All defs for regno in the instruction may be put into
		     the gen set.  */
		  if (!(DF_REF_FLAGS (def) 
			& (DF_REF_MUST_CLOBBER | DF_REF_MAY_CLOBBER)))
		    bitmap_set_bit (bb_info->gen, DF_REF_ID (def));
		}
	    }
	}
      def_rec++;
    }
}

/* Compute local reaching def info for basic block BB.  */

static void
df_rd_bb_local_compute (unsigned int bb_index)
{
  basic_block bb = BASIC_BLOCK (bb_index);
  struct df_rd_bb_info *bb_info = df_rd_get_bb_info (bb_index);
  rtx insn;

  bitmap_clear (seen_in_block);
  bitmap_clear (seen_in_insn);

  /* Artificials are only hard regs.  */
  if (!(df->changeable_flags & DF_NO_HARD_REGS))
    df_rd_bb_local_compute_process_def (bb_info, 
					df_get_artificial_defs (bb_index),
					0);

  FOR_BB_INSNS_REVERSE (bb, insn)
    {
      unsigned int uid = INSN_UID (insn);

      if (!INSN_P (insn))
	continue;

      df_rd_bb_local_compute_process_def (bb_info, 
					  DF_INSN_UID_DEFS (uid), 0);

      /* This complex dance with the two bitmaps is required because
	 instructions can assign twice to the same pseudo.  This
	 generally happens with calls that will have one def for the
	 result and another def for the clobber.  If only one vector
	 is used and the clobber goes first, the result will be
	 lost.  */
      bitmap_ior_into (seen_in_block, seen_in_insn);
      bitmap_clear (seen_in_insn);
    }

  /* Process the artificial defs at the top of the block last since we
     are going backwards through the block and these are logically at
     the start.  */
  if (!(df->changeable_flags & DF_NO_HARD_REGS))
    df_rd_bb_local_compute_process_def (bb_info, 
					df_get_artificial_defs (bb_index),
					DF_REF_AT_TOP);
}


/* Compute local reaching def info for each basic block within BLOCKS.  */

static void
df_rd_local_compute (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
  unsigned int regno;
  struct df_rd_problem_data *problem_data
    = (struct df_rd_problem_data *) df_rd->problem_data;
  bitmap sparse_invalidated = problem_data->sparse_invalidated_by_call;
  bitmap dense_invalidated = problem_data->dense_invalidated_by_call;

  df_set_seen ();

  df_maybe_reorganize_def_refs (DF_REF_ORDER_BY_REG);

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      df_rd_bb_local_compute (bb_index);
    }
  
  /* Set up the knockout bit vectors to be applied across EH_EDGES.  */
  EXECUTE_IF_SET_IN_BITMAP (df_invalidated_by_call, 0, regno, bi)
    {
      if (DF_DEFS_COUNT (regno) > DF_SPARSE_THRESHOLD)
	bitmap_set_bit (sparse_invalidated, regno);
      else
	bitmap_set_range (dense_invalidated, 
			  DF_DEFS_BEGIN (regno), 
			  DF_DEFS_COUNT (regno));
    }
  df_unset_seen ();
}


/* Initialize the solution bit vectors for problem.  */

static void 
df_rd_init_solution (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_rd_bb_info *bb_info = df_rd_get_bb_info (bb_index);
      
      bitmap_copy (bb_info->out, bb_info->gen);
      bitmap_clear (bb_info->in);
    }
}

/* In of target gets or of out of source.  */

static void
df_rd_confluence_n (edge e)
{
  bitmap op1 = df_rd_get_bb_info (e->dest->index)->in;
  bitmap op2 = df_rd_get_bb_info (e->src->index)->out;

  if (e->flags & EDGE_EH)
    {
      struct df_rd_problem_data *problem_data
	= (struct df_rd_problem_data *) df_rd->problem_data;
      bitmap sparse_invalidated = problem_data->sparse_invalidated_by_call;
      bitmap dense_invalidated = problem_data->dense_invalidated_by_call;
      bitmap_iterator bi;
      unsigned int regno;
      bitmap tmp = BITMAP_ALLOC (&df_bitmap_obstack);

      bitmap_copy (tmp, op2);
      bitmap_and_compl_into (tmp, dense_invalidated);

      EXECUTE_IF_SET_IN_BITMAP (sparse_invalidated, 0, regno, bi)
 	{
 	  bitmap_clear_range (tmp, 
 			      DF_DEFS_BEGIN (regno), 
 			      DF_DEFS_COUNT (regno));
	}
      bitmap_ior_into (op1, tmp);
      BITMAP_FREE (tmp);
    }
  else
    bitmap_ior_into (op1, op2);
}


/* Transfer function.  */

static bool
df_rd_transfer_function (int bb_index)
{
  struct df_rd_bb_info *bb_info = df_rd_get_bb_info (bb_index);
  unsigned int regno;
  bitmap_iterator bi;
  bitmap in = bb_info->in;
  bitmap out = bb_info->out;
  bitmap gen = bb_info->gen;
  bitmap kill = bb_info->kill;
  bitmap sparse_kill = bb_info->sparse_kill;

  if (bitmap_empty_p (sparse_kill))
    return  bitmap_ior_and_compl (out, gen, in, kill);
  else 
    {
      struct df_rd_problem_data *problem_data;
      bool changed = false;
      bitmap tmp;

      /* Note that TMP is _not_ a temporary bitmap if we end up replacing
	 OUT with TMP.  Therefore, allocate TMP in the RD bitmaps obstack.  */
      problem_data = (struct df_rd_problem_data *) df_rd->problem_data;
      tmp = BITMAP_ALLOC (&problem_data->rd_bitmaps);

      bitmap_copy (tmp, in);
      EXECUTE_IF_SET_IN_BITMAP (sparse_kill, 0, regno, bi)
	{
	  bitmap_clear_range (tmp, 
			      DF_DEFS_BEGIN (regno), 
			      DF_DEFS_COUNT (regno));
	}
      bitmap_and_compl_into (tmp, kill);
      bitmap_ior_into (tmp, gen);
      changed = !bitmap_equal_p (tmp, out);
      if (changed)
	{
	  BITMAP_FREE (out);
	  bb_info->out = tmp;
	}
      else 
	  BITMAP_FREE (tmp);
      return changed;
    }
}


/* Free all storage associated with the problem.  */

static void
df_rd_free (void)
{
  unsigned int i;
  struct df_rd_problem_data *problem_data
    = (struct df_rd_problem_data *) df_rd->problem_data;

  if (problem_data)
    {
      for (i = 0; i < df_rd->block_info_size; i++)
	{
	  struct df_rd_bb_info *bb_info = df_rd_get_bb_info (i);
	  if (bb_info)
	    {
	      BITMAP_FREE (bb_info->kill);
	      BITMAP_FREE (bb_info->sparse_kill);
	      BITMAP_FREE (bb_info->gen);
	      BITMAP_FREE (bb_info->in);
	      BITMAP_FREE (bb_info->out);
	    }
	}
      
      free_alloc_pool (df_rd->block_pool);
      BITMAP_FREE (problem_data->sparse_invalidated_by_call);
      BITMAP_FREE (problem_data->dense_invalidated_by_call);
      bitmap_obstack_release (&problem_data->rd_bitmaps);
      
      df_rd->block_info_size = 0;
      free (df_rd->block_info);
      free (df_rd->problem_data);
    }
  free (df_rd);
}


/* Debugging info.  */

static void
df_rd_start_dump (FILE *file)
{
  struct df_rd_problem_data *problem_data
    = (struct df_rd_problem_data *) df_rd->problem_data;
  unsigned int m = DF_REG_SIZE(df);
  unsigned int regno;
  
  if (!df_rd->block_info) 
    return;

  fprintf (file, ";; Reaching defs:\n\n");

  fprintf (file, "  sparse invalidated \t");
  dump_bitmap (file, problem_data->sparse_invalidated_by_call);
  fprintf (file, "  dense invalidated \t");
  dump_bitmap (file, problem_data->dense_invalidated_by_call);

  for (regno = 0; regno < m; regno++)
    if (DF_DEFS_COUNT (regno))
      fprintf (file, "%d[%d,%d] ", regno, 
	       DF_DEFS_BEGIN (regno), 
	       DF_DEFS_COUNT (regno));
  fprintf (file, "\n");

}


/* Debugging info at top of bb.  */

static void
df_rd_top_dump (basic_block bb, FILE *file)
{
  struct df_rd_bb_info *bb_info = df_rd_get_bb_info (bb->index);
  if (!bb_info || !bb_info->in)
    return;
  
  fprintf (file, ";; rd  in  \t(%d)\n", (int) bitmap_count_bits (bb_info->in));
  dump_bitmap (file, bb_info->in);
  fprintf (file, ";; rd  gen \t(%d)\n", (int) bitmap_count_bits (bb_info->gen));
  dump_bitmap (file, bb_info->gen);
  fprintf (file, ";; rd  kill\t(%d)\n", (int) bitmap_count_bits (bb_info->kill));
  dump_bitmap (file, bb_info->kill);
}


/* Debugging info at top of bb.  */

static void
df_rd_bottom_dump (basic_block bb, FILE *file)
{
  struct df_rd_bb_info *bb_info = df_rd_get_bb_info (bb->index);
  if (!bb_info || !bb_info->out)
    return;
  
  fprintf (file, ";; rd  out \t(%d)\n", (int) bitmap_count_bits (bb_info->out));
  dump_bitmap (file, bb_info->out);
}

/* All of the information associated with every instance of the problem.  */

static struct df_problem problem_RD =
{
  DF_RD,                      /* Problem id.  */
  DF_FORWARD,                 /* Direction.  */
  df_rd_alloc,                /* Allocate the problem specific data.  */
  NULL,                       /* Reset global information.  */
  df_rd_free_bb_info,         /* Free basic block info.  */
  df_rd_local_compute,        /* Local compute function.  */
  df_rd_init_solution,        /* Init the solution specific data.  */
  df_worklist_dataflow,       /* Worklist solver.  */
  NULL,                       /* Confluence operator 0.  */ 
  df_rd_confluence_n,         /* Confluence operator n.  */ 
  df_rd_transfer_function,    /* Transfer function.  */
  NULL,                       /* Finalize function.  */
  df_rd_free,                 /* Free all of the problem information.  */
  df_rd_free,                 /* Remove this problem from the stack of dataflow problems.  */
  df_rd_start_dump,           /* Debugging.  */
  df_rd_top_dump,             /* Debugging start block.  */
  df_rd_bottom_dump,          /* Debugging end block.  */
  NULL,                       /* Incremental solution verify start.  */
  NULL,                       /* Incremental solution verfiy end.  */
  NULL,                       /* Dependent problem.  */
  TV_DF_RD                    /* Timing variable.  */ 
};



/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_rd_add_problem (void)
{
  df_add_problem (&problem_RD);
}



/*----------------------------------------------------------------------------
   LIVE REGISTERS

   Find the locations in the function where any use of a pseudo can
   reach in the backwards direction.  In and out bitvectors are built
   for each basic block.  The regnum is used to index into these sets.
   See df.h for details.
   ----------------------------------------------------------------------------*/

/* Private data used to verify the solution for this problem.  */
struct df_lr_problem_data
{
  bitmap *in;
  bitmap *out;
};


/* Set basic block info.  */

static void
df_lr_set_bb_info (unsigned int index, 
		   struct df_lr_bb_info *bb_info)
{
  gcc_assert (df_lr);
  gcc_assert (index < df_lr->block_info_size);
  df_lr->block_info[index] = bb_info;
}

 
/* Free basic block info.  */

static void
df_lr_free_bb_info (basic_block bb ATTRIBUTE_UNUSED, 
		    void *vbb_info)
{
  struct df_lr_bb_info *bb_info = (struct df_lr_bb_info *) vbb_info;
  if (bb_info)
    {
      BITMAP_FREE (bb_info->use);
      BITMAP_FREE (bb_info->def);
      if (bb_info->in == bb_info->top)
        bb_info->top = NULL;
      else
	{
          BITMAP_FREE (bb_info->top);
          BITMAP_FREE (bb_info->ause);
          BITMAP_FREE (bb_info->adef);
	}
      BITMAP_FREE (bb_info->in);
      BITMAP_FREE (bb_info->out);
      pool_free (df_lr->block_pool, bb_info);
    }
}


/* Allocate or reset bitmaps for DF_LR blocks. The solution bits are
   not touched unless the block is new.  */

static void 
df_lr_alloc (bitmap all_blocks ATTRIBUTE_UNUSED)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  if (!df_lr->block_pool)
    df_lr->block_pool = create_alloc_pool ("df_lr_block pool", 
					   sizeof (struct df_lr_bb_info), 50);

  df_grow_bb_info (df_lr);

  EXECUTE_IF_SET_IN_BITMAP (df_lr->out_of_date_transfer_functions, 0, bb_index, bi)
    {
      struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb_index);
      if (bb_info)
	{ 
	  bitmap_clear (bb_info->def);
	  bitmap_clear (bb_info->use);
	  if (bb_info->adef)
	    {
	      bitmap_clear (bb_info->adef);
	      bitmap_clear (bb_info->ause);
	    }
	}
      else
	{ 
	  bb_info = (struct df_lr_bb_info *) pool_alloc (df_lr->block_pool);
	  df_lr_set_bb_info (bb_index, bb_info);
	  bb_info->use = BITMAP_ALLOC (NULL);
	  bb_info->def = BITMAP_ALLOC (NULL);
	  bb_info->in = BITMAP_ALLOC (NULL);
	  bb_info->out = BITMAP_ALLOC (NULL);
          bb_info->top = bb_info->in;
          bb_info->adef = NULL;
          bb_info->ause = NULL;
	}
    }
}


/* Reset the global solution for recalculation.  */

static void 
df_lr_reset (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb_index);
      gcc_assert (bb_info);
      bitmap_clear (bb_info->in);
      bitmap_clear (bb_info->out);
      bitmap_clear (bb_info->top);
    }
}


/* Compute local live register info for basic block BB.  */

static void
df_lr_bb_local_compute (unsigned int bb_index)
{
  basic_block bb = BASIC_BLOCK (bb_index);
  struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb_index);
  rtx insn;
  struct df_ref **def_rec;
  struct df_ref **use_rec;

  /* Process the registers set in an exception handler.  */
  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if (((DF_REF_FLAGS (def) & DF_REF_AT_TOP) == 0)
	  && (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL))))
	{
	  unsigned int dregno = DF_REF_REGNO (def);
	  bitmap_set_bit (bb_info->def, dregno);
	  bitmap_clear_bit (bb_info->use, dregno);
	}
    }

  /* Process the hardware registers that are always live.  */
  for (use_rec = df_get_artificial_uses (bb_index); *use_rec; use_rec++)
    {
      struct df_ref *use = *use_rec;
      /* Add use to set of uses in this BB.  */
      if ((DF_REF_FLAGS (use) & DF_REF_AT_TOP) == 0)
	bitmap_set_bit (bb_info->use, DF_REF_REGNO (use));
    }

  FOR_BB_INSNS_REVERSE (bb, insn)
    {
      unsigned int uid = INSN_UID (insn);

      if (!INSN_P (insn))
	continue;	

      if (CALL_P (insn))
	{
	  for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	    {
	      struct df_ref *def = *def_rec;
	      unsigned int dregno = DF_REF_REGNO (def);
	      
	      if (DF_REF_FLAGS (def) & DF_REF_MUST_CLOBBER)
		{
		  if (dregno >= FIRST_PSEUDO_REGISTER
		      || !(SIBLING_CALL_P (insn)
			   && bitmap_bit_p (df->exit_block_uses, dregno)
			   && !refers_to_regno_p (dregno, dregno+1,
						  current_function_return_rtx,
						  (rtx *)0)))
		    {
		      /* If the def is to only part of the reg, it does
			 not kill the other defs that reach here.  */
		      if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
			{
			  bitmap_set_bit (bb_info->def, dregno);
			  bitmap_clear_bit (bb_info->use, dregno);
			}
		    }
		}
	      else
		/* This is the return value.  */
		if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
		  {
		    bitmap_set_bit (bb_info->def, dregno);
		    bitmap_clear_bit (bb_info->use, dregno);
		  }
	    }
	}
      else
	{
	  for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	    {
	      struct df_ref *def = *def_rec;
	      /* If the def is to only part of the reg, it does
		     not kill the other defs that reach here.  */
	      if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
		{
		  unsigned int dregno = DF_REF_REGNO (def);
		  bitmap_set_bit (bb_info->def, dregno);
		  bitmap_clear_bit (bb_info->use, dregno);
		}
	    }
	}

      for (use_rec = DF_INSN_UID_USES (uid); *use_rec; use_rec++)
	{
	  struct df_ref *use = *use_rec;
	  /* Add use to set of uses in this BB.  */
	  bitmap_set_bit (bb_info->use, DF_REF_REGNO (use));
	}
    }
  /* Process the registers set in an exception handler.  */
  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if ((DF_REF_FLAGS (def) & DF_REF_AT_TOP)
	  && (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL))))
	{
	  unsigned int dregno = DF_REF_REGNO (def);
	  if (bb_info->adef == NULL)
	    {
	      gcc_assert (bb_info->ause == NULL);
	      gcc_assert (bb_info->top == bb_info->in);
	      bb_info->adef = BITMAP_ALLOC (NULL);
	      bb_info->ause = BITMAP_ALLOC (NULL);
	      bb_info->top = BITMAP_ALLOC (NULL);
	    }
	  bitmap_set_bit (bb_info->adef, dregno);
	}
    }
  
#ifdef EH_USES
  /* Process the uses that are live into an exception handler.  */
  for (use_rec = df_get_artificial_uses (bb_index); *use_rec; use_rec++)
    {
      struct df_ref *use = *use_rec;
      /* Add use to set of uses in this BB.  */
      if (DF_REF_FLAGS (use) & DF_REF_AT_TOP)
	{
	  if (bb_info->adef == NULL)
	    {
	      gcc_assert (bb_info->ause == NULL);
	      gcc_assert (bb_info->top == bb_info->in);
	      bb_info->adef = BITMAP_ALLOC (NULL);
	      bb_info->ause = BITMAP_ALLOC (NULL);
	      bb_info->top = BITMAP_ALLOC (NULL);
	    }
	  bitmap_set_bit (bb_info->ause, DF_REF_REGNO (use));
	}
    }
#endif
}


/* Compute local live register info for each basic block within BLOCKS.  */

static void
df_lr_local_compute (bitmap all_blocks ATTRIBUTE_UNUSED)
{
  unsigned int bb_index;
  bitmap_iterator bi;
    
  bitmap_clear (df->hardware_regs_used);
  
  /* The all-important stack pointer must always be live.  */
  bitmap_set_bit (df->hardware_regs_used, STACK_POINTER_REGNUM);
  
  /* Before reload, there are a few registers that must be forced
     live everywhere -- which might not already be the case for
     blocks within infinite loops.  */
  if (!reload_completed)
    {
      /* Any reference to any pseudo before reload is a potential
	 reference of the frame pointer.  */
      bitmap_set_bit (df->hardware_regs_used, FRAME_POINTER_REGNUM);
      
#if FRAME_POINTER_REGNUM != ARG_POINTER_REGNUM
      /* Pseudos with argument area equivalences may require
	 reloading via the argument pointer.  */
      if (fixed_regs[ARG_POINTER_REGNUM])
	bitmap_set_bit (df->hardware_regs_used, ARG_POINTER_REGNUM);
#endif
      
      /* Any constant, or pseudo with constant equivalences, may
	 require reloading from memory using the pic register.  */
      if ((unsigned) PIC_OFFSET_TABLE_REGNUM != INVALID_REGNUM
	  && fixed_regs[PIC_OFFSET_TABLE_REGNUM])
	bitmap_set_bit (df->hardware_regs_used, PIC_OFFSET_TABLE_REGNUM);
    }
  
  EXECUTE_IF_SET_IN_BITMAP (df_lr->out_of_date_transfer_functions, 0, bb_index, bi)
    {
      if (bb_index == EXIT_BLOCK)
	{
	  /* The exit block is special for this problem and its bits are
	     computed from thin air.  */
	  struct df_lr_bb_info *bb_info = df_lr_get_bb_info (EXIT_BLOCK);
	  bitmap_copy (bb_info->use, df->exit_block_uses);
	}
      else
	df_lr_bb_local_compute (bb_index);
    }

  bitmap_clear (df_lr->out_of_date_transfer_functions);
}


/* Initialize the solution vectors.  */

static void 
df_lr_init (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb_index);
      bitmap_copy (bb_info->in, bb_info->use);
      bitmap_clear (bb_info->out);
    }
}


/* Confluence function that processes infinite loops.  This might be a
   noreturn function that throws.  And even if it isn't, getting the
   unwind info right helps debugging.  */
static void
df_lr_confluence_0 (basic_block bb)
{
  bitmap op1 = df_lr_get_bb_info (bb->index)->out;
  if (bb != EXIT_BLOCK_PTR)
    bitmap_copy (op1, df->hardware_regs_used);
} 


/* Confluence function that ignores fake edges.  */

static void
df_lr_confluence_n (edge e)
{
  bitmap op1 = df_lr_get_bb_info (e->src->index)->out;
  bitmap op2 = df_lr_get_bb_info (e->dest->index)->in;
 
  /* Call-clobbered registers die across exception and call edges.  */
  /* ??? Abnormal call edges ignored for the moment, as this gets
     confused by sibling call edges, which crashes reg-stack.  */
  if (e->flags & EDGE_EH)
    bitmap_ior_and_compl_into (op1, op2, df_invalidated_by_call);
  else
    bitmap_ior_into (op1, op2);

  bitmap_ior_into (op1, df->hardware_regs_used);
} 


/* Transfer function.  */

static bool
df_lr_transfer_function (int bb_index)
{
  struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb_index);
  bitmap in = bb_info->in;
  bitmap out = bb_info->out;
  bitmap use = bb_info->use;
  bitmap def = bb_info->def;
  bitmap top = bb_info->top;
  bitmap ause = bb_info->ause;
  bitmap adef = bb_info->adef;
  bool changed;

  changed = bitmap_ior_and_compl (top, use, out, def);
  if (in != top)
    {
      gcc_assert (ause && adef);
      changed |= bitmap_ior_and_compl (in, ause, top, adef);
    }

  return changed;
}


/* Run the fast dce as a side effect of building LR.  */

static void
df_lr_local_finalize (bitmap all_blocks ATTRIBUTE_UNUSED)
{
  if (df->changeable_flags & DF_LR_RUN_DCE)
    {
      run_fast_df_dce ();
      if (df_lr->problem_data && df_lr->solutions_dirty)
	{
	  /* If we are here, then it is because we are both verifying
	  the solution and the dce changed the function.  In that case
	  the verification info built will be wrong.  So we leave the
	  dirty flag true so that the verifier will skip the checking
	  part and just clean up.*/
	  df_lr->solutions_dirty = true;
	}
      else
	df_lr->solutions_dirty = false;
    }
  else
    df_lr->solutions_dirty = false;
}


/* Free all storage associated with the problem.  */

static void
df_lr_free (void)
{
  if (df_lr->block_info)
    {
      unsigned int i;
      for (i = 0; i < df_lr->block_info_size; i++)
	{
	  struct df_lr_bb_info *bb_info = df_lr_get_bb_info (i);
	  if (bb_info)
	    {
	      BITMAP_FREE (bb_info->use);
	      BITMAP_FREE (bb_info->def);
	      if (bb_info->in == bb_info->top)
	        bb_info->top = NULL;
	      else
		{
	          BITMAP_FREE (bb_info->top);
                  BITMAP_FREE (bb_info->ause);
                  BITMAP_FREE (bb_info->adef);
		}
	      BITMAP_FREE (bb_info->in);
	      BITMAP_FREE (bb_info->out);
	    }
	}
      free_alloc_pool (df_lr->block_pool);
      
      df_lr->block_info_size = 0;
      free (df_lr->block_info);
    }

  BITMAP_FREE (df_lr->out_of_date_transfer_functions);
  free (df_lr);
}


/* Public auxillary functions.  */

/* Get the variables live at the end of BB and apply the artifical
   uses and defs at the end of BB.  */

void 
df_lr_simulate_artificial_refs_at_end (basic_block bb, bitmap live)
{
  struct df_ref **def_rec;
  struct df_ref **use_rec;
  int bb_index = bb->index;
  
  bitmap_copy (live, DF_LR_OUT (bb));
  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if (((DF_REF_FLAGS (def) & DF_REF_AT_TOP) == 0)
	  && (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL))))
	bitmap_clear_bit (live, DF_REF_REGNO (def));
    }

  for (use_rec = df_get_artificial_uses (bb_index); *use_rec; use_rec++)
    {
      struct df_ref *use = *use_rec;
      if ((DF_REF_FLAGS (use) & DF_REF_AT_TOP) == 0)
	bitmap_set_bit (live, DF_REF_REGNO (use));
    }
}


/* Simulate the effects of INSN on the bitmap LIVE.  */
void 
df_lr_simulate_one_insn (basic_block bb, rtx insn, bitmap live)
{
  struct df_ref **def_rec;
  struct df_ref **use_rec;
  unsigned int uid = INSN_UID (insn);

  if (! INSN_P (insn))
    return;	
  
  if (CALL_P (insn))
    {
      for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	{
	  struct df_ref *def = *def_rec;
	  unsigned int dregno = DF_REF_REGNO (def);
	  
	  if (DF_REF_FLAGS (def) & DF_REF_MUST_CLOBBER)
	    {
	      if (dregno >= FIRST_PSEUDO_REGISTER
		  || !(SIBLING_CALL_P (insn)
		       && bitmap_bit_p (df->exit_block_uses, dregno)
		       && !refers_to_regno_p (dregno, dregno+1,
					      current_function_return_rtx,
					      (rtx *)0)))
		{
		  /* If the def is to only part of the reg, it does
		     not kill the other defs that reach here.  */
		  if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
		    bitmap_clear_bit (live, dregno);
		}
	    }
	  else
	    /* This is the return value.  */
	    if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
	      bitmap_clear_bit (live, dregno);
	}
    }
  else
    {
      for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	{
	  struct df_ref *def = *def_rec;
	  unsigned int dregno = DF_REF_REGNO (def);
  
	  /* If the def is to only part of the reg, it does
	     not kill the other defs that reach here.  */
	  if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
	    bitmap_clear_bit (live, dregno);
	}
    }
  
  for (use_rec = DF_INSN_UID_USES (uid); *use_rec; use_rec++)
    {
      struct df_ref *use = *use_rec;
      /* Add use to set of uses in this BB.  */
      bitmap_set_bit (live, DF_REF_REGNO (use));
    }

  /* These regs are considered always live so if they end up dying
     because of some def, we need to bring the back again.  */
  if (df_has_eh_preds (bb))
    bitmap_ior_into (live, df->eh_block_artificial_uses);
  else
    bitmap_ior_into (live, df->regular_block_artificial_uses);
}


/* Debugging info at top of bb.  */

static void
df_lr_top_dump (basic_block bb, FILE *file)
{
  struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb->index);
  struct df_lr_problem_data *problem_data;
  if (!bb_info || !bb_info->in)
    return;
      
  fprintf (file, ";; lr  in  \t");
  df_print_regset (file, bb_info->in);
  if (df_lr->problem_data)
    {
      problem_data = (struct df_lr_problem_data *)df_lr->problem_data;
      fprintf (file, ";;  old in  \t");
      df_print_regset (file, problem_data->in[bb->index]);
    }
  fprintf (file, ";; lr  use \t");
  df_print_regset (file, bb_info->use);
  fprintf (file, ";; lr  def \t");
  df_print_regset (file, bb_info->def);
}  


/* Debugging info at bottom of bb.  */

static void
df_lr_bottom_dump (basic_block bb, FILE *file)
{
  struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb->index);
  struct df_lr_problem_data *problem_data;
  if (!bb_info || !bb_info->out)
    return;
  
  fprintf (file, ";; lr  out \t");
  df_print_regset (file, bb_info->out);
  if (df_lr->problem_data)
    {
      problem_data = (struct df_lr_problem_data *)df_lr->problem_data;
      fprintf (file, ";;  old out  \t");
      df_print_regset (file, problem_data->out[bb->index]);
    }
}  


/* Build the datastructure to verify that the solution to the dataflow
   equations is not dirty.  */

static void
df_lr_verify_solution_start (void)
{
  basic_block bb;
  struct df_lr_problem_data *problem_data;
  if (df_lr->solutions_dirty)
    {
      df_lr->problem_data = NULL;
      return;
    }

  /* Set it true so that the solution is recomputed.  */ 
  df_lr->solutions_dirty = true;

  problem_data = XNEW (struct df_lr_problem_data);
  df_lr->problem_data = problem_data;
  problem_data->in = XNEWVEC (bitmap, last_basic_block);
  problem_data->out = XNEWVEC (bitmap, last_basic_block);

  FOR_ALL_BB (bb)
    {
      problem_data->in[bb->index] = BITMAP_ALLOC (NULL);
      problem_data->out[bb->index] = BITMAP_ALLOC (NULL);
      bitmap_copy (problem_data->in[bb->index], DF_LR_IN (bb));
      bitmap_copy (problem_data->out[bb->index], DF_LR_OUT (bb));
    }
}


/* Compare the saved datastructure and the new solution to the dataflow
   equations.  */

static void
df_lr_verify_solution_end (void)
{
  struct df_lr_problem_data *problem_data;
  basic_block bb;

  if (df_lr->problem_data == NULL)
    return;

  problem_data = (struct df_lr_problem_data *)df_lr->problem_data;

  if (df_lr->solutions_dirty)
    /* Do not check if the solution is still dirty.  See the comment
       in df_lr_local_finalize for details.  */
    df_lr->solutions_dirty = false;
  else
    FOR_ALL_BB (bb)
      {
	if ((!bitmap_equal_p (problem_data->in[bb->index], DF_LR_IN (bb)))
	    || (!bitmap_equal_p (problem_data->out[bb->index], DF_LR_OUT (bb))))
	  {
	    /*df_dump (stderr);*/
	    gcc_unreachable ();
	  }
      }

  /* Cannot delete them immediately because you may want to dump them
     if the comparison fails.  */
  FOR_ALL_BB (bb)
    {
      BITMAP_FREE (problem_data->in[bb->index]);
      BITMAP_FREE (problem_data->out[bb->index]);
    }

  free (problem_data->in);
  free (problem_data->out);
  free (problem_data);
  df_lr->problem_data = NULL;
}


/* All of the information associated with every instance of the problem.  */

static struct df_problem problem_LR =
{
  DF_LR,                      /* Problem id.  */
  DF_BACKWARD,                /* Direction.  */
  df_lr_alloc,                /* Allocate the problem specific data.  */
  df_lr_reset,                /* Reset global information.  */
  df_lr_free_bb_info,         /* Free basic block info.  */
  df_lr_local_compute,        /* Local compute function.  */
  df_lr_init,                 /* Init the solution specific data.  */
  df_worklist_dataflow,       /* Worklist solver.  */
  df_lr_confluence_0,         /* Confluence operator 0.  */ 
  df_lr_confluence_n,         /* Confluence operator n.  */ 
  df_lr_transfer_function,    /* Transfer function.  */
  df_lr_local_finalize,       /* Finalize function.  */
  df_lr_free,                 /* Free all of the problem information.  */
  NULL,                       /* Remove this problem from the stack of dataflow problems.  */
  NULL,                       /* Debugging.  */
  df_lr_top_dump,             /* Debugging start block.  */
  df_lr_bottom_dump,          /* Debugging end block.  */
  df_lr_verify_solution_start,/* Incremental solution verify start.  */
  df_lr_verify_solution_end,  /* Incremental solution verify end.  */
  NULL,                       /* Dependent problem.  */
  TV_DF_LR                    /* Timing variable.  */ 
};


/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_lr_add_problem (void)
{
  df_add_problem (&problem_LR);
  /* These will be initialized when df_scan_blocks processes each
     block.  */
  df_lr->out_of_date_transfer_functions = BITMAP_ALLOC (NULL);
}


/* Verify that all of the lr related info is consistent and
   correct.  */

void
df_lr_verify_transfer_functions (void)
{
  basic_block bb;
  bitmap saved_def;
  bitmap saved_use;
  bitmap saved_adef;
  bitmap saved_ause;
  bitmap all_blocks;
  bool need_as;

  if (!df)
    return;

  saved_def = BITMAP_ALLOC (NULL);
  saved_use = BITMAP_ALLOC (NULL);
  saved_adef = BITMAP_ALLOC (NULL);
  saved_ause = BITMAP_ALLOC (NULL);
  all_blocks = BITMAP_ALLOC (NULL);

  FOR_ALL_BB (bb)
    {
      struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb->index);
      bitmap_set_bit (all_blocks, bb->index);

      if (bb_info)
	{
	  /* Make a copy of the transfer functions and then compute
	     new ones to see if the transfer functions have
	     changed.  */
	  if (!bitmap_bit_p (df_lr->out_of_date_transfer_functions, 
			     bb->index))
	    {
	      bitmap_copy (saved_def, bb_info->def);
	      bitmap_copy (saved_use, bb_info->use);
	      bitmap_clear (bb_info->def);
	      bitmap_clear (bb_info->use);

	      if (bb_info->adef)
		{
		  need_as = true;
		  bitmap_copy (saved_adef, bb_info->adef);
		  bitmap_copy (saved_ause, bb_info->ause);
		  bitmap_clear (bb_info->adef);
		  bitmap_clear (bb_info->ause);
		}
	      else
		need_as = false;

	      df_lr_bb_local_compute (bb->index);
	      gcc_assert (bitmap_equal_p (saved_def, bb_info->def));
	      gcc_assert (bitmap_equal_p (saved_use, bb_info->use));

	      if (need_as)
		{
		  gcc_assert (bb_info->adef);
		  gcc_assert (bb_info->ause);
		  gcc_assert (bitmap_equal_p (saved_adef, bb_info->adef));
		  gcc_assert (bitmap_equal_p (saved_ause, bb_info->ause));
		}
	      else
		{
		  gcc_assert (!bb_info->adef);
		  gcc_assert (!bb_info->ause);
		}
	    }
	}
      else
	{
	  /* If we do not have basic block info, the block must be in
	     the list of dirty blocks or else some one has added a
	     block behind our backs. */
	  gcc_assert (bitmap_bit_p (df_lr->out_of_date_transfer_functions, 
				    bb->index));
	}
      /* Make sure no one created a block without following
	 procedures.  */
      gcc_assert (df_scan_get_bb_info (bb->index));
    }

  /* Make sure there are no dirty bits in blocks that have been deleted.  */
  gcc_assert (!bitmap_intersect_compl_p (df_lr->out_of_date_transfer_functions, 
					 all_blocks)); 

  BITMAP_FREE (saved_def);
  BITMAP_FREE (saved_use);
  BITMAP_FREE (saved_adef);
  BITMAP_FREE (saved_ause);
  BITMAP_FREE (all_blocks);
}



/*----------------------------------------------------------------------------
   UNINITIALIZED REGISTERS

   Find the set of uses for registers that are reachable from the entry
   block without passing thru a definition.  In and out bitvectors are built
   for each basic block.  The regnum is used to index into these sets.
   See df.h for details.
----------------------------------------------------------------------------*/

/* Private data used to verify the solution for this problem.  */
struct df_ur_problem_data
{
  bitmap *in;
  bitmap *out;
};


/* Set basic block info.  */

static void
df_ur_set_bb_info (unsigned int index, 
		   struct df_ur_bb_info *bb_info)
{
  gcc_assert (df_ur);
  gcc_assert (index < df_ur->block_info_size);
  df_ur->block_info[index] = bb_info;
}


/* Free basic block info.  */

static void
df_ur_free_bb_info (basic_block bb ATTRIBUTE_UNUSED, 
		    void *vbb_info)
{
  struct df_ur_bb_info *bb_info = (struct df_ur_bb_info *) vbb_info;
  if (bb_info)
    {
      BITMAP_FREE (bb_info->gen);
      BITMAP_FREE (bb_info->kill);
      BITMAP_FREE (bb_info->in);
      BITMAP_FREE (bb_info->out);
      pool_free (df_ur->block_pool, bb_info);
    }
}


/* Allocate or reset bitmaps for DF_UR blocks. The solution bits are
   not touched unless the block is new.  */

static void 
df_ur_alloc (bitmap all_blocks ATTRIBUTE_UNUSED)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  if (!df_ur->block_pool)
    df_ur->block_pool = create_alloc_pool ("df_ur_block pool", 
					   sizeof (struct df_ur_bb_info), 100);

  df_grow_bb_info (df_ur);

  EXECUTE_IF_SET_IN_BITMAP (df_ur->out_of_date_transfer_functions, 0, bb_index, bi)
    {
      struct df_ur_bb_info *bb_info = df_ur_get_bb_info (bb_index);
      if (bb_info)
	{ 
	  bitmap_clear (bb_info->kill);
	  bitmap_clear (bb_info->gen);
	}
      else
	{ 
	  bb_info = (struct df_ur_bb_info *) pool_alloc (df_ur->block_pool);
	  df_ur_set_bb_info (bb_index, bb_info);
	  bb_info->kill = BITMAP_ALLOC (NULL);
	  bb_info->gen = BITMAP_ALLOC (NULL);
	  bb_info->in = BITMAP_ALLOC (NULL);
	  bb_info->out = BITMAP_ALLOC (NULL);
	}
    }
}


/* Reset the global solution for recalculation.  */

static void 
df_ur_reset (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_lr_bb_info *bb_info = df_lr_get_bb_info (bb_index);
      gcc_assert (bb_info);
      bitmap_clear (bb_info->in);
      bitmap_clear (bb_info->out);
    }
}


/* Compute local uninitialized register info for basic block BB.  */

static void
df_ur_bb_local_compute (unsigned int bb_index)
{
  basic_block bb = BASIC_BLOCK (bb_index);
  struct df_ur_bb_info *bb_info = df_ur_get_bb_info (bb_index);
  rtx insn;
  struct df_ref **def_rec;
  int luid = 0;

  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if (DF_REF_FLAGS (def) & DF_REF_AT_TOP)
	bitmap_set_bit (bb_info->gen, DF_REF_REGNO (def));
    }

  FOR_BB_INSNS (bb, insn)
    {
      unsigned int uid = INSN_UID (insn);
      struct df_insn_info *insn_info = DF_INSN_UID_GET (uid);

      /* Inserting labels does not always trigger the incremental
	 rescanning.  */
      if (!insn_info)
	{
	  gcc_assert (!INSN_P (insn));
	  df_insn_create_insn_record (insn);
	}

      DF_INSN_LUID (insn) = luid;
      if (!INSN_P (insn))
	continue;

      luid++;
      for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	{
	  struct df_ref *def = *def_rec;
	  unsigned int regno = DF_REF_REGNO (def);

	  if (DF_REF_FLAGS_IS_SET (def,
				   DF_REF_PARTIAL | DF_REF_CONDITIONAL))
	    /* All partial or conditional def
	       seen are included in the gen set. */
	    bitmap_set_bit (bb_info->gen, regno);
	  else if (DF_REF_FLAGS_IS_SET (def, DF_REF_MUST_CLOBBER))
	    /* Only must clobbers for the entire reg destroy the
	       value.  */
	    bitmap_set_bit (bb_info->kill, regno);
	  else if (! DF_REF_FLAGS_IS_SET (def, DF_REF_MAY_CLOBBER))
	    bitmap_set_bit (bb_info->gen, regno);
	}
    }

  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if ((DF_REF_FLAGS (def) & DF_REF_AT_TOP) == 0)
	bitmap_set_bit (bb_info->gen, DF_REF_REGNO (def));
    }
}


/* Compute local uninitialized register info.  */

static void
df_ur_local_compute (bitmap all_blocks ATTRIBUTE_UNUSED)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  df_grow_insn_info ();

  EXECUTE_IF_SET_IN_BITMAP (df_ur->out_of_date_transfer_functions, 
			    0, bb_index, bi)
    {
      df_ur_bb_local_compute (bb_index);
    }

  bitmap_clear (df_ur->out_of_date_transfer_functions);
}


/* Initialize the solution vectors.  */

static void 
df_ur_init (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_ur_bb_info *bb_info = df_ur_get_bb_info (bb_index);

      bitmap_copy (bb_info->out, bb_info->gen);
      bitmap_clear (bb_info->in);
    }
}

/* Confluence function that ignores fake edges.  */

static void
df_ur_confluence_n (edge e)
{
  bitmap op1 = df_ur_get_bb_info (e->dest->index)->in;
  bitmap op2 = df_ur_get_bb_info (e->src->index)->out;
 
  if (e->flags & EDGE_FAKE) 
    return;

  bitmap_ior_into (op1, op2);
} 


/* Transfer function.  */

static bool
df_ur_transfer_function (int bb_index)
{
  struct df_ur_bb_info *bb_info = df_ur_get_bb_info (bb_index);
  bitmap in = bb_info->in;
  bitmap out = bb_info->out;
  bitmap gen = bb_info->gen;
  bitmap kill = bb_info->kill;

  return bitmap_ior_and_compl (out, gen, in, kill);
}


/* Run the fast dce as a side effect of building LR.  */

static void
df_ur_local_finalize (bitmap all_blocks ATTRIBUTE_UNUSED)
{
  df_ur->solutions_dirty = false;
}


/* Free all storage associated with the problem.  */

static void
df_ur_free (void)
{
  if (df_ur->block_info)
    {
      unsigned int i;
      
      for (i = 0; i < df_ur->block_info_size; i++)
	{
	  struct df_ur_bb_info *bb_info = df_ur_get_bb_info (i);
	  if (bb_info)
	    {
	      BITMAP_FREE (bb_info->gen);
	      BITMAP_FREE (bb_info->kill);
	      BITMAP_FREE (bb_info->in);
	      BITMAP_FREE (bb_info->out);
	    }
	}
      
      free_alloc_pool (df_ur->block_pool);
      df_ur->block_info_size = 0;
      free (df_ur->block_info);
    }
  BITMAP_FREE (df_ur->out_of_date_transfer_functions);
  free (df_ur);
}


/* Debugging info at top of bb.  */

static void
df_ur_top_dump (basic_block bb, FILE *file)
{
  struct df_ur_bb_info *bb_info = df_ur_get_bb_info (bb->index);
  struct df_ur_problem_data *problem_data;

  if (!bb_info || !bb_info->in)
    return;
      
  fprintf (file, ";; ur  in  \t");
  df_print_regset (file, bb_info->in);
  if (df_ur->problem_data)
    {
      problem_data = (struct df_ur_problem_data *)df_ur->problem_data;
      fprintf (file, ";;  old in  \t");
      df_print_regset (file, problem_data->in[bb->index]);
    }
  fprintf (file, ";; ur  gen \t");
  df_print_regset (file, bb_info->gen);
  fprintf (file, ";; ur  kill\t");
  df_print_regset (file, bb_info->kill);
}


/* Debugging info at bottom of bb.  */

static void
df_ur_bottom_dump (basic_block bb, FILE *file)
{
  struct df_ur_bb_info *bb_info = df_ur_get_bb_info (bb->index);
  struct df_ur_problem_data *problem_data;

  if (!bb_info || !bb_info->out)
    return;
      
  fprintf (file, ";; ur  out \t");
  df_print_regset (file, bb_info->out);
  if (df_ur->problem_data)
    {
      problem_data = (struct df_ur_problem_data *)df_ur->problem_data;
      fprintf (file, ";;  old out  \t");
      df_print_regset (file, problem_data->out[bb->index]);
    }
}


/* Build the datastructure to verify that the solution to the dataflow
   equations is not dirty.  */

static void
df_ur_verify_solution_start (void)
{
  basic_block bb;
  struct df_ur_problem_data *problem_data;
  if (df_ur->solutions_dirty)
    {
      df_ur->problem_data = NULL;
      return;
    }

  /* Set it true so that the solution is recomputed.  */ 
  df_ur->solutions_dirty = true;

  problem_data = XNEW (struct df_ur_problem_data);
  df_ur->problem_data = problem_data;
  problem_data->in = XNEWVEC (bitmap, last_basic_block);
  problem_data->out = XNEWVEC (bitmap, last_basic_block);

  FOR_ALL_BB (bb)
    {
      problem_data->in[bb->index] = BITMAP_ALLOC (NULL);
      problem_data->out[bb->index] = BITMAP_ALLOC (NULL);
      bitmap_copy (problem_data->in[bb->index], DF_UR_IN (bb));
      bitmap_copy (problem_data->out[bb->index], DF_UR_OUT (bb));
    }
}


/* Compare the saved datastructure and the new solution to the dataflow
   equations.  */

static void
df_ur_verify_solution_end (void)
{
  struct df_ur_problem_data *problem_data;
  basic_block bb;

  if (df_ur->problem_data == NULL)
    return;

  problem_data = (struct df_ur_problem_data *)df_ur->problem_data;

  FOR_ALL_BB (bb)
    {
      if ((!bitmap_equal_p (problem_data->in[bb->index], DF_UR_IN (bb)))
	  || (!bitmap_equal_p (problem_data->out[bb->index], DF_UR_OUT (bb))))
	{
	  /*df_dump (stderr);*/
	  gcc_unreachable ();
	}
    }

  /* Cannot delete them immediately because you may want to dump them
     if the comparison fails.  */
  FOR_ALL_BB (bb)
    {
      BITMAP_FREE (problem_data->in[bb->index]);
      BITMAP_FREE (problem_data->out[bb->index]);
    }

  free (problem_data->in);
  free (problem_data->out);
  free (problem_data);
  df_ur->problem_data = NULL;
}


/* All of the information associated with every instance of the problem.  */

static struct df_problem problem_UR =
{
  DF_UR,                      /* Problem id.  */
  DF_FORWARD,                 /* Direction.  */
  df_ur_alloc,                /* Allocate the problem specific data.  */
  df_ur_reset,                /* Reset global information.  */
  df_ur_free_bb_info,         /* Free basic block info.  */
  df_ur_local_compute,        /* Local compute function.  */
  df_ur_init,                 /* Init the solution specific data.  */
  df_worklist_dataflow,       /* Worklist solver.  */
  NULL,                       /* Confluence operator 0.  */ 
  df_ur_confluence_n,         /* Confluence operator n.  */ 
  df_ur_transfer_function,    /* Transfer function.  */
  df_ur_local_finalize,       /* Finalize function.  */
  df_ur_free,                 /* Free all of the problem information.  */
  df_ur_free,                 /* Remove this problem from the stack of dataflow problems.  */
  NULL,                       /* Debugging.  */
  df_ur_top_dump,             /* Debugging start block.  */
  df_ur_bottom_dump,          /* Debugging end block.  */
  df_ur_verify_solution_start,/* Incremental solution verify start.  */
  df_ur_verify_solution_end,  /* Incremental solution verify end.  */
  &problem_LR,                /* Dependent problem.  */
  TV_DF_UR                    /* Timing variable.  */ 
};


/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_ur_add_problem (void)
{
  df_add_problem (&problem_UR);
  /* These will be initialized when df_scan_blocks processes each
     block.  */
  df_ur->out_of_date_transfer_functions = BITMAP_ALLOC (NULL);
}


/* Verify that all of the lr related info is consistent and
   correct.  */

void
df_ur_verify_transfer_functions (void)
{
  basic_block bb;
  bitmap saved_gen;
  bitmap saved_kill;
  bitmap all_blocks;

  if (!df)
    return;

  saved_gen = BITMAP_ALLOC (NULL);
  saved_kill = BITMAP_ALLOC (NULL);
  all_blocks = BITMAP_ALLOC (NULL);

  df_grow_insn_info ();

  FOR_ALL_BB (bb)
    {
      struct df_ur_bb_info *bb_info = df_ur_get_bb_info (bb->index);
      bitmap_set_bit (all_blocks, bb->index);

      if (bb_info)
	{
	  /* Make a copy of the transfer functions and then compute
	     new ones to see if the transfer functions have
	     changed.  */
	  if (!bitmap_bit_p (df_ur->out_of_date_transfer_functions, 
			     bb->index))
	    {
	      bitmap_copy (saved_gen, bb_info->gen);
	      bitmap_copy (saved_kill, bb_info->kill);
	      bitmap_clear (bb_info->gen);
	      bitmap_clear (bb_info->kill);

	      df_ur_bb_local_compute (bb->index);
	      gcc_assert (bitmap_equal_p (saved_gen, bb_info->gen));
	      gcc_assert (bitmap_equal_p (saved_kill, bb_info->kill));
	    }
	}
      else
	{
	  /* If we do not have basic block info, the block must be in
	     the list of dirty blocks or else some one has added a
	     block behind our backs. */
	  gcc_assert (bitmap_bit_p (df_ur->out_of_date_transfer_functions, 
				    bb->index));
	}
      /* Make sure no one created a block without following
	 procedures.  */
      gcc_assert (df_scan_get_bb_info (bb->index));
    }

  /* Make sure there are no dirty bits in blocks that have been deleted.  */
  gcc_assert (!bitmap_intersect_compl_p (df_ur->out_of_date_transfer_functions, 
					 all_blocks)); 
  BITMAP_FREE (saved_gen);
  BITMAP_FREE (saved_kill);
  BITMAP_FREE (all_blocks);
}



/*----------------------------------------------------------------------------
   COMBINED LIVE REGISTERS AND UNINITIALIZED REGISTERS.

   The in and out sets here are the anded results of the in and out
   sets from the lr and ur problems. 
 ----------------------------------------------------------------------------*/

/* Set basic block info.  */

static void
df_live_set_bb_info (unsigned int index, 
		     struct df_live_bb_info *bb_info)
{
  gcc_assert (df_live);
  gcc_assert (index < df_live->block_info_size);
  df_live->block_info[index] = bb_info;
}


/* Free basic block info.  */

static void
df_live_free_bb_info (basic_block bb ATTRIBUTE_UNUSED, 
		      void *vbb_info)
{
  struct df_live_bb_info *bb_info = (struct df_live_bb_info *) vbb_info;
  if (bb_info)
    {
      BITMAP_FREE (bb_info->in);
      BITMAP_FREE (bb_info->out);
      pool_free (df_live->block_pool, bb_info);
    }
}


/* Allocate or reset bitmaps for DF_LIVE blocks. The solution bits are
   not touched unless the block is new.  */

static void 
df_live_alloc (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  if (!df_live->block_pool)
    df_live->block_pool = create_alloc_pool ("df_live_block pool", 
					     sizeof (struct df_live_bb_info), 100);

  df_grow_bb_info (df_live);

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_live_bb_info *bb_info = df_live_get_bb_info (bb_index);
      if (!bb_info)
	{ 
	  bb_info = (struct df_live_bb_info *) pool_alloc (df_live->block_pool);
	  df_live_set_bb_info (bb_index, bb_info);
	  bb_info->in = BITMAP_ALLOC (NULL);
	  bb_info->out = BITMAP_ALLOC (NULL);
	}
    }
}


/* And the LR and UR info to produce the LIVE info.  */

static void
df_live_local_finalize (bitmap all_blocks)
{

  if (df_live->solutions_dirty)
    {
      bitmap_iterator bi;
      unsigned int bb_index;

      EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
	{
	  struct df_ur_bb_info *bb_ur_info = df_ur_get_bb_info (bb_index);
	  struct df_lr_bb_info *bb_lr_info = df_lr_get_bb_info (bb_index);
	  struct df_live_bb_info *bb_live_info = df_live_get_bb_info (bb_index);
	  
	  /* No register may reach a location where it is not used.  Thus
	     we trim the rr result to the places where it is used.  */
	  bitmap_and (bb_live_info->in, bb_ur_info->in, bb_lr_info->in);
	  bitmap_and (bb_live_info->out, bb_ur_info->out, bb_lr_info->out);
	}
      
      df_live->solutions_dirty = false;
    }
}


/* Free all storage associated with the problem.  */

static void
df_live_free (void)
{
  if (df_live->block_info)
    {
      unsigned int i;
      
      for (i = 0; i < df_live->block_info_size; i++)
	{
	  struct df_live_bb_info *bb_info = df_live_get_bb_info (i);
	  if (bb_info)
	    {
	      BITMAP_FREE (bb_info->in);
	      BITMAP_FREE (bb_info->out);
	    }
	}
      
      free_alloc_pool (df_live->block_pool);
      df_live->block_info_size = 0;
      free (df_live->block_info);
    }
  free (df_live);
}


/* Debugging info at top of bb.  */

static void
df_live_top_dump (basic_block bb, FILE *file)
{
  struct df_live_bb_info *bb_info = df_live_get_bb_info (bb->index);
  if (!bb_info || !bb_info->in)
    return;
  
  fprintf (file, ";; live  in  \t");
  df_print_regset (file, bb_info->in);
}


/* Debugging info at bottom of bb.  */

static void
df_live_bottom_dump (basic_block bb, FILE *file)
{
  struct df_live_bb_info *bb_info = df_live_get_bb_info (bb->index);
  if (!bb_info || !bb_info->out)
    return;
  
  fprintf (file, ";; live  out  \t");
  df_print_regset (file, bb_info->out);
}


/* All of the information associated with every instance of the problem.  */

static struct df_problem problem_LIVE =
{
  DF_LIVE,                    /* Problem id.  */
  DF_NONE,                    /* Direction.  */
  df_live_alloc,              /* Allocate the problem specific data.  */
  NULL,                       /* Reset global information.  */
  df_live_free_bb_info,       /* Free basic block info.  */
  NULL,                       /* Local compute function.  */
  NULL,                       /* Init the solution specific data.  */
  NULL,                       /* Iterative solver.  */
  NULL,                       /* Confluence operator 0.  */ 
  NULL,                       /* Confluence operator n.  */ 
  NULL,                       /* Transfer function.  */
  df_live_local_finalize,     /* Finalize function.  */
  df_live_free,               /* Free all of the problem information.  */
  df_live_free,                /* Remove this problem from the stack of dataflow problems.  */
  NULL,                       /* Debugging.  */
  df_live_top_dump,           /* Debugging start block.  */
  df_live_bottom_dump,        /* Debugging end block.  */
  NULL,                       /* Incremental solution verify start.  */
  NULL,                       /* Incremental solution verfiy end.  */
  &problem_UR,                /* Dependent problem.  */
  TV_DF_LIVE                  /* Timing variable.  */ 
};


/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_live_add_problem (void)
{
  df_add_problem (&problem_LIVE);
}


/*----------------------------------------------------------------------------
   UNINITIALIZED REGISTERS WITH EARLYCLOBBER

   Find the set of uses for registers that are reachable from the entry
   block without passing thru a definition.  In and out bitvectors are built
   for each basic block.  The regnum is used to index into these sets.
   See df.h for details.

   This is a variant of the UR problem above that has a lot of special
   features just for the register allocation phase.  This problem
   should go a away if someone would fix the interference graph.

   ----------------------------------------------------------------------------*/

/* Private data used to compute the solution for this problem.  These
   data structures are not accessible outside of this module.  */
struct df_urec_problem_data
{
  bool earlyclobbers_found;     /* True if any instruction contains an
				   earlyclobber.  */
#ifdef STACK_REGS
  bitmap stack_regs;		/* Registers that may be allocated to a STACK_REGS.  */
#endif
};


/* Set basic block info.  */

static void
df_urec_set_bb_info (unsigned int index, 
		     struct df_urec_bb_info *bb_info)
{
  gcc_assert (df_urec);
  gcc_assert (index < df_urec->block_info_size);
  df_urec->block_info[index] = bb_info;
}


/* Free basic block info.  */

static void
df_urec_free_bb_info (basic_block bb ATTRIBUTE_UNUSED, 
		      void *vbb_info)
{
  struct df_urec_bb_info *bb_info = (struct df_urec_bb_info *) vbb_info;
  if (bb_info)
    {
      BITMAP_FREE (bb_info->gen);
      BITMAP_FREE (bb_info->kill);
      BITMAP_FREE (bb_info->in);
      BITMAP_FREE (bb_info->out);
      BITMAP_FREE (bb_info->earlyclobber);
      pool_free (df_urec->block_pool, bb_info);
    }
}


/* Allocate or reset bitmaps for DF_UREC blocks. The solution bits are
   not touched unless the block is new.  */

static void 
df_urec_alloc (bitmap all_blocks)

{
  unsigned int bb_index;
  bitmap_iterator bi;
  struct df_urec_problem_data *problem_data
    = (struct df_urec_problem_data *) df_urec->problem_data;

  if (!df_urec->block_pool)
    df_urec->block_pool = create_alloc_pool ("df_urec_block pool", 
					   sizeof (struct df_urec_bb_info), 50);

  if (!df_urec->problem_data)
    {
      problem_data = XNEW (struct df_urec_problem_data);
      df_urec->problem_data = problem_data;
    }
  problem_data->earlyclobbers_found = false;

  df_grow_bb_info (df_urec);

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_urec_bb_info *bb_info = df_urec_get_bb_info (bb_index);
      if (bb_info)
	{ 
	  bitmap_clear (bb_info->kill);
	  bitmap_clear (bb_info->gen);
	  bitmap_clear (bb_info->earlyclobber);
	}
      else
	{ 
	  bb_info = (struct df_urec_bb_info *) pool_alloc (df_urec->block_pool);
	  df_urec_set_bb_info (bb_index, bb_info);
	  bb_info->kill = BITMAP_ALLOC (NULL);
	  bb_info->gen = BITMAP_ALLOC (NULL);
	  bb_info->in = BITMAP_ALLOC (NULL);
	  bb_info->out = BITMAP_ALLOC (NULL);
          bb_info->top = BITMAP_ALLOC (NULL);
	  bb_info->earlyclobber = BITMAP_ALLOC (NULL);
	}
    }
}


/* The function modifies local info for register REG being changed in
   SETTER.  DATA is used to pass the current basic block info.  */

static void
df_urec_mark_reg_change (rtx reg, rtx setter, void *data)
{
  int regno;
  int endregno;
  int i;
  struct df_urec_bb_info *bb_info = (struct df_urec_bb_info*) data;

  if (GET_CODE (reg) == SUBREG)
    reg = SUBREG_REG (reg);

  if (!REG_P (reg))
    return;
  
  
  endregno = regno = REGNO (reg);
  if (regno < FIRST_PSEUDO_REGISTER)
    {
      endregno +=hard_regno_nregs[regno][GET_MODE (reg)];
      
      for (i = regno; i < endregno; i++)
	{
	  bitmap_set_bit (bb_info->kill, i);
	  
	  if (GET_CODE (setter) != CLOBBER)
	    bitmap_set_bit (bb_info->gen, i);
	  else
	    bitmap_clear_bit (bb_info->gen, i);
	}
    }
  else
    {
      bitmap_set_bit (bb_info->kill, regno);
      
      if (GET_CODE (setter) != CLOBBER)
	bitmap_set_bit (bb_info->gen, regno);
      else
	bitmap_clear_bit (bb_info->gen, regno);
    }
}
/* Classes of registers which could be early clobbered in the current
   insn.  */

static VEC(int,heap) *earlyclobber_regclass;

/* This function finds and stores register classes that could be early
   clobbered in INSN.  If any earlyclobber classes are found, the function
   returns TRUE, in all other cases it returns FALSE.  */

static bool
df_urec_check_earlyclobber (rtx insn)
{
  int opno;
  bool found = false;

  extract_insn (insn);

  VEC_truncate (int, earlyclobber_regclass, 0);
  for (opno = 0; opno < recog_data.n_operands; opno++)
    {
      char c;
      bool amp_p;
      int i;
      enum reg_class class;
      const char *p = recog_data.constraints[opno];

      class = NO_REGS;
      amp_p = false;
      for (;;)
	{
	  c = *p;
	  switch (c)
	    {
	    case '=':  case '+':  case '?':
	    case '#':  case '!':
	    case '*':  case '%':
	    case 'm':  case '<':  case '>':  case 'V':  case 'o':
	    case 'E':  case 'F':  case 'G':  case 'H':
	    case 's':  case 'i':  case 'n':
	    case 'I':  case 'J':  case 'K':  case 'L':
	    case 'M':  case 'N':  case 'O':  case 'P':
	    case 'X':
	    case '0': case '1':  case '2':  case '3':  case '4':
	    case '5': case '6':  case '7':  case '8':  case '9':
	      /* These don't say anything we care about.  */
	      break;

	    case '&':
	      amp_p = true;
	      break;
	    case '\0':
	    case ',':
	      if (amp_p && class != NO_REGS)
		{
		  int rc;

		  found = true;
		  for (i = 0;
		       VEC_iterate (int, earlyclobber_regclass, i, rc);
		       i++)
		    {
		      if (rc == (int) class)
			goto found_rc;
		    }

		  /* We use VEC_quick_push here because
		     earlyclobber_regclass holds no more than
		     N_REG_CLASSES elements. */
		  VEC_quick_push (int, earlyclobber_regclass, (int) class);
		found_rc:
		  ;
		}
	      
	      amp_p = false;
	      class = NO_REGS;
	      break;

	    case 'r':
	      class = GENERAL_REGS;
	      break;

	    default:
	      class = REG_CLASS_FROM_CONSTRAINT (c, p);
	      break;
	    }
	  if (c == '\0')
	    break;
	  p += CONSTRAINT_LEN (c, p);
	}
    }

  return found;
}

/* The function checks that pseudo-register *X has a class
   intersecting with the class of pseudo-register could be early
   clobbered in the same insn.

   This function is a no-op if earlyclobber_regclass is empty. 

   Reload can assign the same hard register to uninitialized
   pseudo-register and early clobbered pseudo-register in an insn if
   the pseudo-register is used first time in given BB and not lived at
   the BB start.  To prevent this we don't change life information for
   such pseudo-registers.  */

static int
df_urec_mark_reg_use_for_earlyclobber (rtx *x, void *data)
{
  enum reg_class pref_class, alt_class;
  int i, regno;
  struct df_urec_bb_info *bb_info = (struct df_urec_bb_info*) data;

  if (REG_P (*x) && REGNO (*x) >= FIRST_PSEUDO_REGISTER)
    {
      int rc;

      regno = REGNO (*x);
      if (bitmap_bit_p (bb_info->kill, regno)
	  || bitmap_bit_p (bb_info->gen, regno))
	return 0;
      pref_class = reg_preferred_class (regno);
      alt_class = reg_alternate_class (regno);
      for (i = 0; VEC_iterate (int, earlyclobber_regclass, i, rc); i++)
	{
	  if (reg_classes_intersect_p (rc, pref_class)
	      || (rc != NO_REGS
		  && reg_classes_intersect_p (rc, alt_class)))
	    {
	      bitmap_set_bit (bb_info->earlyclobber, regno);
	      break;
	    }
	}
    }
  return 0;
}

/* The function processes all pseudo-registers in *X with the aid of
   previous function.  */

static void
df_urec_mark_reg_use_for_earlyclobber_1 (rtx *x, void *data)
{
  for_each_rtx (x, df_urec_mark_reg_use_for_earlyclobber, data);
}


/* Compute local uninitialized register info for basic block BB.  */

static void
df_urec_bb_local_compute (unsigned int bb_index)
{
  basic_block bb = BASIC_BLOCK (bb_index);
  struct df_urec_bb_info *bb_info = df_urec_get_bb_info (bb_index);
  rtx insn;
  struct df_ref **def_rec;

  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if (DF_REF_FLAGS (def) & DF_REF_AT_TOP)
	{
	  unsigned int regno = DF_REF_REGNO (def);
	  bitmap_set_bit (bb_info->gen, regno);
	}
    }
  
  FOR_BB_INSNS (bb, insn)
    {
      if (INSN_P (insn))
	{
	  note_stores (PATTERN (insn), df_urec_mark_reg_change, bb_info);
	  if (df_urec_check_earlyclobber (insn))
	    {
	      struct df_urec_problem_data *problem_data
		= (struct df_urec_problem_data *) df_urec->problem_data;
	      problem_data->earlyclobbers_found = true;
	      note_uses (&PATTERN (insn), 
			 df_urec_mark_reg_use_for_earlyclobber_1, bb_info);
	    }
	}
    }

  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if ((DF_REF_FLAGS (def) & DF_REF_AT_TOP) == 0)
	{
	  unsigned int regno = DF_REF_REGNO (def);
	  bitmap_set_bit (bb_info->gen, regno);
	}
    }
}


/* Compute local uninitialized register info.  */

static void
df_urec_local_compute (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
#ifdef STACK_REGS
  int i;
  HARD_REG_SET zero, stack_hard_regs, used;
  struct df_urec_problem_data *problem_data
    = (struct df_urec_problem_data *) df_urec->problem_data;
  
  /* Any register that MAY be allocated to a register stack (like the
     387) is treated poorly.  Each such register is marked as being
     live everywhere.  This keeps the register allocator and the
     subsequent passes from doing anything useful with these values.

     FIXME: This seems like an incredibly poor idea.  */

  CLEAR_HARD_REG_SET (zero);
  CLEAR_HARD_REG_SET (stack_hard_regs);
  for (i = FIRST_STACK_REG; i <= LAST_STACK_REG; i++)
    SET_HARD_REG_BIT (stack_hard_regs, i);
  problem_data->stack_regs = BITMAP_ALLOC (NULL);
  for (i = FIRST_PSEUDO_REGISTER; i < max_regno; i++)
    {
      COPY_HARD_REG_SET (used, reg_class_contents[reg_preferred_class (i)]);
      IOR_HARD_REG_SET (used, reg_class_contents[reg_alternate_class (i)]);
      AND_HARD_REG_SET (used, stack_hard_regs);
      GO_IF_HARD_REG_EQUAL (used, zero, skip);
      bitmap_set_bit (problem_data->stack_regs, i);
    skip:
      ;
    }
#endif

  /* We know that earlyclobber_regclass holds no more than
    N_REG_CLASSES elements.  See df_urec_check_earlyclobber.  */
  earlyclobber_regclass = VEC_alloc (int, heap, N_REG_CLASSES);

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      df_urec_bb_local_compute (bb_index);
    }

  VEC_free (int, heap, earlyclobber_regclass);
}


/* Initialize the solution vectors.  */

static void 
df_urec_init (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_urec_bb_info *bb_info = df_urec_get_bb_info (bb_index);

      bitmap_copy (bb_info->out, bb_info->gen);
      bitmap_clear (bb_info->in);
    }
}


/* Or in the stack regs, hard regs and early clobber regs into the
   ur_in sets of all of the blocks.  */
 

static void
df_urec_local_finalize (bitmap all_blocks)
{
  bitmap tmp = BITMAP_ALLOC (NULL);
  bitmap_iterator bi;
  unsigned int bb_index;
  struct df_urec_problem_data *problem_data
    = (struct df_urec_problem_data *) df_urec->problem_data;

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      struct df_urec_bb_info *bb_info = df_urec_get_bb_info (bb_index);
      struct df_lr_bb_info *bb_lr_info = df_lr_get_bb_info (bb_index);

      if (bb_index != ENTRY_BLOCK && bb_index != EXIT_BLOCK)
	{
	  if (problem_data->earlyclobbers_found)
	    bitmap_ior_into (bb_info->in, bb_info->earlyclobber);
	
#ifdef STACK_REGS
	  /* We can not use the same stack register for uninitialized
	     pseudo-register and another living pseudo-register
	     because if the uninitialized pseudo-register dies,
	     subsequent pass reg-stack will be confused (it will
	     believe that the other register dies).  */
	  bitmap_ior_into (bb_info->in, problem_data->stack_regs);
	  bitmap_ior_into (bb_info->out, problem_data->stack_regs);
#endif
	}

      /* No register may reach a location where it is not used.  Thus
	 we trim the rr result to the places where it is used.  */
      bitmap_and_into (bb_info->in, bb_lr_info->in);
      bitmap_and_into (bb_info->out, bb_lr_info->out);
      bitmap_copy (bb_info->top, bb_info->in);
      if (bb_lr_info->adef)
        bitmap_ior_into (bb_info->top, bb_lr_info->adef);
      bitmap_and_into (bb_info->top, bb_lr_info->top);
#if 0
      /* Hard registers may still stick in the ur_out set, but not
	 be in the ur_in set, if their only mention was in a call
	 in this block.  This is because a call kills in the lr
	 problem but does not kill in the rr problem.  To clean
	 this up, we execute the transfer function on the lr_in
	 set and then use that to knock bits out of ur_out.  */
      bitmap_ior_and_compl (tmp, bb_info->gen, bb_lr_info->in, 
			    bb_info->kill);
      bitmap_and_into (bb_info->out, tmp);
#endif
    }
  
#ifdef STACK_REGS
  BITMAP_FREE (problem_data->stack_regs);
#endif
  BITMAP_FREE (tmp);
}


/* Confluence function that ignores fake edges.  */

static void
df_urec_confluence_n (edge e)
{
  bitmap op1 = df_urec_get_bb_info (e->dest->index)->in;
  bitmap op2 = df_urec_get_bb_info (e->src->index)->out;
 
  if (e->flags & EDGE_FAKE) 
    return;

  bitmap_ior_into (op1, op2);
} 


/* Transfer function.  */

static bool
df_urec_transfer_function (int bb_index)
{
  struct df_urec_bb_info *bb_info = df_urec_get_bb_info (bb_index);
  bitmap in = bb_info->in;
  bitmap out = bb_info->out;
  bitmap gen = bb_info->gen;
  bitmap kill = bb_info->kill;

  return bitmap_ior_and_compl (out, gen, in, kill);
}


/* Free all storage associated with the problem.  */

static void
df_urec_free (void)
{
  if (df_urec->block_info)
    {
      unsigned int i;
      
      for (i = 0; i < df_urec->block_info_size; i++)
	{
	  struct df_urec_bb_info *bb_info = df_urec_get_bb_info (i);
	  if (bb_info)
	    {
	      BITMAP_FREE (bb_info->gen);
	      BITMAP_FREE (bb_info->kill);
	      BITMAP_FREE (bb_info->in);
	      BITMAP_FREE (bb_info->out);
	      BITMAP_FREE (bb_info->earlyclobber);
              BITMAP_FREE (bb_info->top);
	    }
	}
      
      free_alloc_pool (df_urec->block_pool);
      
      df_urec->block_info_size = 0;
      free (df_urec->block_info);
      free (df_urec->problem_data);
    }
  free (df_urec);
}


/* Debugging info at top of bb.  */

static void
df_urec_top_dump (basic_block bb, FILE *file)
{
  struct df_urec_bb_info *bb_info = df_urec_get_bb_info (bb->index);
  if (!bb_info || !bb_info->in)
    return;
      
  fprintf (file, ";; urec  in  \t");
  df_print_regset (file, bb_info->in);
  fprintf (file, ";; urec  gen \t");
  df_print_regset (file, bb_info->gen);
  fprintf (file, ";; urec  kill\t");
  df_print_regset (file, bb_info->kill);
  fprintf (file, ";; urec  ec\t");
  df_print_regset (file, bb_info->earlyclobber);
}


/* Debugging info at bottom of bb.  */

static void
df_urec_bottom_dump (basic_block bb, FILE *file)
{
  struct df_urec_bb_info *bb_info = df_urec_get_bb_info (bb->index);
  if (!bb_info || !bb_info->out)
    return;
  fprintf (file, ";; urec  out \t");
  df_print_regset (file, bb_info->out);
}


/* All of the information associated with every instance of the problem.  */

static struct df_problem problem_UREC =
{
  DF_UREC,                    /* Problem id.  */
  DF_FORWARD,                 /* Direction.  */
  df_urec_alloc,              /* Allocate the problem specific data.  */
  NULL,                       /* Reset global information.  */
  df_urec_free_bb_info,       /* Free basic block info.  */
  df_urec_local_compute,      /* Local compute function.  */
  df_urec_init,               /* Init the solution specific data.  */
  df_worklist_dataflow,       /* Worklist solver.  */
  NULL,                       /* Confluence operator 0.  */ 
  df_urec_confluence_n,       /* Confluence operator n.  */ 
  df_urec_transfer_function,  /* Transfer function.  */
  df_urec_local_finalize,     /* Finalize function.  */
  df_urec_free,               /* Free all of the problem information.  */
  df_urec_free,               /* Remove this problem from the stack of dataflow problems.  */
  NULL,                       /* Debugging.  */
  df_urec_top_dump,           /* Debugging start block.  */
  df_urec_bottom_dump,        /* Debugging end block.  */
  NULL,                       /* Incremental solution verify start.  */
  NULL,                       /* Incremental solution verfiy end.  */
  &problem_LR,                /* Dependent problem.  */
  TV_DF_UREC                  /* Timing variable.  */ 
};


/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_urec_add_problem (void)
{
  df_add_problem (&problem_UREC);
}



/*----------------------------------------------------------------------------
   CREATE DEF_USE (DU) and / or USE_DEF (UD) CHAINS

   Link either the defs to the uses and / or the uses to the defs.

   These problems are set up like the other dataflow problems so that
   they nicely fit into the framework.  They are much simpler and only
   involve a single traversal of instructions and an examination of
   the reaching defs information (the dependent problem).
----------------------------------------------------------------------------*/

#define df_chain_problem_p(FLAG) (((enum df_chain_flags)df_chain->local_flags)&(FLAG))

/* Create a du or ud chain from SRC to DST and link it into SRC.   */

struct df_link *
df_chain_create (struct df_ref *src, struct df_ref *dst)
{
  struct df_link *head = DF_REF_CHAIN (src);
  struct df_link *link = pool_alloc (df_chain->block_pool);;
  
  DF_REF_CHAIN (src) = link;
  link->next = head;
  link->ref = dst;
  return link;
}


/* Delete any du or ud chains that start at REF and point to
   TARGET.  */ 
static void
df_chain_unlink_1 (struct df_ref *ref, struct df_ref *target)
{
  struct df_link *chain = DF_REF_CHAIN (ref);
  struct df_link *prev = NULL;

  while (chain)
    {
      if (chain->ref == target)
	{
	  if (prev)
	    prev->next = chain->next;
	  else
	    DF_REF_CHAIN (ref) = chain->next;
	  pool_free (df_chain->block_pool, chain);
	  return;
	}
      prev = chain;
      chain = chain->next;
    }
}


/* Delete a du or ud chain that leave or point to REF.  */

void
df_chain_unlink (struct df_ref *ref)
{
  struct df_link *chain = DF_REF_CHAIN (ref);
  while (chain)
    {
      struct df_link *next = chain->next;
      /* Delete the other side if it exists.  */
      df_chain_unlink_1 (chain->ref, ref);
      pool_free (df_chain->block_pool, chain);
      chain = next;
    }
  DF_REF_CHAIN (ref) = NULL;
}


/* Copy the du or ud chain starting at FROM_REF and attach it to
   TO_REF.  */ 

void 
df_chain_copy (struct df_ref *to_ref, 
	       struct df_link *from_ref)
{
  while (from_ref)
    {
      df_chain_create (to_ref, from_ref->ref);
      from_ref = from_ref->next;
    }
}


/* Remove this problem from the stack of dataflow problems.  */

static void
df_chain_remove_problem (void)
{
  bitmap_iterator bi;
  unsigned int bb_index;

  /* Wholesale destruction of the old chains.  */ 
  if (df_chain->block_pool)
    free_alloc_pool (df_chain->block_pool);

  EXECUTE_IF_SET_IN_BITMAP (df_chain->out_of_date_transfer_functions, 0, bb_index, bi)
    {
      rtx insn;
      struct df_ref **def_rec;
      struct df_ref **use_rec;
      basic_block bb = BASIC_BLOCK (bb_index);

      if (df_chain_problem_p (DF_DU_CHAIN))
	for (def_rec = df_get_artificial_defs (bb->index); *def_rec; def_rec++)
	  DF_REF_CHAIN (*def_rec) = NULL;
      if (df_chain_problem_p (DF_UD_CHAIN))
	for (use_rec = df_get_artificial_uses (bb->index); *use_rec; use_rec++)
	  DF_REF_CHAIN (*use_rec) = NULL;
      
      FOR_BB_INSNS (bb, insn)
	{
	  unsigned int uid = INSN_UID (insn);
	  
	  if (INSN_P (insn))
	    {
	      if (df_chain_problem_p (DF_DU_CHAIN))
		for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
		  DF_REF_CHAIN (*def_rec) = NULL;
	      if (df_chain_problem_p (DF_UD_CHAIN))
		{
		  for (use_rec = DF_INSN_UID_USES (uid); *use_rec; use_rec++)
		    DF_REF_CHAIN (*use_rec) = NULL;
		  for (use_rec = DF_INSN_UID_EQ_USES (uid); *use_rec; use_rec++)
		    DF_REF_CHAIN (*use_rec) = NULL;
		}
	    }
	}
    }

  bitmap_clear (df_chain->out_of_date_transfer_functions);
  df_chain->block_pool = NULL;
}


/* Remove the chain problem completely.  */

static void
df_chain_fully_remove_problem (void)
{
  df_chain_remove_problem ();
  BITMAP_FREE (df_chain->out_of_date_transfer_functions);
  free (df_chain);
}


/* Create def-use or use-def chains.  */

static void  
df_chain_alloc (bitmap all_blocks ATTRIBUTE_UNUSED)

{
  df_chain_remove_problem ();
  df_chain->block_pool = create_alloc_pool ("df_chain_block pool", 
					 sizeof (struct df_link), 100);
}


/* Reset all of the chains when the set of basic blocks changes.  */

static void
df_chain_reset (bitmap blocks_to_clear ATTRIBUTE_UNUSED)
{
  df_chain_remove_problem ();
}


/* Create the chains for a list of USEs.  */

static void
df_chain_create_bb_process_use (bitmap local_rd,
				struct df_ref **use_rec,
				enum df_ref_flags top_flag)
{
  bitmap_iterator bi;
  unsigned int def_index;
  
  while (*use_rec)
    {
      struct df_ref *use = *use_rec;
      unsigned int uregno = DF_REF_REGNO (use);
      if ((!(df->changeable_flags & DF_NO_HARD_REGS))
	  || (uregno >= FIRST_PSEUDO_REGISTER))
	{
	  /* Do not want to go through this for an uninitialized var.  */
	  int count = DF_DEFS_COUNT (uregno);
	  if (count)
	    {
	      if (top_flag == (DF_REF_FLAGS (use) & DF_REF_AT_TOP))
		{
		  unsigned int first_index = DF_DEFS_BEGIN (uregno);
		  unsigned int last_index = first_index + count - 1;
		  
		  EXECUTE_IF_SET_IN_BITMAP (local_rd, first_index, def_index, bi)
		    {
		      struct df_ref *def;
		      if (def_index > last_index) 
			break;
		      
		      def = DF_DEFS_GET (def_index);
		      if (df_chain_problem_p (DF_DU_CHAIN))
			df_chain_create (def, use);
		      if (df_chain_problem_p (DF_UD_CHAIN))
			df_chain_create (use, def);
		    }
		}
	    }
	}

      use_rec++;
    }
}


/* Create chains from reaching defs bitmaps for basic block BB.  */

static void
df_chain_create_bb (unsigned int bb_index)
{
  basic_block bb = BASIC_BLOCK (bb_index);
  struct df_rd_bb_info *bb_info = df_rd_get_bb_info (bb_index);
  rtx insn;
  bitmap cpy = BITMAP_ALLOC (NULL);
  struct df_ref **def_rec;

  bitmap_copy (cpy, bb_info->in);
  bitmap_set_bit (df_chain->out_of_date_transfer_functions, bb_index);

  /* Since we are going forwards, process the artificial uses first
     then the artificial defs second.  */

#ifdef EH_USES
  /* Create the chains for the artificial uses from the EH_USES at the
     beginning of the block.  */
  
  /* Artificials are only hard regs.  */
  if (!(df->changeable_flags & DF_NO_HARD_REGS))
    df_chain_create_bb_process_use (cpy,
				    df_get_artificial_uses (bb->index), 
				    DF_REF_AT_TOP);
#endif

  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if (DF_REF_FLAGS (def) & DF_REF_AT_TOP)
	{
	  unsigned int dregno = DF_REF_REGNO (def);
	  if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
	    bitmap_clear_range (cpy, 
				DF_DEFS_BEGIN (dregno), 
				DF_DEFS_COUNT (dregno));
	  bitmap_set_bit (cpy, DF_REF_ID (def));
	}
    }
  
  /* Process the regular instructions next.  */
  FOR_BB_INSNS (bb, insn)
    {
      struct df_ref **def_rec;
      unsigned int uid = INSN_UID (insn);

      if (!INSN_P (insn))
	continue;

      /* Now scan the uses and link them up with the defs that remain
	 in the cpy vector.  */
      
      df_chain_create_bb_process_use (cpy, DF_INSN_UID_USES (uid), 0);

      if (df->changeable_flags & DF_EQ_NOTES)
	df_chain_create_bb_process_use (cpy, DF_INSN_UID_EQ_USES (uid), 0);


      /* Since we are going forwards, process the defs second.  This
         pass only changes the bits in cpy.  */
      for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	{
	  struct df_ref *def = *def_rec;
	  unsigned int dregno = DF_REF_REGNO (def);
	  if ((!(df->changeable_flags & DF_NO_HARD_REGS))
	      || (dregno >= FIRST_PSEUDO_REGISTER))
	    {
	      if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
		bitmap_clear_range (cpy, 
				    DF_DEFS_BEGIN (dregno), 
				    DF_DEFS_COUNT (dregno));
	      if (!(DF_REF_FLAGS (def) 
		    & (DF_REF_MUST_CLOBBER | DF_REF_MAY_CLOBBER)))
		bitmap_set_bit (cpy, DF_REF_ID (def));
	    }
	}
    }

  /* Create the chains for the artificial uses of the hard registers
     at the end of the block.  */
  if (!(df->changeable_flags & DF_NO_HARD_REGS))
    df_chain_create_bb_process_use (cpy,
				    df_get_artificial_uses (bb->index), 
				    0);

  BITMAP_FREE (cpy);
}

/* Create def-use chains from reaching use bitmaps for basic blocks
   in BLOCKS.  */

static void
df_chain_finalize (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
  
  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
    {
      df_chain_create_bb (bb_index);
    }
}


/* Free all storage associated with the problem.  */

static void
df_chain_free (void)
{
  free_alloc_pool (df_chain->block_pool);
  BITMAP_FREE (df_chain->out_of_date_transfer_functions);
  free (df_chain);
}


/* Debugging info.  */

static void
df_chain_top_dump (basic_block bb, FILE *file)
{
  if (df_chain_problem_p (DF_DU_CHAIN))
    {
      rtx insn;
      struct df_ref **def_rec = df_get_artificial_defs (bb->index);
      if (*def_rec)
	{
	  
	  fprintf (file, ";;  DU chains for artificial defs\n");
	  while (*def_rec)
	    {
	      struct df_ref *def = *def_rec;
	      fprintf (file, ";;   reg %d ", DF_REF_REGNO (def));
	      df_chain_dump (DF_REF_CHAIN (def), file);
	      fprintf (file, "\n");
	      def_rec++;
	    }
	}      

      FOR_BB_INSNS (bb, insn)
	{
	  unsigned int uid = INSN_UID (insn);
	  if (INSN_P (insn))
	    {
	      def_rec = DF_INSN_UID_DEFS (uid);
	      if (*def_rec)
		{
		  fprintf (file, ";;   DU chains for insn luid %d uid %d\n", 
			   DF_INSN_LUID (insn), uid);
		  
		  while (*def_rec)
		    {
		      struct df_ref *def = *def_rec;
		      fprintf (file, ";;      reg %d ", DF_REF_REGNO (def));
		      if (def->flags & DF_REF_READ_WRITE)
			fprintf (file, "read/write ");
		      df_chain_dump (DF_REF_CHAIN (def), file);
		      fprintf (file, "\n");
		      def_rec++;
		    }
		}
	    }
	}
    }
}


static void
df_chain_bottom_dump (basic_block bb, FILE *file)
{
  if (df_chain_problem_p (DF_UD_CHAIN))
    {
      rtx insn;
      struct df_ref **use_rec = df_get_artificial_uses (bb->index);

      if (*use_rec)
	{
	  fprintf (file, ";;  UD chains for artificial uses\n");
	  while (*use_rec)
	    {
	      struct df_ref *use = *use_rec;
	      fprintf (file, ";;   reg %d ", DF_REF_REGNO (use));
	      df_chain_dump (DF_REF_CHAIN (use), file);
	      fprintf (file, "\n");
	      use_rec++;
	    }
	}      

      FOR_BB_INSNS (bb, insn)
	{
	  unsigned int uid = INSN_UID (insn);
	  if (INSN_P (insn))
	    {
	      struct df_ref **eq_use_rec = DF_INSN_UID_EQ_USES (uid);
	      use_rec = DF_INSN_UID_USES (uid);
	      if (*use_rec || *eq_use_rec)
		{
		  fprintf (file, ";;   UD chains for insn luid %d uid %d\n", 
			   DF_INSN_LUID (insn), uid);
		  
		  while (*use_rec)
		    {
		      struct df_ref *use = *use_rec;
		      fprintf (file, ";;      reg %d ", DF_REF_REGNO (use));
		      if (use->flags & DF_REF_READ_WRITE)
			fprintf (file, "read/write ");
		      df_chain_dump (DF_REF_CHAIN (use), file);
		      fprintf (file, "\n");
		      use_rec++;
		    }
		  while (*eq_use_rec)
		    {
		      struct df_ref *use = *eq_use_rec;
		      fprintf (file, ";;   eq_note reg %d ", DF_REF_REGNO (use));
		      df_chain_dump (DF_REF_CHAIN (use), file);
		      fprintf (file, "\n");
		      eq_use_rec++;
		    }
		}
	    }
	}
    }
}


static struct df_problem problem_CHAIN =
{
  DF_CHAIN,                   /* Problem id.  */
  DF_NONE,                    /* Direction.  */
  df_chain_alloc,             /* Allocate the problem specific data.  */
  df_chain_reset,             /* Reset global information.  */
  NULL,                       /* Free basic block info.  */
  NULL,                       /* Local compute function.  */
  NULL,                       /* Init the solution specific data.  */
  NULL,                       /* Iterative solver.  */
  NULL,                       /* Confluence operator 0.  */ 
  NULL,                       /* Confluence operator n.  */ 
  NULL,                       /* Transfer function.  */
  df_chain_finalize,          /* Finalize function.  */
  df_chain_free,              /* Free all of the problem information.  */
  df_chain_fully_remove_problem,/* Remove this problem from the stack of dataflow problems.  */
  NULL,                       /* Debugging.  */
  df_chain_top_dump,          /* Debugging start block.  */
  df_chain_bottom_dump,       /* Debugging end block.  */
  NULL,                       /* Incremental solution verify start.  */
  NULL,                       /* Incremental solution verfiy end.  */
  &problem_RD,                /* Dependent problem.  */
  TV_DF_CHAIN                 /* Timing variable.  */ 
};


/* Indexed by n, giving various register information */

VEC(reg_info_p,heap) *reg_n_info;

/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_chain_add_problem (enum df_chain_flags chain_flags)
{
  df_add_problem (&problem_CHAIN);
  df_chain->local_flags = (unsigned int)chain_flags;
  df_chain->out_of_date_transfer_functions = BITMAP_ALLOC (NULL);
}

#undef df_chain_problem_p



/*----------------------------------------------------------------------------
   REGISTER INFORMATION

   This pass properly computes REG_DEAD and REG_UNUSED notes.

   If the DF_RI_LIFE flag is set the following vectors containing
   information about register usage are properly set: REG_N_REFS,
   REG_N_DEATHS, REG_N_SETS, REG_LIVE_LENGTH, REG_N_CALLS_CROSSED,
   REG_N_THROWING_CALLS_CROSSED and REG_BASIC_BLOCK.

   ----------------------------------------------------------------------------*/

#define df_ri_problem_p(FLAG) (((enum df_ri_flags)df_ri->local_flags)&(FLAG))

struct df_ri_problem_data
{
  bitmap setjmp_crosses;
};


#ifdef REG_DEAD_DEBUGGING
static void 
print_note (const char *prefix, rtx insn, rtx note)
{
  if (dump_file)
    {
      fprintf (dump_file, "%s %d ", prefix, INSN_UID (insn));
      print_rtl (dump_file, note);
      fprintf (dump_file, "\n");
    }
}
#endif

/* Allocate the lifetime information.  */

static void 
df_ri_alloc (bitmap all_blocks ATTRIBUTE_UNUSED)
{
  int i;
  struct df_ri_problem_data *problem_data =
    (struct df_ri_problem_data *) df_ri->problem_data;

  df_grow_reg_info ();

  if (!df_ri->problem_data)
    {
      problem_data = XNEW (struct df_ri_problem_data);
      df_ri->problem_data = problem_data;
      problem_data->setjmp_crosses = NULL;
    }

  if (df_ri_problem_p (DF_RI_SETJMP))
    {
      if (problem_data->setjmp_crosses)
	bitmap_clear (problem_data->setjmp_crosses);
      else 
	problem_data->setjmp_crosses = BITMAP_ALLOC (&df_bitmap_obstack);
    }

  if (df_ri_problem_p (DF_RI_LIFE))
    {
      max_regno = max_reg_num ();
      allocate_reg_info (max_regno, FALSE, FALSE);
      
      /* Reset all the data we'll collect.  */
      for (i = 0; i < max_regno; i++)
	{
	  REG_N_SETS (i) = DF_REG_DEF_COUNT (i);
	  REG_N_REFS (i) = DF_REG_USE_COUNT (i) + REG_N_SETS (i);
	  REG_N_DEATHS (i) = 0;
	  REG_N_CALLS_CROSSED (i) = 0;
	  REG_N_THROWING_CALLS_CROSSED (i) = 0;
	  REG_LIVE_LENGTH (i) = 0;
	  REG_FREQ (i) = 0;
	  REG_BASIC_BLOCK (i) = REG_BLOCK_UNKNOWN;
	}
    }
}


/* After reg-stack, the x86 floating point stack regs are difficult to
   analyze because of all of the pushes, pops and rotations.  Thus, we
   just leave the notes alone. */

#ifdef STACK_REGS
static inline bool 
df_ignore_stack_reg (int regno)
{
  return regstack_completed
    && IN_RANGE (regno, FIRST_STACK_REG, LAST_STACK_REG);
}
#else
static inline bool 
df_ignore_stack_reg (int regno ATTRIBUTE_UNUSED)
{
  return false;
}
#endif


/* Remove all of the REG_DEAD or REG_UNUSED notes from INSN.  */

static void
df_kill_notes (rtx insn, enum df_ri_flags flags)
{
  rtx *pprev = &REG_NOTES (insn);
  rtx link = *pprev;
  
  while (link)
    {
      switch (REG_NOTE_KIND (link))
	{
	case REG_DEAD:
	  if (flags & DF_RI_LIFE)
	    if (df_ignore_stack_reg (REGNO (XEXP (link, 0))))
	      REG_N_DEATHS (REGNO (XEXP (link, 0)))++;

	  /* Fallthru */
	case REG_UNUSED:
	  if (!df_ignore_stack_reg (REGNO (XEXP (link, 0))))
	    {
	      rtx next = XEXP (link, 1);
#ifdef REG_DEAD_DEBUGGING
	      print_note ("deleting: ", insn, link);
#endif
	      free_EXPR_LIST_node (link);
	      *pprev = link = next;
	    }
	  break;
	  
	default:
	  pprev = &XEXP (link, 1);
	  link = *pprev;
	  break;
	}
    }
}


/* Set the REG_UNUSED notes for the multiword hardreg defs in INSN
   based on the bits in LIVE.  Do not generate notes for registers in
   artificial uses.  DO_NOT_GEN is updated so that REG_DEAD notes are
   not generated if the reg is both read and written by the
   instruction.
*/

static void
df_set_unused_notes_for_mw (rtx insn, struct df_mw_hardreg *mws,
			    bitmap live, bitmap do_not_gen, 
			    bitmap artificial_uses, enum df_ri_flags flags)
{
  bool all_dead = true;
  unsigned int r;
  
#ifdef REG_DEAD_DEBUGGING
  if (dump_file)
    fprintf (dump_file, "mw_set_unused looking at mws[%d..%d]\n", 
	     mws->start_regno, mws->end_regno);
#endif
  for (r=mws->start_regno; r <= mws->end_regno; r++)
    if ((bitmap_bit_p (live, r))
	|| bitmap_bit_p (artificial_uses, r))
      {
	all_dead = false;
	break;
      }
  
  if (all_dead)
    {
      unsigned int regno = mws->start_regno;
      rtx note = alloc_EXPR_LIST (REG_UNUSED, *(mws->loc), 
				  REG_NOTES (insn));
      REG_NOTES (insn) = note;
#ifdef REG_DEAD_DEBUGGING
      print_note ("adding 1: ", insn, note);
#endif
      bitmap_set_bit (do_not_gen, regno);
      /* Only do this if the value is totally dead.  */
      if (flags & DF_RI_LIFE)
	{
	  REG_N_DEATHS (regno) ++;
	  REG_LIVE_LENGTH (regno)++;
	}
    }
  else
    for (r=mws->start_regno; r <= mws->end_regno; r++)
      {
	
	if ((!bitmap_bit_p (live, r))
	    && (!bitmap_bit_p (artificial_uses, r)))
	  {
	    rtx note = alloc_EXPR_LIST (REG_UNUSED, regno_reg_rtx[r], 
					REG_NOTES (insn));
	    REG_NOTES (insn) = note;
#ifdef REG_DEAD_DEBUGGING
	    print_note ("adding 2: ", insn, note);
#endif
	  }
	bitmap_set_bit (do_not_gen, r);
      }
}


/* Set the REG_DEAD notes for the multiword hardreg use in INSN based
   on the bits in LIVE.  DO_NOT_GEN is used to keep REG_DEAD notes
   from being set if the instruction both reads and writes the
   register.  */

static void
df_set_dead_notes_for_mw (rtx insn, struct df_mw_hardreg *mws,
			  bitmap live, bitmap do_not_gen,
			  bitmap artificial_uses, enum df_ri_flags flags)
{
  bool all_dead = true;
  unsigned int r;
  
#ifdef REG_DEAD_DEBUGGING
  if (dump_file)
    {
      fprintf (dump_file, "mw_set_dead looking at mws[%d..%d]\n  do_not_gen =", 
	       mws->start_regno, mws->end_regno);
      df_print_regset (dump_file, do_not_gen);
      fprintf (dump_file, "  live =");
      df_print_regset (dump_file, live);
      fprintf (dump_file, "  artificial uses =");
      df_print_regset (dump_file, artificial_uses);
    }
#endif

  for (r = mws->start_regno; r <= mws->end_regno; r++)
    if ((bitmap_bit_p (live, r))
	|| bitmap_bit_p (artificial_uses, r)
	|| bitmap_bit_p (do_not_gen, r))
      {
	all_dead = false;
	break;
      }
  
  if (all_dead)
    {
      if (!bitmap_bit_p (do_not_gen, mws->start_regno))
	{
	  /* Add a dead note for the entire multi word register.  */
	  rtx note = alloc_EXPR_LIST (REG_DEAD, *(mws->loc), 
				      REG_NOTES (insn));
	  REG_NOTES (insn) = note;
#ifdef REG_DEAD_DEBUGGING
	  print_note ("adding 1: ", insn, note);
#endif

	  if (flags & DF_RI_LIFE)
	    for (r = mws->start_regno; r <= mws->end_regno; r++)
	      REG_N_DEATHS (r)++;
	}
    }
  else
    {
      for (r = mws->start_regno; r <= mws->end_regno; r++)
	{
	  if ((!bitmap_bit_p (live, r))
	      && (!bitmap_bit_p (artificial_uses, r))
	      && (!bitmap_bit_p (do_not_gen, r)))
	    {
	      rtx note = alloc_EXPR_LIST (REG_DEAD, regno_reg_rtx[r], 
					  REG_NOTES (insn));
	      REG_NOTES (insn) = note;
	      if (flags & DF_RI_LIFE)
		REG_N_DEATHS (r)++;
#ifdef REG_DEAD_DEBUGGING
	      print_note ("adding 2: ", insn, note);
#endif
	    }
	}
    }
}


/* Create a REG_UNUSED note if necessary for DEF in INSN updating LIVE
   and DO_NOT_GEN.  Do not generate notes for registers in artificial
   uses.  */

static void
df_create_unused_note (basic_block bb, rtx insn, struct df_ref *def, 
		       bitmap live, bitmap do_not_gen, bitmap artificial_uses, 
		       bitmap local_live, bitmap local_processed, 
		       enum df_ri_flags flags, int luid)
{
  unsigned int dregno = DF_REF_REGNO (def);
  
#ifdef REG_DEAD_DEBUGGING
  if (dump_file)
    {
      fprintf (dump_file, "  regular looking at def ");
      df_ref_debug (def, dump_file);
    }
#endif

  if (bitmap_bit_p (live, dregno))
    {
      if (flags & DF_RI_LIFE)
	{
	  /* If we have seen this regno, then it has already been
	     processed correctly with the per insn increment.  If we
	     have not seen it we need to add the length from here to
	     the end of the block to the live length.  */
	  if (bitmap_bit_p (local_processed, dregno))
	    {
	      if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
		bitmap_clear_bit (local_live, dregno);
	    }
	  else
	    {
	      bitmap_set_bit (local_processed, dregno);
	      REG_LIVE_LENGTH (dregno) += luid;
	    }
	}
    }
  else if ((!(DF_REF_FLAGS (def) & DF_REF_MW_HARDREG))
	    && (!bitmap_bit_p (artificial_uses, dregno)) 
	    && (!df_ignore_stack_reg (dregno)))
    {
      rtx reg = (DF_REF_LOC (def)) 
                ? *DF_REF_REAL_LOC (def): DF_REF_REG (def);
      rtx note = alloc_EXPR_LIST (REG_UNUSED, reg, REG_NOTES (insn));
      REG_NOTES (insn) = note;
#ifdef REG_DEAD_DEBUGGING
      print_note ("adding 3: ", insn, note);
#endif
      if (flags & DF_RI_LIFE)
	{
	  REG_N_DEATHS (dregno) ++;
	  REG_LIVE_LENGTH (dregno)++;
	}
    }
  
  if ((flags & DF_RI_LIFE) && (dregno >= FIRST_PSEUDO_REGISTER))
    {
      REG_FREQ (dregno) += REG_FREQ_FROM_BB (bb);
      if (REG_BASIC_BLOCK (dregno) == REG_BLOCK_UNKNOWN)
	REG_BASIC_BLOCK (dregno) = bb->index;
      else if (REG_BASIC_BLOCK (dregno) != bb->index)
	REG_BASIC_BLOCK (dregno) = REG_BLOCK_GLOBAL;
    }

  if (!(DF_REF_FLAGS (def) & (DF_REF_MUST_CLOBBER + DF_REF_MAY_CLOBBER)))
    bitmap_set_bit (do_not_gen, dregno);
  
  /* Kill this register if it is not a subreg store or conditional store.  */
  if (!(DF_REF_FLAGS (def) & (DF_REF_PARTIAL | DF_REF_CONDITIONAL)))
    bitmap_clear_bit (live, dregno);
}


/* Recompute the REG_DEAD and REG_UNUSED notes and compute register
   info: lifetime, bb, and number of defs and uses for basic block
   BB.  The three bitvectors are scratch regs used here.  */

static void
df_ri_bb_compute (unsigned int bb_index, 
		  bitmap live, bitmap do_not_gen, bitmap artificial_uses,
		  bitmap local_live, bitmap local_processed, bitmap setjmp_crosses)
{
  basic_block bb = BASIC_BLOCK (bb_index);
  rtx insn;
  struct df_ref **def_rec;
  struct df_ref **use_rec;
  int luid = 0;

  bitmap_copy (live, df_get_live_out (bb));
  bitmap_clear (artificial_uses);

#ifdef REG_DEAD_DEBUGGING
  if (dump_file)
    {
      fprintf (dump_file, "live at bottom ");
      df_print_regset (dump_file, live);
    }
#endif

  if (df_ri_problem_p (DF_RI_LIFE))
    {
      /* Process the regs live at the end of the block.  Mark them as
	 not local to any one basic block.  */
      bitmap_iterator bi;
      unsigned int regno;
      EXECUTE_IF_SET_IN_BITMAP (live, 0, regno, bi)
	REG_BASIC_BLOCK (regno) = REG_BLOCK_GLOBAL;
    }

  /* Process the artificial defs and uses at the bottom of the block
     to begin processing.  */
  for (def_rec = df_get_artificial_defs (bb_index); *def_rec; def_rec++)
    {
      struct df_ref *def = *def_rec;
      if (dump_file)
	fprintf (dump_file, "artificial def %d\n", DF_REF_REGNO (def));

      if ((DF_REF_FLAGS (def) & DF_REF_AT_TOP) == 0)
	bitmap_clear_bit (live, DF_REF_REGNO (def));
    }

  for (use_rec = df_get_artificial_uses (bb_index); *use_rec; use_rec++)
    {
      struct df_ref *use = *use_rec;
      if ((DF_REF_FLAGS (use) & DF_REF_AT_TOP) == 0)
	{
	  unsigned int regno = DF_REF_REGNO (use);
	  bitmap_set_bit (live, regno);
	  
	  /* Notes are not generated for any of the artificial registers
	     at the bottom of the block.  */
	  bitmap_set_bit (artificial_uses, regno);
	}
    }
  
#ifdef REG_DEAD_DEBUGGING
  if (dump_file)
    {
      fprintf (dump_file, "live before artificials out ");
      df_print_regset (dump_file, live);
    }
#endif

  FOR_BB_INSNS_REVERSE (bb, insn)
    {
      unsigned int uid = INSN_UID (insn);
      unsigned int regno;
      bitmap_iterator bi;
      struct df_mw_hardreg **mws_rec;
      
      if (!INSN_P (insn))
	continue;

      if (df_ri_problem_p (DF_RI_LIFE))
	{
	  /* Increment the live_length for all of the registers that
	     are are referenced in this block and live at this
	     particular point.  */
	  bitmap_iterator bi;
	  unsigned int regno;
	  EXECUTE_IF_SET_IN_BITMAP (local_live, 0, regno, bi)
	    {
	      REG_LIVE_LENGTH (regno)++;
	    }
	  luid++;
	}

      bitmap_clear (do_not_gen);
      df_kill_notes (insn, (enum df_ri_flags)df_ri->local_flags);

      /* Process the defs.  */
      if (CALL_P (insn))
	{
#ifdef REG_DEAD_DEBUGGING
	  if (dump_file)
	    {
	      fprintf (dump_file, "processing call %d\n  live =", INSN_UID (insn));
	      df_print_regset (dump_file, live);
	    }
#endif
	  if (df_ri_problem_p (DF_RI_LIFE | DF_RI_SETJMP))
	    {
	      bool can_throw = can_throw_internal (insn); 
	      bool set_jump = (find_reg_note (insn, REG_SETJMP, NULL) != NULL);
	      EXECUTE_IF_SET_IN_BITMAP (live, 0, regno, bi)
		{
		  if (df_ri_problem_p (DF_RI_LIFE))
		    {
		      REG_N_CALLS_CROSSED (regno)++;
		      if (can_throw)
			REG_N_THROWING_CALLS_CROSSED (regno)++;
		    }
		  /* We have a problem with any pseudoreg that lives
		     across the setjmp.  ANSI says that if a user
		     variable does not change in value between the
		     setjmp and the longjmp, then the longjmp
		     preserves it.  This includes longjmp from a place
		     where the pseudo appears dead.  (In principle,
		     the value still exists if it is in scope.)  If
		     the pseudo goes in a hard reg, some other value
		     may occupy that hard reg where this pseudo is
		     dead, thus clobbering the pseudo.  Conclusion:
		     such a pseudo must not go in a hard reg.  */
		  if (set_jump)
		    bitmap_set_bit (setjmp_crosses, regno);
		}
	    }
	  
	  /* We only care about real sets for calls.  Clobbers only
	     may clobbers cannot be depended on.  */
	  mws_rec = DF_INSN_UID_MWS (uid);
	  while (*mws_rec)
	    {
	      struct df_mw_hardreg *mws = *mws_rec; 
	      if ((mws->type == DF_REF_REG_DEF) 
		  && !df_ignore_stack_reg (REGNO (mws->mw_reg)))
		df_set_unused_notes_for_mw (insn, mws, live, do_not_gen, 
					    artificial_uses, 
					    (enum df_ri_flags)df_ri->local_flags);
	      mws_rec++;
	    }

	  /* All of the defs except the return value are some sort of
	     clobber.  This code is for the return.  */
	  for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	    {
	      struct df_ref *def = *def_rec;
	      if (!(DF_REF_FLAGS (def) & (DF_REF_MUST_CLOBBER | DF_REF_MAY_CLOBBER)))
		df_create_unused_note (bb, insn, def, live, do_not_gen, 
				       artificial_uses, local_live, 
				       local_processed, 
				       (enum df_ri_flags)df_ri->local_flags, luid);
	    }
	}
      else
	{
	  /* Regular insn.  */
	  mws_rec = DF_INSN_UID_MWS (uid);
	  while (*mws_rec)
	    {
	      struct df_mw_hardreg *mws = *mws_rec; 
	      if (mws->type == DF_REF_REG_DEF)
		df_set_unused_notes_for_mw (insn, mws, live, do_not_gen, 
					    artificial_uses, 
					    (enum df_ri_flags)df_ri->local_flags);
	      mws_rec++;
	    }

	  for (def_rec = DF_INSN_UID_DEFS (uid); *def_rec; def_rec++)
	    {
	      struct df_ref *def = *def_rec;
	      df_create_unused_note (bb, insn, def, live, do_not_gen, 
				     artificial_uses, local_live, 
				     local_processed, 
				     (enum df_ri_flags)df_ri->local_flags, luid);
	    }
	}
      
      /* Process the uses.  */
      mws_rec = DF_INSN_UID_MWS (uid);
      while (*mws_rec)
	{
	  struct df_mw_hardreg *mws = *mws_rec; 
	  if ((mws->type != DF_REF_REG_DEF)  
	      && !df_ignore_stack_reg (REGNO (mws->mw_reg)))
	    df_set_dead_notes_for_mw (insn, mws, live, do_not_gen,
				      artificial_uses, 
				      (enum df_ri_flags)df_ri->local_flags);
	  mws_rec++;
	}

      for (use_rec = DF_INSN_UID_USES (uid); *use_rec; use_rec++)
	{
	  struct df_ref *use = *use_rec;
	  unsigned int uregno = DF_REF_REGNO (use);

	  if (df_ri_problem_p (DF_RI_LIFE) && (uregno >= FIRST_PSEUDO_REGISTER))
	    {
	      REG_FREQ (uregno) += REG_FREQ_FROM_BB (bb);
	      if (REG_BASIC_BLOCK (uregno) == REG_BLOCK_UNKNOWN)
		REG_BASIC_BLOCK (uregno) = bb->index;
	      else if (REG_BASIC_BLOCK (uregno) != bb->index)
		REG_BASIC_BLOCK (uregno) = REG_BLOCK_GLOBAL;
	    }
	  
#ifdef REG_DEAD_DEBUGGING
	  if (dump_file)
	    {
	      fprintf (dump_file, "  regular looking at use ");
	      df_ref_debug (use, dump_file);
	    }
#endif
	  if (!bitmap_bit_p (live, uregno))
	    {
	      if ( (!(DF_REF_FLAGS (use) & DF_REF_MW_HARDREG))
		   && (!bitmap_bit_p (do_not_gen, uregno))
		   && (!bitmap_bit_p (artificial_uses, uregno))
		   && (!(DF_REF_FLAGS (use) & DF_REF_READ_WRITE))
		   && (!df_ignore_stack_reg (uregno)))
		{
		  rtx reg = (DF_REF_LOC (use)) 
                            ? *DF_REF_REAL_LOC (use) : DF_REF_REG (use);
		  rtx note = alloc_EXPR_LIST (REG_DEAD, reg, REG_NOTES (insn));
		  REG_NOTES (insn) = note;
		  if (df_ri_problem_p (DF_RI_LIFE))
		    REG_N_DEATHS (uregno)++;

#ifdef REG_DEAD_DEBUGGING
		  print_note ("adding 4: ", insn, note);
#endif
		}
	      /* This register is now live.  */
	      bitmap_set_bit (live, uregno);

	      if (df_ri_problem_p (DF_RI_LIFE))
		{
		  /* If we have seen this regno, then it has already
		     been processed correctly with the per insn
		     increment.  If we have not seen it we set the bit
		     so that begins to get processed locally.  Note
		     that we don't even get here if the variable was
		     live at the end of the block since just a ref
		     inside the block does not effect the
		     calculations.  */
		  REG_LIVE_LENGTH (uregno) ++;
		  bitmap_set_bit (local_live, uregno);
		  bitmap_set_bit (local_processed, uregno);
		}
	    }
	}
    }
  
  if (df_ri_problem_p (DF_RI_LIFE))
    {
      /* Add the length of the block to all of the registers that were
	 not referenced, but still live in this block.  */
      bitmap_iterator bi;
      unsigned int regno;
      bitmap_and_compl_into (live, local_processed);
      EXECUTE_IF_SET_IN_BITMAP (live, 0, regno, bi)
	{
	  REG_LIVE_LENGTH (regno) += luid;
	}
      bitmap_clear (local_processed);
      bitmap_clear (local_live);
    }
}


/* Compute register info: lifetime, bb, and number of defs and uses.  */
static void
df_ri_compute (bitmap all_blocks)
{
  unsigned int bb_index;
  bitmap_iterator bi;
  bitmap live = BITMAP_ALLOC (&df_bitmap_obstack);
  bitmap do_not_gen = BITMAP_ALLOC (&df_bitmap_obstack);
  bitmap artificial_uses = BITMAP_ALLOC (&df_bitmap_obstack);
  bitmap local_live = NULL;
  bitmap local_processed = NULL;
  bitmap setjmp_crosses = NULL;
  struct df_ri_problem_data *problem_data =
    (struct df_ri_problem_data *) df_ri->problem_data;

  if (df_ri_problem_p (DF_RI_LIFE))
    {
      local_live = BITMAP_ALLOC (&df_bitmap_obstack);
      local_processed = BITMAP_ALLOC (&df_bitmap_obstack);
      if (df_ri_problem_p (DF_RI_SETJMP))
	setjmp_crosses = problem_data->setjmp_crosses;
      else
	setjmp_crosses = BITMAP_ALLOC (&df_bitmap_obstack);
    }
  else if (df_ri_problem_p (DF_RI_SETJMP))
    setjmp_crosses = problem_data->setjmp_crosses;


#ifdef REG_DEAD_DEBUGGING
  if (dump_file)
    print_rtl_with_bb (dump_file, get_insns());
#endif

  EXECUTE_IF_SET_IN_BITMAP (all_blocks, 0, bb_index, bi)
  {
    df_ri_bb_compute (bb_index, live, do_not_gen, artificial_uses,
		      local_live, local_processed, setjmp_crosses);
  }

  BITMAP_FREE (live);
  BITMAP_FREE (do_not_gen);
  BITMAP_FREE (artificial_uses);
  if (df_ri_problem_p (DF_RI_LIFE))
    {
      bitmap_iterator bi;
      unsigned int regno;
      /* See the setjmp comment in df_ri_bb_compute.  */
      EXECUTE_IF_SET_IN_BITMAP (setjmp_crosses, FIRST_PSEUDO_REGISTER, 
				regno, bi)
	{
	  REG_BASIC_BLOCK (regno) = REG_BLOCK_UNKNOWN;
	  REG_LIVE_LENGTH (regno) = -1;
	}	  

      BITMAP_FREE (local_live);
      BITMAP_FREE (local_processed);
      if (!df_ri_problem_p (DF_RI_SETJMP))
	BITMAP_FREE (setjmp_crosses);
    }
}


/* Free all storage associated with the problem.  */

static void
df_ri_free (void)
{
  struct df_ri_problem_data *problem_data =
    (struct df_ri_problem_data *) df_ri->problem_data;

  if (df_ri_problem_p (DF_RI_SETJMP))
    BITMAP_FREE (problem_data->setjmp_crosses);

  free (df_ri->problem_data);
  free (df_ri);
}


/* Debugging info.  */

static void
df_ri_start_dump (FILE *file)
{
  if (df_ri_problem_p (DF_RI_LIFE))
    {
      fprintf (file, ";; Register info:\n");
      dump_reg_info (file);
    }
}

/* All of the information associated every instance of the problem.  */

static struct df_problem problem_RI =
{
  DF_RI,                      /* Problem id.  */
  DF_NONE,                    /* Direction.  */
  df_ri_alloc,                /* Allocate the problem specific data.  */
  NULL,                       /* Reset global information.  */
  NULL,                       /* Free basic block info.  */
  df_ri_compute,              /* Local compute function.  */
  NULL,                       /* Init the solution specific data.  */
  NULL,                       /* Iterative solver.  */
  NULL,                       /* Confluence operator 0.  */ 
  NULL,                       /* Confluence operator n.  */ 
  NULL,                       /* Transfer function.  */
  NULL,                       /* Finalize function.  */
  df_ri_free,                 /* Free all of the problem information.  */
  df_ri_free,                 /* Remove this problem from the stack of dataflow problems.  */
  df_ri_start_dump,           /* Debugging.  */
  NULL,                       /* Debugging start block.  */
  NULL,                       /* Debugging end block.  */
  NULL,                       /* Incremental solution verify start.  */
  NULL,                       /* Incremental solution verfiy end.  */

  /* Technically this is only dependent on the live registers problem
     but it will produce information if built one of uninitialized
     register problems (UR, UREC) is also run.  */
  &problem_LR,                /* Dependent problem.  */
  TV_DF_RI                    /* Timing variable.  */ 
};


/* Create a new DATAFLOW instance and add it to an existing instance
   of DF.  The returned structure is what is used to get at the
   solution.  */

void
df_ri_add_problem (enum df_ri_flags flags)
{
  df_add_problem (&problem_RI);
  df_ri->local_flags = (unsigned int)flags;
}


/* Return a bitmap containing the set of registers that cross a setjmp.  
   The client should not change or delete this bitmap.  */

bitmap
df_ri_get_setjmp_crosses (void)
{
  struct df_ri_problem_data *problem_data =
    (struct df_ri_problem_data *) df_ri->problem_data;

  return problem_data->setjmp_crosses;
}

#undef df_ri_problem_p
