/* Instruction scheduling pass.
   Copyright (C) 2006, 2007 Free Software Foundation, Inc.

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


/* FIXME: check whether we need all these headers, and check the makefile.  */
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "toplev.h"
#include "rtl.h"
#include "tm_p.h"
#include "hard-reg-set.h"
#include "regs.h"
#include "function.h"
#include "flags.h"
#include "insn-config.h"
#include "insn-attr.h"
#include "except.h"
#include "toplev.h"
#include "recog.h"
#include "cfglayout.h"
#include "params.h"
#include "target.h"
#include "output.h"
#include "timevar.h"
#include "tree-pass.h"
#include "sched-rgn.h"
#include "sched-int.h"
#include "sched-deps.h"
#include "cselib.h"
#include "ggc.h"
#include "tree.h"
#include "vec.h"
#include "langhooks.h"
#include "rtlhooks-def.h"

#ifdef INSN_SCHEDULING
#include "sel-sched-ir.h"
#include "sel-sched-dump.h"
#include "sel-sched.h"

/* FIXME: extend this comment.

   Implementation of selective scheduling approach.
   References:
   S.-M. Moon and K. Ebcioglu. Parallelizing nonnumerical code with
   selective scheduling and software pipelining. 
   ACM TOPLAS, Vol 19, No. 6, pages 853--898, Nov. 1997.  */

/* Software lookahead window size.
   According to the results in Nakatani and Ebcioglu [1993], window size of 16 
   is enough to extract most ILP in integer code.  */
#define MAX_WS (PARAM_VALUE (PARAM_SELSCHED_MAX_LOOKAHEAD))

/* True when moveup_set_path functionality should be used.  */
bool enable_moveup_set_path_p;

/* True when separable insns should be scheduled as RHSes.  */
bool enable_schedule_as_rhs_p;

/* True when pipelining is enabled.  */
bool pipelining_p;

/* True if bookkeeping is enabled.  */
bool bookkeeping_p;

/* True if we should make an aditional pass to set somewhat correct
   sched cycles.  */
bool reset_sched_cycles_p;


/* Definitions of local types and macros.  */

/* Represents possible outcomes of moving an expression through an insn.  */
enum MOVEUP_RHS_CODE { MOVEUP_RHS_SAME, MOVEUP_RHS_NULL, MOVEUP_RHS_CHANGED };

/* The container to be passed into rtx search & replace functions.  */
struct rtx_search_arg
{
  /* What we are searching for.  */
  rtx x;

  /* With what X will be replaced.  */
  rtx y;

  /* The occurence counter.  */
  int n;

  /* Stop after finding MAX_OCCUR occurences, if it is positive.  */
  int max_occur;

  /* Iterator of the av_set to remove rhs from.  */
  av_set_iterator *iter;

  /* Bitmask according to which substitutions are performed.  */
  unsigned long bitmask;
};
typedef struct rtx_search_arg *rtx_search_arg_p;

/* This descibes the data given to sel_sched_region_2.  */
struct sel_sched_region_2_data_def
{  
  int orig_max_seqno;
  int highest_seqno_in_use;
};
typedef struct sel_sched_region_2_data_def *sel_sched_region_2_data_t;


/* True if/when we want to emulate Haifa scheduler in the common code.  
   This is used in sched_rgn_local_init and in various places in 
   sched-deps.c.  */
int sched_emulate_haifa_p;

/* GLOBAL_LEVEL is used to discard information stored in basic block headers
   av_sets.  Av_set of bb header is valid if its (bb header's) level is equal
   to GLOBAL_LEVEL.  And invalid if lesser.  This is primarily used to advance
   scheduling window.  */
int global_level;

/* Current fences.  */
flist_t fences;

/* !!! Remove me: not used.
   Stage of the loop pipelining process.  */
static int stage;

/* All newly emitted insns will have their uids greater than this value.  */
static int first_emitted_uid;

/* Set of basic blocks that are forced to start new ebbs.  This is a subset
   of all the ebb heads.  */
static bitmap_head _forced_ebb_heads;
bitmap_head *forced_ebb_heads = &_forced_ebb_heads;

/* True when the first lv set should be ignored when updating liveness.  */
static bool ignore_first = false;

/* Number of insns max_issue has initialized data structures for.  */
static int max_issue_size = 0;

/* Whether do we need a stall currently.  */
static int need_stall = 0;

/* Whether we can issue more instructions.  */
static int can_issue_more;

/* A vector of expressions is used to be able to sort them.  */
DEF_VEC_P(rhs_t);
DEF_VEC_ALLOC_P(rhs_t,heap);
static VEC(rhs_t, heap) *vec_av_set = NULL;

/* This shows how many times the scheduler was run.  */
static int sel_sched_region_run = 0;


basic_block (*old_create_basic_block) (void *, void *, basic_block);
static void (*old_delete_basic_block) (basic_block);

/* Forward declarations of static functions.  */
static bool rtx_search (rtx, rtx);
static int sel_rank_for_schedule (const void *, const void *);
static bool equal_after_moveup_path_p (rhs_t, ilist_t, rhs_t);
static regset compute_live (insn_t);
static basic_block generate_bookkeeping_insn (rhs_t, insn_t, edge, edge);
static bool find_used_regs (insn_t, av_set_t, regset, HARD_REG_SET *, 
                            def_list_t *);
static bool move_op (insn_t, av_set_t, ilist_t, edge, edge, rhs_t);
static void sel_sched_region_1 (void);
static void sel_sched_region_2 (sel_sched_region_2_data_t);


/* Functions that work with fences.  */

/* Advance one cycle on FENCE.  */
static void
advance_one_cycle (fence_t fence)
{
  advance_state (FENCE_STATE (fence));
  FENCE_CYCLE (fence)++;
  FENCE_ISSUED_INSNS (fence) = 0;
  FENCE_STARTS_CYCLE_P (fence) = 1;
  can_issue_more = issue_rate;
  print ("Finished a cycle.  Current cycle = %d", FENCE_CYCLE (fence));
}

/* Construct successor fences from FENCE and put them in NEW_FENCES. 
   When a successor will continue a ebb, transfer all FENCE's parameters
   to the new fence.  */
static void
extract_new_fences_from (fence_t fence, flist_tail_t new_fences,
			 sel_sched_region_2_data_t data)
{
  int orig_max_seqno = data->orig_max_seqno;
  bool was_here_p = false;
  insn_t insn = NULL_RTX;
  ilist_iterator ii;

  /* Get the only element of FENCE_BNDS (fence).  */
  FOR_EACH_INSN (insn, ii, FENCE_BNDS (fence))
    {
      gcc_assert (!was_here_p);
      was_here_p = true;
    }

  gcc_assert (was_here_p && insn != NULL_RTX);

  if (was_here_p /* true */)
    {
      insn_t succ;
      succ_iterator si;

      FOR_EACH_SUCC_1 (succ, si, insn, SUCCS_NORMAL | SUCCS_SKIP_TO_LOOP_EXITS)
	{
	  int seqno = INSN_SEQNO (succ);

	  if (0 < seqno && seqno <= orig_max_seqno
	      && (pipelining_p || INSN_SCHED_CYCLE (succ) <= 0))
	    {
	      bool b = in_same_ebb_p (insn, succ);

	      print ("%d[%d] (state %s); ", INSN_UID (succ),
		     BLOCK_NUM (succ), b ? "continue" : "reset");

	      if (b)
		new_fences_add_dirty (new_fences, succ, fence);
	      else
		{
		  /* Mark block of the SUCC as head of the new ebb.  */
		  bitmap_set_bit (forced_ebb_heads, BLOCK_NUM (succ));
		  new_fences_add_clean (new_fences, succ, fence);
		}
	    }
	}
    }
}


/* Functions to support substitution.  */

/* Substitute all occurences of INSN's destination in RHS' vinsn with INSN's 
   source (if INSN is eligible for substitution).  Returns TRUE if
   substitution was actually performed, FALSE otherwise.  Substitution might
   be not performed because it's either RHS' vinsn doesn't contain INSN's
   destination or the resulting insn is invalid for the target machine.  */
static bool
substitute_reg_in_rhs (rhs_t rhs, insn_t insn)
{
  rtx *where;
  bool new_insn_valid;
  vinsn_t *vi = &RHS_VINSN (rhs);

  if (VINSN_UNIQUE_P (*vi))
    /* Unique vinsns can't play this game.  */
    return false;

  /* Do not try to replace in SET_DEST.  Although we'll choose new
     register for the RHS, we don't want to change RHS' original reg.  
     If the insn is not SET, we may still be able to substitute something
     in it, and if we're here (don't have deps), it doesn't write INSN's 
     dest.  */
  where = (VINSN_SEPARABLE_P (*vi)
	   ? &VINSN_RHS (*vi)
	   : &PATTERN (VINSN_INSN (*vi)));

  /* Substitute if INSN has a form of x:=y and LHS(INSN) occurs in *VI.  */
  if (insn_eligible_for_subst_p (insn) && rtx_search (INSN_LHS (insn), *where))
    {
      rtx new_insn;
      rtx *where_replace;

      /* We should copy these rtxes before substitution.  */
      new_insn = create_copy_of_insn_rtx (VINSN_INSN_RTX (*vi));

      /* Where we'll replace.  */
      /* WHERE_REPLACE should point inside NEW_INSN, so INSN_RHS couldn't be
	 used instead of SET_SRC.  */
      where_replace = (VINSN_SEPARABLE_P (*vi)
		       ? &SET_SRC (PATTERN (new_insn))
		       : &PATTERN (new_insn));

      new_insn_valid = validate_replace_rtx_part (INSN_LHS (insn),
						  copy_rtx (INSN_RHS (insn)),
						  where_replace, new_insn);

      /* ??? Actually, constrain_operands result depends upon choice of
         destination register.  E.g. if we allow single register to be an rhs,
	 and if we try to move dx=ax(as rhs) through ax=dx, we'll result 
	 in invalid insn dx=dx, so we'll loose this rhs here.
	 Just can't come up with significant testcase for this, so just
	 leaving it for now.  */
      if (new_insn_valid)
	{
	  if (!insn_rtx_valid (new_insn))
	    gcc_unreachable ();

	  change_vinsn_in_expr (rhs, create_vinsn_from_insn_rtx (new_insn));

	  return true;
	}
      else
	{
	  /* It would nice to free new_insn or return it to some pool.  */
	  return false;
	}
    }
  else
    return false;
}

/* Helper function for replace_in_vinsn_using_bitmask.  See comment to that
   function for the details.  */
static int 
replace_in_vinsn_using_bitmask_1 (rtx *cur_rtx, void *arg)
{
  struct rtx_search_arg *p = (struct rtx_search_arg *) arg;

  /* If matched...  */
  if (exp_equiv_p (*cur_rtx, p->x, 0, true))
    {
      /* Check whether we should replace it according to bitmask.  */
      if (p->bitmask & 1)
        {
	  /* Replace.  */
	  (*cur_rtx) = copy_rtx (p->y);
        }
      /* Advance to next bit.  */
      p->bitmask = p->bitmask >> 1;

      /* Finish search if no more occurences to replace, or continue search
	 (not traversing subexprs of *CUR_RTX). */
      if (!p->bitmask)
	return 1;
      else
	return -1;
    }
  return 0;
}

/* Performs the substitution according to a BITMASK: every "one" with 
   index K in bitmap mask means that the K-th occurence of WHAT will be
   replaced by BY_WHAT in WHERE, "0" means that it won't be changed.  */
static void
replace_in_rtx_using_bitmask (rtx *wherep, rtx what, rtx by_what, 
			      unsigned long bitmask)
{
  struct rtx_search_arg arg;

  gcc_assert (bitmask);

  arg.x = what;
  arg.y = by_what;
  arg.bitmask = bitmask;

  for_each_rtx (wherep,
		&replace_in_vinsn_using_bitmask_1, (void *) &arg);

  /* Make sure all occurences has been replaced.  */
  gcc_assert (!arg.bitmask);
}

/* Helper function for count_occurences_equiv.  */
static int 
count_occurrences_1 (rtx *cur_rtx, void *arg)
{
  rtx_search_arg_p p = (rtx_search_arg_p) arg;

  /* The last param FOR_GCSE is true, because otherwise it performs excessive
    substitutions like
	r8 = r33
	r16 = r33
    for the last insn it presumes r33 equivalent to r8, so it changes it to
    r33.  Actually, there's no change, but it spoils debugging.  */
  if (exp_equiv_p (*cur_rtx, p->x, 0, true))
    {
      p->n++;

      /* Stop search, if we've already found the requested number of
	 occurences.  */
      if (p->max_occur > 0 && p->n >= p->max_occur)
	return 1;

      /* Do not traverse subexprs.  */
      return -1;
    }

  if (GET_CODE (*cur_rtx) == SUBREG
      && REG_P (p->x)
      && REGNO (SUBREG_REG (*cur_rtx)) == REGNO (p->x))
    {
      /* ??? Do not support substituting regs inside subregs.  In that case,
         simplify_subreg will be called by validate_replace_rtx, and 
         unsubstitution will fail later.  */
      p->n = 0;
      p->max_occur = 0;
      return -1;
    }

  /* Continue search.  */
  return 0;
}

/* Return the number of places WHAT appears within WHERE.  Find no more
   than MAX_OCCUR occurences.  This function is analogous to
   count_occurrences but it counts not only subexprs that have 
   equal pointers, but also those for which exp_equiv_p is true.  */
static int 
count_occurrences_equiv (rtx what, rtx where, int max_occur)
{
  struct rtx_search_arg arg;

  arg.x = what;
  arg.n = 0;
  arg.max_occur = max_occur;

  for_each_rtx (&where, &count_occurrences_1, (void *) &arg);

  return arg.n;
}

/* Returns TRUE if WHAT is found in WHERE rtx tree.  */
static bool
rtx_search (rtx what, rtx where)
{
  return (count_occurrences_equiv (what, where, 1) > 0);
}

/* if INSN is a copy x:=y and if there is an rhs r in AV that uses y, 
   it adds all variants of r to AV derived by replacing one or more
   occurrences of y by x.  */
static void
un_substitute (rhs_t rhs, rtx insn, av_set_t *new_set_ptr)
{
  rtx pat, src_reg, dst_reg;
  vinsn_t vi;
  rtx where_count;
  unsigned n_occur;
  unsigned long bitmask;
  
  if (!flag_sel_sched_substitution)
    return;

  /* Catch X := Y insns, where X and Y are regs.  Otherwise return.  */
  if (!insn_eligible_for_subst_p (insn))
    return;

  pat = PATTERN (insn);
  src_reg = SET_SRC (pat);
  dst_reg = SET_DEST (pat);
  vi = RHS_VINSN (rhs);

  where_count = (VINSN_SEPARABLE_P (vi) 
                 ? VINSN_RHS (vi) : VINSN_PATTERN (vi));

  n_occur = count_occurrences_equiv (src_reg, where_count, 0);

  /* Try next rhs, if no occurences.  */
  if (!n_occur)
    return;
  
  /* Make sure we have enough bits to handle all substitutions.  */
  gcc_assert (n_occur < sizeof (unsigned long) * 8);

  /* Generate the replacement mask and perform the substitution
     according to it.  Every "one" with index K in bitmap mask means 
     we change the K-th occurence of SRC_REG with DST_REG, "0" means we
     skip it.  We already have the replacement configuration for 
     bitmask == 0 in the original set.  */	 
  for (bitmask = 1; bitmask < ((unsigned long)1 << n_occur); bitmask++)
    {
      expr_def _tmp_rhs, *tmp_rhs = &_tmp_rhs;
      rtx new_insn;
      vinsn_t new_vi;
      rtx *wherep;

      new_insn = create_copy_of_insn_rtx (VINSN_INSN (vi));

      wherep = (VINSN_SEPARABLE_P (vi)
                ? &SET_SRC (PATTERN (new_insn))
                : &PATTERN (new_insn));
      
      replace_in_rtx_using_bitmask (wherep, src_reg, dst_reg, bitmask);
      
      if (!insn_rtx_valid (new_insn))
        continue;
      
      new_vi = create_vinsn_from_insn_rtx (new_insn);

      gcc_assert (VINSN_SEPARABLE_P (new_vi) == EXPR_SEPARABLE_P (rhs));
      
      copy_expr (tmp_rhs, rhs);
      change_vinsn_in_expr (tmp_rhs, vi);
      
      av_set_add (new_set_ptr, tmp_rhs);
      
      clear_expr (tmp_rhs);
    }
}


/* Functions to support register renaming.  */

/* Substitute VI's set source with REGNO.  Returns newly created pattern
   that has REGNO as its source.  */
static rtx
create_insn_rtx_with_rhs (vinsn_t vi, rtx rhs_rtx)
{
  rtx lhs_rtx;
  rtx pattern;
  rtx insn_rtx;

  lhs_rtx = copy_rtx (VINSN_LHS (vi));

  pattern = gen_rtx_SET (VOIDmode, lhs_rtx, rhs_rtx);
  insn_rtx = create_insn_rtx_from_pattern (pattern);

  return insn_rtx;
}

/* Returns whether INSN's src can be replaced with register number 
   NEW_SRC_REG. E.g. the following insn is valid for i386:

    (insn:HI 2205 6585 2207 727 ../../gcc/libiberty/regex.c:3337 
      (set (mem/s:QI (plus:SI (plus:SI (reg/f:SI 7 sp)
			(reg:SI 0 ax [orig:770 c1 ] [770]))
		    (const_int 288 [0x120])) [0 str S1 A8])
	    (const_int 0 [0x0])) 43 {*movqi_1} (nil)
	(nil))

  But if we change (const_int 0 [0x0]) to (reg:QI 4 si), it will be invalid
  because of operand constraints: 

    (define_insn "*movqi_1"
      [(set (match_operand:QI 0 "nonimmediate_operand" "=q,q ,q ,r,r ,?r,m")
	    (match_operand:QI 1 "general_operand"      " q,qn,qm,q,rn,qm,qn")
	    )]
    
  So do constrain_operands here, before choosing NEW_SRC_REG as best 
  reg for rhs.  */

static bool
replace_src_with_reg_ok_p (insn_t insn, rtx new_src_reg)
{
  vinsn_t vi = INSN_VI (insn);
  enum machine_mode mode;
  rtx dst_loc;
  bool res;

  gcc_assert (VINSN_SEPARABLE_P (vi));

  get_dest_and_mode (insn, &dst_loc, &mode);
  gcc_assert (mode == GET_MODE (new_src_reg));

  if (REG_P (dst_loc) && REGNO (new_src_reg) == REGNO (dst_loc))
    return true;

  /* See whether SET_SRC can be replaced with this register.  */
  validate_change (insn, &SET_SRC (PATTERN (insn)), new_src_reg, 1);
  res = verify_changes (0);
  cancel_changes (0);

  return res;
}

/* Returns whether INSN still be valid after replacing it's DEST with
   register NEW_REG.  */
static bool
replace_dest_with_reg_ok_p (insn_t insn, rtx new_reg)
{
  vinsn_t vi = INSN_VI (insn);
  bool res;

  /* We should deal here only with separable insns.  */
  gcc_assert (VINSN_SEPARABLE_P (vi));
  gcc_assert (GET_MODE (VINSN_LHS (vi)) == GET_MODE (new_reg));

  /* See whether SET_DEST can be replaced with this register.  */
  validate_change (insn, &SET_DEST (PATTERN (insn)), new_reg, 1);
  res = verify_changes (0);
  cancel_changes (0);

  return res;
}

/* Create a pattern with rhs of VI and lhs of LHS_RTX.  */
static rtx
create_insn_rtx_with_lhs (vinsn_t vi, rtx lhs_rtx)
{
  rtx rhs_rtx;
  rtx pattern;
  rtx insn_rtx;

  rhs_rtx = copy_rtx (VINSN_RHS (vi));

  pattern = gen_rtx_SET (VOIDmode, lhs_rtx, rhs_rtx);
  insn_rtx = create_insn_rtx_from_pattern (pattern);

  return insn_rtx;
}

/* Substitute lhs in the given expression RHS for the register with number 
   NEW_REGNO.  SET_DEST may be arbitrary rtx, not only register.  */
static void
replace_dest_with_reg_in_rhs (rhs_t rhs, rtx new_reg)
{
  rtx insn_rtx;
  vinsn_t vinsn;

  insn_rtx = create_insn_rtx_with_lhs (RHS_VINSN (rhs), new_reg);
  vinsn = create_vinsn_from_insn_rtx (insn_rtx);

  change_vinsn_in_expr (rhs, vinsn);
}

/* Return a destination register, if any, of EXPR.  */
static rtx
expr_dest_reg (expr_t expr)
{
  rtx dest = VINSN_LHS (RHS_VINSN (expr));

  if (dest != NULL_RTX && REG_P (dest))
    return dest;

  return NULL_RTX;
}

/* Returns the REGNO of the R's destination.  */
static unsigned
rhs_dest_regno (rhs_t r)
{
  rtx dest = expr_dest_reg (r);

  gcc_assert (dest != NULL_RTX);
  return REGNO (dest);
}

/* Returns whether VI writes one of the REGS.  */
static bool
vinsn_writes_one_of_regs_p (vinsn_t vi, regset used_regs, 
                            HARD_REG_SET unavailable_hard_regs)
{
  unsigned regno;
  reg_set_iterator rsi;

  EXECUTE_IF_SET_IN_REG_SET (VINSN_REG_SETS (vi), 0, regno, rsi)
    {
      if (REGNO_REG_SET_P (used_regs, regno))
        return true;
      if (HARD_REGISTER_NUM_P (regno)
          && TEST_HARD_REG_BIT (unavailable_hard_regs, regno))
	return true;
    }

  return false;
}

