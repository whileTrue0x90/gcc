/* Subroutines for insn-output.c for Matsushita MN10300 series
   Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
   2005, 2006, 2007, 2008, 2009, 2010 Free Software Foundation, Inc.
   Contributed by Jeff Law (law@cygnus.com).

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "rtl.h"
#include "tree.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "insn-config.h"
#include "conditions.h"
#include "output.h"
#include "insn-attr.h"
#include "flags.h"
#include "recog.h"
#include "reload.h"
#include "expr.h"
#include "optabs.h"
#include "function.h"
#include "obstack.h"
#include "diagnostic-core.h"
#include "tm_p.h"
#include "target.h"
#include "target-def.h"
#include "df.h"

/* This is used by GOTaddr2picreg to uniquely identify
   UNSPEC_INT_LABELs.  */
int mn10300_unspec_int_label_counter;

/* This is used in the am33_2.0-linux-gnu port, in which global symbol
   names are not prefixed by underscores, to tell whether to prefix a
   label with a plus sign or not, so that the assembler can tell
   symbol names from register names.  */
int mn10300_protect_label;

/* The selected processor.  */
enum processor_type mn10300_processor = PROCESSOR_DEFAULT;

/* Processor type to select for tuning.  */
static const char * mn10300_tune_string = NULL;

/* Selected processor type for tuning.  */
enum processor_type mn10300_tune_cpu = PROCESSOR_DEFAULT;

/* The size of the callee register save area.  Right now we save everything
   on entry since it costs us nothing in code size.  It does cost us from a
   speed standpoint, so we want to optimize this sooner or later.  */
#define REG_SAVE_BYTES (4 * df_regs_ever_live_p (2)		\
			+ 4 * df_regs_ever_live_p (3)		\
		        + 4 * df_regs_ever_live_p (6)		\
			+ 4 * df_regs_ever_live_p (7)		\
			+ 16 * (df_regs_ever_live_p (14)	\
				|| df_regs_ever_live_p (15)	\
				|| df_regs_ever_live_p (16)	\
				|| df_regs_ever_live_p (17)))

static int mn10300_address_cost (rtx, bool);

/* Implement TARGET_OPTION_OPTIMIZATION_TABLE.  */
static const struct default_options mn10300_option_optimization_table[] =
  {
    { OPT_LEVELS_1_PLUS, OPT_fomit_frame_pointer, NULL, 1 },
    { OPT_LEVELS_NONE, 0, NULL, 0 }
  };

/* Implement TARGET_HANDLE_OPTION.  */

static bool
mn10300_handle_option (size_t code,
		       const char *arg ATTRIBUTE_UNUSED,
		       int value)
{
  switch (code)
    {
    case OPT_mam33:
      mn10300_processor = value ? PROCESSOR_AM33 : PROCESSOR_MN10300;
      return true;

    case OPT_mam33_2:
      mn10300_processor = (value
			   ? PROCESSOR_AM33_2
			   : MIN (PROCESSOR_AM33, PROCESSOR_DEFAULT));
      return true;

    case OPT_mam34:
      mn10300_processor = (value ? PROCESSOR_AM34 : PROCESSOR_DEFAULT);
      return true;

    case OPT_mtune_:
      mn10300_tune_string = arg;
      return true;

    default:
      return true;
    }
}

/* Implement TARGET_OPTION_OVERRIDE.  */

static void
mn10300_option_override (void)
{
  if (TARGET_AM33)
    target_flags &= ~MASK_MULT_BUG;
  else
    {
      /* Disable scheduling for the MN10300 as we do
	 not have timing information available for it.  */
      flag_schedule_insns = 0;
      flag_schedule_insns_after_reload = 0;
    }
  
  if (mn10300_tune_string)
    {
      if (strcasecmp (mn10300_tune_string, "mn10300") == 0)
	mn10300_tune_cpu = PROCESSOR_MN10300;
      else if (strcasecmp (mn10300_tune_string, "am33") == 0)
	mn10300_tune_cpu = PROCESSOR_AM33;
      else if (strcasecmp (mn10300_tune_string, "am33-2") == 0)
	mn10300_tune_cpu = PROCESSOR_AM33_2;
      else if (strcasecmp (mn10300_tune_string, "am34") == 0)
	mn10300_tune_cpu = PROCESSOR_AM34;
      else
	error ("-mtune= expects mn10300, am33, am33-2, or am34");
    }
}

static void
mn10300_file_start (void)
{
  default_file_start ();

  if (TARGET_AM33_2)
    fprintf (asm_out_file, "\t.am33_2\n");
  else if (TARGET_AM33)
    fprintf (asm_out_file, "\t.am33\n");
}

/* Print operand X using operand code CODE to assembly language output file
   FILE.  */

void
mn10300_print_operand (FILE *file, rtx x, int code)
{
  switch (code)
    {
      case 'b':
      case 'B':
	if (GET_MODE (XEXP (x, 0)) == CC_FLOATmode)
	  {
	    switch (code == 'b' ? GET_CODE (x)
		    : reverse_condition_maybe_unordered (GET_CODE (x)))
	      {
	      case NE:
		fprintf (file, "ne");
		break;
	      case EQ:
		fprintf (file, "eq");
		break;
	      case GE:
		fprintf (file, "ge");
		break;
	      case GT:
		fprintf (file, "gt");
		break;
	      case LE:
		fprintf (file, "le");
		break;
	      case LT:
		fprintf (file, "lt");
		break;
	      case ORDERED:
		fprintf (file, "lge");
		break;
	      case UNORDERED:
		fprintf (file, "uo");
		break;
	      case LTGT:
		fprintf (file, "lg");
		break;
	      case UNEQ:
		fprintf (file, "ue");
		break;
	      case UNGE:
		fprintf (file, "uge");
		break;
	      case UNGT:
		fprintf (file, "ug");
		break;
	      case UNLE:
		fprintf (file, "ule");
		break;
	      case UNLT:
		fprintf (file, "ul");
		break;
	      default:
		gcc_unreachable ();
	      }
	    break;
	  }
	/* These are normal and reversed branches.  */
	switch (code == 'b' ? GET_CODE (x) : reverse_condition (GET_CODE (x)))
	  {
	  case NE:
	    fprintf (file, "ne");
	    break;
	  case EQ:
	    fprintf (file, "eq");
	    break;
	  case GE:
	    fprintf (file, "ge");
	    break;
	  case GT:
	    fprintf (file, "gt");
	    break;
	  case LE:
	    fprintf (file, "le");
	    break;
	  case LT:
	    fprintf (file, "lt");
	    break;
	  case GEU:
	    fprintf (file, "cc");
	    break;
	  case GTU:
	    fprintf (file, "hi");
	    break;
	  case LEU:
	    fprintf (file, "ls");
	    break;
	  case LTU:
	    fprintf (file, "cs");
	    break;
	  default:
	    gcc_unreachable ();
	  }
	break;
      case 'C':
	/* This is used for the operand to a call instruction;
	   if it's a REG, enclose it in parens, else output
	   the operand normally.  */
	if (REG_P (x))
	  {
	    fputc ('(', file);
	    mn10300_print_operand (file, x, 0);
	    fputc (')', file);
	  }
	else
	  mn10300_print_operand (file, x, 0);
	break;

      case 'D':
	switch (GET_CODE (x))
	  {
	  case MEM:
	    fputc ('(', file);
	    output_address (XEXP (x, 0));
	    fputc (')', file);
	    break;

	  case REG:
	    fprintf (file, "fd%d", REGNO (x) - 18);
	    break;

	  default:
	    gcc_unreachable ();
	  }
	break;

      /* These are the least significant word in a 64bit value.  */
      case 'L':
	switch (GET_CODE (x))
	  {
	  case MEM:
	    fputc ('(', file);
	    output_address (XEXP (x, 0));
	    fputc (')', file);
	    break;

	  case REG:
	    fprintf (file, "%s", reg_names[REGNO (x)]);
	    break;

	  case SUBREG:
	    fprintf (file, "%s", reg_names[subreg_regno (x)]);
	    break;

	  case CONST_DOUBLE:
	      {
		long val[2];
		REAL_VALUE_TYPE rv;

		switch (GET_MODE (x))
		  {
		    case DFmode:
		      REAL_VALUE_FROM_CONST_DOUBLE (rv, x);
		      REAL_VALUE_TO_TARGET_DOUBLE (rv, val);
		      fprintf (file, "0x%lx", val[0]);
		      break;;
		    case SFmode:
		      REAL_VALUE_FROM_CONST_DOUBLE (rv, x);
		      REAL_VALUE_TO_TARGET_SINGLE (rv, val[0]);
		      fprintf (file, "0x%lx", val[0]);
		      break;;
		    case VOIDmode:
		    case DImode:
		      mn10300_print_operand_address (file,
						     GEN_INT (CONST_DOUBLE_LOW (x)));
		      break;
		    default:
		      break;
		  }
		break;
	      }

	  case CONST_INT:
	    {
	      rtx low, high;
	      split_double (x, &low, &high);
	      fprintf (file, "%ld", (long)INTVAL (low));
	      break;
	    }

	  default:
	    gcc_unreachable ();
	  }
	break;

      /* Similarly, but for the most significant word.  */
      case 'H':
	switch (GET_CODE (x))
	  {
	  case MEM:
	    fputc ('(', file);
	    x = adjust_address (x, SImode, 4);
	    output_address (XEXP (x, 0));
	    fputc (')', file);
	    break;

	  case REG:
	    fprintf (file, "%s", reg_names[REGNO (x) + 1]);
	    break;

	  case SUBREG:
	    fprintf (file, "%s", reg_names[subreg_regno (x) + 1]);
	    break;

	  case CONST_DOUBLE:
	      {
		long val[2];
		REAL_VALUE_TYPE rv;

		switch (GET_MODE (x))
		  {
		    case DFmode:
		      REAL_VALUE_FROM_CONST_DOUBLE (rv, x);
		      REAL_VALUE_TO_TARGET_DOUBLE (rv, val);
		      fprintf (file, "0x%lx", val[1]);
		      break;;
		    case SFmode:
		      gcc_unreachable ();
		    case VOIDmode:
		    case DImode:
		      mn10300_print_operand_address (file,
						     GEN_INT (CONST_DOUBLE_HIGH (x)));
		      break;
		    default:
		      break;
		  }
		break;
	      }

	  case CONST_INT:
	    {
	      rtx low, high;
	      split_double (x, &low, &high);
	      fprintf (file, "%ld", (long)INTVAL (high));
	      break;
	    }

	  default:
	    gcc_unreachable ();
	  }
	break;

      case 'A':
	fputc ('(', file);
	if (REG_P (XEXP (x, 0)))
	  output_address (gen_rtx_PLUS (SImode, XEXP (x, 0), const0_rtx));
	else
	  output_address (XEXP (x, 0));
	fputc (')', file);
	break;

      case 'N':
	gcc_assert (INTVAL (x) >= -128 && INTVAL (x) <= 255);
	fprintf (file, "%d", (int)((~INTVAL (x)) & 0xff));
	break;

      case 'U':
	gcc_assert (INTVAL (x) >= -128 && INTVAL (x) <= 255);
	fprintf (file, "%d", (int)(INTVAL (x) & 0xff));
	break;

      /* For shift counts.  The hardware ignores the upper bits of
	 any immediate, but the assembler will flag an out of range
	 shift count as an error.  So we mask off the high bits
	 of the immediate here.  */
      case 'S':
	if (CONST_INT_P (x))
	  {
	    fprintf (file, "%d", (int)(INTVAL (x) & 0x1f));
	    break;
	  }
	/* FALL THROUGH */

      default:
	switch (GET_CODE (x))
	  {
	  case MEM:
	    fputc ('(', file);
	    output_address (XEXP (x, 0));
	    fputc (')', file);
	    break;

	  case PLUS:
	    output_address (x);
	    break;

	  case REG:
	    fprintf (file, "%s", reg_names[REGNO (x)]);
	    break;

	  case SUBREG:
	    fprintf (file, "%s", reg_names[subreg_regno (x)]);
	    break;

	  /* This will only be single precision....  */
	  case CONST_DOUBLE:
	    {
	      unsigned long val;
	      REAL_VALUE_TYPE rv;

	      REAL_VALUE_FROM_CONST_DOUBLE (rv, x);
	      REAL_VALUE_TO_TARGET_SINGLE (rv, val);
	      fprintf (file, "0x%lx", val);
	      break;
	    }

	  case CONST_INT:
	  case SYMBOL_REF:
	  case CONST:
	  case LABEL_REF:
	  case CODE_LABEL:
	  case UNSPEC:
	    mn10300_print_operand_address (file, x);
	    break;
	  default:
	    gcc_unreachable ();
	  }
	break;
   }
}

