/* Perform simple optimizations to clean up the result of reload.
   Copyright (C) 1987, 1988, 1989, 1992, 1993, 1994, 1995, 1996, 1997,
   1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009,
   2010 Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 3, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"

#include "machmode.h"
#include "hard-reg-set.h"
#include "rtl.h"
#include "tm_p.h"
#include "obstack.h"
#include "insn-config.h"
#include "flags.h"
#include "function.h"
#include "expr.h"
#include "optabs.h"
#include "regs.h"
#include "basic-block.h"
#include "reload.h"
#include "recog.h"
#include "output.h"
#include "cselib.h"
#include "diagnostic-core.h"
#include "toplev.h"
#include "except.h"
#include "tree.h"
#include "timevar.h"
#include "tree-pass.h"
#include "df.h"
#include "dbgcnt.h"

static int reload_cse_noop_set_p (rtx);
static void reload_cse_simplify (rtx, rtx);
static void reload_cse_regs_1 (rtx);
static int reload_cse_simplify_set (rtx, rtx);
static int reload_cse_simplify_operands (rtx, rtx);

static void reload_combine (void);
static void reload_combine_note_use (rtx *, rtx);
static void reload_combine_note_store (rtx, const_rtx, void *);

static void reload_cse_move2add (rtx);
static void move2add_note_store (rtx, const_rtx, void *);

/* Call cse / combine like post-reload optimization phases.
   FIRST is the first instruction.  */
void
reload_cse_regs (rtx first ATTRIBUTE_UNUSED)
{
  reload_cse_regs_1 (first);
  reload_combine ();
  reload_cse_move2add (first);
  if (flag_expensive_optimizations)
    reload_cse_regs_1 (first);
}

/* See whether a single set SET is a noop.  */
static int
reload_cse_noop_set_p (rtx set)
{
  if (cselib_reg_set_mode (SET_DEST (set)) != GET_MODE (SET_DEST (set)))
    return 0;

  return rtx_equal_for_cselib_p (SET_DEST (set), SET_SRC (set));
}

/* Try to simplify INSN.  */
static void
reload_cse_simplify (rtx insn, rtx testreg)
{
  rtx body = PATTERN (insn);

  if (GET_CODE (body) == SET)
    {
      int count = 0;

      /* Simplify even if we may think it is a no-op.
         We may think a memory load of a value smaller than WORD_SIZE
         is redundant because we haven't taken into account possible
         implicit extension.  reload_cse_simplify_set() will bring
         this out, so it's safer to simplify before we delete.  */
      count += reload_cse_simplify_set (body, insn);

      if (!count && reload_cse_noop_set_p (body))
	{
	  rtx value = SET_DEST (body);
	  if (REG_P (value)
	      && ! REG_FUNCTION_VALUE_P (value))
	    value = 0;
	  delete_insn_and_edges (insn);
	  return;
	}

      if (count > 0)
	apply_change_group ();
      else
	reload_cse_simplify_operands (insn, testreg);
    }
  else if (GET_CODE (body) == PARALLEL)
    {
      int i;
      int count = 0;
      rtx value = NULL_RTX;

      /* Registers mentioned in the clobber list for an asm cannot be reused
	 within the body of the asm.  Invalidate those registers now so that
	 we don't try to substitute values for them.  */
      if (asm_noperands (body) >= 0)
	{
	  for (i = XVECLEN (body, 0) - 1; i >= 0; --i)
	    {
	      rtx part = XVECEXP (body, 0, i);
	      if (GET_CODE (part) == CLOBBER && REG_P (XEXP (part, 0)))
		cselib_invalidate_rtx (XEXP (part, 0));
	    }
	}

      /* If every action in a PARALLEL is a noop, we can delete
	 the entire PARALLEL.  */
      for (i = XVECLEN (body, 0) - 1; i >= 0; --i)
	{
	  rtx part = XVECEXP (body, 0, i);
	  if (GET_CODE (part) == SET)
	    {
	      if (! reload_cse_noop_set_p (part))
		break;
	      if (REG_P (SET_DEST (part))
		  && REG_FUNCTION_VALUE_P (SET_DEST (part)))
		{
		  if (value)
		    break;
		  value = SET_DEST (part);
		}
	    }
	  else if (GET_CODE (part) != CLOBBER)
	    break;
	}

      if (i < 0)
	{
	  delete_insn_and_edges (insn);
	  /* We're done with this insn.  */
	  return;
	}

      /* It's not a no-op, but we can try to simplify it.  */
      for (i = XVECLEN (body, 0) - 1; i >= 0; --i)
	if (GET_CODE (XVECEXP (body, 0, i)) == SET)
	  count += reload_cse_simplify_set (XVECEXP (body, 0, i), insn);

      if (count > 0)
	apply_change_group ();
      else
	reload_cse_simplify_operands (insn, testreg);
    }
}

/* Do a very simple CSE pass over the hard registers.

   This function detects no-op moves where we happened to assign two
   different pseudo-registers to the same hard register, and then
   copied one to the other.  Reload will generate a useless
   instruction copying a register to itself.

   This function also detects cases where we load a value from memory
   into two different registers, and (if memory is more expensive than
   registers) changes it to simply copy the first register into the
   second register.

   Another optimization is performed that scans the operands of each
   instruction to see whether the value is already available in a
   hard register.  It then replaces the operand with the hard register
   if possible, much like an optional reload would.  */

static void
reload_cse_regs_1 (rtx first)
{
  rtx insn;
  rtx testreg = gen_rtx_REG (VOIDmode, -1);

  cselib_init (CSELIB_RECORD_MEMORY);
  init_alias_analysis ();

  for (insn = first; insn; insn = NEXT_INSN (insn))
    {
      if (INSN_P (insn))
	reload_cse_simplify (insn, testreg);

      cselib_process_insn (insn);
    }

  /* Clean up.  */
  end_alias_analysis ();
  cselib_finish ();
}

/* Try to simplify a single SET instruction.  SET is the set pattern.
   INSN is the instruction it came from.
   This function only handles one case: if we set a register to a value
   which is not a register, we try to find that value in some other register
   and change the set into a register copy.  */