/* Returns register class of the output register in INSN.  
   Returns NO_REGS for call insns because some targets have constraints on
   destination register of a call insn.
   
   Code adopted from regrename.c::build_def_use.  */
static enum reg_class
get_reg_class (rtx insn)
{
  int alt, i, n_ops;
  int predicated;

  extract_insn (insn);
  if (! constrain_operands (1))
    fatal_insn_not_found (insn);
  preprocess_constraints ();
  alt = which_alternative;
  n_ops = recog_data.n_operands;

  predicated = GET_CODE (PATTERN (insn)) == COND_EXEC;
  for (i = 0; i < n_ops; ++i)
    {
      int matches = recog_op_alt[i][alt].matches;
      if (matches >= 0)
	recog_op_alt[i][alt].cl = recog_op_alt[matches][alt].cl;
    }

  if (asm_noperands (PATTERN (insn)) > 0)
    {
      for (i = 0; i < n_ops; i++)
	if (recog_data.operand_type[i] == OP_OUT)
	  {
	    rtx *loc = recog_data.operand_loc[i];
	    rtx op = *loc;
	    enum reg_class cl = recog_op_alt[i][alt].cl;

	    if (REG_P (op)
		&& REGNO (op) == ORIGINAL_REGNO (op))
	      continue;

	    return cl;
	  }
    }
  else if (!CALL_P (insn))
    {
      for (i = 0; i < n_ops + recog_data.n_dups; i++)
       {
	 int opn = i < n_ops ? i : recog_data.dup_num[i - n_ops];
	 enum reg_class cl = recog_op_alt[opn][alt].cl;
  
	 if (recog_data.operand_type[opn] == OP_OUT ||
	     recog_data.operand_type[opn] == OP_INOUT)
	   return cl;
       }
    }

/*  Insns like
    (insn (set (reg:CCZ 17 flags) (compare:CCZ ...)))
    may result in returning NO_REGS, cause flags is written implicitly through
    CMP insn, which has no OP_OUT | OP_INOUT operands.  */
  return NO_REGS;
}

/* Mark hardware regs in UNAVAILABLE_HARD_REGS that are not suitable 
   for renaming rhs in INSN due to hardware restrictions (register class,
   modes compatibility etc).  This doesn't affect original insn's dest reg,
   if it isn't in USED_REGS.  DEF is a definition insn of rhs for which the
   destination register is sought.  LHS (DEF->ORIG_INSN) may be REG or MEM.
   Registers that are in used_regs are always marked in
   unavailable_hard_regs as well.  */

static void
mark_unavailable_hard_regs (def_t def, HARD_REG_SET *unavailable_hard_regs, 
                            regset used_regs)
{
  enum machine_mode mode;
  enum reg_class cl = NO_REGS;
  rtx orig_dest;
  int cur_reg;
  HARD_REG_SET hard_regs_ok;

  gcc_assert (GET_CODE (PATTERN (def->orig_insn)) == SET);
  gcc_assert (unavailable_hard_regs);

  orig_dest = SET_DEST (PATTERN (def->orig_insn));

  /* If before reload, don't try to work with pseudos.  */
  if (!reload_completed && REGNO (orig_dest) >= FIRST_PSEUDO_REGISTER)
    return;

  mode = GET_MODE (orig_dest);
  gcc_assert (orig_dest != pc_rtx);

  /* Can't proceed with renaming if the original register
     is one of the fixed_regs, global_regs or frame pointer.  */
  if (REG_P (orig_dest) && (fixed_regs[REGNO (orig_dest)] 
			  || global_regs[REGNO (orig_dest)]
#if FRAME_POINTER_REGNUM != HARD_FRAME_POINTER_REGNUM
	|| (frame_pointer_needed 
	    && REGNO (orig_dest) == HARD_FRAME_POINTER_REGNUM)
#else
	|| (frame_pointer_needed && REGNO (orig_dest) == 
	    FRAME_POINTER_REGNUM)
#endif
      ))
    {
      SET_HARD_REG_SET (*unavailable_hard_regs);

      /* Give a chance for original register, if it isn't in used_regs.  */
      if (REG_P (orig_dest) && !REGNO_REG_SET_P (used_regs, 
						 REGNO (orig_dest)))
	CLEAR_HARD_REG_BIT (*unavailable_hard_regs, REGNO (orig_dest));

      return;
    }

#ifdef STACK_REGS
  /* For the stack registers the presence of FIRST_STACK_REG in USED_REGS
     is equivalent to as if all stack regs were in this set.
     I.e. no stack register can be renamed, and even if it's an original
     register here we make sure it won't be lifted over it's previous def 
     (it's previous def will appear as if it's a FIRST_STACK_REG def.  
     The HARD_REGNO_RENAME_OK covers other cases in condition below.  */
  if (IN_RANGE (REGNO (orig_dest), FIRST_STACK_REG, LAST_STACK_REG)
      && REGNO_REG_SET_P (used_regs, FIRST_STACK_REG)) 
    {
      for (cur_reg = FIRST_STACK_REG; cur_reg <= LAST_STACK_REG; cur_reg++)
	SET_HARD_REG_BIT (*unavailable_hard_regs, cur_reg);
    }
#endif    

  /* If there's a call on this path, make regs from call_used_reg_set 
     unavailable.  */
  if (def->crosses_call)
    IOR_HARD_REG_SET (*unavailable_hard_regs, call_used_reg_set);

  /* Stop here before reload: we need FRAME_REGS, STACK_REGS, and crosses_call, 
     but not register classes.  */
  if (!reload_completed)
    return;

  /* Leave regs as 'available' only from the current 
     register class.  */
  if (REG_P (orig_dest))
    {
      cl = get_reg_class (def->orig_insn);
      gcc_assert (cl != NO_REGS);
      IOR_COMPL_HARD_REG_SET (*unavailable_hard_regs, reg_class_contents[cl]);
    }

  CLEAR_HARD_REG_SET (hard_regs_ok);

  /* No matter what kind of reg it is if it's original reg and it's available
     from liveness analysis (not present in used_regs set) - add it to
     the 'ok' registers so it bypass these restrictions.  
     But it still may appear in unavailable regs (see above).  */
  if (REG_P (orig_dest) && !REGNO_REG_SET_P (used_regs, REGNO (orig_dest)))
    SET_HARD_REG_BIT (hard_regs_ok, REGNO (orig_dest));

  for (cur_reg = 0; cur_reg < FIRST_PSEUDO_REGISTER; cur_reg++)
    {
      int nregs = hard_regno_nregs[cur_reg][mode];
      int i;

      gcc_assert (nregs > 0);

      for (i = nregs - 1; i >= 0; --i)
	{
	  if (
	      /* If this reg is in USED_REGS, set it also in 
		 UNAVAILABLE_HARD_REGS.  */
	      REGNO_REG_SET_P (used_regs, cur_reg + i)
	      || fixed_regs[cur_reg + i]
	      || global_regs[cur_reg + i]
	      /* Can't use regs which aren't saved by 
		 the prologue.  */
	      /* FIXME: throwing out this condition may reveal more
		 possibilities for renaming, but in this case we'll
		 need to set regs_ever_live for that reg and rewrite
		 the prologue.  Furthermore, when choosing best reg
		 among available for renaming we'll need to count
		 whether it adds a new reg to the function.  */
	      /* FIXME: ? this loop can be optimized 
		 if traversing union of regs_ever_live 
		 and call_used_regs?  */
	      || !REGNO_REG_SET_P (sel_all_regs, (cur_reg + i))
#ifdef LEAF_REGISTERS
	      /* We can't use a non-leaf register if we're in a
		 leaf function.  */
	      || (current_function_is_leaf
		  && !LEAF_REGISTERS[cur_reg + i])
#endif
#ifdef HARD_REGNO_RENAME_OK
	      /* Hardware specific registers that 
		 can't be clobbered.  Counts only if the dest
		 is a register, cause if it's a MEM, we always
		 can store it in register as well (with mode
		 restrictions).  */
	      || (REG_P (orig_dest)
		  && !HARD_REGNO_RENAME_OK (
		    REGNO (orig_dest)+ i, cur_reg + i))
#endif
	      )
	    break;
	}
      if (i >= 0) 
	continue;

      /* See whether it accepts all modes that occur in
	 original insns.  */
      if (! HARD_REGNO_MODE_OK (cur_reg, mode)
	  || (def->crosses_call
	      /* We shouldn't care about call clobberedness
		 unless the DST_LOC is a reg. 
		 Btw HARD_REGNO_CALL_PART_CLOBBERED defined
		 to 0 for most platforms including x86 & 
		 ia64.  */
	      && (REG_P (orig_dest)
		  && !(HARD_REGNO_CALL_PART_CLOBBERED
		     (REGNO (orig_dest), mode)))
	      && (HARD_REGNO_CALL_PART_CLOBBERED
		  (cur_reg, mode))))
	  continue;

      /* If the CUR_REG passed all the checks above, 
	 then it's ok.  */
      SET_HARD_REG_BIT (hard_regs_ok, cur_reg);
    }

  /* Exclude all hard regs but HARD_REGS_OK.  */
  IOR_COMPL_HARD_REG_SET (*unavailable_hard_regs, hard_regs_ok);
}

/* reg_rename_tick[REG1] > reg_rename_tick[REG2] if REG1 was chosen as the
   best register more recently than REG2.  */
static int reg_rename_tick[FIRST_PSEUDO_REGISTER];

/* Choose the register among free, that is suitable for storing 
   the rhs value.

   ORIGINAL_INSNS is the list of insns where the operation (rhs)
   originally appears.  There could be multiple original operations 
   for single rhs since we moving it up and merging along different 
   paths.

   Some code is adapted from regrename.c (regrename_optimize).
   If original register is available, function returns it.
   Otherwise it performs the checks, so the new register should
   comply with the following:
    - it should not be in the UNAVAILABLE set;
    - it should be in the class compatible with original uses;
    - it should not be clobbered through reference with different mode;
    - if we're in the leaf function, then the new register should 
      not be in the LEAF_REGISTERS;
    - etc.

   Most of this conditions are checked in find_used_regs_1, and
   unavailable registers due to this restrictions are already included
   in UNAVAILABLE set.

   If several registers meet the conditions, the register with smallest
   tick is returned to achieve more even register allocation.

   If no register satisfies the above conditions, NULL_RTX is returned.  */
static rtx
choose_best_reg_1 (HARD_REG_SET unavailable, def_list_t original_insns,
		   bool *is_orig_reg_p_ptr)
{
  int best_new_reg;
  int cur_reg;
  enum machine_mode mode = VOIDmode;
  def_list_iterator i;
  def_t def;

  /* Don't clobber traceback for noreturn functions.  */
  /* If something allocated on stack in this function, mark frame pointer
     register unavailable, considering also modes.  */
  if (frame_pointer_needed)
    {
      int i;

      for (i = hard_regno_nregs[FRAME_POINTER_REGNUM][Pmode]; i--;)
	SET_HARD_REG_BIT (unavailable, FRAME_POINTER_REGNUM + i);

#if FRAME_POINTER_REGNUM != HARD_FRAME_POINTER_REGNUM
      for (i = hard_regno_nregs[HARD_FRAME_POINTER_REGNUM][Pmode]; i--;)
	SET_HARD_REG_BIT (unavailable, HARD_FRAME_POINTER_REGNUM + i);
#endif
    }

  /* If original register is available, return it.  */
  *is_orig_reg_p_ptr = true;

  FOR_EACH_DEF (def, i, original_insns)
    {
      rtx orig_dest = SET_DEST (PATTERN (def->orig_insn));

      gcc_assert (REG_P (orig_dest));

      /* Check that all original operations have the same mode.  
         This is done for the next loop; if we'd return from this
         loop, we'd check only part of them, but in this case 
         it doesn't matter.  */
      if (mode == VOIDmode)
        mode = GET_MODE (orig_dest);
      gcc_assert (mode == GET_MODE (orig_dest));

      if (!TEST_HARD_REG_BIT (unavailable, REGNO (orig_dest)))
        {
          gcc_assert (mode != VOIDmode);

          /* Hard registers should not be shared.  */
	  return gen_rtx_REG (mode, REGNO (orig_dest));
        }
    }

  *is_orig_reg_p_ptr = false;

  best_new_reg = -1;

  /* Among all available regs choose the register that was 
     allocated earliest.  */
  for (cur_reg = 0; cur_reg < FIRST_PSEUDO_REGISTER; cur_reg++)
    if (!TEST_HARD_REG_BIT (unavailable, cur_reg))
      {
	if (best_new_reg < 0
	    ||reg_rename_tick[cur_reg] < reg_rename_tick[best_new_reg])
	  best_new_reg = cur_reg;
      }

  if (best_new_reg >= 0)
    {
      /* Use the check from the above loop.  */
      gcc_assert (mode != VOIDmode);
      return gen_rtx_REG (mode, best_new_reg);
    }

  return NULL_RTX;
}

/* A regset that includes all registers that present in current region.
   Used in assertions to check that we don't introduce new registers when
   should not.  */
static regset_head _sel_all_regs;
regset sel_all_regs = &_sel_all_regs;

/* A wrapper around choose_best_reg_1 () to verify that we make correct
   assumptions about available registers in the function.  */
static rtx
choose_best_reg (HARD_REG_SET unavailable, def_list_t original_insns,
		 bool *is_orig_reg_p_ptr)
{
  rtx best_reg = choose_best_reg_1 (unavailable, original_insns,
				    is_orig_reg_p_ptr);

  gcc_assert (best_reg == NULL_RTX
	      || REGNO_REG_SET_P (sel_all_regs, REGNO (best_reg)));

  return best_reg;
}

/* Choose the pseudo register for storing rhs value.  As this is supposed 
   to work before reload, we return either the original register or make
   the new one.  If we work with hard regs, check also UNAVAILABLE_HARD_REGS.

   TODO: take into account register pressure while doing this.  Up to this 
   moment, this function would never return NULL for pseudos, but we should 
   not rely on this.  */
static rtx
choose_best_pseudo_reg (regset used_regs, 
                        HARD_REG_SET unavailable_hard_regs, 
                        def_list_t original_insns, bool *is_orig_reg_p_ptr)
{
  def_list_iterator i;
  def_t def;
  enum machine_mode mode = VOIDmode;
  bool bad_hard_regs = false;
  
  /* We should not use this after reload.  */
  gcc_assert (!reload_completed);

  /* If original register is available, return it.  */
  *is_orig_reg_p_ptr = true;

  FOR_EACH_DEF (def, i, original_insns)
    {
      rtx dest = SET_DEST (PATTERN (def->orig_insn));
      int orig_regno;
      
      gcc_assert (REG_P (dest));
      
      /* Check that all original operations have the same mode.  */
      if (mode == VOIDmode)
        mode = GET_MODE (dest);
      else
        gcc_assert (mode == GET_MODE (dest));
      orig_regno = REGNO (dest);
      
      if (!REGNO_REG_SET_P (used_regs, orig_regno))
        {
          if (orig_regno < FIRST_PSEUDO_REGISTER)
            {
              gcc_assert (regs_ever_live [orig_regno]);
              
              /* For hard registers, we have to check hardware imposed 
                 limitations (frame/stack registers, calls crossed).  */
              if (!TEST_HARD_REG_BIT (unavailable_hard_regs, orig_regno))
                return gen_rtx_REG (mode, orig_regno);
              
              bad_hard_regs = true;
            }
          else
            return dest;
        }
     }

  *is_orig_reg_p_ptr = false;
 
  /* We had some original hard registers that couldn't be used.
     Those were likely special.  Don't try to create a pseudo.  */
  if (bad_hard_regs)
    return NULL_RTX;
  
  /* We haven't found a register from original operations.  Get a new one.  
     FIXME: control register pressure somehow.  */
  gcc_assert (mode != VOIDmode);
  return gen_reg_rtx (mode);
}

/* Returns best register for given rhs, or NULL_RTX, if no register can be
   chosen.  The latter could happen when:
     - RHS_SCHEDULE_AS_RHS is true but we were unable to find suitable
       register;
     - RHS_SCHEDULE_AS_RHS is false but the insn sets/clobbers one of
       the registers that are used on the moving path.  */
static bool
find_best_reg_for_rhs (rhs_t rhs, blist_t bnds)
{
  av_set_iterator i2;
  rhs_t rhs_orig;
  regset used_regs;
  HARD_REG_SET hard_regs_used, unavailable_hard_regs;
  rtx best_reg = NULL_RTX;
  blist_t bnds1 = bnds;
  def_list_t original_insns = NULL;
  int res = 0;
  bool reg_ok = true;
  bool is_orig_reg_p = false;

  used_regs = get_clear_regset_from_pool ();
  CLEAR_HARD_REG_SET (unavailable_hard_regs);

  /* Collect unavailable registers from all boundaries into USED_REGS.  */
  do
    {
      av_set_t orig_ops = NULL;
      bnd_t bnd = BLIST_BND (bnds1);

      /* If the chosen best rhs doesn't belong to current boundary, 
         skip it.  */
      if (!av_set_is_in_p (BND_AV1 (bnd), RHS_VINSN (rhs)))
	continue;

      /* Put in ORIG_OPS all rhses from this boundary that became
	 RES on top.  */
      FOR_EACH_RHS (rhs_orig, i2, BND_AV (bnd))
	{
          ilist_t root = enable_moveup_set_path_p ? BND_PTR (bnd) : NULL;

	  if (equal_after_moveup_path_p (rhs_orig, root, rhs))
	    av_set_add (&orig_ops, rhs_orig);
	}

      /* Compute used regs and OR it into the USED_REGS.  */
      res = find_used_regs (BND_TO (bnd), orig_ops, used_regs, 
                            &unavailable_hard_regs, &original_insns);

      av_set_clear (&orig_ops);

      if (res)
	{
          if (false)
            /* FIXME: conditionalize dumping of regsets.  */
	    {
	      dump_hard_reg_set ("unavailable hard regs: ",
				 unavailable_hard_regs);
	      print ("unavailable regs (live issues): ");
	      dump_used_regs (used_regs);
	    }
	}
      else
	{
	  gcc_unreachable ();
	  print ("Unable to find original op with find_used_regs.");
	  break;
	}
    }
  while ((bnds1 = BLIST_NEXT (bnds1)));

  if (res)
    {
#ifdef ENABLE_CHECKING
      /* If after reload, make sure we're working with hard regs here.  */
      if (reload_completed) {
	reg_set_iterator rsi;
	unsigned i;
	EXECUTE_IF_SET_IN_REG_SET (used_regs, FIRST_PSEUDO_REGISTER, i, rsi)
	  gcc_unreachable ();
      }
#endif

      if (EXPR_SEPARABLE_P (rhs))
	{
          /* Turn everything in hard regs after reload.  */
          if (reload_completed)
            {
              REG_SET_TO_HARD_REG_SET (hard_regs_used, used_regs);
              /* Join hard registers unavailable due to register class 
                 restrictions and live range intersection.  */
              IOR_HARD_REG_SET (hard_regs_used, unavailable_hard_regs);
              best_reg = choose_best_reg (hard_regs_used, original_insns,
					  &is_orig_reg_p);
            }
          else
            best_reg = choose_best_pseudo_reg (used_regs, 
                                               unavailable_hard_regs, 
                                               original_insns,
					       &is_orig_reg_p);

	  if (!is_orig_reg_p && sel_vinsn_cost (EXPR_VINSN (rhs)) < 2)
	    best_reg = NULL_RTX;

	  if (best_reg != NULL_RTX)
	    /* Try whether we'll be able to generate the insn
	       'dest := best_reg' at the place of the original operation.  */
	    {
	      ilist_t p = original_insns;
    
	      gcc_assert (original_insns);
    
	      while (p) 
		{
		  def_t def = DEF_LIST_DEF (p);
    
		  gcc_assert (EXPR_SEPARABLE_P (INSN_EXPR (def->orig_insn)));
    
		  if (!replace_src_with_reg_ok_p (def->orig_insn, best_reg)
		      || !replace_dest_with_reg_ok_p (def->orig_insn,
						      best_reg))
		    {
		      /* Insn will be removed from rhs_vliw below.  */
		      /* FIXME: may be it will work with other regs?
			 Not with example above, though - we can't figure
			 that the only option is to generate subreg.  */
		      reg_ok = false;
		      break;
		    }

		  p = ILIST_NEXT (p);
		}		
		
	      if (reg_ok)
		{
		  RGN_HAS_RENAMING_P (CONTAINING_RGN (BB_TO_BLOCK (0))) = 1;

		  /* Make sure that RHS has the right destination
		     register.  */
		  if (rhs_dest_regno (rhs) != REGNO (best_reg))
		    {
		      replace_dest_with_reg_in_rhs (rhs, best_reg);

		      /* The resulting insn should be valid.  */
		      if (!insn_rtx_valid (RHS_INSN (rhs)))
			gcc_unreachable ();
		    }
		}
	    }
	  else
	    reg_ok = false;
    	}
      else
	{
	  /* If !RHS_SCHEDULE_AS_RHS (RHS), just make sure INSN doesn't set
	     any of the HARD_REGS_USED set.  */
	  if (vinsn_writes_one_of_regs_p (RHS_VINSN (rhs), used_regs,
                                          unavailable_hard_regs))
	    reg_ok = false;
	  else
	    {
	      gcc_assert (reg_ok);
	      best_reg = NULL_RTX;
	    }
	}
    }
  else
    reg_ok = false;

  ilist_clear (&original_insns);
  return_regset_to_pool (used_regs);

  return reg_ok;
}