/* Output assembly language output for the address ADDR to FILE.  */

void
mn10300_print_operand_address (FILE *file, rtx addr)
{
  switch (GET_CODE (addr))
    {
    case POST_INC:
      mn10300_print_operand_address (file, XEXP (addr, 0));
      fputc ('+', file);
      break;
    case REG:
      mn10300_print_operand (file, addr, 0);
      break;
    case PLUS:
      {
	rtx base, index;
	if (REG_P (XEXP (addr, 0))
	    && REG_OK_FOR_BASE_P (XEXP (addr, 0)))
	  base = XEXP (addr, 0), index = XEXP (addr, 1);
	else if (REG_P (XEXP (addr, 1))
	    && REG_OK_FOR_BASE_P (XEXP (addr, 1)))
	  base = XEXP (addr, 1), index = XEXP (addr, 0);
      	else
	  gcc_unreachable ();
	mn10300_print_operand (file, index, 0);
	fputc (',', file);
	mn10300_print_operand (file, base, 0);;
	break;
      }
    case SYMBOL_REF:
      output_addr_const (file, addr);
      break;
    default:
      output_addr_const (file, addr);
      break;
    }
}

/* Implement TARGET_ASM_OUTPUT_ADDR_CONST_EXTRA.

   Used for PIC-specific UNSPECs.  */

static bool
mn10300_asm_output_addr_const_extra (FILE *file, rtx x)
{
  if (GET_CODE (x) == UNSPEC)
    {
      switch (XINT (x, 1))
	{
	case UNSPEC_INT_LABEL:
	  asm_fprintf (file, ".%LLIL" HOST_WIDE_INT_PRINT_DEC,
		       INTVAL (XVECEXP (x, 0, 0)));
	  break;
	case UNSPEC_PIC:
	  /* GLOBAL_OFFSET_TABLE or local symbols, no suffix.  */
	  output_addr_const (file, XVECEXP (x, 0, 0));
	  break;
	case UNSPEC_GOT:
	  output_addr_const (file, XVECEXP (x, 0, 0));
	  fputs ("@GOT", file);
	  break;
	case UNSPEC_GOTOFF:
	  output_addr_const (file, XVECEXP (x, 0, 0));
	  fputs ("@GOTOFF", file);
	  break;
	case UNSPEC_PLT:
	  output_addr_const (file, XVECEXP (x, 0, 0));
	  fputs ("@PLT", file);
	  break;
	case UNSPEC_GOTSYM_OFF:
	  assemble_name (file, GOT_SYMBOL_NAME);
	  fputs ("-(", file);
	  output_addr_const (file, XVECEXP (x, 0, 0));
	  fputs ("-.)", file);
	  break;
	default:
	  return false;
	}
      return true;
    }
  else
    return false;
}

/* Count the number of FP registers that have to be saved.  */
static int
fp_regs_to_save (void)
{
  int i, n = 0;

  if (! TARGET_AM33_2)
    return 0;

  for (i = FIRST_FP_REGNUM; i <= LAST_FP_REGNUM; ++i)
    if (df_regs_ever_live_p (i) && ! call_really_used_regs[i])
      ++n;

  return n;
}

/* Print a set of registers in the format required by "movm" and "ret".
   Register K is saved if bit K of MASK is set.  The data and address
   registers can be stored individually, but the extended registers cannot.
   We assume that the mask already takes that into account.  For instance,
   bits 14 to 17 must have the same value.  */

void
mn10300_print_reg_list (FILE *file, int mask)
{
  int need_comma;
  int i;

  need_comma = 0;
  fputc ('[', file);

  for (i = 0; i < FIRST_EXTENDED_REGNUM; i++)
    if ((mask & (1 << i)) != 0)
      {
	if (need_comma)
	  fputc (',', file);
	fputs (reg_names [i], file);
	need_comma = 1;
      }

  if ((mask & 0x3c000) != 0)
    {
      gcc_assert ((mask & 0x3c000) == 0x3c000);
      if (need_comma)
	fputc (',', file);
      fputs ("exreg1", file);
      need_comma = 1;
    }

  fputc (']', file);
}

int
mn10300_can_use_return_insn (void)
{
  /* size includes the fixed stack space needed for function calls.  */
  int size = get_frame_size () + crtl->outgoing_args_size;

  /* And space for the return pointer.  */
  size += crtl->outgoing_args_size ? 4 : 0;

  return (reload_completed
	  && size == 0
	  && !df_regs_ever_live_p (2)
	  && !df_regs_ever_live_p (3)
	  && !df_regs_ever_live_p (6)
	  && !df_regs_ever_live_p (7)
	  && !df_regs_ever_live_p (14)
	  && !df_regs_ever_live_p (15)
	  && !df_regs_ever_live_p (16)
	  && !df_regs_ever_live_p (17)
	  && fp_regs_to_save () == 0
	  && !frame_pointer_needed);
}

/* Returns the set of live, callee-saved registers as a bitmask.  The
   callee-saved extended registers cannot be stored individually, so
   all of them will be included in the mask if any one of them is used.  */

int
mn10300_get_live_callee_saved_regs (void)
{
  int mask;
  int i;

  mask = 0;
  for (i = 0; i <= LAST_EXTENDED_REGNUM; i++)
    if (df_regs_ever_live_p (i) && ! call_really_used_regs[i])
      mask |= (1 << i);
  if ((mask & 0x3c000) != 0)
    mask |= 0x3c000;

  return mask;
}

static rtx
F (rtx r)
{
  RTX_FRAME_RELATED_P (r) = 1;
  return r;
}

/* Generate an instruction that pushes several registers onto the stack.
   Register K will be saved if bit K in MASK is set.  The function does
   nothing if MASK is zero.

   To be compatible with the "movm" instruction, the lowest-numbered
   register must be stored in the lowest slot.  If MASK is the set
   { R1,...,RN }, where R1...RN are ordered least first, the generated
   instruction will have the form:

       (parallel
         (set (reg:SI 9) (plus:SI (reg:SI 9) (const_int -N*4)))
	 (set (mem:SI (plus:SI (reg:SI 9)
	                       (const_int -1*4)))
	      (reg:SI RN))
	 ...
	 (set (mem:SI (plus:SI (reg:SI 9)
	                       (const_int -N*4)))
	      (reg:SI R1))) */

void
mn10300_gen_multiple_store (int mask)
{
  if (mask != 0)
    {
      int i;
      int count;
      rtx par;
      int pari;

      /* Count how many registers need to be saved.  */
      count = 0;
      for (i = 0; i <= LAST_EXTENDED_REGNUM; i++)
	if ((mask & (1 << i)) != 0)
	  count += 1;

      /* We need one PARALLEL element to update the stack pointer and
	 an additional element for each register that is stored.  */
      par = gen_rtx_PARALLEL (VOIDmode, rtvec_alloc (count + 1));

      /* Create the instruction that updates the stack pointer.  */
      XVECEXP (par, 0, 0)
	= F (gen_rtx_SET (SImode,
			  stack_pointer_rtx,
			  gen_rtx_PLUS (SImode,
					stack_pointer_rtx,
					GEN_INT (-count * 4))));

      /* Create each store.  */
      pari = 1;
      for (i = LAST_EXTENDED_REGNUM; i >= 0; i--)
	if ((mask & (1 << i)) != 0)
	  {
	    rtx address = gen_rtx_PLUS (SImode,
					stack_pointer_rtx,
					GEN_INT (-pari * 4));
	    XVECEXP(par, 0, pari)
	      = F (gen_rtx_SET (VOIDmode,
				gen_rtx_MEM (SImode, address),
				gen_rtx_REG (SImode, i)));
	    pari += 1;
	  }

      F (emit_insn (par));
    }
}

void
mn10300_expand_prologue (void)
{
  HOST_WIDE_INT size;

  /* SIZE includes the fixed stack space needed for function calls.  */
  size = get_frame_size () + crtl->outgoing_args_size;
  size += (crtl->outgoing_args_size ? 4 : 0);

  /* If we use any of the callee-saved registers, save them now.  */
  mn10300_gen_multiple_store (mn10300_get_live_callee_saved_regs ());

  if (TARGET_AM33_2 && fp_regs_to_save ())
    {
      int num_regs_to_save = fp_regs_to_save (), i;
      HOST_WIDE_INT xsize;
      enum
      {
	save_sp_merge,
	save_sp_no_merge,
	save_sp_partial_merge,
	save_a0_merge,
	save_a0_no_merge
      } strategy;
      unsigned int strategy_size = (unsigned)-1, this_strategy_size;
      rtx reg;

      /* We have several different strategies to save FP registers.
	 We can store them using SP offsets, which is beneficial if
	 there are just a few registers to save, or we can use `a0' in
	 post-increment mode (`a0' is the only call-clobbered address
	 register that is never used to pass information to a
	 function).  Furthermore, if we don't need a frame pointer, we
	 can merge the two SP adds into a single one, but this isn't
	 always beneficial; sometimes we can just split the two adds
	 so that we don't exceed a 16-bit constant size.  The code
	 below will select which strategy to use, so as to generate
	 smallest code.  Ties are broken in favor or shorter sequences
	 (in terms of number of instructions).  */

#define SIZE_ADD_AX(S) ((((S) >= (1 << 15)) || ((S) < -(1 << 15))) ? 6 \
			: (((S) >= (1 << 7)) || ((S) < -(1 << 7))) ? 4 : 2)
#define SIZE_ADD_SP(S) ((((S) >= (1 << 15)) || ((S) < -(1 << 15))) ? 6 \
			: (((S) >= (1 << 7)) || ((S) < -(1 << 7))) ? 4 : 3)