static int
reload_cse_simplify_set (rtx set, rtx insn)
{
  int did_change = 0;
  int dreg;
  rtx src;
  enum reg_class dclass;
  int old_cost;
  cselib_val *val;
  struct elt_loc_list *l;
#ifdef LOAD_EXTEND_OP
  enum rtx_code extend_op = UNKNOWN;
#endif
  bool speed = optimize_bb_for_speed_p (BLOCK_FOR_INSN (insn));

  dreg = true_regnum (SET_DEST (set));
  if (dreg < 0)
    return 0;

  src = SET_SRC (set);
  if (side_effects_p (src) || true_regnum (src) >= 0)
    return 0;

  dclass = REGNO_REG_CLASS (dreg);

#ifdef LOAD_EXTEND_OP
  /* When replacing a memory with a register, we need to honor assumptions
     that combine made wrt the contents of sign bits.  We'll do this by
     generating an extend instruction instead of a reg->reg copy.  Thus
     the destination must be a register that we can widen.  */
  if (MEM_P (src)
      && GET_MODE_BITSIZE (GET_MODE (src)) < BITS_PER_WORD
      && (extend_op = LOAD_EXTEND_OP (GET_MODE (src))) != UNKNOWN
      && !REG_P (SET_DEST (set)))
    return 0;
#endif

  val = cselib_lookup (src, GET_MODE (SET_DEST (set)), 0);
  if (! val)
    return 0;

  /* If memory loads are cheaper than register copies, don't change them.  */
  if (MEM_P (src))
    old_cost = memory_move_cost (GET_MODE (src), dclass, true);
  else if (REG_P (src))
    old_cost = register_move_cost (GET_MODE (src),
				   REGNO_REG_CLASS (REGNO (src)), dclass);
  else
    old_cost = rtx_cost (src, SET, speed);

  for (l = val->locs; l; l = l->next)
    {
      rtx this_rtx = l->loc;
      int this_cost;

      if (CONSTANT_P (this_rtx) && ! references_value_p (this_rtx, 0))
	{
#ifdef LOAD_EXTEND_OP
	  if (extend_op != UNKNOWN)
	    {
	      HOST_WIDE_INT this_val;

	      /* ??? I'm lazy and don't wish to handle CONST_DOUBLE.  Other
		 constants, such as SYMBOL_REF, cannot be extended.  */
	      if (!CONST_INT_P (this_rtx))
		continue;

	      this_val = INTVAL (this_rtx);
	      switch (extend_op)
		{
		case ZERO_EXTEND:
		  this_val &= GET_MODE_MASK (GET_MODE (src));
		  break;
		case SIGN_EXTEND:
		  /* ??? In theory we're already extended.  */
		  if (this_val == trunc_int_for_mode (this_val, GET_MODE (src)))
		    break;
		default:
		  gcc_unreachable ();
		}
	      this_rtx = GEN_INT (this_val);
	    }
#endif
	  this_cost = rtx_cost (this_rtx, SET, speed);
	}
      else if (REG_P (this_rtx))
	{
#ifdef LOAD_EXTEND_OP
	  if (extend_op != UNKNOWN)
	    {
	      this_rtx = gen_rtx_fmt_e (extend_op, word_mode, this_rtx);
	      this_cost = rtx_cost (this_rtx, SET, speed);
	    }
	  else
#endif
	    this_cost = register_move_cost (GET_MODE (this_rtx),
					    REGNO_REG_CLASS (REGNO (this_rtx)),
					    dclass);
	}
      else
	continue;

      /* If equal costs, prefer registers over anything else.  That
	 tends to lead to smaller instructions on some machines.  */
      if (this_cost < old_cost
	  || (this_cost == old_cost
	      && REG_P (this_rtx)
	      && !REG_P (SET_SRC (set))))
	{
#ifdef LOAD_EXTEND_OP
	  if (GET_MODE_BITSIZE (GET_MODE (SET_DEST (set))) < BITS_PER_WORD
	      && extend_op != UNKNOWN
#ifdef CANNOT_CHANGE_MODE_CLASS
	      && !CANNOT_CHANGE_MODE_CLASS (GET_MODE (SET_DEST (set)),
					    word_mode,
					    REGNO_REG_CLASS (REGNO (SET_DEST (set))))
#endif
	      )
	    {
	      rtx wide_dest = gen_rtx_REG (word_mode, REGNO (SET_DEST (set)));
	      ORIGINAL_REGNO (wide_dest) = ORIGINAL_REGNO (SET_DEST (set));
	      validate_change (insn, &SET_DEST (set), wide_dest, 1);
	    }
#endif

	  validate_unshare_change (insn, &SET_SRC (set), this_rtx, 1);
	  old_cost = this_cost, did_change = 1;
	}
    }

  return did_change;
}

/* Try to replace operands in INSN with equivalent values that are already
   in registers.  This can be viewed as optional reloading.

   For each non-register operand in the insn, see if any hard regs are
   known to be equivalent to that operand.  Record the alternatives which
   can accept these hard registers.  Among all alternatives, select the
   ones which are better or equal to the one currently matching, where
   "better" is in terms of '?' and '!' constraints.  Among the remaining
   alternatives, select the one which replaces most operands with
   hard registers.  */