/* Flag to enable / disable ia64 speculation.  */
static bool sel_speculation_p = true;

/* Return true if dependence described by DS can be overcomed.  */
static bool
can_overcome_dep_p (ds_t ds)
{
  if (spec_info == NULL || !sel_speculation_p)
    return false;

  /* Leave only speculative data.  */
  ds &= SPECULATIVE;

  if (ds == 0)
    return false;

  {
    /* !!! FIXME:
       1. Make sched-deps.c produce only those non-hard dependencies,
       that we can overcome.
       2. Introduce several machine flags to turn on/off different kinds of
       speculation.  */
    ds_t spec_mask = spec_info->mask;

    if ((ds & spec_mask) != ds)
      return false;
  }

  if (ds_weak (ds) < spec_info->weakness_cutoff)
    return false;

  return true;
}

/* Get a speculation check instruction from the target.  SPEC_EXPR is a
   speculative expression.  */
static rtx
create_speculation_check_insn_rtx (rtx spec_insn_rtx, ds_t check_ds)
{
  rtx check_pattern;

  check_pattern = targetm.sched.gen_spec_check (spec_insn_rtx, NULL_RTX,
						check_ds);

  gcc_assert (check_pattern != NULL);

  return create_insn_rtx_from_pattern (check_pattern);
}

/* Try to transform EXPR to data speculative version.  Return true on
   success.  */
static bool
apply_spec_to_expr (expr_t expr, ds_t ds)
{
  int res;
  rtx orig_insn_rtx;
  rtx spec_pat;

  orig_insn_rtx = EXPR_INSN_RTX (expr);

  res = sched_speculate_insn (orig_insn_rtx, ds, &spec_pat);

  switch (res)
    {
    case 0:
      spec_pat = copy_rtx (PATTERN (orig_insn_rtx));

      /* FALLTHRU */

    case 1:
      {
	rtx spec_insn_rtx = create_insn_rtx_from_pattern (spec_pat);
	vinsn_t spec_vinsn = create_vinsn_from_insn_rtx (spec_insn_rtx);

	change_vinsn_in_expr (expr, spec_vinsn);
	EXPR_SPEC_DONE_DS (expr) = ds;

	return true;
      }

    case -1:
      return false;

    default:
      gcc_unreachable ();
      return false;
    }
}

/* Try to make EXPR speculative and on success remove from DSP those
   dependencies that were overcame.  Return true on success.  */
static bool
speculate_expr (expr_t expr, ds_t ds)
{
  ds_t target_ds = (ds & SPECULATIVE);
  ds_t current_ds = EXPR_SPEC_DONE_DS (expr);
  ds_t combined_ds = ds_full_merge (current_ds, target_ds, NULL_RTX, NULL_RTX);

  if (apply_spec_to_expr (expr, combined_ds))
    /* We already have all necessary speculations.  */
    return true;

  return false;
}

/* Return true if there is a speculative dependence between INSN and EXPR.  */
static ds_t
has_spec_dependence_p (expr_t expr, insn_t insn)
{
  ds_t *has_dep_p;
  ds_t full_ds;

  full_ds = has_dependence_p (expr, insn, &has_dep_p);

  if (full_ds == 0)
    return 0;

  if (EXPR_SEPARABLE_P (expr)
      && can_overcome_dep_p (has_dep_p [DEPS_IN_RHS]))
    return has_dep_p [DEPS_IN_RHS];

  if (can_overcome_dep_p (full_ds))
    return full_ds;

  return 0;
}

/* Add to AVP those exprs that might have been transformed to their speculative
   versions when moved through INSN.  */
static void
un_speculate (expr_t expr, insn_t insn, av_set_t *new_set_ptr)
{
  ds_t expr_spec_done_ds;
  ds_t full_ds;

  if (spec_info == NULL || !sel_speculation_p)
    return;
  
  expr_spec_done_ds = EXPR_SPEC_DONE_DS (expr);
  if (expr_spec_done_ds == 0)
    return;

  full_ds = has_spec_dependence_p (expr, insn);
  if (full_ds == 0)
    return;
  
  {
    expr_def _new_expr, *new_expr = &_new_expr;
    
    copy_expr (new_expr, expr);
    
    {
      bool b;
      
      full_ds = ds_get_speculation_types (full_ds);
      expr_spec_done_ds &= ~full_ds;
      
      b = apply_spec_to_expr (new_expr, expr_spec_done_ds);
      gcc_assert (b);
      
      EXPR_SPEC_TO_CHECK_DS (new_expr) |= full_ds;
    }
    
    av_set_add (new_set_ptr, new_expr);
    
    clear_expr (new_expr);
  }
}


/* Undo all transformations on *AV_PTR that were done when 
   moving through INSN.  */
static void
undo_transformations (av_set_t *av_ptr, rtx insn)
{
  av_set_iterator av_iter;
  rhs_t rhs;
  av_set_t new_set = NULL;

  FOR_EACH_RHS (rhs, av_iter, *av_ptr)
    {
      if (bitmap_bit_p (EXPR_CHANGED_ON_INSNS (rhs), INSN_LUID (insn)))
        un_speculate (rhs, insn, &new_set);
    }
  
  av_set_union_and_clear (av_ptr, &new_set);

  FOR_EACH_RHS (rhs, av_iter, *av_ptr)
    {
      if (bitmap_bit_p (EXPR_CHANGED_ON_INSNS (rhs), INSN_LUID (insn)))
        un_substitute (rhs, insn, &new_set);
    }
  
  av_set_union_and_clear (av_ptr, &new_set);
}


/* Moveup_* helpers for code motion and computing av sets.  */

/* Propagates INSN_TO_MOVE_UP inside an insn group through THROUGH_INSN.
   The difference from the below function is that only substitution is 
   performed.  */
static enum MOVEUP_RHS_CODE
moveup_rhs_inside_insn_group (rhs_t insn_to_move_up, insn_t through_insn)
{
  vinsn_t vi = RHS_VINSN (insn_to_move_up);
  insn_t insn = VINSN_INSN (vi);
  ds_t *has_dep_p;
  ds_t full_ds;

  full_ds = has_dependence_p (insn_to_move_up, through_insn, &has_dep_p);

  if (full_ds == 0)
    return MOVEUP_RHS_SAME;

  /* Substitution is the only cure in this case.  */
  if (has_dep_p[DEPS_IN_RHS])
    {
      /* Can't substitute UNIQUE VINSNs.  */
      gcc_assert (!VINSN_UNIQUE_P (vi));
      
      if (flag_sel_sched_substitution
          && insn_eligible_for_subst_p (through_insn))
	{
	  /* Substitute in vinsn.  */
	  line_start ();
	  print ("Substituting in moveup_rhs inside insn group:\nBefore: ");
	  sel_print_rtl (insn);
	  print (" Moving through: ");
	  sel_print_rtl (through_insn);
	  print (" After: ");

	  if (substitute_reg_in_rhs (insn_to_move_up, through_insn))
            {
              sel_print_rtl (insn);
              line_finish ();
          
	      return MOVEUP_RHS_CHANGED;
            }
          else
            print (" Can't move up due to architecture constraints.\n");
          
	  line_finish ();
	}
    }

  return MOVEUP_RHS_NULL;
}

#define CANT_MOVE_TRAPPING(insn_to_move_up, through_insn) \
  (VINSN_MAY_TRAP_P (EXPR_VINSN (insn_to_move_up))        \
   && !sel_insn_has_single_succ_p ((through_insn), SUCCS_ALL))

/* Modifies INSN_TO_MOVE_UP so it can be moved through the THROUGH_INSN,
   performing necessary transformations.  When INSIDE_INSN_GROUP, 
   permit all dependencies except true ones, and try to remove those
   too via forward substitution.  All cases when a non-eliminable 
   non-zero cost dependency exists inside an insn group will be fixed 
   in tick_check_p instead.  */
static enum MOVEUP_RHS_CODE
moveup_rhs (rhs_t insn_to_move_up, insn_t through_insn, bool inside_insn_group)
{
  vinsn_t vi = RHS_VINSN (insn_to_move_up);
  insn_t insn = VINSN_INSN (vi);
  ds_t *has_dep_p;
  ds_t full_ds;

  /* When inside_insn_group, delegate to the helper.  */
  if (inside_insn_group)
    return moveup_rhs_inside_insn_group (insn_to_move_up, through_insn);

  if (VINSN_UNIQUE_P (vi))
    {
      if (/* Don't move branches for now.  */
	  control_flow_insn_p (insn)
	  /* Don't try to clone unique insns.  */
	  || bookkeeping_can_be_created_if_moved_through_p (through_insn))
	return MOVEUP_RHS_NULL;

      if (CANT_MOVE (insn)
	  && BLOCK_FOR_INSN (through_insn) != BLOCK_FOR_INSN (insn))
	/* Don't move what we can't move.  */
	return MOVEUP_RHS_NULL;

      if (SCHED_GROUP_P (insn))
	/* Don't move SCHED_GROUP instruction through anything.
	   If we don't force this, then it will be possible to start
	   scheduling a sched_group before all its dependencies are
	   resolved.
	   ??? Haifa deals with this issue by delaying the SCHED_GROUP
	   as late as possible through rank_for_schedule ().  */
	return MOVEUP_RHS_NULL;
    }
  else
    {
      gcc_assert (!control_flow_insn_p (insn));

      if (!bookkeeping_p
	  && bookkeeping_can_be_created_if_moved_through_p (through_insn))
	/* If bookkeeping is disabled don't try to move those insns that can
	   possible generate one.  */
	return MOVEUP_RHS_NULL;
    }

  full_ds = has_dependence_p (insn_to_move_up, through_insn, &has_dep_p);

  if (full_ds == 0)
    {
      if (!CANT_MOVE_TRAPPING (insn_to_move_up, through_insn))
	return MOVEUP_RHS_SAME;
    }
  else
    {
      if (VINSN_UNIQUE_P (vi))
	/* We can move UNIQUE insn up only as a whole and unchanged, 
	   so it shouldn't have any dependencies.  */
	return MOVEUP_RHS_NULL;
    }

  if (full_ds != 0 && can_overcome_dep_p (full_ds))
    {
      if (speculate_expr (insn_to_move_up, full_ds))
	/* Speculation was successful.  */
	{
	  full_ds = 0;
	  sel_clear_has_dependence ();
	}
    }

  if (has_dep_p[DEPS_IN_INSN])
    /* We have some dependency that cannot be discarded.  */
    return MOVEUP_RHS_NULL;

  if (has_dep_p[DEPS_IN_LHS] && !EXPR_SEPARABLE_P (insn_to_move_up))
    return MOVEUP_RHS_NULL;

  /* At this point we have either separable insns, that will be lifted
     up only as RHSes, or non-separable insns with no dependency in lhs.
     If dependency is in RHS, then try perform substitution and move up
     substituted RHS:

      Ex. 1:				  Ex.2
	y = x;				    y = x;
	z = y*2;			    y = y*2;

    In Ex.1 y*2 can be substituted for x*2 and the whole operation can be 
    moved above y=x assignment as z=x*2.

    In Ex.2 y*2 also can be substituted for x*2, but only the right hand 
    side can be moved because of the output dependency.  The operation was
    cropped to its rhs above.  */
  if (has_dep_p[DEPS_IN_RHS])
    {
      ds_t *rhs_dsp = &has_dep_p[DEPS_IN_RHS];

      /* Can't substitute UNIQUE VINSNs.  */
      gcc_assert (!VINSN_UNIQUE_P (vi));

      if (can_overcome_dep_p (*rhs_dsp))
	{
	  if (speculate_expr (insn_to_move_up, *rhs_dsp))
	    /* Speculation was successful.  */
	    *rhs_dsp = 0;
	  else
	    return MOVEUP_RHS_NULL;
	}
      else if (flag_sel_sched_substitution
	       && insn_eligible_for_subst_p (through_insn))
	{
	  /* Substitute in vinsn.  */
	  line_start ();
	  print ("Substituting in moveup_rhs:\nBefore: ");
	  sel_print_rtl (insn);
	  print ("Moving through: ");
	  sel_print_rtl (through_insn);
	  print ("After: ");

	  if (substitute_reg_in_rhs (insn_to_move_up, through_insn))
	    sel_print_rtl (insn);
	  else
	    {
	      print ("Can't move up due to architecture constraints.\n");
	      return MOVEUP_RHS_NULL;
	    }

	  line_finish ();
	}
      else
	return MOVEUP_RHS_NULL;
    }

  /* Don't move trapping insns through jumps.
     This check should be at the end to give a chance to control speculation
     to perform its duties.  */
  if (CANT_MOVE_TRAPPING (insn_to_move_up, through_insn))
    return MOVEUP_RHS_NULL;

  return MOVEUP_RHS_CHANGED;
}


/* Moves an av set AVP up through INSN, performing necessary 
   transformations.  */
static void
moveup_set_rhs (av_set_t *avp, insn_t insn, bool inside_insn_group)
{
  av_set_iterator i;
  rhs_t rhs;

  print ("moveup_set_rhs (%d)", INSN_UID (insn));
  block_start ();

  FOR_EACH_RHS_1 (rhs, i, avp)    
    { 
      int rhs_uid = INSN_UID (EXPR_INSN_RTX (rhs));
      bool unique_p = VINSN_UNIQUE_P (RHS_VINSN (rhs));

      line_start ();
      dump_rhs (rhs);

      /* First check whether we've analyzed this situation already.  */
      if (bitmap_bit_p (INSN_ANALYZED_DEPS (insn), rhs_uid))
        {
          if (bitmap_bit_p (INSN_FOUND_DEPS (insn), rhs_uid))
            {
              av_set_iter_remove (&i);
              print (" - removed (cached)");
            }
          else
            print (" - unchanged (cached)");

          line_finish ();
          continue;
        }
      
      switch (moveup_rhs (rhs, insn, inside_insn_group))
	{
	case MOVEUP_RHS_NULL:
          /* Cache that there is a hard dependence.  */
          if (!unique_p)
            {
              bitmap_set_bit (INSN_ANALYZED_DEPS (insn), rhs_uid);
              bitmap_set_bit (INSN_FOUND_DEPS (insn), rhs_uid);
            }

	  av_set_iter_remove (&i);
	  print (" - removed");
	  break;
	case MOVEUP_RHS_CHANGED:
	  print (" - changed");
          
          /* Mark that this insn changed this expr.  */
          bitmap_set_bit (EXPR_CHANGED_ON_INSNS (rhs), INSN_LUID (insn));

	  {
	    rhs_t rhs2 = av_set_lookup_other_equiv_rhs (*avp, RHS_VINSN (rhs));

	    /* If the resulting insn after substitution is already in av_set,
	       remove it.  */
	    if (rhs2 != NULL)
	      {
		merge_expr (rhs2, rhs);

		av_set_iter_remove (&i);
		print (" and removed.");

		rhs = rhs2;
	      }

	    print (" result: ");
	    dump_rhs (rhs);
	  }
	  break;
	case MOVEUP_RHS_SAME:
          /* Cache that there is a no dependence.  */
          if (!unique_p)
            {
              bitmap_set_bit (INSN_ANALYZED_DEPS (insn), rhs_uid);
              bitmap_clear_bit (INSN_FOUND_DEPS (insn), rhs_uid);
            }

	  print (" - unchanged");
	  break;
	default:
	  gcc_unreachable ();
	}

      line_finish ();
    }

  block_finish ();
}

/* Moves an av set AVP through PATH.  A helper for moveup_set_path.
   FIXME: kill recursion.  */
static void
moveup_set_path_1 (av_set_t *avp, ilist_t path)
{
  if (ILIST_NEXT (path))
    moveup_set_path_1 (avp, ILIST_NEXT (path));

  moveup_set_rhs (avp, ILIST_INSN (path), true);
}

/* Moves AVP set along PATH.  */
static void
moveup_set_path (av_set_t *avp, ilist_t path)
{
  print ("moveup_set_path");
  block_start ();

  if (path)
    {
      ilist_t path1 = ilist_invert (path);

      moveup_set_path_1 (avp, path1);
      ilist_clear (&path1);
    }
  
  block_finish ();
}

/* A helper to equal_after_moveup_path_p.  
   FIXME: kill recursion.  */
static bool
equal_after_moveup_path_p_1 (rhs_t rhs, ilist_t path)
{
  bool res;

  if (ILIST_NEXT (path))
    res = equal_after_moveup_path_p_1 (rhs, ILIST_NEXT (path));
  else
    res = true;

  if (res)
    res = (moveup_rhs (rhs, ILIST_INSN (path), true) != MOVEUP_RHS_NULL);

  return res;
}

/* Returns true if after moving CUR_RHS along PATH it equals to RHS_VLIW.  */
static bool
equal_after_moveup_path_p (rhs_t cur_rhs, ilist_t path, rhs_t rhs_vliw)
{
  expr_def _tmp, *tmp = &_tmp;
  bool res_p;

  copy_expr_onside (tmp, cur_rhs);

  if (path)
    {
      ilist_t path1 = ilist_invert (path);

      res_p = equal_after_moveup_path_p_1 (tmp, path1);
      ilist_clear (&path1);
    }
  else
    res_p = true;

  if (res_p)
    {
      vinsn_t tmp_vinsn = RHS_VINSN (tmp);
      vinsn_t rhs_vliw_vinsn = RHS_VINSN (rhs_vliw);

      if (tmp_vinsn != rhs_vliw_vinsn)
	res_p = vinsns_correlate_as_rhses_p (tmp_vinsn, rhs_vliw_vinsn);
    }

  clear_expr (tmp);

  return res_p;
}


/* Functions that compute av and lv sets.  */

/* Compute av set before INSN.
   INSN - the current operation (actual rtx INSN)
   P - the current path, which is list of insns visited so far
   WS - software lookahead window size.
   UNIQUE_P - TRUE, if returned av_set will be changed, hence
   if we want to save computed av_set in s_i_d, we should make a copy of it.

   In the resulting set we will have only expressions that don't have delay
   stalls and nonsubstitutable dependences.  */
static av_set_t
compute_av_set (insn_t insn, ilist_t p, int ws, bool unique_p)
{
  av_set_t av1;
  av_set_t rhs_in_all_succ_branches;
  int succs_n, real_succs_n;
  insn_t *succs;
  int succ;

  line_start ();
  print ("compute_av_set");
  dump_insn (insn);
  line_finish ();
  block_start ();

  /* Return NULL if insn is not on the legitimate downward path.  */
  if (is_ineligible_successor (insn, p))
    {
      print ("ineligible_successor (%d)", INSN_UID (insn));
      block_finish ();
      return NULL;
    }

  /* If insn already has valid av(insn) computed, just return it.  */ 
  if (INSN_AV_VALID_P (insn))
    {
      line_start ();
      print ("found valid av (%d): ", INSN_UID (insn));
      dump_av_set (AV_SET (insn));
      line_finish ();
      block_finish ();

      return unique_p ? av_set_copy (AV_SET (insn)) : AV_SET (insn);
    }

  /* If the window size exceeds at insn during the first computation of 
     av(group), leave a window boundary mark at insn, so further 
     update_data_sets calls do not compute past insn.  */
  if (ws > MAX_WS)
    {
      print ("Max software lookahead window size reached");
      
      /* We can reach max lookahead size at bb_header, so clean av_set 
	 first.  */
      av_set_clear (&AV_SET (insn));

      AV_LEVEL (insn) = global_level;
      block_finish ();
      return NULL;
    }

  cfg_succs (insn, &succs, &succs_n);

  /* If there are successors that lead out of the region, then all rhses from
     the below av_sets should be speculative.  */
  real_succs_n = cfg_succs_n (insn, SUCCS_ALL);

  /* Debug output.  */
  line_start ();
  print ("successors (%d): ", INSN_UID (insn));
  dump_insn_array (succs, succs_n);
  line_finish ();

  /* Add insn to to the tail of current path.  */
  ilist_add (&p, insn);
  /* If there are any successors.  */
  /* Create empty av_set and make union of successors' av_sets or add rhs
     for current instruction to it later.  */
  /* TODO: optimize - do copy_shallow instead of union for 
     num_successors == 1.  */
  /* FIXME: avoid recursion here wherever possible, e.g.
     when num_successors == 1.  */
  av1 = NULL;
  rhs_in_all_succ_branches = NULL;

  for (succ = 0; succ < succs_n; succ++)
    {
      av_set_t succ_set;

      /* We will edit SUCC_SET and RHS_SPEC field of its elements.  */
      succ_set = compute_av_set (succs[succ], p, ws + 1, true);

      if (real_succs_n > 1)
	{
          /* Consider successors not from the current region.  */
          if (real_succs_n == succs_n)
            {
              /* Find RHS'es that came from *all* successors and save them 
                 into rhs_in_all_succ_branches.  This set will be used later
                 for calculating speculation attributes of RHS'es.  */
              if (succ == 0)
                rhs_in_all_succ_branches = av_set_copy (succ_set);
              else
                {
                  av_set_iterator i;
                  rhs_t rhs;
  
                  FOR_EACH_RHS_1 (rhs, i, &rhs_in_all_succ_branches)
                    if (!av_set_is_in_p (succ_set, RHS_VINSN (rhs)))
                      av_set_iter_remove (&i);
                }
            }
	}

      /* Union the av_sets.  */
      av_set_union_and_clear (&av1, &succ_set);
    }

  if (real_succs_n > 1)
    {
      av_set_iterator i;
      rhs_t rhs;
      
      /* Increase the spec attribute of all RHS'es that didn't come 
	 from all successors.  */
      FOR_EACH_RHS (rhs, i, av1)
	if (!av_set_is_in_p (rhs_in_all_succ_branches, RHS_VINSN (rhs)))
	  RHS_SPEC (rhs)++;

      av_set_clear (&rhs_in_all_succ_branches);
      
      /* Do not move conditional branches through other 
	 conditional branches.  So, remove all conditional 
	 branches from av_set if current operator is a conditional
	 branch.  */
      av_set_substract_cond_branches (&av1);
    }
  
  free (succs);
  ilist_remove (&p);

  line_start ();
  print ("av_succs (%d): ", INSN_UID (insn));
  dump_av_set (av1);
  line_finish ();

  /* Then, compute av1 above insn.  */
  if (!INSN_NOP_P (insn))
    {
      expr_t expr;
      vinsn_t vi = INSN_VI (insn);

      moveup_set_rhs (&av1, insn, false);
      
      expr = av_set_lookup (av1, vi);

      if (expr != NULL)
	/* ??? It is not clear if we should replace or merge exprs in this
	   case.  But it looks reasonable to me that if we unify (merge) all
	   operations beyond current point then we should find them all during
	   move_op () - which is not the case yet.  On the other hand if during
	   move_op () we find only the first operation, then we should replace
	   the expression here - which is the case.  */
	{
	  clear_expr (expr);
	  copy_expr (expr, INSN_EXPR (insn));
	}
      else
	av_set_add (&av1, INSN_EXPR (insn));
    }

  line_start ();
  print ("insn: ");
  dump_insn_1 (insn, 1);
  line_finish ();

  line_start ();
  print ("av (%d): ", INSN_UID (insn));
  dump_av_set (av1);
  line_finish ();

  /* INSN might have been a bb_header, so free its AV_SET in any case.  */
  av_set_clear (&AV_SET (insn));

  /* If insn is a bb_header, leave a copy of av1 here.  */
  if (sel_bb_header_p (insn))
    {
      print ("Save av(%d) in bb header", INSN_UID (insn));

      AV_SET (insn) = unique_p ? av_set_copy (av1) : av1;
      AV_LEVEL (insn) = global_level;
    }
  
  block_finish ();
  return av1;
}