/* We add 0 * (S) in two places to promote to the type of S,
   so that all arms of the conditional have the same type.  */
#define SIZE_FMOV_LIMIT(S,N,L,SIZE1,SIZE2,ELSE) \
  (((S) >= (L)) ? 0 * (S) + (SIZE1) * (N) \
   : ((S) + 4 * (N) >= (L)) ? (((L) - (S)) / 4 * (SIZE2) \
			       + ((S) + 4 * (N) - (L)) / 4 * (SIZE1)) \
   : 0 * (S) + (ELSE))
#define SIZE_FMOV_SP_(S,N) \
  (SIZE_FMOV_LIMIT ((S), (N), (1 << 24), 7, 6, \
                   SIZE_FMOV_LIMIT ((S), (N), (1 << 8), 6, 4, \
				    (S) ? 4 * (N) : 3 + 4 * ((N) - 1))))
#define SIZE_FMOV_SP(S,N) (SIZE_FMOV_SP_ ((unsigned HOST_WIDE_INT)(S), (N)))

      /* Consider alternative save_sp_merge only if we don't need the
	 frame pointer and size is nonzero.  */
      if (! frame_pointer_needed && size)
	{
	  /* Insn: add -(size + 4 * num_regs_to_save), sp.  */
	  this_strategy_size = SIZE_ADD_SP (-(size + 4 * num_regs_to_save));
	  /* Insn: fmov fs#, (##, sp), for each fs# to be saved.  */
	  this_strategy_size += SIZE_FMOV_SP (size, num_regs_to_save);

	  if (this_strategy_size < strategy_size)
	    {
	      strategy = save_sp_merge;
	      strategy_size = this_strategy_size;
	    }
	}

      /* Consider alternative save_sp_no_merge unconditionally.  */
      /* Insn: add -4 * num_regs_to_save, sp.  */
      this_strategy_size = SIZE_ADD_SP (-4 * num_regs_to_save);
      /* Insn: fmov fs#, (##, sp), for each fs# to be saved.  */
      this_strategy_size += SIZE_FMOV_SP (0, num_regs_to_save);
      if (size)
	{
	  /* Insn: add -size, sp.  */
	  this_strategy_size += SIZE_ADD_SP (-size);
	}

      if (this_strategy_size < strategy_size)
	{
	  strategy = save_sp_no_merge;
	  strategy_size = this_strategy_size;
	}

      /* Consider alternative save_sp_partial_merge only if we don't
	 need a frame pointer and size is reasonably large.  */
      if (! frame_pointer_needed && size + 4 * num_regs_to_save > 128)
	{
	  /* Insn: add -128, sp.  */
	  this_strategy_size = SIZE_ADD_SP (-128);
	  /* Insn: fmov fs#, (##, sp), for each fs# to be saved.  */
	  this_strategy_size += SIZE_FMOV_SP (128 - 4 * num_regs_to_save,
					      num_regs_to_save);
	  if (size)
	    {
	      /* Insn: add 128-size, sp.  */
	      this_strategy_size += SIZE_ADD_SP (128 - size);
	    }

	  if (this_strategy_size < strategy_size)
	    {
	      strategy = save_sp_partial_merge;
	      strategy_size = this_strategy_size;
	    }
	}

      /* Consider alternative save_a0_merge only if we don't need a
	 frame pointer, size is nonzero and the user hasn't
	 changed the calling conventions of a0.  */
      if (! frame_pointer_needed && size
	  && call_really_used_regs [FIRST_ADDRESS_REGNUM]
	  && ! fixed_regs[FIRST_ADDRESS_REGNUM])
	{
	  /* Insn: add -(size + 4 * num_regs_to_save), sp.  */
	  this_strategy_size = SIZE_ADD_SP (-(size + 4 * num_regs_to_save));
	  /* Insn: mov sp, a0.  */
	  this_strategy_size++;
	  if (size)
	    {
	      /* Insn: add size, a0.  */
	      this_strategy_size += SIZE_ADD_AX (size);
	    }
	  /* Insn: fmov fs#, (a0+), for each fs# to be saved.  */
	  this_strategy_size += 3 * num_regs_to_save;

	  if (this_strategy_size < strategy_size)
	    {
	      strategy = save_a0_merge;
	      strategy_size = this_strategy_size;
	    }
	}

      /* Consider alternative save_a0_no_merge if the user hasn't
	 changed the calling conventions of a0.  */
      if (call_really_used_regs [FIRST_ADDRESS_REGNUM]
	  && ! fixed_regs[FIRST_ADDRESS_REGNUM])
	{
	  /* Insn: add -4 * num_regs_to_save, sp.  */
	  this_strategy_size = SIZE_ADD_SP (-4 * num_regs_to_save);
	  /* Insn: mov sp, a0.  */
	  this_strategy_size++;
	  /* Insn: fmov fs#, (a0+), for each fs# to be saved.  */
	  this_strategy_size += 3 * num_regs_to_save;
	  if (size)
	    {
	      /* Insn: add -size, sp.  */
	      this_strategy_size += SIZE_ADD_SP (-size);
	    }

	  if (this_strategy_size < strategy_size)
	    {
	      strategy = save_a0_no_merge;
	      strategy_size = this_strategy_size;
	    }
	}

      /* Emit the initial SP add, common to all strategies.  */
      switch (strategy)
	{
	case save_sp_no_merge:
	case save_a0_no_merge:
	  F (emit_insn (gen_addsi3 (stack_pointer_rtx,
				    stack_pointer_rtx,
				    GEN_INT (-4 * num_regs_to_save))));
	  xsize = 0;
	  break;

	case save_sp_partial_merge:
	  F (emit_insn (gen_addsi3 (stack_pointer_rtx,
				    stack_pointer_rtx,
				    GEN_INT (-128))));
	  xsize = 128 - 4 * num_regs_to_save;
	  size -= xsize;
	  break;

	case save_sp_merge:
	case save_a0_merge:
	  F (emit_insn (gen_addsi3 (stack_pointer_rtx,
				    stack_pointer_rtx,
				    GEN_INT (-(size + 4 * num_regs_to_save)))));
	  /* We'll have to adjust FP register saves according to the
	     frame size.  */
	  xsize = size;
	  /* Since we've already created the stack frame, don't do it
	     again at the end of the function.  */
	  size = 0;
	  break;

	default:
	  gcc_unreachable ();
	}

      /* Now prepare register a0, if we have decided to use it.  */
      switch (strategy)
	{
	case save_sp_merge:
	case save_sp_no_merge:
	case save_sp_partial_merge:
	  reg = 0;
	  break;

	case save_a0_merge:
	case save_a0_no_merge:
	  reg = gen_rtx_REG (SImode, FIRST_ADDRESS_REGNUM);
	  F (emit_insn (gen_movsi (reg, stack_pointer_rtx)));
	  if (xsize)
	    F (emit_insn (gen_addsi3 (reg, reg, GEN_INT (xsize))));
	  reg = gen_rtx_POST_INC (SImode, reg);
	  break;

	default:
	  gcc_unreachable ();
	}

      /* Now actually save the FP registers.  */
      for (i = FIRST_FP_REGNUM; i <= LAST_FP_REGNUM; ++i)
	if (df_regs_ever_live_p (i) && ! call_really_used_regs [i])
	  {
	    rtx addr;

	    if (reg)
	      addr = reg;
	    else
	      {
		/* If we aren't using `a0', use an SP offset.  */
		if (xsize)
		  {
		    addr = gen_rtx_PLUS (SImode,
					 stack_pointer_rtx,
					 GEN_INT (xsize));
		  }
		else
		  addr = stack_pointer_rtx;

		xsize += 4;
	      }

	    F (emit_insn (gen_movsf (gen_rtx_MEM (SFmode, addr),
				     gen_rtx_REG (SFmode, i))));
	  }
    }

  /* Now put the frame pointer into the frame pointer register.  */
  if (frame_pointer_needed)
    F (emit_move_insn (frame_pointer_rtx, stack_pointer_rtx));

  /* Allocate stack for this frame.  */
  if (size)
    F (emit_insn (gen_addsi3 (stack_pointer_rtx,
			      stack_pointer_rtx,
			      GEN_INT (-size))));

  if (flag_pic && df_regs_ever_live_p (PIC_OFFSET_TABLE_REGNUM))
    emit_insn (gen_GOTaddr2picreg ());
}