static int
reload_cse_simplify_operands (rtx insn, rtx testreg)
{
  int i, j;

  /* For each operand, all registers that are equivalent to it.  */
  HARD_REG_SET equiv_regs[MAX_RECOG_OPERANDS];

  const char *constraints[MAX_RECOG_OPERANDS];

  /* Vector recording how bad an alternative is.  */
  int *alternative_reject;
  /* Vector recording how many registers can be introduced by choosing
     this alternative.  */
  int *alternative_nregs;
  /* Array of vectors recording, for each operand and each alternative,
     which hard register to substitute, or -1 if the operand should be
     left as it is.  */
  int *op_alt_regno[MAX_RECOG_OPERANDS];
  /* Array of alternatives, sorted in order of decreasing desirability.  */
  int *alternative_order;

  extract_insn (insn);

  if (recog_data.n_alternatives == 0 || recog_data.n_operands == 0)
    return 0;

  /* Figure out which alternative currently matches.  */
  if (! constrain_operands (1))
    fatal_insn_not_found (insn);

  alternative_reject = XALLOCAVEC (int, recog_data.n_alternatives);
  alternative_nregs = XALLOCAVEC (int, recog_data.n_alternatives);
  alternative_order = XALLOCAVEC (int, recog_data.n_alternatives);
  memset (alternative_reject, 0, recog_data.n_alternatives * sizeof (int));
  memset (alternative_nregs, 0, recog_data.n_alternatives * sizeof (int));

  /* For each operand, find out which regs are equivalent.  */
  for (i = 0; i < recog_data.n_operands; i++)
    {
      cselib_val *v;
      struct elt_loc_list *l;
      rtx op;

      CLEAR_HARD_REG_SET (equiv_regs[i]);

      /* cselib blows up on CODE_LABELs.  Trying to fix that doesn't seem
	 right, so avoid the problem here.  Likewise if we have a constant
         and the insn pattern doesn't tell us the mode we need.  */
      if (LABEL_P (recog_data.operand[i])
	  || (CONSTANT_P (recog_data.operand[i])
	      && recog_data.operand_mode[i] == VOIDmode))
	continue;

      op = recog_data.operand[i];
#ifdef LOAD_EXTEND_OP
      if (MEM_P (op)
	  && GET_MODE_BITSIZE (GET_MODE (op)) < BITS_PER_WORD
	  && LOAD_EXTEND_OP (GET_MODE (op)) != UNKNOWN)
	{
	  rtx set = single_set (insn);

	  /* We might have multiple sets, some of which do implicit
	     extension.  Punt on this for now.  */
	  if (! set)
	    continue;
	  /* If the destination is also a MEM or a STRICT_LOW_PART, no
	     extension applies.
	     Also, if there is an explicit extension, we don't have to
	     worry about an implicit one.  */
	  else if (MEM_P (SET_DEST (set))
		   || GET_CODE (SET_DEST (set)) == STRICT_LOW_PART
		   || GET_CODE (SET_SRC (set)) == ZERO_EXTEND
		   || GET_CODE (SET_SRC (set)) == SIGN_EXTEND)
	    ; /* Continue ordinary processing.  */
#ifdef CANNOT_CHANGE_MODE_CLASS
	  /* If the register cannot change mode to word_mode, it follows that
	     it cannot have been used in word_mode.  */
	  else if (REG_P (SET_DEST (set))
		   && CANNOT_CHANGE_MODE_CLASS (GET_MODE (SET_DEST (set)),
						word_mode,
						REGNO_REG_CLASS (REGNO (SET_DEST (set)))))
	    ; /* Continue ordinary processing.  */
#endif
	  /* If this is a straight load, make the extension explicit.  */
	  else if (REG_P (SET_DEST (set))
		   && recog_data.n_operands == 2
		   && SET_SRC (set) == op
		   && SET_DEST (set) == recog_data.operand[1-i])
	    {
	      validate_change (insn, recog_data.operand_loc[i],
			       gen_rtx_fmt_e (LOAD_EXTEND_OP (GET_MODE (op)),
					      word_mode, op),
			       1);
	      validate_change (insn, recog_data.operand_loc[1-i],
			       gen_rtx_REG (word_mode, REGNO (SET_DEST (set))),
			       1);
	      if (! apply_change_group ())
		return 0;
	      return reload_cse_simplify_operands (insn, testreg);
	    }
	  else
	    /* ??? There might be arithmetic operations with memory that are
	       safe to optimize, but is it worth the trouble?  */
	    continue;
	}
#endif /* LOAD_EXTEND_OP */
      v = cselib_lookup (op, recog_data.operand_mode[i], 0);
      if (! v)
	continue;

      for (l = v->locs; l; l = l->next)
	if (REG_P (l->loc))
	  SET_HARD_REG_BIT (equiv_regs[i], REGNO (l->loc));
    }

  for (i = 0; i < recog_data.n_operands; i++)
    {
      enum machine_mode mode;
      int regno;
      const char *p;

      op_alt_regno[i] = XALLOCAVEC (int, recog_data.n_alternatives);
      for (j = 0; j < recog_data.n_alternatives; j++)
	op_alt_regno[i][j] = -1;

      p = constraints[i] = recog_data.constraints[i];
      mode = recog_data.operand_mode[i];

      /* Add the reject values for each alternative given by the constraints
	 for this operand.  */
      j = 0;
      while (*p != '\0')
	{
	  char c = *p++;
	  if (c == ',')
	    j++;
	  else if (c == '?')
	    alternative_reject[j] += 3;
	  else if (c == '!')
	    alternative_reject[j] += 300;
	}

      /* We won't change operands which are already registers.  We
	 also don't want to modify output operands.  */
      regno = true_regnum (recog_data.operand[i]);
      if (regno >= 0
	  || constraints[i][0] == '='
	  || constraints[i][0] == '+')
	continue;

      for (regno = 0; regno < FIRST_PSEUDO_REGISTER; regno++)
	{
	  enum reg_class rclass = NO_REGS;

	  if (! TEST_HARD_REG_BIT (equiv_regs[i], regno))
	    continue;

	  SET_REGNO (testreg, regno);
	  PUT_MODE (testreg, mode);

	  /* We found a register equal to this operand.  Now look for all
	     alternatives that can accept this register and have not been
	     assigned a register they can use yet.  */
	  j = 0;
	  p = constraints[i];
	  for (;;)
	    {
	      char c = *p;

	      switch (c)
		{
		case '=':  case '+':  case '?':
		case '#':  case '&':  case '!':
		case '*':  case '%':
		case '0':  case '1':  case '2':  case '3':  case '4':
		case '5':  case '6':  case '7':  case '8':  case '9':
		case '<':  case '>':  case 'V':  case 'o':
		case 'E':  case 'F':  case 'G':  case 'H':
		case 's':  case 'i':  case 'n':
		case 'I':  case 'J':  case 'K':  case 'L':
		case 'M':  case 'N':  case 'O':  case 'P':
		case 'p':  case 'X':  case TARGET_MEM_CONSTRAINT:
		  /* These don't say anything we care about.  */
		  break;

		case 'g': case 'r':
		  rclass = reg_class_subunion[(int) rclass][(int) GENERAL_REGS];
		  break;

		default:
		  rclass
		    = (reg_class_subunion
		       [(int) rclass]
		       [(int) REG_CLASS_FROM_CONSTRAINT ((unsigned char) c, p)]);
		  break;

		case ',': case '\0':
		  /* See if REGNO fits this alternative, and set it up as the
		     replacement register if we don't have one for this
		     alternative yet and the operand being replaced is not
		     a cheap CONST_INT.  */
		  if (op_alt_regno[i][j] == -1
		      && reg_fits_class_p (testreg, rclass, 0, mode)
		      && (!CONST_INT_P (recog_data.operand[i])
			  || (rtx_cost (recog_data.operand[i], SET,
			  		optimize_bb_for_speed_p (BLOCK_FOR_INSN (insn)))
			      > rtx_cost (testreg, SET,
			  		optimize_bb_for_speed_p (BLOCK_FOR_INSN (insn))))))
		    {
		      alternative_nregs[j]++;
		      op_alt_regno[i][j] = regno;
		    }
		  j++;
		  rclass = NO_REGS;
		  break;
		}
	      p += CONSTRAINT_LEN (c, p);

	      if (c == '\0')
		break;
	    }
	}
    }

  /* Record all alternatives which are better or equal to the currently
     matching one in the alternative_order array.  */
  for (i = j = 0; i < recog_data.n_alternatives; i++)
    if (alternative_reject[i] <= alternative_reject[which_alternative])
      alternative_order[j++] = i;
  recog_data.n_alternatives = j;

  /* Sort it.  Given a small number of alternatives, a dumb algorithm
     won't hurt too much.  */
  for (i = 0; i < recog_data.n_alternatives - 1; i++)
    {
      int best = i;
      int best_reject = alternative_reject[alternative_order[i]];
      int best_nregs = alternative_nregs[alternative_order[i]];
      int tmp;

      for (j = i + 1; j < recog_data.n_alternatives; j++)
	{
	  int this_reject = alternative_reject[alternative_order[j]];
	  int this_nregs = alternative_nregs[alternative_order[j]];

	  if (this_reject < best_reject
	      || (this_reject == best_reject && this_nregs > best_nregs))
	    {
	      best = j;
	      best_reject = this_reject;
	      best_nregs = this_nregs;
	    }
	}

      tmp = alternative_order[best];
      alternative_order[best] = alternative_order[i];
      alternative_order[i] = tmp;
    }

  /* Substitute the operands as determined by op_alt_regno for the best
     alternative.  */
  j = alternative_order[0];

  for (i = 0; i < recog_data.n_operands; i++)
    {
      enum machine_mode mode = recog_data.operand_mode[i];
      if (op_alt_regno[i][j] == -1)
	continue;

      validate_change (insn, recog_data.operand_loc[i],
		       gen_rtx_REG (mode, op_alt_regno[i][j]), 1);
    }

  for (i = recog_data.n_dups - 1; i >= 0; i--)
    {
      int op = recog_data.dup_num[i];
      enum machine_mode mode = recog_data.operand_mode[op];

      if (op_alt_regno[op][j] == -1)
	continue;

      validate_change (insn, recog_data.dup_loc[i],
		       gen_rtx_REG (mode, op_alt_regno[op][j]), 1);
    }

  return apply_change_group ();
}

/* If reload couldn't use reg+reg+offset addressing, try to use reg+reg
   addressing now.
   This code might also be useful when reload gave up on reg+reg addressing
   because of clashes between the return register and INDEX_REG_CLASS.  */

/* The maximum number of uses of a register we can keep track of to
   replace them with reg+reg addressing.  */
#define RELOAD_COMBINE_MAX_USES 6

/* INSN is the insn where a register has been used, and USEP points to the
   location of the register within the rtl.  */
struct reg_use { rtx insn, *usep; };

/* If the register is used in some unknown fashion, USE_INDEX is negative.
   If it is dead, USE_INDEX is RELOAD_COMBINE_MAX_USES, and STORE_RUID
   indicates where it becomes live again.
   Otherwise, USE_INDEX is the index of the last encountered use of the
   register (which is first among these we have seen since we scan backwards),
   OFFSET contains the constant offset that is added to the register in
   all encountered uses, and USE_RUID indicates the first encountered, i.e.
   last, of these uses.
   STORE_RUID is always meaningful if we only want to use a value in a
   register in a different place: it denotes the next insn in the insn
   stream (i.e. the last encountered) that sets or clobbers the register.  */
static struct
  {
    struct reg_use reg_use[RELOAD_COMBINE_MAX_USES];
    int use_index;
    rtx offset;
    int store_ruid;
    int use_ruid;
  } reg_state[FIRST_PSEUDO_REGISTER];

/* Reverse linear uid.  This is increased in reload_combine while scanning
   the instructions from last to first.  It is used to set last_label_ruid
   and the store_ruid / use_ruid fields in reg_state.  */
static int reload_combine_ruid;

#define LABEL_LIVE(LABEL) \
  (label_live[CODE_LABEL_NUMBER (LABEL) - min_labelno])