/* Propagate a liveness set LV through INSN.  */
static void
propagate_lv_set (regset lv, insn_t insn)
{
  gcc_assert (INSN_P (insn));

  if (INSN_NOP_P (insn))
    return;

  /* LV1 = LV1 \ { DEST (insn) }  */
  if (GET_CODE (PATTERN (insn)) != COND_EXEC) 
    /* May-defs should not kill other sets.  */
    AND_COMPL_REG_SET (lv, INSN_REG_SETS (insn));

  /* LV1 = LV1 U { SOURCES (insn) } */
  /* FIXME: Should we consider the whole register clobbered in the cases of
     STRICT_LOW_PART or SUBREG?  */
  /* Yes, and that's what sched-deps do: if there's subreg or 
     strict_low_part, then the whole register (as it appear in that expr)
     will be included in REG_PUSES counting all it's relevant subregs
     in the used mode.  */
  IOR_REG_SET (lv, INSN_REG_USES (insn));
}

static regset
compute_live_after_bb (basic_block bb)
{
  insn_t succ;
  succ_iterator si;
  regset lv = get_clear_regset_from_pool ();

  gcc_assert (!ignore_first);

  FOR_EACH_SUCC_1 (succ, si, BB_END (bb), SUCCS_ALL)
    IOR_REG_SET (lv, compute_live (succ));

  return lv;
}

/* Compute the set of all live registers at the point before INSN and save
   it at INSN if INSN is bb header.  */
static regset
compute_live (insn_t insn)
{
  if (LV_SET_VALID_P (insn) && !ignore_first)
    {
      regset lv = get_regset_from_pool ();

      COPY_REG_SET (lv, LV_SET (insn));
      return_regset_to_pool (lv);
      return lv;
    }

  /* We've skipped the wrong lv_set.  Don't skip the right one.  */
  ignore_first = false;
  
  {
    basic_block bb = BLOCK_FOR_INSN (insn);
    insn_t bb_end = BB_END (bb);
    regset lv = compute_live_after_bb (bb);

    while (bb_end != insn)
      {
	propagate_lv_set (lv, bb_end);

	bb_end = PREV_INSN (bb_end);
      }
    /* Now LV contains live set below INSN.  */
      
    /* Compute live set above INSN.  */
    propagate_lv_set (lv, insn);

    if (sel_bb_header_p (insn))
      {
	gcc_assert (LV_SET (insn) != NULL);

	COPY_REG_SET (LV_SET (insn), lv);
      }
    else
      gcc_assert (LV_SET (insn) == NULL);

    /* We return LV to the pool, but will not clear it there.  Thus we can
       legimatelly use LV till the next use of regset_pool_get ().  */
    return_regset_to_pool (lv);

    return lv;
  }
}

/* Compute the set of live registers right below the INSN and save it to 
   REGS.  */
static void
compute_live_below_insn (insn_t insn, regset regs)
{
  insn_t succ;
  succ_iterator succ_i;

  FOR_EACH_SUCC_1 (succ, succ_i, insn, SUCCS_ALL)
    IOR_REG_SET (regs, compute_live (succ));
}

/* Update the data gathered in av and lv sets starting from INSN.  */
static void
update_data_sets (rtx insn)
{
  gcc_assert (LV_SET (insn) != NULL
	      && INSN_AV_VALID_P (insn)
	      && sel_bb_header_p (insn));

  /* Recompute the first LV_SET as it may have got invalid.  */
  ignore_first = true;
  compute_live (insn);

  line_start ();
  print ("update_data_sets");
  dump_insn (insn);
  line_finish ();

  block_start ();

  if (LV_SET_VALID_P (insn))
    {
      line_start ();
      print ("live regs set:");
      dump_lv_set (LV_SET (insn));
      line_finish ();
    }

  block_finish ();

  /* Invalidate AV_SET.  */
  AV_LEVEL (insn) = 0;
  if (sel_bb_header_p (insn))
    compute_av_set (insn, NULL, 0, 0);
  /* If INSN is not a bb_header any longer, its av_set will be
     deleted on the next compute_av_set ().  */
}


/* Helper for move_op () and find_used_regs ().
   Return speculation type for which a check should be created on the place
   of INSN.  EXPR is one of the original ops we are searching for.  */
static ds_t
get_spec_check_type_for_insn (insn_t insn, expr_t expr)
{
  ds_t to_check_ds;
  ds_t already_checked_ds = EXPR_SPEC_DONE_DS (INSN_EXPR (insn));

  to_check_ds = EXPR_SPEC_TO_CHECK_DS (expr);

  if (targetm.sched.get_insn_checked_ds)
    already_checked_ds |= targetm.sched.get_insn_checked_ds (insn);

  if (spec_info != NULL
      && (spec_info->flags & SEL_SCHED_SPEC_DONT_CHECK_CONTROL))
    already_checked_ds |= BEGIN_CONTROL;

  already_checked_ds = ds_get_speculation_types (already_checked_ds);

  to_check_ds &= ~already_checked_ds;

  return to_check_ds;
}

/* Functions to check liveness restrictions on available instructions.  */

/* Helper function for find_used_regs.
   Finds registers that are not available for storing rhses while moving
   ORIG_OPS up on the path starting from INSN. The register is used on 
   the moving path, if one of the following conditions is false 

      (1) not set or read on any path from xi to an instance of the original
          operation, 
      (2) not among the live registers of the point immediately following the 
          first original operation on a given downward path, except for the
	  original target register of the operation,
      (3) not live on the other path of any conditional branch that is passed 
	  by the operation, in case original operations are not present on
	  both paths of the conditional branch.
   
   The routine traverses the program in the way similar to move_op.
   PATH represents the edges traversed so far.  The return value is only used 
   for correct work of this function itself.  The used registers are added to
   USED_REGS set (USED_REGS should be allocated elsewhere, registers found are
   ORed into the set).

   All the original operations found during the traversal are saved in the
   ORIGINAL_INSNS list.

   UNAVAILABLE_HARD_REGS denotes the set of hardware registers that
   can not be used with renaming due to the register class restrictions,
   mode restrictions and other (the register we'll choose should be 
   compatible class with the original uses, shouldn't be in call_used_regs,
   should be HARD_REGNO_RENAME_OK etc).
   This parameter can have NULL value.  It should happen if we compute
   used registers before reload, and thus don't have information about hard
   registers.  

   CROSSES_CALL is true, if there is a call insn on the path from INSN to 
   original insn. In this case CALL_USED_REG_SET will be added to the 
   UNAVAILABLE_HARD_REGS at the point original operation is found.   

   Actually we need a complement set to the one computed by this routine,
   but it's more natural to have the inverted set.  */

static int
find_used_regs_1 (insn_t insn, av_set_t orig_ops, ilist_t path, 
		regset used_regs, HARD_REG_SET *unavailable_hard_regs,
		bool crosses_call, def_list_t *original_insns)
{
  rhs_t rhs;
  bool is_orig_op = false;
  regset tmp;
  int res;
  
  line_start ();
  print ("find_used_regs_1(");
  dump_insn (insn);
  print (",");
  dump_av_set (orig_ops);
  print (")");
  line_finish ();
  block_start ();

  gcc_assert (orig_ops);

  /* Check that we are still inside a dag we schedule.  */
  if (is_ineligible_successor (insn, path))
    {
      print ("ineligible_successor");
      block_finish ();
      return 0;
    }

  orig_ops = av_set_copy (orig_ops);

  /* If we've found valid av set, then filter the orig_ops set.  */
  if (INSN_AV_VALID_P (insn))
    {
      line_start ();
      print ("av");
      dump_insn (insn);
      print ("=");
      dump_av_set (AV_SET (insn));
      line_finish ();

      av_set_intersect (&orig_ops, AV_SET (insn));

      /* If no more original ops, return immediately.  */
      if (!orig_ops)
	{
	  print ("no intersection");
	  block_finish ();
	  return 0;
	}

      /* !!! When using different types of speculation we must not leave
	 just one element in orig_ops.  */
      /*av_set_leave_one (&orig_ops);*/
    }

  if (CALL_P (insn))
    crosses_call = true;

  /* Look at the insn and decide if it could be an ancestor of currently 
     scheduling operation.  If it is so, then the insn "dest = op" could 
     either be replaced with "dest = reg", because REG now holds the result
     of OP, or just removed, if we've scheduled the insn as a whole.  

     If this insn doesn't contain currently scheduling OP, then proceed
     with searching and look at its successors.  Operations we're searching 
     for could have changed when moving up through this insn via 
     substituting.  In this case, firstly perform unsubstitution on them. 

     When traversing the DAG below this insn is finished, insert bookkeeping 
     code, if the insn is a joint point, and remove leftovers.  */

  rhs = av_set_lookup (orig_ops, INSN_VI (insn));
  if (rhs)
    {
      /* We have found the original operation. Mark the registers that do not
	 meet the following condition:
	(2) not among the live registers of the point immediately following 
	    the first original operation on a given downward path, except 
	    for the original target register of the operation.  */

      print ("found original operation!");

      {
	bool needs_spec_check_p;

	needs_spec_check_p = (get_spec_check_type_for_insn (insn, rhs) != 0);

	def_list_add (original_insns, insn, crosses_call, needs_spec_check_p);
      }

      res = 1;
      is_orig_op = true;

      tmp = get_clear_regset_from_pool ();

      compute_live_below_insn (insn, tmp);

      AND_COMPL_REG_SET (tmp, INSN_REG_SETS (insn));
      IOR_REG_SET (used_regs, tmp);

      return_regset_to_pool (tmp);

      /* (*1) We need to add to USED_REGS registers that are read by 
	 INSN's lhs. This may lead to choosing wrong src register. 
	 E.g. (scheduling const rhs enabled):  

	    429: ax=0x0	<- Can't use AX for this rhs (0x0)
	    433: dx=[bp-0x18]
	    427: [ax+dx+0x1]=ax 
	      REG_DEAD: ax
	    168: di=dx
	      REG_DEAD: dx    
	 */
      /* FIXME: see comment above and enable MEM_P in vinsn_separable_p.  */
      gcc_assert (!VINSN_SEPARABLE_P (INSN_VI (insn))
                  || !MEM_P (INSN_LHS (insn)));
    }
  else
    {
      succ_iterator succ_i;
      rtx succ;
      bool mutexed;
      rhs_t r;
      av_set_iterator avi;
      regset live_on_other_branches = NULL;

      res = 0;

      /* Av set ops could have changed when moving through this insn.
	 To find them below it, we have to un-speculate and un-substitute
	 them.  */

      undo_transformations (&orig_ops, insn);

      /* If all original operands have been filtered on this branch,
	 return.  */
      if (!orig_ops)
	{
	  block_finish ();
	  return 0;
	}

      /* Continue searching.  Do recursion here.  */
      ilist_add (&path, insn);

      FOR_EACH_SUCC (succ, succ_i, insn)
        {
	  int b;

	  if (succ_i.e1)
	    {
	      gcc_assert (succ_i.e2);

	      gcc_assert (succ_i.e1->src == BLOCK_FOR_INSN (insn)
			  && succ_i.e2->dest == BLOCK_FOR_INSN (succ));
	    }

	  b = find_used_regs_1 (succ, orig_ops, path,
			      used_regs, unavailable_hard_regs, crosses_call,
			      original_insns);

	  if (b == 0)
	    /* Mark all registers that do not meet the following condition:
	       (3) not live on the other path of any conditional branch 
	       that is passed by the operation, in case original
	       operations are not present on both paths of the 
	       conditional branch.

	       if b is false, then it's not present on that branch.  */
	    {
	      regset succ_live = compute_live (succ);

	      if (live_on_other_branches == NULL)
		/* We're getting succ_live out of the pool.  */
		{
		  regset tmp = get_regset_from_pool ();

		  gcc_assert (tmp == succ_live);

		  live_on_other_branches = succ_live;
		}
	      else
		/* We're leaving succ_live in the pool.  */
		IOR_REG_SET (live_on_other_branches, succ_live);
	    }
	  else if (res != -1)
	    res = b;
	}

      if (live_on_other_branches != NULL)
	{
	  IOR_REG_SET (used_regs, live_on_other_branches);
	  return_regset_to_pool (live_on_other_branches);
	  live_on_other_branches = NULL;
	}

      if (res == 0)
	res = -1;

      if (res == 1)
	{
	  /* Additionally, we need to include all live registers on 
	     the successors of all conditional branches that do not lead to 
	     the original operation.  We need to do this only for conditional
	     branches that are inside actual code motion paths for original 
	     operations.  The actual code motion path will have non-empty
	     orig_ops set.
	     This loop completes the set that is evaluated for 
	     condition (3).  */
    
	  FOR_EACH_SUCC_1 (succ, succ_i, insn, SUCCS_BACK | SUCCS_OUT)
	    IOR_REG_SET (used_regs, compute_live (succ));
    
	  /* If current insn we are looking at cannot be executed together 
	     with original insn, then we can skip it safely.
    
	     Example: ORIG_OPS = { (p6) r14 = sign_extend (r15); }
		      INSN = (!p6) r14 = r14 + 1; 
    
	     Here we can schedule ORIG_OP with lhs = r14, though only 
	     looking at the set of used and set registers of INSN we must 
	     forbid it.  So, add set/used in INSN registers to the 
	     untouchable set only if there is an insn in ORIG_OPS that can 
	     affect INSN.  
    
	     FIXME: I believe that it is enough to check only 1 insn of 
	     ORIG_OPS (because other insns must be variations of this insn, 
	     created by substitution, and so their execution conditions 
	     must be same on all insns in ORIG_OPS), 
	     though here we are more conservative.  */
	  mutexed = true;
	  gcc_assert (orig_ops);
	  FOR_EACH_RHS (r, avi, orig_ops)
	    if (!sched_insns_conditions_mutex_p (insn, RHS_INSN (r)))
	      {
		mutexed = false;
		break;
	      }
    
	  /* Mark all registers that do not meet the following condition:
	     (1) Not set or read on any path from xi to an instance of the 
		 original operation.  */
	  if (!mutexed)
	    {
	      IOR_REG_SET (used_regs, INSN_REG_SETS (insn));
	      IOR_REG_SET (used_regs, INSN_REG_USES (insn));
	    }	  
	}

      ilist_remove (&path);
    }

  av_set_clear (&orig_ops);

  gcc_assert (!sel_bb_header_p (insn) || INSN_AV_VALID_P (insn)
	      || AV_LEVEL (insn) == -1);

  if (res == -1 && AV_LEVEL (insn) == -1)
    /* INSN has an incorrect av_set on which we've couldn't have filtered
       ORIG_OPS.  */
    res = 0;

  block_finish ();
  return res;
}


/* Find the set of registers that are unavailable for storing rhses 
   while moving ORIG_OPS up on the path starting from INSN due to
   liveness (USED_REGS) or hardware restrictions (UNAVAILABLE_HARD_REGS).

   All the original operations found during the traversal are saved in the
   ORIGINAL_INSNS list.

   Returns TRUE if we've found all original insns, FALSE otherwise.

   See also comment for find_used_regs_1 for the details.  */

static bool
find_used_regs (insn_t insn, av_set_t orig_ops, regset used_regs,
		HARD_REG_SET *unavailable_hard_regs, 
		def_list_t *original_insns)
{
  def_list_iterator i;
  def_t def;
  int res;

  res = find_used_regs_1 (insn, orig_ops, NULL, used_regs,
			  unavailable_hard_regs, false, original_insns);

  if (res == -1)
    return false;

  gcc_assert (res == 1);
  gcc_assert (original_insns && *original_insns);

  /* Mark hardware regs in UNAVAILABLE_HARD_REGS that are not suitable 
     for renaming rhs in INSN due to hardware restrictions (register class,
     modes compatibility etc).  This doesn't affect original insn's dest regs.
     Registers that are in used_regs are always marked also in
     unavailable_hard_regs.  */
  FOR_EACH_DEF (def, i, *original_insns)
    {
      vinsn_t vinsn = INSN_VI (def->orig_insn);

      if (VINSN_SEPARABLE_P (vinsn))
	mark_unavailable_hard_regs (def, unavailable_hard_regs, used_regs);

      if (def->needs_spec_check_p)
	/* Do not allow clobbering of ld.[sa] address.  */
	IOR_REG_SET (used_regs, VINSN_REG_USES (vinsn));
    }

  return true;
}


/* Functions to choose the best insn from available ones.  */

/* Rank two available rhses for schedule.  Never return 0 here.  */
static int 
sel_rank_for_schedule (const void *x, const void *y)
{
  rhs_t tmp = *(const rhs_t *) y;
  rhs_t tmp2 = *(const rhs_t *) x;
  insn_t tmp_insn, tmp2_insn;
  vinsn_t tmp_vinsn, tmp2_vinsn;
  int val;

  tmp_vinsn = RHS_VINSN (tmp);
  tmp2_vinsn = RHS_VINSN (tmp2);
  tmp_insn = RHS_INSN (tmp);
  tmp2_insn = RHS_INSN (tmp2);
  
  /* Prefer SCHED_GROUP_P insns to any others.  */
  if (SCHED_GROUP_P (tmp_insn) != SCHED_GROUP_P (tmp2_insn))
    {
      if (VINSN_UNIQUE_P (tmp_vinsn) && VINSN_UNIQUE_P (tmp2_vinsn)) 
        return SCHED_GROUP_P (tmp2_insn) ? 1 : -1;
      /* Now uniqueness means SCHED_GROUP_P is set, because schedule groups
         cannot be cloned.  */
      if (VINSN_UNIQUE_P (tmp2_vinsn))
        return 1;
      return -1;
    }

  /* Prefer not scheduled insn over scheduled one.  */
  val = EXPR_SCHED_TIMES (tmp) - EXPR_SCHED_TIMES (tmp2);
  if (val)
    return val;

  /* FIXME: Want to try first clear all insns except with minimal SPEC.  */
  /* Prefer a rhs with smaller SPEC attribute.  
     FIXME: should we use probabilities here?  */
  val = EXPR_SPEC (tmp) - EXPR_SPEC (tmp2);
  if (val)
    return val;

  /* Prefer an expr with greater priority.  */
  val = EXPR_PRIORITY (tmp2) - EXPR_PRIORITY (tmp);
  if (val)
    return val;

  if (spec_info != NULL && spec_info->mask != 0)
    /* This code was taken from haifa-sched.c: rank_for_schedule ().  */
    {
      ds_t ds1, ds2;
      dw_t dw1, dw2;
      int dw;

      ds1 = EXPR_SPEC_DONE_DS (tmp);
      if (ds1)
	dw1 = ds_weak (ds1);
      else
	dw1 = NO_DEP_WEAK;

      ds2 = EXPR_SPEC_DONE_DS (tmp2);
      if (ds2)
	dw2 = ds_weak (ds2);
      else
	dw2 = NO_DEP_WEAK;

      dw = dw2 - dw1;
      if (dw > (NO_DEP_WEAK / 8) || dw < -(NO_DEP_WEAK / 8))
	return dw;
    }

  tmp_insn = EXPR_INSN_RTX (tmp);
  tmp2_insn = EXPR_INSN_RTX (tmp2);

  /* Prefer an old insn to a bookkeeping insn.  */
  if (INSN_UID (tmp_insn) < first_emitted_uid 
      && INSN_UID (tmp2_insn) >= first_emitted_uid)
    return -1;
  if (INSN_UID (tmp_insn) >= first_emitted_uid 
      && INSN_UID (tmp2_insn) < first_emitted_uid)
    return 1;

  /* Prefer an insn with smaller UID, as a last resort.  
     We can't safely use INSN_LUID as it is defined only for those insns
     that are in the stream.  */
  return INSN_UID (tmp_insn) - INSN_UID (tmp2_insn);
}