void
mn10300_expand_epilogue (void)
{
  HOST_WIDE_INT size;

  /* SIZE includes the fixed stack space needed for function calls.  */
  size = get_frame_size () + crtl->outgoing_args_size;
  size += (crtl->outgoing_args_size ? 4 : 0);
  
  if (TARGET_AM33_2 && fp_regs_to_save ())
    {
      int num_regs_to_save = fp_regs_to_save (), i;
      rtx reg = 0;

      /* We have several options to restore FP registers.  We could
	 load them from SP offsets, but, if there are enough FP
	 registers to restore, we win if we use a post-increment
	 addressing mode.  */

      /* If we have a frame pointer, it's the best option, because we
	 already know it has the value we want.  */
      if (frame_pointer_needed)
	reg = gen_rtx_REG (SImode, FRAME_POINTER_REGNUM);
      /* Otherwise, we may use `a1', since it's call-clobbered and
	 it's never used for return values.  But only do so if it's
	 smaller than using SP offsets.  */
      else
	{
	  enum { restore_sp_post_adjust,
		 restore_sp_pre_adjust,
		 restore_sp_partial_adjust,
		 restore_a1 } strategy;
	  unsigned int this_strategy_size, strategy_size = (unsigned)-1;

	  /* Consider using sp offsets before adjusting sp.  */
	  /* Insn: fmov (##,sp),fs#, for each fs# to be restored.  */
	  this_strategy_size = SIZE_FMOV_SP (size, num_regs_to_save);
	  /* If size is too large, we'll have to adjust SP with an
		 add.  */
	  if (size + 4 * num_regs_to_save + REG_SAVE_BYTES > 255)
	    {
	      /* Insn: add size + 4 * num_regs_to_save, sp.  */
	      this_strategy_size += SIZE_ADD_SP (size + 4 * num_regs_to_save);
	    }
	  /* If we don't have to restore any non-FP registers,
		 we'll be able to save one byte by using rets.  */
	  if (! REG_SAVE_BYTES)
	    this_strategy_size--;

	  if (this_strategy_size < strategy_size)
	    {
	      strategy = restore_sp_post_adjust;
	      strategy_size = this_strategy_size;
	    }

	  /* Consider using sp offsets after adjusting sp.  */
	  /* Insn: add size, sp.  */
	  this_strategy_size = SIZE_ADD_SP (size);
	  /* Insn: fmov (##,sp),fs#, for each fs# to be restored.  */
	  this_strategy_size += SIZE_FMOV_SP (0, num_regs_to_save);
	  /* We're going to use ret to release the FP registers
		 save area, so, no savings.  */

	  if (this_strategy_size < strategy_size)
	    {
	      strategy = restore_sp_pre_adjust;
	      strategy_size = this_strategy_size;
	    }

	  /* Consider using sp offsets after partially adjusting sp.
	     When size is close to 32Kb, we may be able to adjust SP
	     with an imm16 add instruction while still using fmov
	     (d8,sp).  */
	  if (size + 4 * num_regs_to_save + REG_SAVE_BYTES > 255)
	    {
	      /* Insn: add size + 4 * num_regs_to_save
				+ REG_SAVE_BYTES - 252,sp.  */
	      this_strategy_size = SIZE_ADD_SP (size + 4 * num_regs_to_save
						+ REG_SAVE_BYTES - 252);
	      /* Insn: fmov (##,sp),fs#, fo each fs# to be restored.  */
	      this_strategy_size += SIZE_FMOV_SP (252 - REG_SAVE_BYTES
						  - 4 * num_regs_to_save,
						  num_regs_to_save);
	      /* We're going to use ret to release the FP registers
		 save area, so, no savings.  */

	      if (this_strategy_size < strategy_size)
		{
		  strategy = restore_sp_partial_adjust;
		  strategy_size = this_strategy_size;
		}
	    }

	  /* Consider using a1 in post-increment mode, as long as the
	     user hasn't changed the calling conventions of a1.  */
	  if (call_really_used_regs [FIRST_ADDRESS_REGNUM + 1]
	      && ! fixed_regs[FIRST_ADDRESS_REGNUM+1])
	    {
	      /* Insn: mov sp,a1.  */
	      this_strategy_size = 1;
	      if (size)
		{
		  /* Insn: add size,a1.  */
		  this_strategy_size += SIZE_ADD_AX (size);
		}
	      /* Insn: fmov (a1+),fs#, for each fs# to be restored.  */
	      this_strategy_size += 3 * num_regs_to_save;
	      /* If size is large enough, we may be able to save a
		 couple of bytes.  */
	      if (size + 4 * num_regs_to_save + REG_SAVE_BYTES > 255)
		{
		  /* Insn: mov a1,sp.  */
		  this_strategy_size += 2;
		}
	      /* If we don't have to restore any non-FP registers,
		 we'll be able to save one byte by using rets.  */
	      if (! REG_SAVE_BYTES)
		this_strategy_size--;

	      if (this_strategy_size < strategy_size)
		{
		  strategy = restore_a1;
		  strategy_size = this_strategy_size;
		}
	    }

	  switch (strategy)
	    {
	    case restore_sp_post_adjust:
	      break;

	    case restore_sp_pre_adjust:
	      emit_insn (gen_addsi3 (stack_pointer_rtx,
				     stack_pointer_rtx,
				     GEN_INT (size)));
	      size = 0;
	      break;

	    case restore_sp_partial_adjust:
	      emit_insn (gen_addsi3 (stack_pointer_rtx,
				     stack_pointer_rtx,
				     GEN_INT (size + 4 * num_regs_to_save
					      + REG_SAVE_BYTES - 252)));
	      size = 252 - REG_SAVE_BYTES - 4 * num_regs_to_save;
	      break;

	    case restore_a1:
	      reg = gen_rtx_REG (SImode, FIRST_ADDRESS_REGNUM + 1);
	      emit_insn (gen_movsi (reg, stack_pointer_rtx));
	      if (size)
		emit_insn (gen_addsi3 (reg, reg, GEN_INT (size)));
	      break;

	    default:
	      gcc_unreachable ();
	    }
	}

      /* Adjust the selected register, if any, for post-increment.  */
      if (reg)
	reg = gen_rtx_POST_INC (SImode, reg);

      for (i = FIRST_FP_REGNUM; i <= LAST_FP_REGNUM; ++i)
	if (df_regs_ever_live_p (i) && ! call_really_used_regs [i])
	  {
	    rtx addr;

	    if (reg)
	      addr = reg;
	    else if (size)
	      {
		/* If we aren't using a post-increment register, use an
		   SP offset.  */
		addr = gen_rtx_PLUS (SImode,
				     stack_pointer_rtx,
				     GEN_INT (size));
	      }
	    else
	      addr = stack_pointer_rtx;

	    size += 4;

	    emit_insn (gen_movsf (gen_rtx_REG (SFmode, i),
				  gen_rtx_MEM (SFmode, addr)));
	  }

      /* If we were using the restore_a1 strategy and the number of
	 bytes to be released won't fit in the `ret' byte, copy `a1'
	 to `sp', to avoid having to use `add' to adjust it.  */
      if (! frame_pointer_needed && reg && size + REG_SAVE_BYTES > 255)
	{
	  emit_move_insn (stack_pointer_rtx, XEXP (reg, 0));
	  size = 0;
	}
    }

  /* Maybe cut back the stack, except for the register save area.

     If the frame pointer exists, then use the frame pointer to
     cut back the stack.

     If the stack size + register save area is more than 255 bytes,
     then the stack must be cut back here since the size + register
     save size is too big for a ret/retf instruction.

     Else leave it alone, it will be cut back as part of the
     ret/retf instruction, or there wasn't any stack to begin with.

     Under no circumstances should the register save area be
     deallocated here, that would leave a window where an interrupt
     could occur and trash the register save area.  */
  if (frame_pointer_needed)
    {
      emit_move_insn (stack_pointer_rtx, frame_pointer_rtx);
      size = 0;
    }
  else if (size + REG_SAVE_BYTES > 255)
    {
      emit_insn (gen_addsi3 (stack_pointer_rtx,
			     stack_pointer_rtx,
			     GEN_INT (size)));
      size = 0;
    }

  /* Adjust the stack and restore callee-saved registers, if any.  */
  if (size || df_regs_ever_live_p (2) || df_regs_ever_live_p (3)
      || df_regs_ever_live_p (6) || df_regs_ever_live_p (7)
      || df_regs_ever_live_p (14) || df_regs_ever_live_p (15)
      || df_regs_ever_live_p (16) || df_regs_ever_live_p (17)
      || frame_pointer_needed)
    emit_jump_insn (gen_return_internal_regs
		    (GEN_INT (size + REG_SAVE_BYTES)));
  else
    emit_jump_insn (gen_return_internal ());
}

/* Recognize the PARALLEL rtx generated by mn10300_gen_multiple_store().
   This function is for MATCH_PARALLEL and so assumes OP is known to be
   parallel.  If OP is a multiple store, return a mask indicating which
   registers it saves.  Return 0 otherwise.  */

int
mn10300_store_multiple_operation (rtx op,
				  enum machine_mode mode ATTRIBUTE_UNUSED)
{
  int count;
  int mask;
  int i;
  unsigned int last;
  rtx elt;

  count = XVECLEN (op, 0);
  if (count < 2)
    return 0;

  /* Check that first instruction has the form (set (sp) (plus A B)) */
  elt = XVECEXP (op, 0, 0);
  if (GET_CODE (elt) != SET
      || (! REG_P (SET_DEST (elt)))
      || REGNO (SET_DEST (elt)) != STACK_POINTER_REGNUM
      || GET_CODE (SET_SRC (elt)) != PLUS)
    return 0;

  /* Check that A is the stack pointer and B is the expected stack size.
     For OP to match, each subsequent instruction should push a word onto
     the stack.  We therefore expect the first instruction to create
     COUNT-1 stack slots.  */
  elt = SET_SRC (elt);
  if ((! REG_P (XEXP (elt, 0)))
      || REGNO (XEXP (elt, 0)) != STACK_POINTER_REGNUM
      || (! CONST_INT_P (XEXP (elt, 1)))
      || INTVAL (XEXP (elt, 1)) != -(count - 1) * 4)
    return 0;

  /* Now go through the rest of the vector elements.  They must be
     ordered so that the first instruction stores the highest-numbered
     register to the highest stack slot and that subsequent instructions
     store a lower-numbered register to the slot below.

     LAST keeps track of the smallest-numbered register stored so far.
     MASK is the set of stored registers.  */
  last = LAST_EXTENDED_REGNUM + 1;
  mask = 0;
  for (i = 1; i < count; i++)
    {
      /* Check that element i is a (set (mem M) R) and that R is valid.  */
      elt = XVECEXP (op, 0, i);
      if (GET_CODE (elt) != SET
	  || (! MEM_P (SET_DEST (elt)))
	  || (! REG_P (SET_SRC (elt)))
	  || REGNO (SET_SRC (elt)) >= last)
	return 0;

      /* R was OK, so provisionally add it to MASK.  We return 0 in any
	 case if the rest of the instruction has a flaw.  */
      last = REGNO (SET_SRC (elt));
      mask |= (1 << last);

      /* Check that M has the form (plus (sp) (const_int -I*4)) */
      elt = XEXP (SET_DEST (elt), 0);
      if (GET_CODE (elt) != PLUS
	  || (! REG_P (XEXP (elt, 0)))
	  || REGNO (XEXP (elt, 0)) != STACK_POINTER_REGNUM
	  || (! CONST_INT_P (XEXP (elt, 1)))
	  || INTVAL (XEXP (elt, 1)) != -i * 4)
	return 0;
    }

  /* All or none of the callee-saved extended registers must be in the set.  */
  if ((mask & 0x3c000) != 0
      && (mask & 0x3c000) != 0x3c000)
    return 0;

  return mask;
}

/* Implement TARGET_PREFERRED_RELOAD_CLASS.  */

static reg_class_t
mn10300_preferred_reload_class (rtx x, reg_class_t rclass)
{
  if (x == stack_pointer_rtx && rclass != SP_REGS)
     return ADDRESS_OR_EXTENDED_REGS;
  else if (MEM_P (x)
	   || (REG_P (x) 
	       && !HARD_REGISTER_P (x))
	   || (GET_CODE (x) == SUBREG
	       && REG_P (SUBREG_REG (x))
	       && !HARD_REGISTER_P (SUBREG_REG (x))))
    return LIMIT_RELOAD_CLASS (GET_MODE (x), rclass);
  else
    return rclass;
}

/* Implement TARGET_PREFERRED_OUTPUT_RELOAD_CLASS.  */

static reg_class_t
mn10300_preferred_output_reload_class (rtx x, reg_class_t rclass)
{
  if (x == stack_pointer_rtx && rclass != SP_REGS)
    return ADDRESS_OR_EXTENDED_REGS;

  return rclass;
}

/* What (if any) secondary registers are needed to move IN with mode
   MODE into a register in register class RCLASS.

   We might be able to simplify this.  */