static void
reload_combine (void)
{
  rtx insn, set;
  int first_index_reg = -1;
  int last_index_reg = 0;
  int i;
  basic_block bb;
  unsigned int r;
  int last_label_ruid;
  int min_labelno, n_labels;
  HARD_REG_SET ever_live_at_start, *label_live;

  /* If reg+reg can be used in offsetable memory addresses, the main chunk of
     reload has already used it where appropriate, so there is no use in
     trying to generate it now.  */
  if (double_reg_address_ok && INDEX_REG_CLASS != NO_REGS)
    return;

  /* To avoid wasting too much time later searching for an index register,
     determine the minimum and maximum index register numbers.  */
  for (r = 0; r < FIRST_PSEUDO_REGISTER; r++)
    if (TEST_HARD_REG_BIT (reg_class_contents[INDEX_REG_CLASS], r))
      {
	if (first_index_reg == -1)
	  first_index_reg = r;

	last_index_reg = r;
      }

  /* If no index register is available, we can quit now.  */
  if (first_index_reg == -1)
    return;

  /* Set up LABEL_LIVE and EVER_LIVE_AT_START.  The register lifetime
     information is a bit fuzzy immediately after reload, but it's
     still good enough to determine which registers are live at a jump
     destination.  */
  min_labelno = get_first_label_num ();
  n_labels = max_label_num () - min_labelno;
  label_live = XNEWVEC (HARD_REG_SET, n_labels);
  CLEAR_HARD_REG_SET (ever_live_at_start);

  FOR_EACH_BB_REVERSE (bb)
    {
      insn = BB_HEAD (bb);
      if (LABEL_P (insn))
	{
	  HARD_REG_SET live;
	  bitmap live_in = df_get_live_in (bb);

	  REG_SET_TO_HARD_REG_SET (live, live_in);
	  compute_use_by_pseudos (&live, live_in);
	  COPY_HARD_REG_SET (LABEL_LIVE (insn), live);
	  IOR_HARD_REG_SET (ever_live_at_start, live);
	}
    }

  /* Initialize last_label_ruid, reload_combine_ruid and reg_state.  */
  last_label_ruid = reload_combine_ruid = 0;
  for (r = 0; r < FIRST_PSEUDO_REGISTER; r++)
    {
      reg_state[r].store_ruid = reload_combine_ruid;
      if (fixed_regs[r])
	reg_state[r].use_index = -1;
      else
	reg_state[r].use_index = RELOAD_COMBINE_MAX_USES;
    }

  for (insn = get_last_insn (); insn; insn = PREV_INSN (insn))
    {
      rtx note;

      /* We cannot do our optimization across labels.  Invalidating all the use
	 information we have would be costly, so we just note where the label
	 is and then later disable any optimization that would cross it.  */
      if (LABEL_P (insn))
	last_label_ruid = reload_combine_ruid;
      else if (BARRIER_P (insn))
	for (r = 0; r < FIRST_PSEUDO_REGISTER; r++)
	  if (! fixed_regs[r])
	      reg_state[r].use_index = RELOAD_COMBINE_MAX_USES;

      if (! INSN_P (insn))
	continue;

      reload_combine_ruid++;

      /* Look for (set (REGX) (CONST_INT))
	 (set (REGX) (PLUS (REGX) (REGY)))
	 ...
	 ... (MEM (REGX)) ...
	 and convert it to
	 (set (REGZ) (CONST_INT))
	 ...
	 ... (MEM (PLUS (REGZ) (REGY)))... .

	 First, check that we have (set (REGX) (PLUS (REGX) (REGY)))
	 and that we know all uses of REGX before it dies.
	 Also, explicitly check that REGX != REGY; our life information
	 does not yet show whether REGY changes in this insn.  */
      set = single_set (insn);
      if (set != NULL_RTX
	  && REG_P (SET_DEST (set))
	  && (hard_regno_nregs[REGNO (SET_DEST (set))]
			      [GET_MODE (SET_DEST (set))]
	      == 1)
	  && GET_CODE (SET_SRC (set)) == PLUS
	  && REG_P (XEXP (SET_SRC (set), 1))
	  && rtx_equal_p (XEXP (SET_SRC (set), 0), SET_DEST (set))
	  && !rtx_equal_p (XEXP (SET_SRC (set), 1), SET_DEST (set))
	  && last_label_ruid < reg_state[REGNO (SET_DEST (set))].use_ruid)
	{
	  rtx reg = SET_DEST (set);
	  rtx plus = SET_SRC (set);
	  rtx base = XEXP (plus, 1);
	  rtx prev = prev_nonnote_insn (insn);
	  rtx prev_set = prev ? single_set (prev) : NULL_RTX;
	  unsigned int regno = REGNO (reg);
	  rtx index_reg = NULL_RTX;
	  rtx reg_sum = NULL_RTX;

	  /* Now we need to set INDEX_REG to an index register (denoted as
	     REGZ in the illustration above) and REG_SUM to the expression
	     register+register that we want to use to substitute uses of REG
	     (typically in MEMs) with.  First check REG and BASE for being
	     index registers; we can use them even if they are not dead.  */
	  if (TEST_HARD_REG_BIT (reg_class_contents[INDEX_REG_CLASS], regno)
	      || TEST_HARD_REG_BIT (reg_class_contents[INDEX_REG_CLASS],
				    REGNO (base)))
	    {
	      index_reg = reg;
	      reg_sum = plus;
	    }
	  else
	    {
	      /* Otherwise, look for a free index register.  Since we have
		 checked above that neither REG nor BASE are index registers,
		 if we find anything at all, it will be different from these
		 two registers.  */
	      for (i = first_index_reg; i <= last_index_reg; i++)
		{
		  if (TEST_HARD_REG_BIT (reg_class_contents[INDEX_REG_CLASS],
					 i)
		      && reg_state[i].use_index == RELOAD_COMBINE_MAX_USES
		      && reg_state[i].store_ruid <= reg_state[regno].use_ruid
		      && hard_regno_nregs[i][GET_MODE (reg)] == 1)
		    {
		      index_reg = gen_rtx_REG (GET_MODE (reg), i);
		      reg_sum = gen_rtx_PLUS (GET_MODE (reg), index_reg, base);
		      break;
		    }
		}
	    }

	  /* Check that PREV_SET is indeed (set (REGX) (CONST_INT)) and that
	     (REGY), i.e. BASE, is not clobbered before the last use we'll
	     create.  */
	  if (reg_sum
	      && prev_set
	      && CONST_INT_P (SET_SRC (prev_set))
	      && rtx_equal_p (SET_DEST (prev_set), reg)
	      && reg_state[regno].use_index >= 0
	      && (reg_state[REGNO (base)].store_ruid
		  <= reg_state[regno].use_ruid))
	    {
	      int i;

	      /* Change destination register and, if necessary, the constant
		 value in PREV, the constant loading instruction.  */
	      validate_change (prev, &SET_DEST (prev_set), index_reg, 1);
	      if (reg_state[regno].offset != const0_rtx)
		validate_change (prev,
				 &SET_SRC (prev_set),
				 GEN_INT (INTVAL (SET_SRC (prev_set))
					  + INTVAL (reg_state[regno].offset)),
				 1);

	      /* Now for every use of REG that we have recorded, replace REG
		 with REG_SUM.  */
	      for (i = reg_state[regno].use_index;
		   i < RELOAD_COMBINE_MAX_USES; i++)
		validate_unshare_change (reg_state[regno].reg_use[i].insn,
				 	 reg_state[regno].reg_use[i].usep,
				 	 /* Each change must have its own
				    	    replacement.  */
				 	 reg_sum, 1);

	      if (apply_change_group ())
		{
		  /* For every new use of REG_SUM, we have to record the use
		     of BASE therein, i.e. operand 1.  */
		  for (i = reg_state[regno].use_index;
		       i < RELOAD_COMBINE_MAX_USES; i++)
		    reload_combine_note_use
		      (&XEXP (*reg_state[regno].reg_use[i].usep, 1),
		       reg_state[regno].reg_use[i].insn);

		  if (reg_state[REGNO (base)].use_ruid
		      > reg_state[regno].use_ruid)
		    reg_state[REGNO (base)].use_ruid
		      = reg_state[regno].use_ruid;

		  /* Delete the reg-reg addition.  */
		  delete_insn (insn);

		  if (reg_state[regno].offset != const0_rtx)
		    /* Previous REG_EQUIV / REG_EQUAL notes for PREV
		       are now invalid.  */
		    remove_reg_equal_equiv_notes (prev);

		  reg_state[regno].use_index = RELOAD_COMBINE_MAX_USES;
		  reg_state[REGNO (index_reg)].store_ruid
		    = reload_combine_ruid;
		  continue;
		}
	    }
	}

      note_stores (PATTERN (insn), reload_combine_note_store, NULL);

      if (CALL_P (insn))
	{
	  rtx link;

	  for (r = 0; r < FIRST_PSEUDO_REGISTER; r++)
	    if (call_used_regs[r])
	      {
		reg_state[r].use_index = RELOAD_COMBINE_MAX_USES;
		reg_state[r].store_ruid = reload_combine_ruid;
	      }

	  for (link = CALL_INSN_FUNCTION_USAGE (insn); link;
	       link = XEXP (link, 1))
	    {
	      rtx usage_rtx = XEXP (XEXP (link, 0), 0);
	      if (REG_P (usage_rtx))
	        {
		  unsigned int i;
		  unsigned int start_reg = REGNO (usage_rtx);
		  unsigned int num_regs =
			hard_regno_nregs[start_reg][GET_MODE (usage_rtx)];
		  unsigned int end_reg  = start_reg + num_regs - 1;
		  for (i = start_reg; i <= end_reg; i++)
		    if (GET_CODE (XEXP (link, 0)) == CLOBBER)
		      {
		        reg_state[i].use_index = RELOAD_COMBINE_MAX_USES;
		        reg_state[i].store_ruid = reload_combine_ruid;
		      }
		    else
		      reg_state[i].use_index = -1;
	         }
	     }

	}
      else if (JUMP_P (insn)
	       && GET_CODE (PATTERN (insn)) != RETURN)
	{
	  /* Non-spill registers might be used at the call destination in
	     some unknown fashion, so we have to mark the unknown use.  */
	  HARD_REG_SET *live;

	  if ((condjump_p (insn) || condjump_in_parallel_p (insn))
	      && JUMP_LABEL (insn))
	    live = &LABEL_LIVE (JUMP_LABEL (insn));
	  else
	    live = &ever_live_at_start;

	  for (i = FIRST_PSEUDO_REGISTER - 1; i >= 0; --i)
	    if (TEST_HARD_REG_BIT (*live, i))
	      reg_state[i].use_index = -1;
	}

      reload_combine_note_use (&PATTERN (insn), insn);
      for (note = REG_NOTES (insn); note; note = XEXP (note, 1))
	{
	  if (REG_NOTE_KIND (note) == REG_INC
	      && REG_P (XEXP (note, 0)))
	    {
	      int regno = REGNO (XEXP (note, 0));

	      reg_state[regno].store_ruid = reload_combine_ruid;
	      reg_state[regno].use_index = -1;
	    }
	}
    }

  free (label_live);
}