/* Initialize ready list from the AV for the max_issue () call.
   If any unrecognizable insn found in the AV, return it (and skip
   max_issue).  BND and FENCE are current boundary and fence, 
   respectively.  */
static rhs_t
fill_ready_list (av_set_t *av_ptr, bnd_t bnd, fence_t fence)
{
  rhs_t rhs;
  av_set_iterator si;
  int n, i, stalled, sched_next_worked = 0;
  deps_t dc = BND_DC (bnd);

  /* Don't pipeline already pipelined code as that would increase
     number of unnecessary register moves.  */  
  FOR_EACH_RHS_1 (rhs, si, av_ptr)
    {
      if (EXPR_SCHED_TIMES (rhs)
	  >= PARAM_VALUE (PARAM_SELSCHED_MAX_SCHED_TIMES))
	av_set_iter_remove (&si);
    }

  if (spec_info != NULL)
    {
      bool try_data_p = true;
      bool try_control_p = true;

      /* Scan *AV_PTR to find out if we want to consider speculative
	 instructions for scheduling.  */
      FOR_EACH_RHS (rhs, si, *av_ptr)
	{
	  ds_t ds;

	  ds = EXPR_SPEC_DONE_DS (rhs);

	  if ((spec_info->flags & PREFER_NON_DATA_SPEC)
	      && !(ds & BEGIN_DATA))
	    try_data_p = false;

	  if ((spec_info->flags & PREFER_NON_CONTROL_SPEC)
	      && !(ds & BEGIN_CONTROL))
	    try_control_p = false;
	}

      FOR_EACH_RHS_1 (rhs, si, av_ptr)
	{
	  ds_t ds;

	  ds = EXPR_SPEC_DONE_DS (rhs);

	  if (ds & SPECULATIVE)
	    {
	      if ((ds & BEGIN_DATA) && !try_data_p)
		/* We don't want any data speculative instructions right
		   now.  */
		av_set_iter_remove (&si);

	      if ((ds & BEGIN_CONTROL) && !try_control_p)
		/* We don't want any control speculative instructions right
		   now.  */
		av_set_iter_remove (&si);
	    }
	}
    }

  /* Process USEs in *AV_PTR.  */
  {
    bool uses_present_p = false;
    bool try_uses_p = true;

    FOR_EACH_RHS (rhs, si, *av_ptr)
      {
	if (/* This will also initialize INSN_CODE for max_issue ().  */
	    recog_memoized (EXPR_INSN_RTX (rhs)) < 0)
	  {
	    /* If we have a USE in *AV_PTR that was not scheduled yet,
	       do so because it will do good only.  */
	    if (EXPR_SCHED_TIMES (rhs) <= 0)
	      return rhs;
	    else
	      {
		gcc_assert (pipelining_p);

		uses_present_p = true;
	      }
	  }
	else
	  try_uses_p = false;
      }

    if (uses_present_p)
      {
	if (!try_uses_p)
	  /* If we don't want to schedule any USEs right now and we have some
	     in *AV_PTR, remove them.  */
	  {
	    FOR_EACH_RHS_1 (rhs, si, av_ptr)
	      {
		if (INSN_CODE (EXPR_INSN_RTX (rhs)) < 0)
		  av_set_iter_remove (&si);
	      }
	  }
	else
	  /* If we do want to schedule a USE, return the first one.  */
	  {
	    FOR_EACH_RHS (rhs, si, *av_ptr)
	      {
		gcc_assert (INSN_CODE (EXPR_INSN_RTX (rhs)) < 0);

		return rhs;
	      }
	  }
      }
  }

  /* Allocate the vector for sorting the av set.  */
  if (!vec_av_set)
    vec_av_set = VEC_alloc (rhs_t, heap, 5);
  else
    gcc_assert (VEC_empty (rhs_t, vec_av_set));

  FOR_EACH_RHS (rhs, si, *av_ptr)
    {
      VEC_safe_push (rhs_t, heap, vec_av_set, rhs);
    }

  /* Bail out early when the ready list contained only USEs/CLOBBERs that are
     already scheduled.  */
  if (VEC_length (rhs_t, vec_av_set) == 0)
    {
      ready.n_ready = 0;
      return NULL;
    }

  /* Sort the vector.  */
  qsort (VEC_address (rhs_t, vec_av_set), VEC_length (rhs_t, vec_av_set),
         sizeof (rhs_t), sel_rank_for_schedule);

  /* Allocate and fill the ready list from the sorted vector.  */
  ready.n_ready = VEC_length (rhs_t, vec_av_set);
  if (ready.n_ready > ready.veclen)
    {
      ready.veclen = ready.n_ready;
      ready.vec = xrealloc (ready.vec, ready.veclen * sizeof (*ready.vec));
    }

  if (ready.veclen > max_issue_size)
    {
      max_issue_size = ready.veclen;
      sched_extend_ready_list (ready.veclen);
    }

  line_start ();
  print ("Sorted av set: ");

  for (i = 0, n = 0, stalled = 0; VEC_iterate (rhs_t, vec_av_set, i, rhs); i++)
    {
      vinsn_t vi = EXPR_VINSN (rhs);
      insn_t insn = VINSN_INSN_RTX (vi);

      dump_rhs (rhs);

      /* Don't allow insns from a SCHED_GROUP to be scheduled if their 
	 ancestors havn't been scheduled.
	 !!! This should be dealt with in moveup_rhs ().  */
      if (VINSN_UNIQUE_P (vi) && SCHED_GROUP_P (insn)
	  && !sel_bb_header_p (insn))
        {
          insn_t prev = PREV_INSN (insn);
          
          if (SCHED_GROUP_P (prev) 
              && INSN_SCHED_CYCLE (prev) <= INSN_SCHED_CYCLE (insn))
	    {
	      /* Dealt in moveup_rhs ().  */
	      gcc_unreachable ();
	      continue;
	    }
        }
      
      /* Don't allow any insns other than from SCHED_GROUP if we have one.  */
      if (FENCE_SCHED_NEXT (fence) && insn != FENCE_SCHED_NEXT (fence))
          continue;

      /* Set number of sched_next insns (just in case there 
         could be several).  */
      if (FENCE_SCHED_NEXT (fence))
        sched_next_worked++;

      /* Don't allow any insns whose data is not yet ready.  */
      if (!tick_check_p (rhs, dc, fence))
	{
          /* We need to know whether we do need to stall for any insns.  */
          stalled++;
	  continue;
	}

      ready_try[n] = 0;
      ready.vec[n] = insn;
      n++;

      GET_VINSN_BY_INSN (insn) = RHS_VINSN (rhs);
    }

  line_finish ();

  ready.n_ready = n;
  ready.first = n - 1;

  print ("\nreally_ready: %d stalled: %d \n", n, stalled);
  if (!n)
    {
      if (!enable_moveup_set_path_p)
        /* It seems possible that when no insns are ready, this could be
           due to liveness restrictions.  However, we should always be able
           to schedule the next instruction.  So when the list is empty, then
           all insns must require a stall to be scheduled.  */
        gcc_assert (stalled);

      /* We need to set need_stall to 1 here, because later we skip this code
         when ready list is empty.  */
      need_stall = stalled;
    }

  /* Clear SCHED_NEXT.  */
  if (FENCE_SCHED_NEXT (fence))
    {
      gcc_assert (sched_next_worked == 1);
      FENCE_SCHED_NEXT (fence) = NULL_RTX;
    }

  /* Clear the vector for future use.  */
  VEC_block_remove (rhs_t, vec_av_set, 0, VEC_length (rhs_t, vec_av_set));

  return NULL;
}

/* Wrapper for dfa_new_cycle ().  Returns TRUE if cycle was advanced.  */
static bool
sel_dfa_new_cycle (insn_t insn, fence_t fence)
{
  int last_scheduled_cycle = FENCE_LAST_SCHEDULED_INSN (fence) 
                             ? INSN_SCHED_CYCLE (FENCE_LAST_SCHEDULED_INSN (fence)) 
                             : FENCE_CYCLE (fence) - 1;
  bool res = false;
  int sort_p = 0;

  if (!targetm.sched.dfa_new_cycle)
    return false;

  memcpy (curr_state, FENCE_STATE (fence), dfa_state_size);

  while (!sort_p && targetm.sched.dfa_new_cycle (sched_dump, sched_verbose,
                                                 insn, last_scheduled_cycle,
                                                 FENCE_CYCLE (fence), &sort_p))
    {
      memcpy (FENCE_STATE (fence), curr_state, dfa_state_size);
      advance_one_cycle (fence);
      memcpy (curr_state, FENCE_STATE (fence), dfa_state_size);
      res = true;
    }

  return res;
}

/* Choose the best rhs (BEST_RHS_VLIW) from *AV_VLIW_PTR and a suitable
   register for it (BEST_REGNOP).  BNDS and FENCE are current boundaries
   and scheduling fence respectively.  */ 
static void
find_best_rhs_and_reg_that_fits (av_set_t *av_vliw_ptr, blist_t bnds,
				 fence_t fence, rhs_t *best_rhs_vliw)
{
  rhs_t res = NULL;
  insn_t best = NULL_RTX;
  rhs_t r;
  av_set_iterator av_it;

  /* First we need to choose destination registers for each rhs.  
     Multiple rhses can be given the same register, because the choice 
     is local and there's only one register from av_vliw that will be
     scheduled on this cycle.  */
  FOR_EACH_RHS_1 (r, av_it, av_vliw_ptr)
    {
      {
	ds_t ds = EXPR_SPEC_DONE_DS (r);

	if ((ds & SPECULATIVE)
	    && ds_weak (ds) < spec_info->weakness_cutoff)
	  /* The probability of a success is too low - don't speculate.  */
	  {
	    av_set_iter_remove (&av_it);
	    continue;
	  }
      }

      if (!find_best_reg_for_rhs (r, bnds))
	/* If can't choose a register for the rhs (liveness or 
	   architectural restrictions for for the insn don't allow that),
	   expr should be removed from AV_VLIW.  */
	{
	  av_set_iter_remove (&av_it);
	  continue;
	}
    }

  /* We have one boundary per fence.  */
  gcc_assert (BLIST_NEXT (bnds) == NULL);

  res = fill_ready_list (av_vliw_ptr, BLIST_BND (bnds), fence);

  if (res == NULL && ready.n_ready > 0)
    {
      /* Choose the best insn for scheduling via:
         1) sorting the ready list based on priority;
         2) calling the reorder hook;
         3) calling max_issue.  */

      rhs_t min_spec_rhs;
      insn_t min_spec_insn;
      vinsn_t min_spec_vinsn;
      int i, can_issue, privileged_n;
      int index;

      /* Call the reorder hook at the beginning of the cycle, and call
         the reorder2 hook in the middle of the cycle.  */
      if (FENCE_ISSUED_INSNS (fence) == 0)
        {
          if (targetm.sched.reorder
              && !SCHED_GROUP_P (ready_element (&ready, 0))
              && ready.n_ready > 1)
            {
              /* Don't give reorder the most prioritized insn as it can break
                 pipelining.  */
              if (pipelining_p)
                --ready.n_ready;

              can_issue_more
                = targetm.sched.reorder (sched_dump, sched_verbose,
                                         ready_lastpos (&ready),
                                         &ready.n_ready, FENCE_CYCLE (fence));

              if (pipelining_p)
                ++ready.n_ready;
            }
          else
            /* Initialize can_issue_more for variable_issue.  */
            can_issue_more = issue_rate;
        }
      else if (targetm.sched.reorder2
               && !SCHED_GROUP_P (ready_element (&ready, 0)))
        {
          if (ready.n_ready == 1)
            can_issue_more = 
              targetm.sched.reorder2 (sched_dump, sched_verbose,
                                      ready_lastpos (&ready),
                                      &ready.n_ready, FENCE_CYCLE (fence));
          else
            {
              if (pipelining_p)
                --ready.n_ready;

              can_issue_more =
                targetm.sched.reorder2 (sched_dump, sched_verbose,
                                        ready.n_ready
                                        ? ready_lastpos (&ready) : NULL,
                                        &ready.n_ready, FENCE_CYCLE (fence));

              if (pipelining_p)
                ++ready.n_ready;
            }
        }
      else 
        can_issue_more = issue_rate;
      
      if (can_issue_more == 0)
        {
          *best_rhs_vliw = NULL;
          return;
        }

      min_spec_insn = ready_element (&ready, 0);
      min_spec_vinsn = GET_VINSN_BY_INSN (min_spec_insn);
      min_spec_rhs = av_set_lookup (*av_vliw_ptr, min_spec_vinsn);
      gcc_assert (min_spec_rhs != NULL
		  && RHS_VINSN (min_spec_rhs) == min_spec_vinsn);

      line_start ();
      print ("ready after reorder: ");

      ready_try [0] = 0;

      dump_vinsn (min_spec_vinsn);
      print (":0; ");

      if (targetm.sched.first_cycle_multipass_dfa_lookahead_guard)
        for (i = 1; i < ready.n_ready; i++)
	  {
	    insn_t insn = ready_element (&ready, i);
            int r = 
              !targetm.sched.first_cycle_multipass_dfa_lookahead_guard (insn);

	    gcc_assert (INSN_CODE (insn) >= 0);

            /* Only insns with ready_try = 0 can get here 
               from fill_ready_list.  */
            gcc_assert (ready_try [i] == 0);
	    ready_try[i] = r;
	      
            dump_vinsn (GET_VINSN_BY_INSN (insn));
            print (":%d; ", ready_try[i]);
	  }

      line_finish ();

      privileged_n = 0;

      /* FIXME: when can_issue_more is 0, we may as well finish the game for
         now.  */
      for (i = 0; i < ready.n_ready; i++)
        {
          rhs_t cur_rhs;
	  insn_t cur_insn;
	  vinsn_t cur_vinsn;

	  if (ready_try[i])
	    continue;

	  cur_insn = ready_element (&ready, i);

	  cur_vinsn = GET_VINSN_BY_INSN (cur_insn);

	  cur_rhs = av_set_lookup (*av_vliw_ptr, cur_vinsn);
          gcc_assert (cur_rhs != NULL
		      && RHS_VINSN (cur_rhs) == cur_vinsn);

          if (RHS_SPEC (cur_rhs) > RHS_SPEC (min_spec_rhs))
            break;

	  ++privileged_n;
        }

      if (i == ready.n_ready)
        privileged_n = 0;

      print ("privileged_n: %d insns with SPEC %d; ",
             privileged_n, RHS_SPEC (min_spec_rhs));

      cycle_issued_insns = FENCE_ISSUED_INSNS (fence);

      if (dfa_lookahead > 0)
        {
          can_issue = max_issue (&ready, privileged_n,
                                 FENCE_STATE (fence), &index);

          print ("max_issue: we can issue %d insns, already did %d insns",
                 can_issue, FENCE_ISSUED_INSNS (fence));
        }
      else
        {
          /* We can't use max_issue; just return the first element.  */
	  insn_t insn = ready_element (&ready, 0);
	  vinsn_t vinsn = GET_VINSN_BY_INSN (insn);

	  if (vinsn_dfa_cost (vinsn, fence) >= 1)
	    {
	      can_issue = 0;
	      index = -1;
	    }
	  else
	    {
	      can_issue = 1;
	      index = 0;
	      print ("using first insn from the ready list");
	    }
        }

      if (can_issue)
        {
          best = ready_element (&ready, index);
          res = av_set_lookup (*av_vliw_ptr, GET_VINSN_BY_INSN (best));
          gcc_assert (res);
        }
      else
        {
          best = NULL;
          need_stall = 1;
        }
    }
  else if (res != NULL)
    /* Asm insn won't go in the upper case, but we need to call dfa_new_cycle
       anyway.  */
    best = RHS_INSN (res);

  if (best != NULL_RTX)
    {
      gcc_assert (INSN_P (best));
      sel_dfa_new_cycle (best, fence);

      if (targetm.sched.variable_issue)
       {
         memcpy (curr_state, FENCE_STATE (fence), dfa_state_size);
         can_issue_more = targetm.sched.variable_issue (sched_dump,
                                                        sched_verbose,
                                                        best,
                                                        can_issue_more);
         memcpy (FENCE_STATE (fence), curr_state, dfa_state_size);
       }
      else if (GET_CODE (PATTERN (best)) != USE
              && GET_CODE (PATTERN (best)) != CLOBBER)
       can_issue_more--;
    }

  *best_rhs_vliw = res;

  return;
}


/* Emit an instruction from EXPR with SEQNO after PLACE_TO_INSERT.  */
static insn_t
gen_insn_from_expr_after (expr_t expr, int seqno, insn_t place_to_insert)
{
  {
    insn_t insn = RHS_INSN (expr);

    /* This assert fails when we have identical instructions
       one of which dominates the other.  In this case move_op ()
       finds the first instruction and doesn't search for second one.
       The solution would be to compute av_set after the first found
       insn and, if insn present in that set, continue searching.
       For now we workaround this issue in move_op.  */
    gcc_assert (!INSN_IN_STREAM_P (insn));

    gcc_assert (!LV_SET_VALID_P (insn));
  }

  {
    rtx reg = expr_dest_reg (expr);

    if (reg != NULL_RTX)
      {
	static int reg_rename_this_tick = 0;

	if (HARD_REGISTER_P (reg))
	  {
	    unsigned regno = REGNO (reg);

	    reg_rename_tick[regno] = ++reg_rename_this_tick;
	    regs_ever_live[regno] = 1;
	  }
      }
  }

  {
    insn_t new_insn;

    new_insn = sel_gen_insn_from_expr_after (expr, seqno, place_to_insert);

    return new_insn;
  }
}

/* Functions that implement the core of the scheduler.  */

/* Generate a bookkeeping copy of "REG = CUR_RHS" insn at JOIN_POINT on the 
   ingoing path(s) to E2->dest, other than from E1->src (there could be some 
   empty blocks between E1->src and E2->dest).  If there is only one such path 
   and bookkeeping copy can be created in the last block, that is on this path,
   bookkeeping instruction is inserted at the end of this block.  Otherwise, 
   the function splits E2->dest bb on the two and emits the bookkeeping copy in
   the upper bb, redirecting all other paths to the lower bb and returns the
   newly created bb, which is the lower bb. 
   All scheduler data is initialized for the newly created insn.  */
static basic_block
generate_bookkeeping_insn (rhs_t c_rhs, insn_t join_point, edge e1, edge e2)
{
  basic_block src, bb = e2->dest;
  basic_block new_bb, res = NULL;
  insn_t src_end = NULL_RTX;
  insn_t place_to_insert;
  /* Save the original destination of E1.  */
  basic_block empty_bb = e1->dest;

  print ("generate_bookkeeping_insn(%d->%d)", e1->src->index, 
	 e2->dest->index);

  /* sel_split_block () can emit an unnecessary note if the following isn't
     true.  */
  gcc_assert (bb_note (bb) != BB_END (bb));

  if (EDGE_COUNT (bb->preds) == 2)
    {
      /* SRC is the block, in which we possibly can insert bookkeeping insn
         without creating new basic block.  It is the other (than E2->SRC)
         predecessor block of BB.  */
      src = EDGE_PRED (bb, 0) == e2 
        ? EDGE_PRED (bb, 1)->src : EDGE_PRED (bb, 0)->src;

      /* Instruction, after which we would try to insert bookkeeping insn.  */
      src_end = BB_END (src);
      gcc_assert (in_current_region_p (src));

      if (INSN_P (src_end))
	{
	  if (control_flow_insn_p (src_end))
	    src = NULL;
	}
      else
	gcc_assert (NOTE_INSN_BASIC_BLOCK_P (src_end));
    }
  else
    src = NULL;
    
  if (!src)
    {
      /* We need to create a new basic block for bookkeeping insn.  */
      can_add_real_insns_p = false;

      /* Check that we don't spoil the loop structure.  */
      if (flag_sel_sched_pipelining_outer_loops && current_loop_nest)
        {
          basic_block latch = current_loop_nest->latch;

          /* We do not split header.  */
          gcc_assert (bb != current_loop_nest->header);

          /* We do not redirect the only edge to the latch block.  */
          gcc_assert (e1->dest != latch
                      || !single_pred_p (latch)
                      || e1 != single_pred_edge (latch));
        }

      /* Split the head of the BB to insert BOOK_INSN there.  */
      new_bb = sel_split_block (bb, NULL);
  
      /* Move note_list from the upper bb.  */
      gcc_assert (BB_NOTE_LIST (new_bb) == NULL_RTX);
      BB_NOTE_LIST (new_bb) = BB_NOTE_LIST (bb);
      BB_NOTE_LIST (bb) = NULL_RTX;
  
      gcc_assert (e2->dest == bb);
  
      can_add_real_insns_p = true;
      insn_init.what = INSN_INIT_WHAT_INSN;
  
      /* Make a jump skipping bookkeeping copy.  */
      if (e1->flags & EDGE_FALLTHRU)
        res = sel_redirect_edge_force (e1, new_bb);
      else
        sel_redirect_edge_and_branch (e1, new_bb);

      gcc_assert (e1->dest == new_bb);

      gcc_assert (sel_bb_empty_p (bb));
      
      place_to_insert = BB_END (bb);
    }
  else
    place_to_insert = src_end;

  /* Remove unreachable empty blocks.  */
  while (EDGE_COUNT (empty_bb->preds) == 0)
    {
      basic_block next_bb = empty_bb->next_bb;
      sel_remove_empty_bb (empty_bb, false, true);
      empty_bb = next_bb;
    }

  {
    rtx new_insn_rtx;
    vinsn_t new_vinsn;
    expr_def _new_expr, *new_expr = &_new_expr;
    insn_t new_insn;

    new_insn_rtx = create_copy_of_insn_rtx (EXPR_INSN_RTX (c_rhs));
    new_vinsn = create_vinsn_from_insn_rtx (new_insn_rtx);
    copy_expr_onside (new_expr, c_rhs);
    change_vinsn_in_expr (new_expr, new_vinsn);

    if (EXPR_SCHED_TIMES (new_expr)
	>= PARAM_VALUE (PARAM_SELSCHED_MAX_SCHED_TIMES))
      /* To make scheduling of this bookkeeping copy possible we decrease
	 its scheduling counter.  */
      --EXPR_SCHED_TIMES (new_expr);

    new_insn = gen_insn_from_expr_after (new_expr, INSN_SEQNO (join_point),
					 place_to_insert);

    clear_expr (new_expr);

    gcc_assert ((src == NULL && BB_END (bb) == new_insn
		 && sel_bb_header_p (new_insn))
		|| BB_END (src) == new_insn);

    gcc_assert (AV_SET (new_insn) == NULL && AV_LEVEL (new_insn) == 0);

    /* Set AV_LEVEL to special value to bypass assert in move_op ().  */
    AV_LEVEL (new_insn) = -1;

    gcc_assert (LV_SET (join_point) != NULL);

    if (sel_bb_header_p (new_insn))
      {
	LV_SET (new_insn) = get_regset_from_pool ();
	ignore_first = true;
	compute_live (new_insn);
      }
  }

  return res;
}