enum reg_class
mn10300_secondary_reload_class (enum reg_class rclass, enum machine_mode mode,
				rtx in)
{
  rtx inner = in;

  /* Strip off any SUBREG expressions from IN.  Basically we want
     to know if IN is a pseudo or (subreg (pseudo)) as those can
     turn into MEMs during reload.  */
  while (GET_CODE (inner) == SUBREG)
    inner = SUBREG_REG (inner);

  /* Memory loads less than a full word wide can't have an
     address or stack pointer destination.  They must use
     a data register as an intermediate register.  */
  if ((MEM_P (in)
       || (REG_P (inner)
	   && REGNO (inner) >= FIRST_PSEUDO_REGISTER))
      && (mode == QImode || mode == HImode)
      && (rclass == ADDRESS_REGS || rclass == SP_REGS
	  || rclass == SP_OR_ADDRESS_REGS))
    {
      if (TARGET_AM33)
	return DATA_OR_EXTENDED_REGS;
      return DATA_REGS;
    }

  /* We can't directly load sp + const_int into a data register;
     we must use an address register as an intermediate.  */
  if (rclass != SP_REGS
      && rclass != ADDRESS_REGS
      && rclass != SP_OR_ADDRESS_REGS
      && rclass != SP_OR_EXTENDED_REGS
      && rclass != ADDRESS_OR_EXTENDED_REGS
      && rclass != SP_OR_ADDRESS_OR_EXTENDED_REGS
      && (in == stack_pointer_rtx
	  || (GET_CODE (in) == PLUS
	      && (XEXP (in, 0) == stack_pointer_rtx
		  || XEXP (in, 1) == stack_pointer_rtx))))
    return ADDRESS_REGS;

  if (TARGET_AM33_2
      && rclass == FP_REGS)
    {
      /* We can't load directly into an FP register from a	
	 constant address.  */
      if (MEM_P (in)
	  && CONSTANT_ADDRESS_P (XEXP (in, 0)))
	return DATA_OR_EXTENDED_REGS;

      /* Handle case were a pseudo may not get a hard register
	 but has an equivalent memory location defined.  */
      if (REG_P (inner)
	  && REGNO (inner) >= FIRST_PSEUDO_REGISTER
	  && reg_equiv_mem [REGNO (inner)]
	  && CONSTANT_ADDRESS_P (XEXP (reg_equiv_mem [REGNO (inner)], 0)))
	return DATA_OR_EXTENDED_REGS;
    }

  /* Otherwise assume no secondary reloads are needed.  */
  return NO_REGS;
}