/* Check if DST is a register or a subreg of a register; if it is,
   update reg_state[regno].store_ruid and reg_state[regno].use_index
   accordingly.  Called via note_stores from reload_combine.  */

static void
reload_combine_note_store (rtx dst, const_rtx set, void *data ATTRIBUTE_UNUSED)
{
  int regno = 0;
  int i;
  enum machine_mode mode = GET_MODE (dst);

  if (GET_CODE (dst) == SUBREG)
    {
      regno = subreg_regno_offset (REGNO (SUBREG_REG (dst)),
				   GET_MODE (SUBREG_REG (dst)),
				   SUBREG_BYTE (dst),
				   GET_MODE (dst));
      dst = SUBREG_REG (dst);
    }
  if (!REG_P (dst))
    return;
  regno += REGNO (dst);

  /* note_stores might have stripped a STRICT_LOW_PART, so we have to be
     careful with registers / register parts that are not full words.
     Similarly for ZERO_EXTRACT.  */
  if (GET_CODE (set) != SET
      || GET_CODE (SET_DEST (set)) == ZERO_EXTRACT
      || GET_CODE (SET_DEST (set)) == STRICT_LOW_PART)
    {
      for (i = hard_regno_nregs[regno][mode] - 1 + regno; i >= regno; i--)
	{
	  reg_state[i].use_index = -1;
	  reg_state[i].store_ruid = reload_combine_ruid;
	}
    }
  else
    {
      for (i = hard_regno_nregs[regno][mode] - 1 + regno; i >= regno; i--)
	{
	  reg_state[i].store_ruid = reload_combine_ruid;
	  reg_state[i].use_index = RELOAD_COMBINE_MAX_USES;
	}
    }
}

/* XP points to a piece of rtl that has to be checked for any uses of
   registers.
   *XP is the pattern of INSN, or a part of it.
   Called from reload_combine, and recursively by itself.  */
static void
reload_combine_note_use (rtx *xp, rtx insn)
{
  rtx x = *xp;
  enum rtx_code code = x->code;
  const char *fmt;
  int i, j;
  rtx offset = const0_rtx; /* For the REG case below.  */

  switch (code)
    {
    case SET:
      if (REG_P (SET_DEST (x)))
	{
	  reload_combine_note_use (&SET_SRC (x), insn);
	  return;
	}
      break;

    case USE:
      /* If this is the USE of a return value, we can't change it.  */
      if (REG_P (XEXP (x, 0)) && REG_FUNCTION_VALUE_P (XEXP (x, 0)))
	{
	/* Mark the return register as used in an unknown fashion.  */
	  rtx reg = XEXP (x, 0);
	  int regno = REGNO (reg);
	  int nregs = hard_regno_nregs[regno][GET_MODE (reg)];

	  while (--nregs >= 0)
	    reg_state[regno + nregs].use_index = -1;
	  return;
	}
      break;

    case CLOBBER:
      if (REG_P (SET_DEST (x)))
	{
	  /* No spurious CLOBBERs of pseudo registers may remain.  */
	  gcc_assert (REGNO (SET_DEST (x)) < FIRST_PSEUDO_REGISTER);
	  return;
	}
      break;

    case PLUS:
      /* We are interested in (plus (reg) (const_int)) .  */
      if (!REG_P (XEXP (x, 0))
	  || !CONST_INT_P (XEXP (x, 1)))
	break;
      offset = XEXP (x, 1);
      x = XEXP (x, 0);
      /* Fall through.  */
    case REG:
      {
	int regno = REGNO (x);
	int use_index;
	int nregs;

	/* No spurious USEs of pseudo registers may remain.  */
	gcc_assert (regno < FIRST_PSEUDO_REGISTER);

	nregs = hard_regno_nregs[regno][GET_MODE (x)];

	/* We can't substitute into multi-hard-reg uses.  */
	if (nregs > 1)
	  {
	    while (--nregs >= 0)
	      reg_state[regno + nregs].use_index = -1;
	    return;
	  }

	/* If this register is already used in some unknown fashion, we
	   can't do anything.
	   If we decrement the index from zero to -1, we can't store more
	   uses, so this register becomes used in an unknown fashion.  */
	use_index = --reg_state[regno].use_index;
	if (use_index < 0)
	  return;

	if (use_index != RELOAD_COMBINE_MAX_USES - 1)
	  {
	    /* We have found another use for a register that is already
	       used later.  Check if the offsets match; if not, mark the
	       register as used in an unknown fashion.  */
	    if (! rtx_equal_p (offset, reg_state[regno].offset))
	      {
		reg_state[regno].use_index = -1;
		return;
	      }
	  }
	else
	  {
	    /* This is the first use of this register we have seen since we
	       marked it as dead.  */
	    reg_state[regno].offset = offset;
	    reg_state[regno].use_ruid = reload_combine_ruid;
	  }
	reg_state[regno].reg_use[use_index].insn = insn;
	reg_state[regno].reg_use[use_index].usep = xp;
	return;
      }

    default:
      break;
    }

  /* Recursively process the components of X.  */
  fmt = GET_RTX_FORMAT (code);
  for (i = GET_RTX_LENGTH (code) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'e')
	reload_combine_note_use (&XEXP (x, i), insn);
      else if (fmt[i] == 'E')
	{
	  for (j = XVECLEN (x, i) - 1; j >= 0; j--)
	    reload_combine_note_use (&XVECEXP (x, i, j), insn);
	}
    }
}

/* See if we can reduce the cost of a constant by replacing a move
   with an add.  We track situations in which a register is set to a
   constant or to a register plus a constant.  */