static int fill_insns_run = 0;

/* Gather a parallel group of insns at FENCE and assign their seqno 
   to SEQNO.  All scheduled insns are gathered in SCHEDULED_INSNS_TAILPP 
   list for later recalculation of seqnos.  */
static void
fill_insns (fence_t fence, int seqno, ilist_t **scheduled_insns_tailpp)
{
  blist_t bnds = NULL, *bnds_tailp;
  av_set_t av_vliw = NULL;
  insn_t insn = FENCE_INSN (fence);
  state_t temp_state = alloca (dfa_state_size);

  blist_add (&bnds, insn, NULL, FENCE_DC (fence));
  bnds_tailp = &BLIST_NEXT (bnds);

  set_target_context (FENCE_TC (fence));

  print ("fill_insns (insn: %d, seqno: %d): start", INSN_UID (insn), seqno);
  block_start ();

  target_bb = INSN_BB (insn);

  /* Do while we can add any operation to the current group.  */
  do
    {
      blist_t bnds1, *bnds_tailp1, *bndsp, bnds_tail1;
      rhs_t rhs_vliw;

      line_start ();
      print ("bnds: ");
      dump_blist (bnds);
      line_finish ();

      /* Compute the set of available expressions.  */

      bnds1 = bnds;
      do
        {
	  bnd_t bnd = BLIST_BND (bnds1);
	  av_set_t av1_copy;

	  av_set_clear (&BND_AV (bnd));
	  BND_AV (bnd) = compute_av_set (BND_TO (bnd), NULL, 0, true);

	  av_set_clear (&BND_AV1 (bnd));
	  BND_AV1 (bnd) = av_set_copy (BND_AV (bnd));

          if (enable_moveup_set_path_p)
            moveup_set_path (&BND_AV1 (bnd), BND_PTR (bnd));

	  av1_copy = av_set_copy (BND_AV1 (bnd));
          av_set_union_and_clear (&av_vliw, &av1_copy);
        }
      while ((bnds1 = BLIST_NEXT (bnds1)));

      sel_dump_cfg ("after-compute_av");

      /* If debug parameters tell us to ignore this attempt to move an insn,
	 obey.  */
      {
	int now;
	int start;
	int stop;
	bool do_p;

	now = ++fill_insns_run;
	start = PARAM_VALUE (PARAM_INSN_START);
	stop = PARAM_VALUE (PARAM_INSN_STOP);
	do_p = (PARAM_VALUE (PARAM_INSN_P) == 1);

	if (do_p)
	  do_p = (start <= now) && (now <= stop);
	else
	  do_p = (start > now) || (now > stop);

	if (!do_p)
	  /* Leave only the next insn in av_vliw.  */
	  {
	    av_set_iterator av_it;
	    rhs_t expr;
	    bnd_t bnd = BLIST_BND (bnds);
	    insn_t next = BND_TO (bnd);

	    gcc_assert (BLIST_NEXT (bnds) == NULL);

	    FOR_EACH_RHS_1 (expr, av_it, &av_vliw)
	      if (RHS_INSN (expr) != next)
		av_set_iter_remove (&av_it);
	  }
      }

      /* Now we've computed AV_VLIW - the set of expressions that can be
	 scheduled on FENCE.  */

      line_start ();
      print ("av_vliw: ");
      dump_av_set (av_vliw);
      line_finish ();

      if (av_vliw == NULL)
        break;

      /* Choose the best expression and, if needed, destination register
	 for it.  */
      find_best_rhs_and_reg_that_fits (&av_vliw, bnds, fence, &rhs_vliw);

      if (!rhs_vliw)
        {
          line_start ();
          print ("no best rhs found!");
          line_finish ();

          /* Reorder* hooks told us nothing more to schedule; indicate that
             a stall is needed.  */
          if (can_issue_more == 0)
            need_stall = 1;

	  av_set_clear (&av_vliw);
          break;
        }

      line_start ();
      print ("rhs_vliw: ");
      dump_rhs (rhs_vliw);
      line_finish ();

      bndsp = &bnds;
      bnds_tailp1 = bnds_tailp;
      do
	/* !!! This code is guaranteed to execute only once.  */
        {
	  bnd_t bnd = BLIST_BND (*bndsp);
	  av_set_t rhs_seq = NULL;
	  rhs_t rhs;
	  av_set_iterator i;
	  insn_t *succs;
	  int succs_n;
	  bool asm_p;
	  bool first_p = true;

	  if (!av_set_is_in_p (BND_AV1 (bnd), RHS_VINSN (rhs_vliw)))
	    {
	      bndsp = &BLIST_NEXT (*bndsp);
	      bnds_tail1 = *bnds_tailp1;
	      continue;
	    }

	  FOR_EACH_RHS (rhs, i, BND_AV (bnd))
            {
              ilist_t root = enable_moveup_set_path_p ? BND_PTR (bnd) : NULL;

              if (equal_after_moveup_path_p (rhs, root, rhs_vliw))
		{
		  gcc_assert (first_p);
		  first_p = false;

                  /* The sequential expression has the right form to pass 
                     to move_op except when renaming happened.  Put the 
                     correct register in RHS then.  */
                  if (EXPR_SEPARABLE_P (rhs) && REG_P (EXPR_LHS (rhs))
                      && rhs_dest_regno (rhs) != rhs_dest_regno (rhs_vliw))
                    replace_dest_with_reg_in_rhs (rhs, EXPR_LHS (rhs_vliw));

		  av_set_add (&rhs_seq, rhs);
		}
            }

          line_start ();
          print ("rhs_seq: ");
          dump_av_set (rhs_seq);
          line_finish ();

	  /* Move chosen insn.  */
	  {
	    insn_t place_to_insert;
	    insn_t new_bb_head = NULL_RTX;
	    expr_def _c_rhs, *c_rhs = &_c_rhs;
	    bool b;

	    /* Init place_to_insert before calling move_op, as the later
	       can possibly remove BND_TO (bnd).  */
	    if (/* If this is not the first insn scheduled.  */
		BND_PTR (bnd))
	      {
		gcc_unreachable ();

		/* Add it after last scheduled.  */
		place_to_insert = ILIST_INSN (BND_PTR (bnd));
	      }
	    else
	      /* Add it before BND_TO.  The difference is in the
		 basic block, where INSN will be added.  */
	      place_to_insert = PREV_INSN (BND_TO (bnd));

	    sel_dump_cfg ("before-move_op");

	    /* Marker is useful to bind .dot dumps and the log.  */
	    print_marker_to_log ();

	    /* Make a move.  This call will remove the original operation,
	       insert all necessary bookkeeping instructions and update the
	       data sets.  After that all we have to do is add the operation
	       at before BND_TO (BND).  */
	    b = move_op (BND_TO (bnd), rhs_seq, NULL, NULL, NULL, c_rhs);

	    /* We should be able to find the expression we've chosen for 
	       scheduling.  */
	    gcc_assert (b);

            /* We want to use a pattern from rhs_vliw, because it could've 
               been substituted, and the rest of data from rhs_seq.  */
            if (! rtx_equal_p (EXPR_PATTERN (rhs_vliw), 
                               EXPR_PATTERN (c_rhs)))
              change_vinsn_in_expr (c_rhs, EXPR_VINSN (rhs_vliw));

	    /* Find a place for C_RHS to schedule.
	       We want to have an invariant that only insns that are
	       sel_bb_header_p () have a valid LV_SET.  But, in the same time,
	       we don't want overhead from recomputation of compute_live ()
	       for the half of a block after each movement.  Resolution of
	       this is floating bb header that will advance along with the
	       fence.

	       Please note that the invariant is an implication: e.g. there
	       can be sel_bb_header_p () insns that don't have a valid LV_SET.
	       To make an equivalence out of implication we need to invoke
	       compute_live () after scheduling of an insn that become
	       sel_bb_header_p () - the overhead will be insignificant because
	       this case is only possible when we start scheduling of a new
	       basic block.  Also I've just thought about another concerning
	       issue:
	       suppose we have a function from a single insn.  So far we've
	       stripped that insn from the stream in move_op () - and, hence,
	       deleted the only valid LV_SET - how are we supposed to get a
	       valid LV_SET for the inserted insn out of nowhere?  */

	    {
	      insn_t prev_insn = PREV_INSN (place_to_insert);
	      basic_block bb = BLOCK_FOR_INSN (place_to_insert);
	      basic_block prev_bb = bb->prev_bb;
	      basic_block next_bb;

	      if (!NOTE_INSN_BASIC_BLOCK_P (place_to_insert)
		  || prev_insn == NULL_RTX
		  /* Or it is a label, a barrier or something strange
		     alike.  */
		  || !INSN_P (prev_insn)
		  || BLOCK_FOR_INSN (prev_insn) != prev_bb
		  || !in_current_region_p (prev_bb)
		  || control_flow_insn_p (prev_insn))
		{
		  prev_bb = bb;

		  /* Save new_bb_head to update lv_set on.  */
		  if (!NOTE_INSN_BASIC_BLOCK_P (place_to_insert)
		      && !sel_bb_end_p (place_to_insert))
		    new_bb_head = NEXT_INSN (place_to_insert);

		  /* Split block to generate a new floating bb header.  */
		  next_bb = sel_split_block (bb, place_to_insert);
		}
	      else
		{
		  gcc_assert (single_succ (prev_bb) == bb);

		  place_to_insert = prev_insn;
		  next_bb = bb;
		}

	      if (sel_bb_empty_p (next_bb))
		sel_merge_blocks (prev_bb, next_bb);

	      gcc_assert (BLOCK_FOR_INSN (place_to_insert) == prev_bb);

	      /* Now do some cleanup: remove empty basic blocks after
		 BB.  */

	      next_bb = prev_bb->next_bb;

	      /* !!! Can't use bb_empty_p here because it returns true on
		 empty blocks with labels.  */
	      while (BB_HEAD (next_bb) == BB_END (next_bb)
		     && in_current_region_p (next_bb))
		{
		  bb = next_bb->next_bb;

		  sel_remove_empty_bb (next_bb, true, true);
		  next_bb = bb;
		}
	    }

	    /* Add the instruction.  */
	    insn = gen_insn_from_expr_after (c_rhs, seqno, place_to_insert);
	    clear_expr (c_rhs);

	    ++INSN_SCHED_TIMES (insn);

	    if (NOTE_INSN_BASIC_BLOCK_P (place_to_insert))
	      {
		gcc_assert (new_bb_head == NULL_RTX);
		new_bb_head = insn;
	      }

            /* Initialize LV_SET of the bb header.  */
	    if (new_bb_head != NULL_RTX)
	      {
		/* !!! TODO: We should replace all occurencies of
		   LV_SET_VALID_P () with LV_SET () != NULL.  Overwise it is
		   not clear what a valid and invalid lv set is.  */

		if (LV_SET (new_bb_head) == NULL)
		  LV_SET (new_bb_head) = get_clear_regset_from_pool ();

		ignore_first = true;
		compute_live (new_bb_head);
	      }
	  }

	  av_set_clear (&rhs_seq);

	  /* Advance the DFA.  */
	  if (recog_memoized (insn) >= 0)
	    {
	      gcc_assert (!INSN_ASM_P (insn));
	      asm_p = false;

              memcpy (temp_state, FENCE_STATE (fence), dfa_state_size);

	      succs_n = state_transition (FENCE_STATE (fence), insn);
	      gcc_assert (succs_n < 0);

              if (memcmp (temp_state, FENCE_STATE (fence), dfa_state_size))
                {
                  FENCE_ISSUED_INSNS (fence)++;
                  /* We should never issue more than issue_rate insns.  */
                  if (FENCE_ISSUED_INSNS (fence) > issue_rate)
                    gcc_unreachable ();
                }
	    } 
          else
            {
              asm_p = INSN_ASM_P (insn);
              
              /* This could be an ASM insn which we'd like to schedule 
                 on the next cycle.  */
              if (!FENCE_STARTS_CYCLE_P (fence) && asm_p)
		advance_one_cycle (fence);
            }

	  /* Set instruction scheduling info.  This will be used in bundling,
	     pipelining, tick computations etc.  */

          memcpy (temp_state, FENCE_STATE (fence), dfa_state_size);
          INSN_AFTER_STALL_P (insn) = FENCE_AFTER_STALL_P (fence);
	  INSN_SCHED_CYCLE (insn) = FENCE_CYCLE (fence);

	  if (asm_p)
	    advance_one_cycle (fence);

          FENCE_AFTER_STALL_P (fence) = 0;
          FENCE_STARTS_CYCLE_P (fence) = 0;
	  FENCE_LAST_SCHEDULED_INSN (fence) = insn;
          if (SCHED_GROUP_P (insn))
            {
              FENCE_SCHED_NEXT (fence) = INSN_SCHED_NEXT (insn);
              SCHED_GROUP_P (insn) = 0;
            }
          else
            FENCE_SCHED_NEXT (fence) = NULL_RTX;

	  advance_deps_context (BND_DC (bnd), insn);

	  /* Add insn to the list of scheduled on this cycle instructions.  */
	  ilist_add (*scheduled_insns_tailpp, insn);
	  *scheduled_insns_tailpp = &ILIST_NEXT (**scheduled_insns_tailpp);

	  line_start ();
	  print ("Scheduling insn: ");
	  dump_insn_1 (insn, 1);
	  line_finish ();

	  /* Add new boundaries.  */
	  cfg_succs_1 (insn, SUCCS_NORMAL | SUCCS_SKIP_TO_LOOP_EXITS, 
		       &succs, &succs_n);
	  while (succs_n--)
	    {
	      insn_t succ = succs[succs_n];
	      ilist_t ptr = ilist_copy (BND_PTR (bnd));

	      ilist_add (&ptr, insn);

	      blist_add (bnds_tailp, succ, ptr, FENCE_DC (fence));
	      bnds_tailp = &BLIST_NEXT (*bnds_tailp);
	    }
	  free (succs);

	  bnds_tail1 = *bnds_tailp1;
	  blist_remove (bndsp);

	  /* Check invariants.  */

	  /* Check that the recent movement didn't destroyed loop
	     structure.  */
	  gcc_assert (!flag_sel_sched_pipelining_outer_loops
		      || current_loop_nest == NULL
		      || loop_latch_edge (current_loop_nest));
        }
      while (*bndsp != bnds_tail1);

      av_set_clear (&av_vliw);

      /* Indicate that we've scheduled something on this fence.  */
      FENCE_SCHEDULED_SOMETHING (fence) = true;

      /* When can_issue_more is 0, variable_issue tells us that we should
        advance a cycle.  */
      if (can_issue_more == 0)
       {
         need_stall = 1;
         break;
       }

      /* We currently support information about candidate blocks only for
	 one 'target_bb' block.  Hence we can't schedule after jump insn,
	 as this will bring two boundaries and, hence, necessity to handle
	 information for two or more blocks concurrently.  */
      if (insn == BB_END (BLOCK_FOR_INSN (insn)))
	  break;

      /* !!! This is a possible perfomance regression as we schedule one
	 instruction at a time because of floating bb headers.  We need to
	 change the above condition to control_flow_insn_p (insn) or completely
	 remove it at all.  */
      gcc_unreachable ();
    }
  while (bnds);

  gcc_assert (!FENCE_BNDS (fence));
  
  if (need_stall)
    {
      /* Did we do all we could?  If no, we have a stall.  */ 
      need_stall = FENCE_ISSUED_INSNS (fence) < issue_rate;
      /* First advance a cycle, in case we'd clear after_stall_p flag, 
         and then set it.  */ 
      advance_one_cycle (fence);
      if (need_stall)
        FENCE_AFTER_STALL_P (fence) = 1;
      need_stall = 0;
    }

  /* Update boundaries of the FENCE.  */
  while (bnds)
    {
      ilist_t ptr = BND_PTR (BLIST_BND (bnds));

      if (ptr)
	{
	  insn = ILIST_INSN (ptr);

	  if (!ilist_is_in_p (FENCE_BNDS (fence), insn))
	    ilist_add (&FENCE_BNDS (fence), insn);
	}
      
      blist_remove (&bnds);
    }

  /* Update target context on the fence.  */
  reset_target_context (FENCE_TC (fence), false);

  block_finish ();
}

/* All exprs in ORIG_OPS must have the same destination register.
   Return that register.  */
static rtx
get_dest_reg_from_orig_ops (av_set_t orig_ops)
{
  rtx reg = NULL_RTX;
  av_set_iterator av_it;
  expr_t expr;
  bool first_p = true;

  FOR_EACH_RHS (expr, av_it, orig_ops)
    {
      rtx x = expr_dest_reg (expr);

      if (first_p)
	{
	  first_p = false;
	  reg = x;
	}
      else
	gcc_assert (reg == x
		    || (reg != NULL_RTX && x != NULL_RTX
			&& REGNO (reg) == REGNO (x)));
    }

  return reg;
}

/* Move up the operations from ORIG_OPS set traversing 
   the dag started from INSN.  PATH represents the edges traversed so far.
   REG is the register chosen for scheduling the current rhs.  Insert
   bookkeeping code in the join points.  Return the current rhs.  */