int
mn10300_initial_offset (int from, int to)
{
  /* The difference between the argument pointer and the frame pointer
     is the size of the callee register save area.  */
  if (from == ARG_POINTER_REGNUM && to == FRAME_POINTER_REGNUM)
    {
      if (df_regs_ever_live_p (2) || df_regs_ever_live_p (3)
	  || df_regs_ever_live_p (6) || df_regs_ever_live_p (7)
	  || df_regs_ever_live_p (14) || df_regs_ever_live_p (15)
	  || df_regs_ever_live_p (16) || df_regs_ever_live_p (17)
	  || fp_regs_to_save ()
	  || frame_pointer_needed)
	return REG_SAVE_BYTES
	  + 4 * fp_regs_to_save ();
      else
	return 0;
    }

  /* The difference between the argument pointer and the stack pointer is
     the sum of the size of this function's frame, the callee register save
     area, and the fixed stack space needed for function calls (if any).  */
  if (from == ARG_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
    {
      if (df_regs_ever_live_p (2) || df_regs_ever_live_p (3)
	  || df_regs_ever_live_p (6) || df_regs_ever_live_p (7)
	  || df_regs_ever_live_p (14) || df_regs_ever_live_p (15)
	  || df_regs_ever_live_p (16) || df_regs_ever_live_p (17)
	  || fp_regs_to_save ()
	  || frame_pointer_needed)
	return (get_frame_size () + REG_SAVE_BYTES
		+ 4 * fp_regs_to_save ()
		+ (crtl->outgoing_args_size
		   ? crtl->outgoing_args_size + 4 : 0));
      else
	return (get_frame_size ()
		+ (crtl->outgoing_args_size
		   ? crtl->outgoing_args_size + 4 : 0));
    }

  /* The difference between the frame pointer and stack pointer is the sum
     of the size of this function's frame and the fixed stack space needed
     for function calls (if any).  */
  if (from == FRAME_POINTER_REGNUM && to == STACK_POINTER_REGNUM)
    return (get_frame_size ()
	    + (crtl->outgoing_args_size
	       ? crtl->outgoing_args_size + 4 : 0));

  gcc_unreachable ();
}

/* Worker function for TARGET_RETURN_IN_MEMORY.  */

static bool
mn10300_return_in_memory (const_tree type, const_tree fntype ATTRIBUTE_UNUSED)
{
  /* Return values > 8 bytes in length in memory.  */
  return (int_size_in_bytes (type) > 8
	  || int_size_in_bytes (type) == 0
	  || TYPE_MODE (type) == BLKmode);
}

/* Flush the argument registers to the stack for a stdarg function;
   return the new argument pointer.  */
static rtx
mn10300_builtin_saveregs (void)
{
  rtx offset, mem;
  tree fntype = TREE_TYPE (current_function_decl);
  int argadj = ((!stdarg_p (fntype))
                ? UNITS_PER_WORD : 0);
  alias_set_type set = get_varargs_alias_set ();

  if (argadj)
    offset = plus_constant (crtl->args.arg_offset_rtx, argadj);
  else
    offset = crtl->args.arg_offset_rtx;

  mem = gen_rtx_MEM (SImode, crtl->args.internal_arg_pointer);
  set_mem_alias_set (mem, set);
  emit_move_insn (mem, gen_rtx_REG (SImode, 0));

  mem = gen_rtx_MEM (SImode,
		     plus_constant (crtl->args.internal_arg_pointer, 4));
  set_mem_alias_set (mem, set);
  emit_move_insn (mem, gen_rtx_REG (SImode, 1));

  return copy_to_reg (expand_binop (Pmode, add_optab,
				    crtl->args.internal_arg_pointer,
				    offset, 0, 0, OPTAB_LIB_WIDEN));
}

static void
mn10300_va_start (tree valist, rtx nextarg)
{
  nextarg = expand_builtin_saveregs ();
  std_expand_builtin_va_start (valist, nextarg);
}

/* Return true when a parameter should be passed by reference.  */

static bool
mn10300_pass_by_reference (CUMULATIVE_ARGS *cum ATTRIBUTE_UNUSED,
			   enum machine_mode mode, const_tree type,
			   bool named ATTRIBUTE_UNUSED)
{
  unsigned HOST_WIDE_INT size;

  if (type)
    size = int_size_in_bytes (type);
  else
    size = GET_MODE_SIZE (mode);

  return (size > 8 || size == 0);
}

/* Return an RTX to represent where a value with mode MODE will be returned
   from a function.  If the result is NULL_RTX, the argument is pushed.  */

static rtx
mn10300_function_arg (CUMULATIVE_ARGS *cum, enum machine_mode mode,
		      const_tree type, bool named ATTRIBUTE_UNUSED)
{
  rtx result = NULL_RTX;
  int size;

  /* We only support using 2 data registers as argument registers.  */
  int nregs = 2;

  /* Figure out the size of the object to be passed.  */
  if (mode == BLKmode)
    size = int_size_in_bytes (type);
  else
    size = GET_MODE_SIZE (mode);

  cum->nbytes = (cum->nbytes + 3) & ~3;

  /* Don't pass this arg via a register if all the argument registers
     are used up.  */
  if (cum->nbytes > nregs * UNITS_PER_WORD)
    return result;

  /* Don't pass this arg via a register if it would be split between
     registers and memory.  */
  if (type == NULL_TREE
      && cum->nbytes + size > nregs * UNITS_PER_WORD)
    return result;

  switch (cum->nbytes / UNITS_PER_WORD)
    {
    case 0:
      result = gen_rtx_REG (mode, FIRST_ARGUMENT_REGNUM);
      break;
    case 1:
      result = gen_rtx_REG (mode, FIRST_ARGUMENT_REGNUM + 1);
      break;
    default:
      break;
    }

  return result;
}

/* Update the data in CUM to advance over an argument
   of mode MODE and data type TYPE.
   (TYPE is null for libcalls where that information may not be available.)  */

static void
mn10300_function_arg_advance (CUMULATIVE_ARGS *cum, enum machine_mode mode,
			      const_tree type, bool named ATTRIBUTE_UNUSED)
{
  cum->nbytes += (mode != BLKmode
		  ? (GET_MODE_SIZE (mode) + 3) & ~3
		  : (int_size_in_bytes (type) + 3) & ~3);
}

/* Return the number of bytes of registers to use for an argument passed
   partially in registers and partially in memory.  */

static int
mn10300_arg_partial_bytes (CUMULATIVE_ARGS *cum, enum machine_mode mode,
			   tree type, bool named ATTRIBUTE_UNUSED)
{
  int size;

  /* We only support using 2 data registers as argument registers.  */
  int nregs = 2;

  /* Figure out the size of the object to be passed.  */
  if (mode == BLKmode)
    size = int_size_in_bytes (type);
  else
    size = GET_MODE_SIZE (mode);

  cum->nbytes = (cum->nbytes + 3) & ~3;

  /* Don't pass this arg via a register if all the argument registers
     are used up.  */
  if (cum->nbytes > nregs * UNITS_PER_WORD)
    return 0;

  if (cum->nbytes + size <= nregs * UNITS_PER_WORD)
    return 0;

  /* Don't pass this arg via a register if it would be split between
     registers and memory.  */
  if (type == NULL_TREE
      && cum->nbytes + size > nregs * UNITS_PER_WORD)
    return 0;

  return nregs * UNITS_PER_WORD - cum->nbytes;
}

/* Return the location of the function's value.  This will be either
   $d0 for integer functions, $a0 for pointers, or a PARALLEL of both
   $d0 and $a0 if the -mreturn-pointer-on-do flag is set.  Note that
   we only return the PARALLEL for outgoing values; we do not want
   callers relying on this extra copy.  */

static rtx
mn10300_function_value (const_tree valtype,
			const_tree fn_decl_or_type ATTRIBUTE_UNUSED,
			bool outgoing)
{
  rtx rv;
  enum machine_mode mode = TYPE_MODE (valtype);

  if (! POINTER_TYPE_P (valtype))
    return gen_rtx_REG (mode, FIRST_DATA_REGNUM);
  else if (! TARGET_PTR_A0D0 || ! outgoing
	   || cfun->returns_struct)
    return gen_rtx_REG (mode, FIRST_ADDRESS_REGNUM);

  rv = gen_rtx_PARALLEL (mode, rtvec_alloc (2));
  XVECEXP (rv, 0, 0)
    = gen_rtx_EXPR_LIST (VOIDmode,
			 gen_rtx_REG (mode, FIRST_ADDRESS_REGNUM),
			 GEN_INT (0));

  XVECEXP (rv, 0, 1)
    = gen_rtx_EXPR_LIST (VOIDmode,
			 gen_rtx_REG (mode, FIRST_DATA_REGNUM),
			 GEN_INT (0));
  return rv;
}

/* Implements TARGET_LIBCALL_VALUE.  */

static rtx
mn10300_libcall_value (enum machine_mode mode,
		       const_rtx fun ATTRIBUTE_UNUSED)
{
  return gen_rtx_REG (mode, FIRST_DATA_REGNUM);
}

/* Implements FUNCTION_VALUE_REGNO_P.  */

bool
mn10300_function_value_regno_p (const unsigned int regno)
{
 return (regno == FIRST_DATA_REGNUM || regno == FIRST_ADDRESS_REGNUM);
}

/* Output a compare insn.  */

const char *
mn10300_output_cmp (rtx operand, rtx insn)
{
  rtx temp;
  int past_call = 0;

  /* We can save a byte if we can find a register which has the value
     zero in it.  */
  temp = PREV_INSN (insn);
  while (optimize && temp)
    {
      rtx set;

      /* We allow the search to go through call insns.  We record
	 the fact that we've past a CALL_INSN and reject matches which
	 use call clobbered registers.  */
      if (LABEL_P (temp)
	  || JUMP_P (temp)
	  || GET_CODE (temp) == BARRIER)
	break;

      if (CALL_P (temp))
	past_call = 1;

      if (GET_CODE (temp) == NOTE)
	{
	  temp = PREV_INSN (temp);
	  continue;
	}

      /* It must be an insn, see if it is a simple set.  */
      set = single_set (temp);
      if (!set)
	{
	  temp = PREV_INSN (temp);
	  continue;
	}

      /* Are we setting a data register to zero (this does not win for
	 address registers)?

	 If it's a call clobbered register, have we past a call?

	 Make sure the register we find isn't the same as ourself;
	 the mn10300 can't encode that.

	 ??? reg_set_between_p return nonzero anytime we pass a CALL_INSN
	 so the code to detect calls here isn't doing anything useful.  */
      if (REG_P (SET_DEST (set))
	  && SET_SRC (set) == CONST0_RTX (GET_MODE (SET_DEST (set)))
	  && !reg_set_between_p (SET_DEST (set), temp, insn)
	  && (REGNO_REG_CLASS (REGNO (SET_DEST (set)))
	      == REGNO_REG_CLASS (REGNO (operand)))
	  && REGNO_REG_CLASS (REGNO (SET_DEST (set))) != EXTENDED_REGS
	  && REGNO (SET_DEST (set)) != REGNO (operand)
	  && (!past_call
	      || ! call_really_used_regs [REGNO (SET_DEST (set))]))
	{
	  rtx xoperands[2];
	  xoperands[0] = operand;
	  xoperands[1] = SET_DEST (set);

	  output_asm_insn ("cmp %1,%0", xoperands);
	  return "";
	}

      if (REGNO_REG_CLASS (REGNO (operand)) == EXTENDED_REGS
	  && REG_P (SET_DEST (set))
	  && SET_SRC (set) == CONST0_RTX (GET_MODE (SET_DEST (set)))
	  && !reg_set_between_p (SET_DEST (set), temp, insn)
	  && (REGNO_REG_CLASS (REGNO (SET_DEST (set)))
	      != REGNO_REG_CLASS (REGNO (operand)))
	  && REGNO_REG_CLASS (REGNO (SET_DEST (set))) == EXTENDED_REGS
	  && REGNO (SET_DEST (set)) != REGNO (operand)
	  && (!past_call
	      || ! call_really_used_regs [REGNO (SET_DEST (set))]))
	{
	  rtx xoperands[2];
	  xoperands[0] = operand;
	  xoperands[1] = SET_DEST (set);

	  output_asm_insn ("cmp %1,%0", xoperands);
	  return "";
	}
      temp = PREV_INSN (temp);
    }
  return "cmp 0,%0";
}

/* Similarly, but when using a zero_extract pattern for a btst where
   the source operand might end up in memory.  */
int
mn10300_mask_ok_for_mem_btst (int len, int bit)
{
  unsigned int mask = 0;

  while (len > 0)
    {
      mask |= (1 << bit);
      bit++;
      len--;
    }

  /* MASK must bit into an 8bit value.  */
  return (((mask & 0xff) == mask)
	  || ((mask & 0xff00) == mask)
	  || ((mask & 0xff0000) == mask)
	  || ((mask & 0xff000000) == mask));
}

/* Return 1 if X contains a symbolic expression.  We know these
   expressions will have one of a few well defined forms, so
   we need only check those forms.  */

int
mn10300_symbolic_operand (rtx op,
			  enum machine_mode mode ATTRIBUTE_UNUSED)
{
  switch (GET_CODE (op))
    {
    case SYMBOL_REF:
    case LABEL_REF:
      return 1;
    case CONST:
      op = XEXP (op, 0);
      return ((GET_CODE (XEXP (op, 0)) == SYMBOL_REF
               || GET_CODE (XEXP (op, 0)) == LABEL_REF)
              && CONST_INT_P (XEXP (op, 1)));
    default:
      return 0;
    }
}

/* Try machine dependent ways of modifying an illegitimate address
   to be legitimate.  If we find one, return the new valid address.
   This macro is used in only one place: `memory_address' in explow.c.

   OLDX is the address as it was before break_out_memory_refs was called.
   In some cases it is useful to look at this to decide what needs to be done.

   Normally it is always safe for this macro to do nothing.  It exists to
   recognize opportunities to optimize the output.

   But on a few ports with segmented architectures and indexed addressing
   (mn10300, hppa) it is used to rewrite certain problematical addresses.  */

static rtx
mn10300_legitimize_address (rtx x, rtx oldx ATTRIBUTE_UNUSED,
			    enum machine_mode mode ATTRIBUTE_UNUSED)
{
  if (flag_pic && ! mn10300_legitimate_pic_operand_p (x))
    x = mn10300_legitimize_pic_address (oldx, NULL_RTX);

  /* Uh-oh.  We might have an address for x[n-100000].  This needs
     special handling to avoid creating an indexed memory address
     with x-100000 as the base.  */
  if (GET_CODE (x) == PLUS
      && mn10300_symbolic_operand (XEXP (x, 1), VOIDmode))
    {
      /* Ugly.  We modify things here so that the address offset specified
         by the index expression is computed first, then added to x to form
         the entire address.  */

      rtx regx1, regy1, regy2, y;

      /* Strip off any CONST.  */
      y = XEXP (x, 1);
      if (GET_CODE (y) == CONST)
        y = XEXP (y, 0);

      if (GET_CODE (y) == PLUS || GET_CODE (y) == MINUS)
	{
	  regx1 = force_reg (Pmode, force_operand (XEXP (x, 0), 0));
	  regy1 = force_reg (Pmode, force_operand (XEXP (y, 0), 0));
	  regy2 = force_reg (Pmode, force_operand (XEXP (y, 1), 0));
	  regx1 = force_reg (Pmode,
			     gen_rtx_fmt_ee (GET_CODE (y), Pmode, regx1,
					     regy2));
	  return force_reg (Pmode, gen_rtx_PLUS (Pmode, regx1, regy1));
	}
    }
  return x;
}

/* Convert a non-PIC address in `orig' to a PIC address using @GOT or
   @GOTOFF in `reg'.  */

rtx
mn10300_legitimize_pic_address (rtx orig, rtx reg)
{
  if (GET_CODE (orig) == LABEL_REF
      || (GET_CODE (orig) == SYMBOL_REF
	  && (CONSTANT_POOL_ADDRESS_P (orig)
	      || ! MN10300_GLOBAL_P (orig))))
    {
      if (reg == 0)
	reg = gen_reg_rtx (Pmode);

      emit_insn (gen_symGOTOFF2reg (reg, orig));
      return reg;
    }
  else if (GET_CODE (orig) == SYMBOL_REF)
    {
      if (reg == 0)
	reg = gen_reg_rtx (Pmode);

      emit_insn (gen_symGOT2reg (reg, orig));
      return reg;
    }
  return orig;
}

/* Return zero if X references a SYMBOL_REF or LABEL_REF whose symbol
   isn't protected by a PIC unspec; nonzero otherwise.  */

int
mn10300_legitimate_pic_operand_p (rtx x)
{
  const char *fmt;
  int i;

  if (GET_CODE (x) == SYMBOL_REF || GET_CODE (x) == LABEL_REF)
    return 0;

  if (GET_CODE (x) == UNSPEC
      && (XINT (x, 1) == UNSPEC_PIC
	  || XINT (x, 1) == UNSPEC_GOT
	  || XINT (x, 1) == UNSPEC_GOTOFF
	  || XINT (x, 1) == UNSPEC_PLT
	  || XINT (x, 1) == UNSPEC_GOTSYM_OFF))
      return 1;

  fmt = GET_RTX_FORMAT (GET_CODE (x));
  for (i = GET_RTX_LENGTH (GET_CODE (x)) - 1; i >= 0; i--)
    {
      if (fmt[i] == 'E')
	{
	  int j;

	  for (j = XVECLEN (x, i) - 1; j >= 0; j--)
	    if (! mn10300_legitimate_pic_operand_p (XVECEXP (x, i, j)))
	      return 0;
	}
      else if (fmt[i] == 'e'
	       && ! mn10300_legitimate_pic_operand_p (XEXP (x, i)))
	return 0;
    }

  return 1;
}

/* Return TRUE if the address X, taken from a (MEM:MODE X) rtx, is
   legitimate, and FALSE otherwise.

   On the mn10300, the value in the address register must be
   in the same memory space/segment as the effective address.

   This is problematical for reload since it does not understand
   that base+index != index+base in a memory reference.

   Note it is still possible to use reg+reg addressing modes,
   it's just much more difficult.  For a discussion of a possible
   workaround and solution, see the comments in pa.c before the
   function record_unscaled_index_insn_codes.  */

static bool
mn10300_legitimate_address_p (enum machine_mode mode, rtx x, bool strict)
{
  if (CONSTANT_ADDRESS_P (x)
      && (! flag_pic || mn10300_legitimate_pic_operand_p (x)))
    return TRUE;

  if (RTX_OK_FOR_BASE_P (x, strict))
    return TRUE;

  if (TARGET_AM33
      && GET_CODE (x) == POST_INC
      && RTX_OK_FOR_BASE_P (XEXP (x, 0), strict)
      && (mode == SImode || mode == SFmode || mode == HImode))
    return TRUE;

  if (GET_CODE (x) == PLUS)
    {
      rtx base = 0, index = 0;

      if (REG_P (XEXP (x, 0))
	  && REGNO_STRICT_OK_FOR_BASE_P (REGNO (XEXP (x, 0)), strict))
	{
	  base = XEXP (x, 0);
	  index = XEXP (x, 1);
	}

      if (REG_P (XEXP (x, 1))
	  && REGNO_STRICT_OK_FOR_BASE_P (REGNO (XEXP (x, 1)), strict))
	{
	  base = XEXP (x, 1);
	  index = XEXP (x, 0);
	}

      if (base != 0 && index != 0)
	{
	  if (CONST_INT_P (index))
	    return TRUE;
	  if (GET_CODE (index) == CONST
	      && GET_CODE (XEXP (index, 0)) != PLUS
	      && (! flag_pic
 		  || (mn10300_legitimate_pic_operand_p (index)
		      && GET_MODE_SIZE (mode) == 4)))
	    return TRUE;
	}
    }

  return FALSE;
}

/* Used by LEGITIMATE_CONSTANT_P().  Returns TRUE if X is a valid
   constant.  Note that some "constants" aren't valid, such as TLS
   symbols and unconverted GOT-based references, so we eliminate
   those here.  */

bool
mn10300_legitimate_constant_p (rtx x)
{
  switch (GET_CODE (x))
    {
    case CONST:
      x = XEXP (x, 0);

      if (GET_CODE (x) == PLUS)
	{
	  if (! CONST_INT_P (XEXP (x, 1)))
	    return false;
	  x = XEXP (x, 0);
	}

      /* Only some unspecs are valid as "constants".  */
      if (GET_CODE (x) == UNSPEC)
	{
	  switch (XINT (x, 1))
	    {
	    case UNSPEC_INT_LABEL:
	    case UNSPEC_PIC:
	    case UNSPEC_GOT:
	    case UNSPEC_GOTOFF:
	    case UNSPEC_PLT:
	      return true;
	    default:
	      return false;
	    }
	}

      /* We must have drilled down to a symbol.  */
      if (! mn10300_symbolic_operand (x, Pmode))
	return false;
      break;

    default:
      break;
    }

  return true;
}

static int
mn10300_address_cost_1 (rtx x, int *unsig)
{
  switch (GET_CODE (x))
    {
    case REG:
      switch (REGNO_REG_CLASS (REGNO (x)))
	{
	case SP_REGS:
	  *unsig = 1;
	  return 0;

	case ADDRESS_REGS:
	  return 1;

	case DATA_REGS:
	case EXTENDED_REGS:
	case FP_REGS:
	  return 3;

	case NO_REGS:
	  return 5;

	default:
	  gcc_unreachable ();
	}

    case PLUS:
    case MINUS:
    case ASHIFT:
    case AND:
    case IOR:
      return (mn10300_address_cost_1 (XEXP (x, 0), unsig)
	      + mn10300_address_cost_1 (XEXP (x, 1), unsig));

    case EXPR_LIST:
    case SUBREG:
    case MEM:
      return mn10300_address_cost (XEXP (x, 0), !optimize_size);

    case ZERO_EXTEND:
      *unsig = 1;
      return mn10300_address_cost_1 (XEXP (x, 0), unsig);

    case CONST_INT:
      if (INTVAL (x) == 0)
	return 0;
      if (INTVAL (x) + (*unsig ? 0 : 0x80) < 0x100)
	return 1;
      if (INTVAL (x) + (*unsig ? 0 : 0x8000) < 0x10000)
	return 3;
      if (INTVAL (x) + (*unsig ? 0 : 0x800000) < 0x1000000)
	return 5;
      return 7;

    case CONST:
    case SYMBOL_REF:
    case LABEL_REF:
      return 8;

    default:
      gcc_unreachable ();

    }
}

static int
mn10300_address_cost (rtx x, bool speed ATTRIBUTE_UNUSED)
{
  int s = 0;
  return mn10300_address_cost_1 (x, &s);
}

static bool
mn10300_rtx_costs (rtx x, int code, int outer_code, int *total,
		   bool speed ATTRIBUTE_UNUSED)
{
  switch (code)
    {
    case CONST_INT:
      /* Zeros are extremely cheap.  */
      if (INTVAL (x) == 0 && (outer_code == SET || outer_code == COMPARE))
	*total = 0;
      /* If it fits in 8 bits, then it's still relatively cheap.  */
      else if (INT_8_BITS (INTVAL (x)))
	*total = 1;
      /* This is the "base" cost, includes constants where either the
	 upper or lower 16bits are all zeros.  */
      else if (INT_16_BITS (INTVAL (x))
	       || (INTVAL (x) & 0xffff) == 0
	       || (INTVAL (x) & 0xffff0000) == 0)
	*total = 2;
      else
	*total = 4;
      return true;

    case CONST:
    case LABEL_REF:
    case SYMBOL_REF:
      /* These are more costly than a CONST_INT, but we can relax them,
	 so they're less costly than a CONST_DOUBLE.  */
      *total = 6;
      return true;

    case CONST_DOUBLE:
      /* We don't optimize CONST_DOUBLEs well nor do we relax them well,
	 so their cost is very high.  */
      *total = 8;
      return true;

    case ZERO_EXTRACT:
      /* This is cheap, we can use btst.  */
      if (outer_code == COMPARE)
	*total = 0;
      return false;

   /* ??? This probably needs more work.  */
    case MOD:
    case DIV:
    case MULT:
      *total = 8;
      return true;

    default:
      return false;
    }
}

/* Check whether a constant used to initialize a DImode or DFmode can
   use a clr instruction.  The code here must be kept in sync with
   movdf and movdi.  */

bool
mn10300_wide_const_load_uses_clr (rtx operands[2])
{
  long val[2] = {0, 0};

  if ((! REG_P (operands[0]))
      || REGNO_REG_CLASS (REGNO (operands[0])) != DATA_REGS)
    return false;

  switch (GET_CODE (operands[1]))
    {
    case CONST_INT:
      {
	rtx low, high;
	split_double (operands[1], &low, &high);
	val[0] = INTVAL (low);
	val[1] = INTVAL (high);
      }
      break;

    case CONST_DOUBLE:
      if (GET_MODE (operands[1]) == DFmode)
	{
	  REAL_VALUE_TYPE rv;

	  REAL_VALUE_FROM_CONST_DOUBLE (rv, operands[1]);
	  REAL_VALUE_TO_TARGET_DOUBLE (rv, val);
	}
      else if (GET_MODE (operands[1]) == VOIDmode
	       || GET_MODE (operands[1]) == DImode)
	{
	  val[0] = CONST_DOUBLE_LOW (operands[1]);
	  val[1] = CONST_DOUBLE_HIGH (operands[1]);
	}
      break;

    default:
      return false;
    }

  return val[0] == 0 || val[1] == 0;
}
/* If using PIC, mark a SYMBOL_REF for a non-global symbol so that we
   may access it using GOTOFF instead of GOT.  */

static void
mn10300_encode_section_info (tree decl, rtx rtl, int first ATTRIBUTE_UNUSED)
{
  rtx symbol;

  if (! MEM_P (rtl))
    return;
  symbol = XEXP (rtl, 0);
  if (GET_CODE (symbol) != SYMBOL_REF)
    return;

  if (flag_pic)
    SYMBOL_REF_FLAG (symbol) = (*targetm.binds_local_p) (decl);
}

/* Dispatch tables on the mn10300 are extremely expensive in terms of code
   and readonly data size.  So we crank up the case threshold value to
   encourage a series of if/else comparisons to implement many small switch
   statements.  In theory, this value could be increased much more if we
   were solely optimizing for space, but we keep it "reasonable" to avoid
   serious code efficiency lossage.  */

static unsigned int
mn10300_case_values_threshold (void)
{
  return 6;
}

/* Worker function for TARGET_ASM_TRAMPOLINE_TEMPLATE.  */

static void
mn10300_asm_trampoline_template (FILE *f)
{
  fprintf (f, "\tadd -4,sp\n");
  fprintf (f, "\t.long 0x0004fffa\n");
  fprintf (f, "\tmov (0,sp),a0\n");
  fprintf (f, "\tadd 4,sp\n");
  fprintf (f, "\tmov (13,a0),a1\n");	
  fprintf (f, "\tmov (17,a0),a0\n");
  fprintf (f, "\tjmp (a0)\n");
  fprintf (f, "\t.long 0\n");
  fprintf (f, "\t.long 0\n");
}

/* Worker function for TARGET_TRAMPOLINE_INIT.  */

static void
mn10300_trampoline_init (rtx m_tramp, tree fndecl, rtx chain_value)
{
  rtx fnaddr = XEXP (DECL_RTL (fndecl), 0);
  rtx mem;

  emit_block_move (m_tramp, assemble_trampoline_template (),
		   GEN_INT (TRAMPOLINE_SIZE), BLOCK_OP_NORMAL);

  mem = adjust_address (m_tramp, SImode, 0x14);
  emit_move_insn (mem, chain_value);
  mem = adjust_address (m_tramp, SImode, 0x18);
  emit_move_insn (mem, fnaddr);
}

/* Output the assembler code for a C++ thunk function.
   THUNK_DECL is the declaration for the thunk function itself, FUNCTION
   is the decl for the target function.  DELTA is an immediate constant
   offset to be added to the THIS parameter.  If VCALL_OFFSET is nonzero
   the word at the adjusted address *(*THIS' + VCALL_OFFSET) should be
   additionally added to THIS.  Finally jump to the entry point of
   FUNCTION.  */

static void
mn10300_asm_output_mi_thunk (FILE *        file,
			     tree          thunk_fndecl ATTRIBUTE_UNUSED,
			     HOST_WIDE_INT delta,
			     HOST_WIDE_INT vcall_offset,
			     tree          function)
{
  const char * _this;

  /* Get the register holding the THIS parameter.  Handle the case
     where there is a hidden first argument for a returned structure.  */
  if (aggregate_value_p (TREE_TYPE (TREE_TYPE (function)), function))
    _this = reg_names [FIRST_ARGUMENT_REGNUM + 1];
  else
    _this = reg_names [FIRST_ARGUMENT_REGNUM];

  fprintf (file, "\t%s Thunk Entry Point:\n", ASM_COMMENT_START);

  if (delta)
    fprintf (file, "\tadd %d, %s\n", (int) delta, _this);

  if (vcall_offset)
    {
      const char * scratch = reg_names [FIRST_ADDRESS_REGNUM + 1];

      fprintf (file, "\tmov %s, %s\n", _this, scratch);
      fprintf (file, "\tmov (%s), %s\n", scratch, scratch);
      fprintf (file, "\tadd %d, %s\n", (int) vcall_offset, scratch);
      fprintf (file, "\tmov (%s), %s\n", scratch, scratch);
      fprintf (file, "\tadd %s, %s\n", scratch, _this);
    }

  fputs ("\tjmp ", file);
  assemble_name (file, XSTR (XEXP (DECL_RTL (function), 0), 0));
  putc ('\n', file);
}

/* Return true if mn10300_output_mi_thunk would be able to output the
   assembler code for the thunk function specified by the arguments
   it is passed, and false otherwise.  */

static bool
mn10300_can_output_mi_thunk (const_tree    thunk_fndecl ATTRIBUTE_UNUSED,
			     HOST_WIDE_INT delta        ATTRIBUTE_UNUSED,
			     HOST_WIDE_INT vcall_offset ATTRIBUTE_UNUSED,
			     const_tree    function     ATTRIBUTE_UNUSED)
{
  return true;
}

bool
mn10300_hard_regno_mode_ok (unsigned int regno, enum machine_mode mode)
{
  if (REGNO_REG_CLASS (regno) == FP_REGS
      || REGNO_REG_CLASS (regno) == FP_ACC_REGS)
    /* Do not store integer values in FP registers.  */
    return GET_MODE_CLASS (mode) == MODE_FLOAT && ((regno & 1) == 0);
  
  if (((regno) & 1) == 0 || GET_MODE_SIZE (mode) == 4)
    return true;

  if (REGNO_REG_CLASS (regno) == DATA_REGS
      || (TARGET_AM33 && REGNO_REG_CLASS (regno) == ADDRESS_REGS)
      || REGNO_REG_CLASS (regno) == EXTENDED_REGS)
    return GET_MODE_SIZE (mode) <= 4;
  
  return false;
}

bool
mn10300_modes_tieable (enum machine_mode mode1, enum machine_mode mode2)
{
  if (GET_MODE_CLASS (mode1) == MODE_FLOAT
      && GET_MODE_CLASS (mode2) != MODE_FLOAT)
    return false;

  if (GET_MODE_CLASS (mode2) == MODE_FLOAT
      && GET_MODE_CLASS (mode1) != MODE_FLOAT)
    return false;

  if (TARGET_AM33
      || mode1 == mode2
      || (GET_MODE_SIZE (mode1) <= 4 && GET_MODE_SIZE (mode2) <= 4))
    return true;

  return false;
}

enum machine_mode
mn10300_select_cc_mode (rtx x)
{
  return (GET_MODE_CLASS (GET_MODE (x)) == MODE_FLOAT) ? CC_FLOATmode : CCmode;
}

static inline bool
is_load_insn (rtx insn)
{
  if (GET_CODE (PATTERN (insn)) != SET)
    return false;

  return MEM_P (SET_SRC (PATTERN (insn)));
}

static inline bool
is_store_insn (rtx insn)
{
  if (GET_CODE (PATTERN (insn)) != SET)
    return false;

  return MEM_P (SET_DEST (PATTERN (insn)));
}

/* Update scheduling costs for situations that cannot be
   described using the attributes and DFA machinery.
   DEP is the insn being scheduled.
   INSN is the previous insn.
   COST is the current cycle cost for DEP.  */

static int
mn10300_adjust_sched_cost (rtx insn, rtx link, rtx dep, int cost)
{
  int timings = get_attr_timings (insn);

  if (!TARGET_AM33)
    return 1;

  if (GET_CODE (insn) == PARALLEL)
    insn = XVECEXP (insn, 0, 0);

  if (GET_CODE (dep) == PARALLEL)
    dep = XVECEXP (dep, 0, 0);

  /* For the AM34 a load instruction that follows a
     store instruction incurs an extra cycle of delay.  */
  if (mn10300_tune_cpu == PROCESSOR_AM34
      && is_load_insn (dep)
      && is_store_insn (insn))
    cost += 1;

  /* For the AM34 a non-store, non-branch FPU insn that follows
     another FPU insn incurs a one cycle throughput increase.  */
  else if (mn10300_tune_cpu == PROCESSOR_AM34
      && ! is_store_insn (insn)
      && ! JUMP_P (insn)
      && GET_CODE (PATTERN (dep)) == SET
      && GET_CODE (PATTERN (insn)) == SET
      && GET_MODE_CLASS (GET_MODE (SET_SRC (PATTERN (dep)))) == MODE_FLOAT
      && GET_MODE_CLASS (GET_MODE (SET_SRC (PATTERN (insn)))) == MODE_FLOAT)
    cost += 1;

  /*  Resolve the conflict described in section 1-7-4 of
      Chapter 3 of the MN103E Series Instruction Manual
      where it says:

        "When the preceeding instruction is a CPU load or
	 store instruction, a following FPU instruction
	 cannot be executed until the CPU completes the
	 latency period even though there are no register
	 or flag dependencies between them."  */

  /* Only the AM33-2 (and later) CPUs have FPU instructions.  */
  if (! TARGET_AM33_2)
    return cost;

  /* If a data dependence already exists then the cost is correct.  */
  if (REG_NOTE_KIND (link) == 0)
    return cost;

  /* Check that the instruction about to scheduled is an FPU instruction.  */
  if (GET_CODE (PATTERN (dep)) != SET)
    return cost;

  if (GET_MODE_CLASS (GET_MODE (SET_SRC (PATTERN (dep)))) != MODE_FLOAT)
    return cost;

  /* Now check to see if the previous instruction is a load or store.  */
  if (! is_load_insn (insn) && ! is_store_insn (insn))
    return cost;

  /* XXX: Verify: The text of 1-7-4 implies that the restriction
     only applies when an INTEGER load/store preceeds an FPU
     instruction, but is this true ?  For now we assume that it is.  */
  if (GET_MODE_CLASS (GET_MODE (SET_SRC (PATTERN (insn)))) != MODE_INT)
    return cost;

  /* Extract the latency value from the timings attribute.  */
  return timings < 100 ? (timings % 10) : (timings % 100);
}

static void
mn10300_conditional_register_usage (void)
{
  unsigned int i;

  if (!TARGET_AM33)
    {
      for (i = FIRST_EXTENDED_REGNUM;
	   i <= LAST_EXTENDED_REGNUM; i++)
	fixed_regs[i] = call_used_regs[i] = 1;
    }
  if (!TARGET_AM33_2)
    {
      for (i = FIRST_FP_REGNUM;
	   i <= LAST_FP_REGNUM; i++)
	fixed_regs[i] = call_used_regs[i] = 1;
    }
  if (flag_pic)
    fixed_regs[PIC_OFFSET_TABLE_REGNUM] =
    call_used_regs[PIC_OFFSET_TABLE_REGNUM] = 1;
}

/* Initialize the GCC target structure.  */

#undef  TARGET_EXCEPT_UNWIND_INFO
#define TARGET_EXCEPT_UNWIND_INFO sjlj_except_unwind_info

#undef  TARGET_ASM_ALIGNED_HI_OP
#define TARGET_ASM_ALIGNED_HI_OP "\t.hword\t"

#undef  TARGET_LEGITIMIZE_ADDRESS
#define TARGET_LEGITIMIZE_ADDRESS mn10300_legitimize_address

#undef  TARGET_RTX_COSTS
#define TARGET_RTX_COSTS mn10300_rtx_costs
#undef  TARGET_ADDRESS_COST
#define TARGET_ADDRESS_COST mn10300_address_cost

#undef  TARGET_ASM_FILE_START
#define TARGET_ASM_FILE_START mn10300_file_start
#undef  TARGET_ASM_FILE_START_FILE_DIRECTIVE
#define TARGET_ASM_FILE_START_FILE_DIRECTIVE true

#undef TARGET_ASM_OUTPUT_ADDR_CONST_EXTRA
#define TARGET_ASM_OUTPUT_ADDR_CONST_EXTRA mn10300_asm_output_addr_const_extra

#undef  TARGET_DEFAULT_TARGET_FLAGS
#define TARGET_DEFAULT_TARGET_FLAGS MASK_MULT_BUG | MASK_PTR_A0D0
#undef  TARGET_HANDLE_OPTION
#define TARGET_HANDLE_OPTION mn10300_handle_option
#undef  TARGET_OPTION_OVERRIDE
#define TARGET_OPTION_OVERRIDE mn10300_option_override
#undef  TARGET_OPTION_OPTIMIZATION_TABLE
#define TARGET_OPTION_OPTIMIZATION_TABLE mn10300_option_optimization_table

#undef  TARGET_ENCODE_SECTION_INFO
#define TARGET_ENCODE_SECTION_INFO mn10300_encode_section_info

#undef  TARGET_PROMOTE_PROTOTYPES
#define TARGET_PROMOTE_PROTOTYPES hook_bool_const_tree_true
#undef  TARGET_RETURN_IN_MEMORY
#define TARGET_RETURN_IN_MEMORY mn10300_return_in_memory
#undef  TARGET_PASS_BY_REFERENCE
#define TARGET_PASS_BY_REFERENCE mn10300_pass_by_reference
#undef  TARGET_CALLEE_COPIES
#define TARGET_CALLEE_COPIES hook_bool_CUMULATIVE_ARGS_mode_tree_bool_true
#undef  TARGET_ARG_PARTIAL_BYTES
#define TARGET_ARG_PARTIAL_BYTES mn10300_arg_partial_bytes
#undef  TARGET_FUNCTION_ARG
#define TARGET_FUNCTION_ARG mn10300_function_arg
#undef  TARGET_FUNCTION_ARG_ADVANCE
#define TARGET_FUNCTION_ARG_ADVANCE mn10300_function_arg_advance

#undef  TARGET_EXPAND_BUILTIN_SAVEREGS
#define TARGET_EXPAND_BUILTIN_SAVEREGS mn10300_builtin_saveregs
#undef  TARGET_EXPAND_BUILTIN_VA_START
#define TARGET_EXPAND_BUILTIN_VA_START mn10300_va_start

#undef  TARGET_CASE_VALUES_THRESHOLD
#define TARGET_CASE_VALUES_THRESHOLD mn10300_case_values_threshold

#undef  TARGET_LEGITIMATE_ADDRESS_P
#define TARGET_LEGITIMATE_ADDRESS_P	mn10300_legitimate_address_p

#undef  TARGET_PREFERRED_RELOAD_CLASS
#define TARGET_PREFERRED_RELOAD_CLASS mn10300_preferred_reload_class
#undef  TARGET_PREFERRED_OUTPUT_RELOAD_CLASS
#define TARGET_PREFERRED_OUTPUT_RELOAD_CLASS mn10300_preferred_output_reload_class

#undef  TARGET_ASM_TRAMPOLINE_TEMPLATE
#define TARGET_ASM_TRAMPOLINE_TEMPLATE mn10300_asm_trampoline_template
#undef  TARGET_TRAMPOLINE_INIT
#define TARGET_TRAMPOLINE_INIT mn10300_trampoline_init

#undef  TARGET_FUNCTION_VALUE
#define TARGET_FUNCTION_VALUE mn10300_function_value
#undef  TARGET_LIBCALL_VALUE
#define TARGET_LIBCALL_VALUE mn10300_libcall_value

#undef  TARGET_ASM_OUTPUT_MI_THUNK
#define TARGET_ASM_OUTPUT_MI_THUNK      mn10300_asm_output_mi_thunk
#undef  TARGET_ASM_CAN_OUTPUT_MI_THUNK
#define TARGET_ASM_CAN_OUTPUT_MI_THUNK  mn10300_can_output_mi_thunk

#undef  TARGET_SCHED_ADJUST_COST
#define TARGET_SCHED_ADJUST_COST mn10300_adjust_sched_cost

#undef  TARGET_CONDITIONAL_REGISTER_USAGE
#define TARGET_CONDITIONAL_REGISTER_USAGE mn10300_conditional_register_usage

struct gcc_target targetm = TARGET_INITIALIZER;