/* We cannot do our optimization across labels.  Invalidating all the
   information about register contents we have would be costly, so we
   use move2add_last_label_luid to note where the label is and then
   later disable any optimization that would cross it.
   reg_offset[n] / reg_base_reg[n] / reg_symbol_ref[n] / reg_mode[n]
   are only valid if reg_set_luid[n] is greater than
   move2add_last_label_luid.  */
static int reg_set_luid[FIRST_PSEUDO_REGISTER];

/* If reg_base_reg[n] is negative, register n has been set to
   reg_offset[n] or reg_symbol_ref[n] + reg_offset[n] in mode reg_mode[n].
   If reg_base_reg[n] is non-negative, register n has been set to the
   sum of reg_offset[n] and the value of register reg_base_reg[n]
   before reg_set_luid[n], calculated in mode reg_mode[n] .  */
static HOST_WIDE_INT reg_offset[FIRST_PSEUDO_REGISTER];
static int reg_base_reg[FIRST_PSEUDO_REGISTER];
static rtx reg_symbol_ref[FIRST_PSEUDO_REGISTER];
static enum machine_mode reg_mode[FIRST_PSEUDO_REGISTER];

/* move2add_luid is linearly increased while scanning the instructions
   from first to last.  It is used to set reg_set_luid in
   reload_cse_move2add and move2add_note_store.  */
static int move2add_luid;

/* move2add_last_label_luid is set whenever a label is found.  Labels
   invalidate all previously collected reg_offset data.  */
static int move2add_last_label_luid;

/* ??? We don't know how zero / sign extension is handled, hence we
   can't go from a narrower to a wider mode.  */
#define MODES_OK_FOR_MOVE2ADD(OUTMODE, INMODE) \
  (GET_MODE_SIZE (OUTMODE) == GET_MODE_SIZE (INMODE) \
   || (GET_MODE_SIZE (OUTMODE) <= GET_MODE_SIZE (INMODE) \
       && TRULY_NOOP_TRUNCATION (GET_MODE_BITSIZE (OUTMODE), \
				 GET_MODE_BITSIZE (INMODE))))

/* This function is called with INSN that sets REG to (SYM + OFF),
   while REG is known to already have value (SYM + offset).
   This function tries to change INSN into an add instruction
   (set (REG) (plus (REG) (OFF - offset))) using the known value.
   It also updates the information about REG's known value.  */

static void
move2add_use_add2_insn (rtx reg, rtx sym, rtx off, rtx insn)
{
  rtx pat = PATTERN (insn);
  rtx src = SET_SRC (pat);
  int regno = REGNO (reg);
  rtx new_src = gen_int_mode (INTVAL (off) - reg_offset[regno],
			      GET_MODE (reg));
  bool speed = optimize_bb_for_speed_p (BLOCK_FOR_INSN (insn));

  /* (set (reg) (plus (reg) (const_int 0))) is not canonical;
     use (set (reg) (reg)) instead.
     We don't delete this insn, nor do we convert it into a
     note, to avoid losing register notes or the return
     value flag.  jump2 already knows how to get rid of
     no-op moves.  */
  if (new_src == const0_rtx)
    {
      /* If the constants are different, this is a
	 truncation, that, if turned into (set (reg)
	 (reg)), would be discarded.  Maybe we should
	 try a truncMN pattern?  */
      if (INTVAL (off) == reg_offset [regno])
	validate_change (insn, &SET_SRC (pat), reg, 0);
    }
  else if (rtx_cost (new_src, PLUS, speed) < rtx_cost (src, SET, speed)
	   && have_add2_insn (reg, new_src))
    {
      rtx tem = gen_rtx_PLUS (GET_MODE (reg), reg, new_src);
      validate_change (insn, &SET_SRC (pat), tem, 0);
    }
  else if (sym == NULL_RTX && GET_MODE (reg) != BImode)
    {
      enum machine_mode narrow_mode;
      for (narrow_mode = GET_CLASS_NARROWEST_MODE (MODE_INT);
	   narrow_mode != VOIDmode
	     && narrow_mode != GET_MODE (reg);
	   narrow_mode = GET_MODE_WIDER_MODE (narrow_mode))
	{
	  if (have_insn_for (STRICT_LOW_PART, narrow_mode)
	      && ((reg_offset[regno]
		   & ~GET_MODE_MASK (narrow_mode))
		  == (INTVAL (off)
		      & ~GET_MODE_MASK (narrow_mode))))
	    {
	      rtx narrow_reg = gen_rtx_REG (narrow_mode,
					    REGNO (reg));
	      rtx narrow_src = gen_int_mode (INTVAL (off),
					     narrow_mode);
	      rtx new_set =
		gen_rtx_SET (VOIDmode,
			     gen_rtx_STRICT_LOW_PART (VOIDmode,
						      narrow_reg),
			     narrow_src);
	      if (validate_change (insn, &PATTERN (insn),
				   new_set, 0))
		break;
	    }
	}
    }
  reg_set_luid[regno] = move2add_luid;
  reg_base_reg[regno] = -1;
  reg_mode[regno] = GET_MODE (reg);
  reg_symbol_ref[regno] = sym;
  reg_offset[regno] = INTVAL (off);
}


/* This function is called with INSN that sets REG to (SYM + OFF),
   but REG doesn't have known value (SYM + offset).  This function
   tries to find another register which is known to already have
   value (SYM + offset) and change INSN into an add instruction
   (set (REG) (plus (the found register) (OFF - offset))) if such
   a register is found.  It also updates the information about
   REG's known value.  */

static void
move2add_use_add3_insn (rtx reg, rtx sym, rtx off, rtx insn)
{
  rtx pat = PATTERN (insn);
  rtx src = SET_SRC (pat);
  int regno = REGNO (reg);
  int min_cost = INT_MAX;
  int min_regno = 0;
  bool speed = optimize_bb_for_speed_p (BLOCK_FOR_INSN (insn));
  int i;

  for (i = 0; i < FIRST_PSEUDO_REGISTER; i++)
    if (reg_set_luid[i] > move2add_last_label_luid
	&& reg_mode[i] == GET_MODE (reg)
	&& reg_base_reg[i] < 0
	&& reg_symbol_ref[i] != NULL_RTX
	&& rtx_equal_p (sym, reg_symbol_ref[i]))
      {
	rtx new_src = gen_int_mode (INTVAL (off) - reg_offset[i],
				    GET_MODE (reg));
	/* (set (reg) (plus (reg) (const_int 0))) is not canonical;
	   use (set (reg) (reg)) instead.
	   We don't delete this insn, nor do we convert it into a
	   note, to avoid losing register notes or the return
	   value flag.  jump2 already knows how to get rid of
	   no-op moves.  */
	if (new_src == const0_rtx)
	  {
	    min_cost = 0;
	    min_regno = i;
	    break;
	  }
	else
	  {
	    int cost = rtx_cost (new_src, PLUS, speed);
	    if (cost < min_cost)
	      {
		min_cost = cost;
		min_regno = i;
	      }
	  }
      }

  if (min_cost < rtx_cost (src, SET, speed))
    {
      rtx tem;

      tem = gen_rtx_REG (GET_MODE (reg), min_regno);
      if (i != min_regno)
	{
	  rtx new_src = gen_int_mode (INTVAL (off) - reg_offset[min_regno],
				      GET_MODE (reg));
	  tem = gen_rtx_PLUS (GET_MODE (reg), tem, new_src);
	}
      validate_change (insn, &SET_SRC (pat), tem, 0);
    }
  reg_set_luid[regno] = move2add_luid;
  reg_base_reg[regno] = -1;
  reg_mode[regno] = GET_MODE (reg);
  reg_symbol_ref[regno] = sym;
  reg_offset[regno] = INTVAL (off);
}