static bool
move_op (insn_t insn, av_set_t orig_ops, ilist_t path, edge e1, edge e2,
	 rhs_t c_rhs)
{
  rhs_t rhs;
  basic_block bb;
  bool c_rhs_inited_p;
  rtx reg;
  
  line_start ();
  print ("move_op(");
  dump_insn (insn);
  print (",");
  dump_av_set (orig_ops);
  print (")");
  line_finish ();
  block_start ();

  /* If no original operations exist below this insn, return immediately.  */
  if (!orig_ops || is_ineligible_successor (insn, path))
    {
      print ("ineligible_successor");
      block_finish ();
      return false;
    }

  orig_ops = av_set_copy (orig_ops);

  /* If we've found valid av set, then filter the orig_ops set.  */
  if (INSN_AV_VALID_P (insn))
    {
      line_start ();
      print ("av");
      dump_insn (insn);
      print ("=");
      dump_av_set (AV_SET (insn));
      line_finish ();

      av_set_intersect (&orig_ops, AV_SET (insn));

      /* If no more original ops, return immediately.  */
      if (!orig_ops)
	{
	  print ("NULL");
	  block_finish ();
	  return false;
	}

      /* !!! When using different types of speculation we must not leave
	 just one element in orig_ops.  */
      /*av_set_leave_one (&orig_ops);*/
    }

  reg = get_dest_reg_from_orig_ops (orig_ops);

  /* Look at the insn and decide if it could be an ancestor of currently 
     scheduling operation.  If it is so, then the insn "dest = op" could 
     either be replaced with "dest = reg", because REG now holds the result
     of OP, or just removed, if we've scheduled the insn as a whole.  

     If this insn doesn't contain currently scheduling OP, then proceed
     with searching and look at its successors.  Operations we're searching 
     for could have changed when moving up through this insn via 
     substituting.  In this case, firstly perform unsubstitution on them. 

     When traversing the DAG below this insn is finished, insert bookkeeping 
     code, if the insn is a joint point, and remove leftovers.  */

  rhs = av_set_lookup (orig_ops, INSN_VI (insn));

  if (rhs != NULL)
    /* We have found the original operation.  Replace it by REG, if 
       it is scheduled as RHS, or just remove it later, if it's an insn.  */
    {
      copy_expr_onside (c_rhs, INSN_EXPR (insn));
      c_rhs_inited_p = true;

      /* When an insn we found is not equal to the insn from the orig_ops 
         set, this means that we've found previously created bookeeping copy.
         We cannot return original insn in C_RHS because it was already
         scheduled, and using it would break an assert saying that 
         insn should not be in stream.  We need to replace it in C_RHS with 
         the exact insn that we found.

	 Our infrastructure handles this such case with setting AV_LEVEL ()
	 to -1 in generate_bookkeeping_insn ().

	 But when there is identical instructions one of which dominates the
	 other we have the same assert failing.  */
      if (RHS_INSN (c_rhs) != insn)
	{
	  /* We now copy INSN_EXPR (insn) to C_RHS, hence the condition is
	     always false.  */
	  gcc_unreachable ();

	  change_vinsn_in_expr (c_rhs, INSN_VI (insn));
	}
      
      /* For instructions we must immediately remove insn from the
	 stream, so subsequent update_data_sets () won't include this
	 insn into av_set.
	 For rhs we must make insn look like "INSN_REG (insn) := c_rhs".  */

      print ("found original operation!");

      {
	insn_t finish_insn = insn;

	{
	  ds_t check_ds = get_spec_check_type_for_insn (insn, rhs);

	  if (check_ds != 0)
	    /* A speculation check should be inserted.  */
	    {
	      rtx x;

	      x = create_speculation_check_insn_rtx (EXPR_INSN_RTX (rhs),
						     check_ds);
	      x = sel_gen_insn_from_rtx_after (x,
					       INSN_EXPR (finish_insn),
					       INSN_SEQNO (finish_insn),
					       finish_insn);

	      EXPR_SPEC_DONE_DS (INSN_EXPR (x)) &= ~check_ds;

	      finish_insn = x;

	      /* If we've generated a data speculation check, make sure
		 that all the bookkeeping instruction we'll create during
		 this move_op () will allocate an ALAT entry so that the
		 check won't fail.
		 ??? We should ask target if somethings needs to be done
		 here.  */
	      /* Commented out due to PR23.  */
	      /*speculate_expr (c_rhs, ds_get_max_dep_weak (check_ds));*/
	    }
	  else
	    EXPR_SPEC_DONE_DS (INSN_EXPR (finish_insn)) = 0;

	  gcc_assert (EXPR_SPEC_DONE_DS (INSN_EXPR (finish_insn)) == 0
		      && EXPR_SPEC_TO_CHECK_DS (INSN_EXPR (finish_insn)) == 0);
	}

	{
	  rtx cur_reg = expr_dest_reg (c_rhs);

	  gcc_assert ((reg == NULL_RTX) == (cur_reg == NULL_RTX));

	  /* If original operation has rhs and the register chosen for
	     that rhs is not original operation's dest reg, substitute
	     operation's right hand side with the register chosen.  */
	  if (reg != NULL_RTX && REGNO (reg) != REGNO (cur_reg))
	    {
	      rtx insn_rtx;
	      insn_t x;

	      replace_dest_with_reg_in_rhs (c_rhs, reg);

	      insn_rtx = create_insn_rtx_with_rhs (INSN_VI (insn), reg);
	      x = sel_gen_insn_from_rtx_after (insn_rtx,
					       INSN_EXPR (finish_insn),
					       INSN_SEQNO (finish_insn),
					       finish_insn);

	      finish_insn = x;
	    }
	}


	if (insn == finish_insn)
	  /* For the insns that don't have rhs just remove insn from the
	     stream.  Also remove insn if substituting it's right hand 
	     side would result in operation like reg:=reg.  This kind of
	     operation is not only excessive, but it may not be supported 
	     on certain platforms, e.g. "mov si, si" is invalid on i386.  */
	  finish_insn = get_nop_from_pool (insn);

	{
	  insn_t new_start_insn = NEXT_INSN (insn);

	  transfer_data_sets (new_start_insn, insn);
	  sched_sel_remove_insn (insn);

	  insn = new_start_insn;
	}
      }
    }
  else
    {
      succ_iterator succ_i;
      rtx succ;

      c_rhs_inited_p = false;

      if (reg != NULL_RTX && lhs_of_insn_equals_to_reg_p (insn, reg))
	/* If we're scheduling separate rhs, in order to generate correct code
	   we need to stop the search at bookkeeping code generated with the 
	   same destination register. This clearing of orig_ops makes 
	   scheduler to choose another destination register for the rhs.  
	   Instructions that are moved as the whole not affected with this
	   issue because the different insns could be never given the same
	   register, as it can happen when dealing with separate rhses.
	   .
	   !!! Change the detection of bookkeeping insns to support
	   clonable but not separable insns.  */
	av_set_clear (&orig_ops);
      else
        /* Av set ops could have been changed when moving through this insn.
           To find them below it, we have to un-substitute them.  */
        undo_transformations (&orig_ops, insn);

      /* If all original opernads have been filtered on this branch,
	 return.  */
      if (!orig_ops)
	{
	  block_finish ();
	  return false;
	}

      /* Continue searching.  Do recursion here.  */
      ilist_add (&path, insn);

      FOR_EACH_SUCC (succ, succ_i, insn)
        {
	  bool b;
	  expr_def _x, *x = &_x;

	  if (succ_i.e1)
	    {
	      gcc_assert (succ_i.e2);

	      gcc_assert (succ_i.e1->src == BLOCK_FOR_INSN (insn)
			  && succ_i.e2->dest == BLOCK_FOR_INSN (succ));
	    }

	  b = move_op (succ, orig_ops, path, succ_i.e1, succ_i.e2, x);

          if (b)
	    {
              enum MOVEUP_RHS_CODE res;
              
              res = moveup_rhs (x, insn, false);
              gcc_assert (res != MOVEUP_RHS_NULL);

	      if (!c_rhs_inited_p)
		{
		  copy_expr_onside (c_rhs, x);
		  c_rhs_inited_p = true;
		}
	      else
		/* We must merge all found expressions to get reasonable
		   EXPR_SPEC_DONE_DS () for the resulting insn.  If we don't
		   do so then we can first find the expr with epsilon
		   speculation success probability and only then with the
		   good probability.  As a result the insn will get epsilon
		   probability and will never be scheduled because of
		   weakness_cutoff in find_best_rhs_and_reg_that_fits ().

		   We also workaround this in can_overcome_dep_p ()
		   that consider low probability speculation success
		   dependencies as hard ones.

		   We call merge_expr_data () here instead of merge_expr ()
		   because due to speculation C_RHS and X may have the
		   same insns with different speculation types.  And as of
		   now such insns are considered non-correlable.  */
		merge_expr_data (c_rhs, x);

	      clear_expr (x);
	    }
	}

      ilist_remove (&path);
    }

  av_set_clear (&orig_ops);

  if (!c_rhs_inited_p)
    {
      block_finish ();
      return false;
    }

  /* We should generate bookkeeping code only if we are not at the
     top level of the move_op.  */
  if (e1 && num_preds_gt_1 (insn))
    {
      /* INSN is a joint point, insert bookkeeping code here.  */
      bb = generate_bookkeeping_insn (c_rhs, insn, e1, e2);
      gcc_assert (sel_bb_header_p (insn));
    }
  else
    bb = NULL;

  if (sel_bb_header_p (insn))
    {
      if (AV_LEVEL (insn) == -1)
	/* This will make assert in update_data_sets () happy.  */
	AV_LEVEL (insn) = global_level;
      else
	gcc_assert (INSN_AV_VALID_P (insn));
    }

  if (sel_bb_header_p (insn))
    {
      update_data_sets (insn);

      if (bb)
	{
	  /* Make assertion in update_data_sets () happy.  */
	  AV_LEVEL (NEXT_INSN (bb_note (bb))) = global_level;

	  /* We created an extra block in generate_bookkeeping_insn ().
	     Initialize av_set for it.  */
	  update_data_sets (NEXT_INSN (bb_note (bb)));
	}
    }
  else
    {
      gcc_assert (!LV_SET_VALID_P (insn)
		  && !INSN_AV_VALID_P (insn));
    }

  /* If INSN was previously marked for deletion, it's time to do it.  */
  if (INSN_NOP_P (insn))
    {
      bool transfered_p = false;

      if (insn == BB_END (BLOCK_FOR_INSN (insn)))
	{
	  succ_iterator succ_i;
	  insn_t succ;

	  FOR_EACH_SUCC (succ, succ_i, insn)
	    /* NB: We can't assert that SUCC has valid AV_SET because SUCC
	       can be an ineligible successor of INSN.  */
	    gcc_assert (LV_SET_VALID_P (succ));
	}

      if (sel_bb_header_p (insn))
	{
	  gcc_assert (LV_SET_VALID_P (insn));

	  if (insn == BB_END (BLOCK_FOR_INSN (insn)))
	    /* We are about to remove the only insn in the block -
	       delete its LV_SET.  */
	    {
	      return_regset_to_pool (LV_SET (insn));
	      LV_SET (insn) = NULL;
	    }
	  else
	    {
	      transfer_data_sets (cfg_succ (insn), insn);
	      transfered_p = true;
	    }
	}

      if (!transfered_p)
	{
	  av_set_clear (&AV_SET (insn));
	  AV_LEVEL (insn) = 0;

	  gcc_assert (!LV_SET_VALID_P (insn));
	}

      return_nop_to_pool (insn);
    }

  block_finish ();
  return c_rhs_inited_p;
}


/* Functions that work with regions.  */

/* Current number of seqno used in init_seqno and init_seqno_1.  */
static int cur_seqno;

/* A helper for init_seqno.  Traverse the region starting from BB and 
   compute seqnos for visited insns, marking visited bbs in VISITED_BBS.  */
static void
init_seqno_1 (basic_block bb, sbitmap visited_bbs)
{
  int bbi = BLOCK_TO_BB (bb->index);
  insn_t insn, note = bb_note (bb);
  insn_t succ_insn;
  succ_iterator si;


  SET_BIT (visited_bbs, bbi);

  FOR_EACH_SUCC_1 (succ_insn, si, BB_END (bb), 
		   SUCCS_NORMAL | SUCCS_SKIP_TO_LOOP_EXITS)
    {
      basic_block succ = BLOCK_FOR_INSN (succ_insn);
      int succ_bbi = BLOCK_TO_BB (succ->index);

      gcc_assert (in_current_region_p (succ));

      if (!TEST_BIT (visited_bbs, succ_bbi))
	{
	  gcc_assert (succ_bbi > bbi);

	  init_seqno_1 (succ, visited_bbs);
	}
    }

  for (insn = BB_END (bb); insn != note; insn = PREV_INSN (insn))
    INSN_SEQNO (insn) = cur_seqno--;
}

/* Initialize seqnos for the current region.  */
static int
init_seqno (bool rescheduling_p)
{
  sbitmap visited_bbs;

  visited_bbs = sbitmap_alloc (current_nr_blocks);
  sbitmap_zero (visited_bbs);

  cur_seqno = sched_max_luid - 1;

  init_seqno_1 (EBB_FIRST_BB (0), visited_bbs);

  gcc_assert (rescheduling_p || cur_seqno == 0);

  sbitmap_free (visited_bbs);

  return sched_max_luid - 1;
}

/* If region is a loop, add an empty basic block before its head, so no
   bookkeeping instructions will be generated on boundary edges.
   Return true if current region is a loop.  */
static bool
add_region_head (void)
{
  basic_block region_head = BASIC_BLOCK (BB_TO_BLOCK (0));

  /* We don't want to pipeline weird loops.  */
  if (EDGE_COUNT (region_head->preds) != 2
      || !(EDGE_PRED (region_head, 0)->flags & EDGE_FALLTHRU)
      || (EDGE_PRED (region_head, 1)->flags & EDGE_FALLTHRU))
    return false;
  else
    {
      basic_block fallthru_pred = EDGE_PRED (region_head, 0)->src;
      basic_block jump_pred = EDGE_PRED (region_head, 1)->src;
      basic_block new_region_head;

      if (in_current_region_p (fallthru_pred)
	  || !in_current_region_p (jump_pred))
	return false;

      can_add_real_insns_p = false;
      new_region_head = sel_create_basic_block_before (region_head);
      can_add_real_insns_p = true;

      RGN_WAS_PIPELINED_P (CONTAINING_RGN (BB_TO_BLOCK (0))) = 1;

      glat_start[new_region_head->index] = glat_start[region_head->index];
      glat_start[region_head->index] = NULL;

      return true;
    }
}

/* Split all edges incoming to current region, but not those that 
   come to loop header, and not those that come to preheader.  */
static void
split_edges_incoming_to_rgn (void)
{
  int i;
  int cur_rgn = CONTAINING_RGN (BB_TO_BLOCK (0));
  edge e;
  VEC(edge, heap) *edges_to_split = NULL;

  if (!current_loop_nest)
    return;

  for (i = 0; i < RGN_NR_BLOCKS (cur_rgn); i++)
    {
      edge_iterator ei;
      basic_block bb;
      bool has_preds_in_rgn;

      bb = BASIC_BLOCK (BB_TO_BLOCK (i));

      /* Skip header, preheaders, and single pred blocks.  */
      if (bb == current_loop_nest->header)
        continue;
      if (bb->loop_depth < current_loop_nest->depth)
        continue;
      if (EDGE_COUNT (bb->preds) < 2)
        continue;

      /* Skip also blocks that don't have preds in the region.  */
      has_preds_in_rgn = false;
      FOR_EACH_EDGE (e, ei, bb->preds)
        if (in_current_region_p (e->src))
          {
            has_preds_in_rgn = true;
            break;
          }
      if (!has_preds_in_rgn)
        continue;

      /* Record all edges we need to split.  */
      FOR_EACH_EDGE (e, ei, bb->preds)
        if (!in_current_region_p (e->src))
          VEC_safe_push (edge, heap, edges_to_split, e);
    }
  
  for (i = 0; VEC_iterate (edge, edges_to_split, i, e); i++)
    /* Some of edges could be already redirected by previous splits.
       So check this again before calling sel_split_edge.  */
    if (!in_current_region_p (e->src))
      sel_split_edge (e);

  VEC_free (edge, heap, edges_to_split);
}

/* Save old RTL hooks here.  */
static struct rtl_hooks old_rtl_hooks;

/* Init scheduling data for RGN.  Returns true when this region should not 
   be scheduled.  */
static bool
sel_region_init (int rgn)
{
  int i;
  bb_vec_t bbs;

  rgn_setup_region (rgn);

  /* If this loop has any saved loop preheaders from nested loops,
     add these basic blocks to the current region.  */
  if (flag_sel_sched_pipelining_outer_loops)
    {
      current_loop_nest = get_loop_nest_for_rgn (rgn);
  
      if (current_loop_nest 
	  && LOOP_PREHEADER_BLOCKS (current_loop_nest))
        {
          sel_add_loop_preheader ();
          
          /* Check that we're starting with a valid information.  */
          gcc_assert (loop_latch_edge (current_loop_nest));
        }
    }

  bbs = VEC_alloc (basic_block, heap, current_nr_blocks);

  for (i = 0; i < current_nr_blocks; i++)
    VEC_quick_push (basic_block, bbs, BASIC_BLOCK (BB_TO_BLOCK (i)));

  sel_init_bbs (bbs, NULL);

  /* Initialize luids and dependence analysis which both sel-sched and haifa
     need.  */
  sched_init_luids (bbs, NULL, NULL, NULL);
  sched_deps_local_init (false);

  /* Initialize haifa data.  */
  {
    rgn_setup_sched_infos ();
    haifa_init_h_i_d (bbs, NULL, NULL, NULL);
  }

  if (sched_rgn_local_preinit (rgn))
    return true;

  /* Compute insn priorities in haifa style.  Then free haifa style 
     dependencies that we've calculated for this.  */
  compute_priorities ();

  if (sched_verbose >= 5)
    debug_dependencies ();

  init_deps_global ();

  /* Setup flags that might affect initialization.  */
  {
    /* We don't need the semantics of moveup_set_path, because filtering of 
       dependencies inside a sched group is handled by tick_check_p and 
       the target.  */
    enable_moveup_set_path_p 
      = flag_sel_sched_substitute_inside_insn_group != 0;

    /* We need to treat insns as RHSes only when renaming is enabled.  */
    enable_schedule_as_rhs_p = (flag_sel_sched_renaming != 0);

    bookkeeping_p = (flag_sel_sched_bookkeeping != 0);

    pipelining_p = bookkeeping_p && (flag_sel_sched_pipelining != 0);

    reset_sched_cycles_p = (pipelining_p
			    && !flag_sel_sched_reschedule_pipelined);
  }

  sel_setup_sched_infos ();

  /* Main initialization.  */
  sel_init_global_and_expr (bbs);

  /* Finalize haifa-specific data.  */
  {
    haifa_finish_h_i_d ();
  }

  VEC_free (basic_block, heap, bbs);

  /* Set hooks so that no newly generated insn will go out unnoticed.  */
  old_rtl_hooks = rtl_hooks;
  rtl_hooks = sel_rtl_hooks;

  /* Save create_basic_block () to use in sel_create_basic_block ().  */
  old_create_basic_block = rtl_cfg_hooks.create_basic_block;
  rtl_cfg_hooks.create_basic_block = sel_create_basic_block;

  old_delete_basic_block = rtl_cfg_hooks.delete_basic_block;
  rtl_cfg_hooks.delete_basic_block = rtl_delete_block_not_barriers;

  if (pipelining_p)
    {
      /* If pipelining of outer loops is enabled, the loop header is
	 already created with loop optimizer, so if current region
	 has a corresponding loop nest, we should pipeline it.  */
      if (flag_sel_sched_pipelining_outer_loops)
	{
	  pipelining_p = (current_loop_nest != NULL);

	  if (pipelining_p)
	    split_edges_incoming_to_rgn ();
	}
      else
	pipelining_p = add_region_head ();
    }

  setup_dump_cfg_params (pipelining_p);

  /* !!! We call target.sched.md_init () for the whole region, but we invoke
     targetm.sched.md_finish () for every ebb.  */
  if (targetm.sched.md_init)
    /* None of the arguments are actually used in any target.  */
    targetm.sched.md_init (sched_dump, sched_verbose, -1);

  first_emitted_uid = get_max_uid () + 1;

  /* Reset register allocation ticks array.  */
  memset (reg_rename_tick, 0, sizeof reg_rename_tick);

  bitmap_initialize (forced_ebb_heads, 0);
  bitmap_clear (forced_ebb_heads);

  setup_empty_vinsn ();

  return false;
}

/* Free the scheduling data for the current region.  */
static void
sel_region_finish (void)
{
  int i;

  sel_finish_new_insns ();

  sched_finish_ready_list ();

  free_nop_pool ();

  /* Free the sort vector.  */
  if (vec_av_set)
    {
      gcc_assert (VEC_empty (rhs_t, vec_av_set));
      VEC_free (rhs_t, heap, vec_av_set);
    }

  /* Emulate the Haifa scheduler for bundling.  */
  if (reload_completed && flag_schedule_emulate_haifa)
    {
      rtx insn, head, tail;
      int clock = 0, last_clock = 0;  
      bitmap scheduled_blocks;

      scheduled_blocks = BITMAP_ALLOC (NULL);

      for (i = 0; i < current_nr_blocks; i++)
        {
          basic_block bb = EBB_FIRST_BB (i), bb1;

          if (bitmap_bit_p (scheduled_blocks, i))
            continue;

	  /* While pipelining outer loops, skip bundling for loop 
             preheaders.  Those will be rescheduled in the outer loop.  */
	  if (sel_is_loop_preheader_p (bb))
	    continue;

          line_start ();
          print ("Finishing schedule in bbs: ");

          bb1 = bb;
          do
            {
              bitmap_set_bit (scheduled_blocks, BLOCK_TO_BB (bb1->index));
              print ("%d; ", bb1->index);
            }
          while (!bb_ends_ebb_p (bb1) && (bb1 = bb_next_bb (bb1)));

          line_finish ();

          get_ebb_head_tail (bb, bb1, &head, &tail);
 
          if (no_real_insns_p (head, tail))
            {
              continue;
            }

          current_sched_info->prev_head = PREV_INSN (head);
          current_sched_info->next_tail = NEXT_INSN (tail);
  
	  if (reset_sched_cycles_p)
	    {
	      int last_clock = 0;
	      int haifa_last_clock = -1;
	      int haifa_clock = 0;
	      insn_t next_tail = current_sched_info->next_tail;

	      if (targetm.sched.md_init)
		{
		  /* None of the arguments are actually used in any target.

		  NB: We should have md_reset () hook for cases like this.  */
		  targetm.sched.md_init (sched_dump, sched_verbose, -1);
		}

	      state_reset (curr_state);
	      advance_state (curr_state);

	      for (insn = head; insn != next_tail; insn = NEXT_INSN (insn))
		{
		  int cost, haifa_cost;
		  int sort_p;
		  bool asm_p;
		  int clock;

		  if (!INSN_P (insn))
		    continue;

		  asm_p = false;
		  clock = INSN_SCHED_CYCLE (insn);

		  cost = clock - last_clock;

		  /* Initialize HAIFA_COST.  */
		  if (recog_memoized (insn) < 0)
		    {
		      asm_p = INSN_ASM_P (insn);

		      if (asm_p)
			/* This is asm insn which *had* to be scheduled first
			   on the cycle.  */
			haifa_cost = 1;
		      else
			/* This is a use/clobber insn.  It should not change 
			   cost.  */
			haifa_cost = 0;
		    }
		  else
		    {
		      state_t tmp_state = alloca (dfa_state_size);

		      memcpy (tmp_state, curr_state, dfa_state_size);
		      haifa_cost = state_transition (tmp_state, insn);

		      print ("haifa_cost: %d", haifa_cost);
		      /* ??? We can't assert anything about cost here yet,
			 because sometimes our scheduler gets out of sync with
			 Haifa.
			 This is to be fixed.  */
		      if (haifa_cost == 0)
			haifa_cost = 1;
		      else if (haifa_cost < 0)
			haifa_cost = 0;
		    }

		  /* Stall for whatever cycles we've stalled before.  */
		  if (INSN_AFTER_STALL_P (insn) && cost > haifa_cost)
		    haifa_cost = cost;

		  if (haifa_cost > 0)
		    {
		      int i = haifa_cost;

		      while (i--)
			{
			  advance_state (curr_state);
			  print ("advance_state ()");
			}

		      haifa_clock += haifa_cost;
		    }
		  else
		    gcc_assert (haifa_cost == 0);

		  if (targetm.sched.dfa_new_cycle)
		    while (targetm.sched.dfa_new_cycle (sched_dump,
							sched_verbose,
							insn,
							haifa_last_clock,
							haifa_clock,
							&sort_p))
		      {
			advance_state (curr_state);
			haifa_clock++;
			print ("advance_state ()");
		      }

		  if (recog_memoized (insn) >= 0)
		    {
		      cost = state_transition (curr_state, insn);
		      gcc_assert (cost < 0);
		    }

		  if (targetm.sched.variable_issue)
		    targetm.sched.variable_issue (sched_dump, sched_verbose,
						  insn, 0);

		  INSN_SCHED_CYCLE (insn) = haifa_clock;

		  last_clock = clock;
		  haifa_last_clock = haifa_clock;
		}
	    }

          if (targetm.sched.md_init)
            /* None of the arguments are actually used in any target.

	       NB: We should have md_reset () hook for cases like this.  */
            targetm.sched.md_init (sched_dump, sched_verbose, -1);

          state_reset (curr_state);
          advance_state (curr_state);
          last_clock = -1;

          for (insn = head; insn != NEXT_INSN (tail);)
            {
              int cost;
  
              if (!INSN_P (insn))
                {
                  insn = NEXT_INSN (insn);
                  continue;
                }

              clock = INSN_SCHED_CYCLE (insn);
              cost = (last_clock == -1) ? 1 : clock - last_clock;
  
	      gcc_assert (cost >= 0);

	      line_start ();
	      print ("cost: %d\t", cost);
	      dump_insn_1 (insn, 1);
	      line_finish ();

              if (issue_rate > 1
                  && GET_CODE (PATTERN (insn)) != USE
                  && GET_CODE (PATTERN (insn)) != CLOBBER)
                {
                  if (reload_completed && cost > 0)
                    PUT_MODE (insn, TImode);

                  last_clock = clock;
                }

              insn = NEXT_INSN (insn);
            }

          if (targetm.sched.md_finish)
	    {
	      /* md_finish () can possibly emit new insns.  Move LV_SETs to
		 ones that happen to be emitted on bb header.  */
	      insn_init.what = INSN_INIT_WHAT_INSN;
	      targetm.sched.md_finish (sched_dump, sched_verbose);

	      /* Extend luids so that insns generated by the target will
		 get zero luid.  */
	      sched_init_luids (NULL, NULL, NULL, NULL);

	      insn_init.todo = INSN_INIT_TODO_MOVE_LV_SET_IF_BB_HEADER;
	      sel_init_new_insns ();
	    }
        }

      /* NB: Don't forget to deallocate SCHEDULED_BLOCKS.  */
      BITMAP_FREE (scheduled_blocks);
    }

  sel_finish_global_and_expr ();

  bitmap_clear (forced_ebb_heads);

  free_empty_vinsn ();

  finish_deps_global ();
  sched_deps_local_finish ();
  sched_finish_luids ();

  sel_finish_bbs ();

  rtl_cfg_hooks.delete_basic_block = old_delete_basic_block;
  rtl_cfg_hooks.create_basic_block = old_create_basic_block;

  rtl_hooks = old_rtl_hooks;

  /* Reset MAX_ISSUE_SIZE.  */
  max_issue_size = 0;
}