static void
reload_cse_move2add (rtx first)
{
  int i;
  rtx insn;

  for (i = FIRST_PSEUDO_REGISTER - 1; i >= 0; i--)
    {
      reg_set_luid[i] = 0;
      reg_offset[i] = 0;
      reg_base_reg[i] = 0;
      reg_symbol_ref[i] = NULL_RTX;
      reg_mode[i] = VOIDmode;
    }

  move2add_last_label_luid = 0;
  move2add_luid = 2;
  for (insn = first; insn; insn = NEXT_INSN (insn), move2add_luid++)
    {
      rtx pat, note;

      if (LABEL_P (insn))
	{
	  move2add_last_label_luid = move2add_luid;
	  /* We're going to increment move2add_luid twice after a
	     label, so that we can use move2add_last_label_luid + 1 as
	     the luid for constants.  */
	  move2add_luid++;
	  continue;
	}
      if (! INSN_P (insn))
	continue;
      pat = PATTERN (insn);
      /* For simplicity, we only perform this optimization on
	 straightforward SETs.  */
      if (GET_CODE (pat) == SET
	  && REG_P (SET_DEST (pat)))
	{
	  rtx reg = SET_DEST (pat);
	  int regno = REGNO (reg);
	  rtx src = SET_SRC (pat);

	  /* Check if we have valid information on the contents of this
	     register in the mode of REG.  */
	  if (reg_set_luid[regno] > move2add_last_label_luid
	      && MODES_OK_FOR_MOVE2ADD (GET_MODE (reg), reg_mode[regno])
              && dbg_cnt (cse2_move2add))
	    {
	      /* Try to transform (set (REGX) (CONST_INT A))
				  ...
				  (set (REGX) (CONST_INT B))
		 to
				  (set (REGX) (CONST_INT A))
				  ...
				  (set (REGX) (plus (REGX) (CONST_INT B-A)))
		 or
				  (set (REGX) (CONST_INT A))
				  ...
				  (set (STRICT_LOW_PART (REGX)) (CONST_INT B))
	      */

	      if (CONST_INT_P (src)
		  && reg_base_reg[regno] < 0
		  && reg_symbol_ref[regno] == NULL_RTX)
		{
		  move2add_use_add2_insn (reg, NULL_RTX, src, insn);
		  continue;
		}

	      /* Try to transform (set (REGX) (REGY))
				  (set (REGX) (PLUS (REGX) (CONST_INT A)))
				  ...
				  (set (REGX) (REGY))
				  (set (REGX) (PLUS (REGX) (CONST_INT B)))
		 to
				  (set (REGX) (REGY))
				  (set (REGX) (PLUS (REGX) (CONST_INT A)))
				  ...
				  (set (REGX) (plus (REGX) (CONST_INT B-A)))  */
	      else if (REG_P (src)
		       && reg_set_luid[regno] == reg_set_luid[REGNO (src)]
		       && reg_base_reg[regno] == reg_base_reg[REGNO (src)]
		       && MODES_OK_FOR_MOVE2ADD (GET_MODE (reg),
						 reg_mode[REGNO (src)]))
		{
		  rtx next = next_nonnote_insn (insn);
		  rtx set = NULL_RTX;
		  if (next)
		    set = single_set (next);
		  if (set
		      && SET_DEST (set) == reg
		      && GET_CODE (SET_SRC (set)) == PLUS
		      && XEXP (SET_SRC (set), 0) == reg
		      && CONST_INT_P (XEXP (SET_SRC (set), 1)))
		    {
		      rtx src3 = XEXP (SET_SRC (set), 1);
		      HOST_WIDE_INT added_offset = INTVAL (src3);
		      HOST_WIDE_INT base_offset = reg_offset[REGNO (src)];
		      HOST_WIDE_INT regno_offset = reg_offset[regno];
		      rtx new_src =
			gen_int_mode (added_offset
				      + base_offset
				      - regno_offset,
				      GET_MODE (reg));
		      bool success = false;
		      bool speed = optimize_bb_for_speed_p (BLOCK_FOR_INSN (insn));

		      if (new_src == const0_rtx)
			/* See above why we create (set (reg) (reg)) here.  */
			success
			  = validate_change (next, &SET_SRC (set), reg, 0);
		      else if ((rtx_cost (new_src, PLUS, speed)
				< COSTS_N_INSNS (1) + rtx_cost (src3, SET, speed))
			       && have_add2_insn (reg, new_src))
			{
			  rtx newpat = gen_rtx_SET (VOIDmode,
						    reg,
						    gen_rtx_PLUS (GET_MODE (reg),
						 		  reg,
								  new_src));
			  success
			    = validate_change (next, &PATTERN (next),
					       newpat, 0);
			}
		      if (success)
			delete_insn (insn);
		      insn = next;
		      reg_mode[regno] = GET_MODE (reg);
		      reg_offset[regno] =
			trunc_int_for_mode (added_offset + base_offset,
					    GET_MODE (reg));
		      continue;
		    }
		}
	    }

	  /* Try to transform
	     (set (REGX) (CONST (PLUS (SYMBOL_REF) (CONST_INT A))))
	     ...
	     (set (REGY) (CONST (PLUS (SYMBOL_REF) (CONST_INT B))))
	     to
	     (set (REGX) (CONST (PLUS (SYMBOL_REF) (CONST_INT A))))
	     ...
	     (set (REGY) (CONST (PLUS (REGX) (CONST_INT B-A))))  */
	  if ((GET_CODE (src) == SYMBOL_REF
	       || (GET_CODE (src) == CONST
		   && GET_CODE (XEXP (src, 0)) == PLUS
		   && GET_CODE (XEXP (XEXP (src, 0), 0)) == SYMBOL_REF
		   && CONST_INT_P (XEXP (XEXP (src, 0), 1))))
	      && dbg_cnt (cse2_move2add))
	    {
	      rtx sym, off;

	      if (GET_CODE (src) == SYMBOL_REF)
		{
		  sym = src;
		  off = const0_rtx;
		}
	      else
		{
		  sym = XEXP (XEXP (src, 0), 0);
		  off = XEXP (XEXP (src, 0), 1);
		}

	      /* If the reg already contains the value which is sum of
		 sym and some constant value, we can use an add2 insn.  */
	      if (reg_set_luid[regno] > move2add_last_label_luid
		  && MODES_OK_FOR_MOVE2ADD (GET_MODE (reg), reg_mode[regno])
		  && reg_base_reg[regno] < 0
		  && reg_symbol_ref[regno] != NULL_RTX
		  && rtx_equal_p (sym, reg_symbol_ref[regno]))
		move2add_use_add2_insn (reg, sym, off, insn);

	      /* Otherwise, we have to find a register whose value is sum
		 of sym and some constant value.  */
	      else
		move2add_use_add3_insn (reg, sym, off, insn);

	      continue;
	    }
	}

      for (note = REG_NOTES (insn); note; note = XEXP (note, 1))
	{
	  if (REG_NOTE_KIND (note) == REG_INC
	      && REG_P (XEXP (note, 0)))
	    {
	      /* Reset the information about this register.  */
	      int regno = REGNO (XEXP (note, 0));
	      if (regno < FIRST_PSEUDO_REGISTER)
		reg_set_luid[regno] = 0;
	    }
	}
      note_stores (PATTERN (insn), move2add_note_store, insn);

      /* If INSN is a conditional branch, we try to extract an
	 implicit set out of it.  */
      if (any_condjump_p (insn))
	{
	  rtx cnd = fis_get_condition (insn);

	  if (cnd != NULL_RTX
	      && GET_CODE (cnd) == NE
	      && REG_P (XEXP (cnd, 0))
	      && !reg_set_p (XEXP (cnd, 0), insn)
	      /* The following two checks, which are also in
		 move2add_note_store, are intended to reduce the
		 number of calls to gen_rtx_SET to avoid memory
		 allocation if possible.  */
	      && SCALAR_INT_MODE_P (GET_MODE (XEXP (cnd, 0)))
	      && hard_regno_nregs[REGNO (XEXP (cnd, 0))][GET_MODE (XEXP (cnd, 0))] == 1
	      && CONST_INT_P (XEXP (cnd, 1)))
	    {
	      rtx implicit_set =
		gen_rtx_SET (VOIDmode, XEXP (cnd, 0), XEXP (cnd, 1));
	      move2add_note_store (SET_DEST (implicit_set), implicit_set, insn);
	    }
	}

      /* If this is a CALL_INSN, all call used registers are stored with
	 unknown values.  */
      if (CALL_P (insn))
	{
	  for (i = FIRST_PSEUDO_REGISTER - 1; i >= 0; i--)
	    {
	      if (call_used_regs[i])
		/* Reset the information about this register.  */
		reg_set_luid[i] = 0;
	    }
	}
    }
}

/* SET is a SET or CLOBBER that sets DST.  DATA is the insn which
   contains SET.
   Update reg_set_luid, reg_offset and reg_base_reg accordingly.
   Called from reload_cse_move2add via note_stores.  */

static void
move2add_note_store (rtx dst, const_rtx set, void *data)
{
  rtx insn = (rtx) data;
  unsigned int regno = 0;
  unsigned int nregs = 0;
  unsigned int i;
  enum machine_mode mode = GET_MODE (dst);

  if (GET_CODE (dst) == SUBREG)
    {
      regno = subreg_regno_offset (REGNO (SUBREG_REG (dst)),
				   GET_MODE (SUBREG_REG (dst)),
				   SUBREG_BYTE (dst),
				   GET_MODE (dst));
      nregs = subreg_nregs (dst);
      dst = SUBREG_REG (dst);
    }

  /* Some targets do argument pushes without adding REG_INC notes.  */

  if (MEM_P (dst))
    {
      dst = XEXP (dst, 0);
      if (GET_CODE (dst) == PRE_INC || GET_CODE (dst) == POST_INC
	  || GET_CODE (dst) == PRE_DEC || GET_CODE (dst) == POST_DEC)
	reg_set_luid[REGNO (XEXP (dst, 0))] = 0;
      return;
    }
  if (!REG_P (dst))
    return;

  regno += REGNO (dst);
  if (!nregs)
    nregs = hard_regno_nregs[regno][mode];

  if (SCALAR_INT_MODE_P (GET_MODE (dst))
      && nregs == 1 && GET_CODE (set) == SET)
    {
      rtx note, sym = NULL_RTX;
      HOST_WIDE_INT off;

      note = find_reg_equal_equiv_note (insn);
      if (note && GET_CODE (XEXP (note, 0)) == SYMBOL_REF)
	{
	  sym = XEXP (note, 0);
	  off = 0;
	}
      else if (note && GET_CODE (XEXP (note, 0)) == CONST
	       && GET_CODE (XEXP (XEXP (note, 0), 0)) == PLUS
	       && GET_CODE (XEXP (XEXP (XEXP (note, 0), 0), 0)) == SYMBOL_REF
	       && CONST_INT_P (XEXP (XEXP (XEXP (note, 0), 0), 1)))
	{
	  sym = XEXP (XEXP (XEXP (note, 0), 0), 0);
	  off = INTVAL (XEXP (XEXP (XEXP (note, 0), 0), 1));
	}

      if (sym != NULL_RTX)
	{
	  reg_base_reg[regno] = -1;
	  reg_symbol_ref[regno] = sym;
	  reg_offset[regno] = off;
	  reg_mode[regno] = mode;
	  reg_set_luid[regno] = move2add_luid;
	  return;
	}
    }

  if (SCALAR_INT_MODE_P (GET_MODE (dst))
      && nregs == 1 && GET_CODE (set) == SET
      && GET_CODE (SET_DEST (set)) != ZERO_EXTRACT
      && GET_CODE (SET_DEST (set)) != STRICT_LOW_PART)
    {
      rtx src = SET_SRC (set);
      rtx base_reg;
      HOST_WIDE_INT offset;
      int base_regno;
      /* This may be different from mode, if SET_DEST (set) is a
	 SUBREG.  */
      enum machine_mode dst_mode = GET_MODE (dst);

      switch (GET_CODE (src))
	{
	case PLUS:
	  if (REG_P (XEXP (src, 0)))
	    {
	      base_reg = XEXP (src, 0);

	      if (CONST_INT_P (XEXP (src, 1)))
		offset = INTVAL (XEXP (src, 1));
	      else if (REG_P (XEXP (src, 1))
		       && (reg_set_luid[REGNO (XEXP (src, 1))]
			   > move2add_last_label_luid)
		       && (MODES_OK_FOR_MOVE2ADD
			   (dst_mode, reg_mode[REGNO (XEXP (src, 1))])))
		{
		  if (reg_base_reg[REGNO (XEXP (src, 1))] < 0)
		    offset = reg_offset[REGNO (XEXP (src, 1))];
		  /* Maybe the first register is known to be a
		     constant.  */
		  else if (reg_set_luid[REGNO (base_reg)]
			   > move2add_last_label_luid
			   && (MODES_OK_FOR_MOVE2ADD
			       (dst_mode, reg_mode[REGNO (XEXP (src, 1))]))
			   && reg_base_reg[REGNO (base_reg)] < 0)
		    {
		      offset = reg_offset[REGNO (base_reg)];
		      base_reg = XEXP (src, 1);
		    }
		  else
		    goto invalidate;
		}
	      else
		goto invalidate;

	      break;
	    }

	  goto invalidate;

	case REG:
	  base_reg = src;
	  offset = 0;
	  break;

	case CONST_INT:
	  /* Start tracking the register as a constant.  */
	  reg_base_reg[regno] = -1;
	  reg_symbol_ref[regno] = NULL_RTX;
	  reg_offset[regno] = INTVAL (SET_SRC (set));
	  /* We assign the same luid to all registers set to constants.  */
	  reg_set_luid[regno] = move2add_last_label_luid + 1;
	  reg_mode[regno] = mode;
	  return;

	default:
	invalidate:
	  /* Invalidate the contents of the register.  */
	  reg_set_luid[regno] = 0;
	  return;
	}

      base_regno = REGNO (base_reg);
      /* If information about the base register is not valid, set it
	 up as a new base register, pretending its value is known
	 starting from the current insn.  */
      if (reg_set_luid[base_regno] <= move2add_last_label_luid)
	{
	  reg_base_reg[base_regno] = base_regno;
	  reg_symbol_ref[base_regno] = NULL_RTX;
	  reg_offset[base_regno] = 0;
	  reg_set_luid[base_regno] = move2add_luid;
	  reg_mode[base_regno] = mode;
	}
      else if (! MODES_OK_FOR_MOVE2ADD (dst_mode,
					reg_mode[base_regno]))
	goto invalidate;

      reg_mode[regno] = mode;

      /* Copy base information from our base register.  */
      reg_set_luid[regno] = reg_set_luid[base_regno];
      reg_base_reg[regno] = reg_base_reg[base_regno];
      reg_symbol_ref[regno] = reg_symbol_ref[base_regno];

      /* Compute the sum of the offsets or constants.  */
      reg_offset[regno] = trunc_int_for_mode (offset
					      + reg_offset[base_regno],
					      dst_mode);
    }
  else
    {
      unsigned int endregno = regno + nregs;

      for (i = regno; i < endregno; i++)
	/* Reset the information about this register.  */
	reg_set_luid[i] = 0;
    }
}

static bool
gate_handle_postreload (void)
{
  return (optimize > 0 && reload_completed);
}


static unsigned int
rest_of_handle_postreload (void)
{
  if (!dbg_cnt (postreload_cse))
    return 0;

  /* Do a very simple CSE pass over just the hard registers.  */
  reload_cse_regs (get_insns ());
  /* Reload_cse_regs can eliminate potentially-trapping MEMs.
     Remove any EH edges associated with them.  */
  if (cfun->can_throw_non_call_exceptions)
    purge_all_dead_edges ();

  return 0;
}

struct rtl_opt_pass pass_postreload_cse =
{
 {
  RTL_PASS,
  "postreload",                         /* name */
  gate_handle_postreload,               /* gate */
  rest_of_handle_postreload,            /* execute */
  NULL,                                 /* sub */
  NULL,                                 /* next */
  0,                                    /* static_pass_number */
  TV_RELOAD_CSE_REGS,                   /* tv_id */
  0,                                    /* properties_required */
  0,                                    /* properties_provided */
  0,                                    /* properties_destroyed */
  0,                                    /* todo_flags_start */
  TODO_df_finish | TODO_verify_rtl_sharing |
  TODO_dump_func                        /* todo_flags_finish */
 }
};