/* Functions that implement the scheduler driver.  */

/* The main driver for scheduling a region.  This function is responsible 
   for correct propagation of fences (i.e. scheduling points) and creating 
   a group of parallel insns at each of them.  It also supports 
   pipelining.  */
static void
sel_sched_region_2 (sel_sched_region_2_data_t data)
{
  int orig_max_seqno = data->orig_max_seqno;
  int highest_seqno_in_use = orig_max_seqno;

  while (fences)
    {
      flist_t fences1;
      struct flist_tail_def _new_fences, *new_fences = &_new_fences;
      int min_f, max_f, new_hs;
      ilist_t scheduled_insns = NULL;
      ilist_t *scheduled_insns_tailp = &scheduled_insns;
      ilist_iterator ii;
      insn_t insn;

      flist_tail_init (new_fences);

      line_start ();
      print ("fences: ");
      dump_flist (fences);
      line_finish ();
	  
      /* Calculate MIN_F and MAX_F.  */
      min_f = max_f = INSN_SEQNO (FENCE_INSN (FLIST_FENCE (fences)));
      fences1 = fences;
      while ((fences1 = FLIST_NEXT (fences1)))
	{
	  int seqno = INSN_SEQNO (FENCE_INSN (FLIST_FENCE (fences1)));

	  if (min_f > seqno)
	    min_f = seqno;
	  else if (max_f < seqno)
	    max_f = seqno;
	}

      fences1 = fences;
      do
	{
	  fence_t fence = NULL;
	  /* SEQNO is set to '0' to avoid 'uninitialized warning'.  */ 
	  int seqno = 0;
	  flist_t fences2 = fences;
	  bool first_p = true;
	  
	  /* Choose the next fence group to schedule.
	     NB: The fact, that insn can be scheduled only once
	     on the cycle is guaranteed by two properties:
	     1. seqnos of parallel groups decrease with each iteration.
	     2. If is_ineligible_successor () sees the larger seqno, it
	     checks if candidate insn is_in_current_fence_p ().  */
	  do
	    {
	      fence_t f = FLIST_FENCE (fences2);

	      if (!FENCE_SCHEDULED (f))
		{
		  int i = INSN_SEQNO (FENCE_INSN (f));

		  if (first_p || i > seqno)
		    {
		      seqno = i;
		      fence = f;

		      first_p = false;
		    }
		  else
		    /* ??? Seqnos of different groups should be different.  */
		    gcc_assert (1 || i != seqno);
		}
	    }
	  while ((fences2 = FLIST_NEXT (fences2)));

	  gcc_assert (fence);

	  /* As FENCE is nonnull, SEQNO is initialized.  */
	  seqno -= max_f + 1;
	  fill_insns (fence, seqno, &scheduled_insns_tailp);
	  FENCE_SCHEDULED (fence) = true;
	}
      /* This condition gives us the number of iterations equal to the number
	 of fence groups in fences.  */
      while ((fences1 = FLIST_NEXT (fences1)));

      line_start ();
      print ("existing fences: ");

      fences1 = fences;
      do
	{
	  fence_t fence = FLIST_FENCE (fences1);
	  insn_t insn;
	  state_t state = FENCE_STATE (fence);

	  if (/* This fence doesn't have any successors.  */
	      !FENCE_BNDS (fence))
	    {
	      if (!FENCE_SCHEDULED_SOMETHING (fence))
		/* Nothing was scheduled on this fence.  */
		{
		  int seqno;

		  insn = FENCE_INSN (fence);
		  seqno = INSN_SEQNO (insn);

		  gcc_assert (seqno > 0 && seqno <= orig_max_seqno);

                  print ("%d[%d] (not changed); ", 
                         INSN_UID (insn),
                         BLOCK_NUM (insn));

		  new_fences_add (new_fences,
				  insn, state, FENCE_DC (fence), 
                                  FENCE_TC (fence), 
                                  FENCE_LAST_SCHEDULED_INSN (fence), 
                                  FENCE_SCHED_NEXT (fence),
                                  FENCE_CYCLE (fence), 
				  FENCE_ISSUED_INSNS (fence),
                                  FENCE_STARTS_CYCLE_P (fence), 
                                  FENCE_AFTER_STALL_P (fence));

		  /* Null these fields so that they won't be freed twice.  */
		  FENCE_STATE (fence) = NULL;
		  FENCE_DC (fence) = NULL;
                  FENCE_TC (fence) = NULL;
		}

	      continue;
	    }

	  extract_new_fences_from (fence, new_fences, data);
	}
      while ((fences1 = FLIST_NEXT (fences1)));

      flist_clear (&fences);

      line_finish();

      fences = FLIST_TAIL_HEAD (new_fences);
      
      /* Actually, new_hs is the seqno of the instruction, that was
	 scheduled first (i.e. it is the first one in SCHEDULED_INSNS).  */
      if (scheduled_insns)
	{
	  new_hs = (INSN_SEQNO (ILIST_INSN (scheduled_insns))
		    + highest_seqno_in_use + max_f - min_f + 2);
	  gcc_assert (new_hs > highest_seqno_in_use);
	}
      else
	new_hs = highest_seqno_in_use;

      FOR_EACH_INSN (insn, ii, scheduled_insns)
	{
	  gcc_assert (INSN_SEQNO (insn) < 0);
	  INSN_SEQNO (insn) += highest_seqno_in_use + max_f - min_f + 2;
	  gcc_assert (INSN_SEQNO (insn) <= new_hs);
	}
      ilist_clear (&scheduled_insns);

      highest_seqno_in_use = new_hs;

      global_level++;
    }

  gcc_assert (data->orig_max_seqno == orig_max_seqno);
  data->highest_seqno_in_use = highest_seqno_in_use;
}

/* Schedule a region.  When pipelining, search for possibly never scheduled 
   bookkeeping code and schedule it.  Reschedule pipelined code without 
   pipelining after.  */
static void
sel_sched_region_1 (void)
{
  struct sel_sched_region_2_data_def _data, *data = &_data;

  data->orig_max_seqno = init_seqno (false);

  if (data->orig_max_seqno < 1)
    {
      print ("empty region.");
      return;
    }

  fences = NULL;

  if (data->orig_max_seqno >= 1)
    {
      /* When pipelining outer loops, create fences on the loop header,
	 not preheader.  */
      if (current_loop_nest)
	init_fences (current_loop_nest->header);
      else
	init_fences (EBB_FIRST_BB (0));
    }

  global_level = 1;
  stage = 1;

  sel_sched_region_2 (data);

  gcc_assert (fences == NULL);

  if (pipelining_p)
    {
      int i;
      insn_t head;
      basic_block bb;
      struct flist_tail_def _new_fences;
      flist_tail_t new_fences = &_new_fences;

      pipelining_p = false;

      if (!flag_sel_sched_reschedule_pipelined)
        {
          /* Schedule newly created code, that has not been scheduled yet.  */
          bool do_p = true;

          while (do_p)
            {
              do_p = false;

              for (i = 0; i < current_nr_blocks; i++)
                {
                  bb = EBB_FIRST_BB (i);
                  head = sel_bb_header (bb);

                  /* While pipelining outer loops, skip bundling for loop 
                     preheaders.  Those will be rescheduled in the outer
		     loop.  */
                  if (sel_is_loop_preheader_p (bb))
                    {
                      clear_outdated_rtx_info (bb);
                      continue;
                    }
                  
                  if (head != NULL_RTX && INSN_SCHED_CYCLE (head) <= 0)
                    {
                      gcc_assert (INSN_SCHED_CYCLE (head) == 0);
                      flist_tail_init (new_fences);
  
                      /* Allow start of the scheduling at the code that was
                         generated during previous stages.  */
                      data->orig_max_seqno = data->highest_seqno_in_use;
  
                      /* Mark BB as head of the new ebb.  */
                      bitmap_set_bit (forced_ebb_heads, bb->index);
  
                      gcc_assert (fences == NULL);
  
                      init_fences (bb);
  
                      sel_sched_region_2 (data);
  
                      do_p = true;
                      break;
                    }
                }
            }
	}
      else
        {
          basic_block loop_entry;

          /* Schedule region pre-header first, if not pipelining 
             outer loops.  */
          bb = EBB_FIRST_BB (0);
          head = sel_bb_header (bb);
          
          if (sel_is_loop_preheader_p (bb))          
            /* Don't leave old flags on insns in bb.  */
            clear_outdated_rtx_info (bb);
          else if (head != NULL_RTX)
            {
              gcc_assert (INSN_SCHED_CYCLE (head) == 0);
              flist_tail_init (new_fences);

              /* Allow start of the scheduling at the code that was
                 generated during previous stages.  */
              data->orig_max_seqno = data->highest_seqno_in_use;

              /* Mark BB as head of the new ebb.  */
              bitmap_set_bit (forced_ebb_heads, bb->index);

              gcc_assert (fences == NULL);

              init_fences (bb);

              sel_sched_region_2 (data);
            }

          /* Reschedule pipelined code without pipelining.  */
          loop_entry = EBB_FIRST_BB (1);

	  /* ??? Why don't we assert that EBB_FIRST_BB (1) is an
	     actual loop entry?  There must be something wrong if we
	     somehow created an extra block before the loop.  */
          while (loop_entry && EDGE_COUNT (loop_entry->preds) == 1)
            loop_entry = loop_entry->next_bb;

          gcc_assert (loop_entry && EDGE_COUNT (loop_entry->preds) == 2);

          for (i = BLOCK_TO_BB (loop_entry->index); i < current_nr_blocks; i++)
            {
              insn_t insn, next_tail;

              bb = EBB_FIRST_BB (i);
              get_ebb_head_tail (bb, bb, &head, &next_tail);

              if (head == NULL_RTX || !INSN_P (head))
                continue;

              next_tail = NEXT_INSN (next_tail);

              /* Clear outdated information.  */
              for (insn = head;
                   insn != next_tail;
                   insn = NEXT_INSN (insn))
                {
                  gcc_assert (INSN_P (insn));
		  INSN_AFTER_STALL_P (insn) = 0;
                  INSN_SCHED_CYCLE (insn) = 0;

		  /* ??? Should we reset those counters which reside in
		     INSN_EXPR field (e.g. SPEC and SCHED_TIMES)?  */
		  /* For now we do need to zero SCHED_TIMES because we don't
		     want to skip dependencies from any instruction.  This
		     will be a subject to consider when we implement better
		     dependency tracking.  */
		  INSN_SCHED_TIMES (insn) = 0;
                }
            }

          data->orig_max_seqno = init_seqno (true);
          flist_tail_init (new_fences);

          /* Mark BB as head of the new ebb.  */
          bitmap_set_bit (forced_ebb_heads, loop_entry->index);

          gcc_assert (fences == NULL);

          init_fences (loop_entry);

          sel_sched_region_2 (data);
        }

      pipelining_p = true;
    }
}

/* Schedule the RGN region.  */
void
sel_sched_region (int rgn)
{
  if (sel_region_init (rgn))
    return;

  sel_dump_cfg ("after-region-init");

  print ("sel_sched_region: start");
  block_start ();

  {
    /* Decide if we want to schedule this region.  */
    int region;
    int region_start;
    int region_stop;
    bool region_p;
    bool schedule_p;
    
    region = ++sel_sched_region_run;
    region_start = PARAM_VALUE (PARAM_REGION_START);
    region_stop = PARAM_VALUE (PARAM_REGION_STOP);
    region_p = (PARAM_VALUE (PARAM_REGION_P) == 1);

    if (region_p)
      schedule_p = (region_start <= region) && (region <= region_stop);
    else
      schedule_p = (region_start > region) || (region > region_stop);

    if (schedule_p)
      sel_sched_region_1 ();
    else
      /* Force initialization of INSN_SCHED_CYCLEs for correct bundling.  */
      reset_sched_cycles_p = true;
  }

  sel_region_finish ();

  sel_dump_cfg_1 ("after-region-finish",
		  SEL_DUMP_CFG_CURRENT_REGION | SEL_DUMP_CFG_LV_SET
		  | SEL_DUMP_CFG_BB_INSNS);

  block_finish ();
  print ("sel_sched_region: finish");
}

/* Perform global init for the scheduler.  */
static void
sel_global_init (void)
{
  /* Pipelining outer loops is only possible when general pipelining
     capabilities are requested.  
     FIXME: move this in opts.c.  */
  if (!flag_sel_sched_pipelining)
    flag_sel_sched_pipelining_outer_loops = 0;

  if (flag_sel_sched_pipelining_outer_loops)
    pipeline_outer_loops_init ();

  setup_sched_dump_to_stderr ();

  /* Setup the only info sched_init () needs.  */
  sel_setup_common_sched_info ();

  /* We want to create new pseudos occasionally.  */
  if (!reload_completed)
    no_new_pseudos = 0;

  sched_init ();

  /* Init lv_sets.  */
  {
    bb_vec_t bbs = VEC_alloc (basic_block, heap, n_basic_blocks);
    basic_block bb;

    FOR_ALL_BB (bb)
      VEC_quick_push (basic_block, bbs, bb);

    sched_init_bbs (bbs, NULL);

    VEC_free (basic_block, heap, bbs);
  }

  sched_rgn_init (flag_sel_sched_single_block_regions != 0,
                  flag_sel_sched_ebb_regions != 0);

  /* Init the set of all registers available in the function.  */
  INIT_REG_SET (sel_all_regs);

  {
    int i;

    for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
      {
	if (regs_ever_live[i]
	    || call_used_regs[i])
	  SET_REGNO_REG_SET (sel_all_regs, i);
      }
  }

  setup_nop_and_exit_insns ();

  sel_extend_insn_rtx_data ();

  init_lv_sets ();
}

/* Free the global data of the scheduler.  */
static void
sel_global_finish (void)
{
  free_lv_sets ();

  sel_finish_insn_rtx_data ();

  free_nop_and_exit_insns ();

  free_regset_pool ();

  CLEAR_REG_SET (sel_all_regs);

  sched_rgn_finish ();
  sched_finish_bbs ();
  sched_finish ();

  if (!reload_completed)
    {
      /* We need to update the life information, because we can add pseudos.
         However, the actual update was already done in sched_rgn_finish.  */
      no_new_pseudos = 1;
    }

  free_sel_dump_data ();

  if (flag_sel_sched_pipelining_outer_loops)
    pipeline_outer_loops_finish ();
}

/* The entry point.  */
void
selective_scheduling_run (void)
{
  int rgn;
  int old_sched_verbose_param = sched_verbose_param;
  int sel_sched_verbose = PARAM_VALUE (PARAM_SELSCHED_VERBOSE);

  /* Taking care of this degenerate case makes the rest of
     this code simpler.  */
  if (n_basic_blocks == NUM_FIXED_BLOCKS)
    return;

  if (flag_sel_sched_verbose)
    sel_sched_verbose = 32;

  if (sel_sched_verbose)
    sched_verbose_param = sel_sched_verbose;

  setup_dump_cfg_params (false);

  sel_dump_cfg_1 ("before-init",
		  (SEL_DUMP_CFG_BB_INSNS | SEL_DUMP_CFG_FUNCTION_NAME
		   | SEL_DUMP_CFG_BB_LIVE));

  sel_global_init ();

  block_start ();
  print ("selective_scheduling_run: start");
  block_start ();

  for (rgn = 0; rgn < nr_regions; rgn++)
    {
      char *buf;
      int buf_len = 1 + snprintf (NULL, 0, "before-region-%d", rgn);

      buf = xmalloc (buf_len * sizeof (*buf));
      snprintf (buf, buf_len, "before-region-%d", rgn);
      sel_dump_cfg_1 (buf, SEL_DUMP_CFG_LV_SET | SEL_DUMP_CFG_BB_INSNS);

      sel_sched_region (rgn);

      snprintf (buf, buf_len, "after-region-%d", rgn);
      sel_dump_cfg_1 (buf, SEL_DUMP_CFG_LV_SET | SEL_DUMP_CFG_BB_INSNS);
      free (buf);
    }

  block_finish ();
  print ("selective_scheduling_run: finish");
  block_finish ();

  sel_global_finish ();

  sel_dump_cfg_1 ("after-finish",
		  (SEL_DUMP_CFG_BB_INSNS | SEL_DUMP_CFG_FUNCTION_NAME
		   | SEL_DUMP_CFG_BB_LIVE));

  sched_verbose_param = old_sched_verbose_param;
}

#endif

/* A gate function for selective scheduling.  */
static bool
gate_handle_sel_sched (void)
{
#ifdef INSN_SCHEDULING
  return (reload_completed 
          ? flag_selective_scheduling2 && flag_schedule_insns_after_reload
          : flag_selective_scheduling && flag_schedule_insns);
#else
  return false;
#endif
}

static int sel1_run = 0;

/* Run instruction scheduler.  */
static unsigned int
handle_sel_sched (void)
{
  if (reload_completed)
    split_all_insns (1);
#ifdef INSN_SCHEDULING
  {
    int now;
    int start;
    int stop;
    bool do_p;

    now = ++sel1_run;
    start = PARAM_VALUE (PARAM_SEL1_START);
    stop = PARAM_VALUE (PARAM_SEL1_STOP);
    do_p = (PARAM_VALUE (PARAM_SEL1_P) == 1);

    if (do_p)
      do_p = (start <= now) && (now <= stop);
    else
      do_p = (start > now) || (now > stop);

    if ((flag_selective_scheduling || flag_selective_scheduling2) && do_p)
      selective_scheduling_run ();
    else
      schedule_insns ();
  }
#endif
  return 0;
}

struct tree_opt_pass pass_sel_sched =
  {
    "sel_sched",                          /* name */
    gate_handle_sel_sched,                /* gate */
    handle_sel_sched,                 	/* execute */
    NULL,                                 /* sub */
    NULL,                                 /* next */
    0,                                    /* static_pass_number */
    TV_SEL_SCHED,                         /* tv_id */
    0,                                    /* properties_required */
    0,                                    /* properties_provided */
    0,                                    /* properties_destroyed */
    0,                                    /* todo_flags_start */
    TODO_dump_func |
    TODO_ggc_collect,                     /* todo_flags_finish */
    'S'                                   /* letter */
  };

