/* Output routines for GCC for ARM/Thumb
   Copyright (C) 1996, 1999 Cygnus Software Technologies Ltd
   The basis of this contribution was generated by
   		Richard Earnshaw, Advanced RISC Machines Ltd

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

#include "config.h"
#include "system.h"
#include "rtl.h"
#include "regs.h"
#include "hard-reg-set.h"
#include "real.h"
#include "insn-config.h"
#include "conditions.h"
#include "insn-flags.h"
#include "output.h"
#include "insn-attr.h"
#include "flags.h"
#include "reload.h"
#include "tree.h"
#include "tm_p.h"
#include "expr.h"
#include "toplev.h"
#include "recog.h"
#include "except.h"
#include "function.h"

static void   arm_restore_machine_status PROTO ((struct function *));
static void   arm_save_machine_status    PROTO ((struct function *));
static int    number_of_first_bit_set    PROTO ((int));
static void   replace_symbols_in_block   PROTO ((tree, rtx, rtx));
static void   thumb_exit                 PROTO ((FILE *, int));
static void   thumb_pushpop              PROTO ((FILE *, int, int));

/* Recursively search through all of the blocks in a function
   checking to see if any of the variables created in that
   function match the RTX called 'orig'.  If they do then
   replace them with the RTX called 'new'.  */

static void
replace_symbols_in_block (block, orig, new)
     tree block;
     rtx orig;
     rtx new;
{
  for (; block; block = BLOCK_CHAIN (block))
    {
      tree sym;
      
      if (! TREE_USED (block))
	continue;

      for (sym = BLOCK_VARS (block); sym; sym = TREE_CHAIN (sym))
	{
	  if (  (DECL_NAME (sym) == 0 && TREE_CODE (sym) != TYPE_DECL)
	      || DECL_IGNORED_P (sym)
	      || TREE_CODE (sym) != VAR_DECL
	      || DECL_EXTERNAL (sym)
	      || ! rtx_equal_p (DECL_RTL (sym), orig)
	      )
	    continue;

	  DECL_RTL (sym) = new;
	}
      
      replace_symbols_in_block (BLOCK_SUBBLOCKS (block), orig, new);
    }
}

/* Return the number (counting from 0) of the least significant set bit in MASK.  */
#ifdef __GNUC__
inline
#endif
static int
number_of_first_bit_set (mask)
     int mask;
{
  int bit;

  for (bit = 0;
       (mask & (1 << bit)) == 0;
       ++ bit)
    continue;

  return bit;
}

/* Generate code to return from a thumb function.  If
   'reg_containing_return_addr' is -1, then the return address is
   actually on the stack, at the stack pointer.  */
void
thumb_exit (f, reg_containing_return_addr)
     FILE * f;
     int    reg_containing_return_addr;
{
  int regs_available_for_popping;
  int regs_to_pop;
  int pops_needed;
  int available;
  int required;
  int mode;
  int size;
  int restore_a4 = FALSE;

  /* Compute the registers we need to pop.  */
  regs_to_pop = 0;
  pops_needed = 0;
  
  if (reg_containing_return_addr == -1)
    {
      regs_to_pop |= 1 << LR_REGNUM;
      ++ pops_needed;
    }

  if (TARGET_BACKTRACE)
    {
      /* Restore the (ARM) frame pointer and stack pointer.  */
      regs_to_pop |= (1 << ARM_HARD_FRAME_POINTER_REGNUM) | (1 << SP_REGNUM);
      pops_needed += 2;
    }

  /* If there is nothing to pop then just emit the BX instruction and return.*/
  if (pops_needed == 0)
    {
      asm_fprintf (f, "\tbx\t%r\n", reg_containing_return_addr);

      return;
    }

  /* Otherwise if we are not supporting interworking and we have not created
     a backtrace structure and the function was not entered in ARM mode then
     just pop the return address straight into the PC. */
  else if (   ! TARGET_INTERWORK
	   && ! TARGET_BACKTRACE
	   && ! is_called_in_ARM_mode (current_function_decl))
    {
      asm_fprintf (f, "\tpop\t{%r}\n", PC_REGNUM);

      return;
    }

  /* Find out how many of the (return) argument registers we can corrupt. */
  regs_available_for_popping = 0;
  
#ifdef RTX_CODE
  /* If we can deduce the registers used from the function's return value.
     This is more reliable that examining regs_ever_live[] because that
     will be set if the register is ever used in the function, not just if
     the register is used to hold a return value.  */

  if (current_function_return_rtx != 0)
      mode = GET_MODE (current_function_return_rtx);
  else
#endif
      mode = DECL_MODE (DECL_RESULT (current_function_decl));

  size = GET_MODE_SIZE (mode);

  if (size == 0)
    {
      /* In a void function we can use any argument register.
	 In a function that returns a structure on the stack
	 we can use the second and third argument registers.  */
      if (mode == VOIDmode)
	regs_available_for_popping =
	    (1 << ARG_REGISTER (1))
	  | (1 << ARG_REGISTER (2))
	  | (1 << ARG_REGISTER (3));
      else
	regs_available_for_popping =
	    (1 << ARG_REGISTER (2))
	  | (1 << ARG_REGISTER (3));
    }
  else if (size <= 4)
    regs_available_for_popping =
        (1 << ARG_REGISTER (2))
      | (1 << ARG_REGISTER (3));
  else if (size <= 8)
    regs_available_for_popping =
      (1 << ARG_REGISTER (3));
  
  /* Match registers to be popped with registers into which we pop them.  */
  for (available = regs_available_for_popping,
       required  = regs_to_pop;
       required != 0 && available != 0;
       available &= ~(available & - available),
       required  &= ~(required  & - required))
    -- pops_needed;

  /* If we have any popping registers left over, remove them.  */
  if (available > 0)
    regs_available_for_popping &= ~ available;
  
  /* Otherwise if we need another popping register we can use
     the fourth argument register.  */
  else if (pops_needed)
    {
      /* If we have not found any free argument registers and
	 reg a4 contains the return address, we must move it.  */
      if (regs_available_for_popping == 0
	  && reg_containing_return_addr == LAST_ARG_REGNUM)
	{
	  asm_fprintf (f, "\tmov\t%r, %r\n", LR_REGNUM, LAST_ARG_REGNUM);
	  reg_containing_return_addr = LR_REGNUM;
	}
      else if (size > 12)
	{
	  /* Register a4 is being used to hold part of the return value,
	     but we have dire need of a free, low register.  */
	  restore_a4 = TRUE;
	  
	  asm_fprintf (f, "\tmov\t%r, %r\n",IP_REGNUM, LAST_ARG_REGNUM);
	}
      
      if (reg_containing_return_addr != LAST_ARG_REGNUM)
	{
	  /* The fourth argument register is available.  */
	  regs_available_for_popping |= 1 << LAST_ARG_REGNUM;
	  
	  -- pops_needed;
	}
    }

  /* Pop as many registers as we can.  */
  thumb_pushpop (f, regs_available_for_popping, FALSE);

  /* Process the registers we popped.  */
  if (reg_containing_return_addr == -1)
    {
      /* The return address was popped into the lowest numbered register.  */
      regs_to_pop &= ~ (1 << LR_REGNUM);
      
      reg_containing_return_addr =
	number_of_first_bit_set (regs_available_for_popping);

      /* Remove this register for the mask of available registers, so that
         the return address will not be corrupted by futher pops.  */
      regs_available_for_popping &= ~ (1 << reg_containing_return_addr);
    }

  /* If we popped other registers then handle them here.  */
  if (regs_available_for_popping)
    {
      int frame_pointer;
      
      /* Work out which register currently contains the frame pointer.  */
      frame_pointer = number_of_first_bit_set (regs_available_for_popping);

      /* Move it into the correct place.  */
      asm_fprintf (f, "\tmov\t%r, %r\n", FP_REGNUM, frame_pointer);

      /* (Temporarily) remove it from the mask of popped registers.  */
      regs_available_for_popping &= ~ (1 << frame_pointer);
      regs_to_pop &= ~ (1 << FP_REGNUM);
      
      if (regs_available_for_popping)
	{
	  int stack_pointer;
	  
	  /* We popped the stack pointer as well, find the register that
	     contains it.*/
	  stack_pointer = number_of_first_bit_set (regs_available_for_popping);

	  /* Move it into the stack register.  */
	  asm_fprintf (f, "\tmov\t%r, %r\n", SP_REGNUM, stack_pointer);
	  
	  /* At this point we have popped all necessary registers, so
	     do not worry about restoring regs_available_for_popping
	     to its correct value:

	     assert (pops_needed == 0)
	     assert (regs_available_for_popping == (1 << frame_pointer))
	     assert (regs_to_pop == (1 << STACK_POINTER))  */
	}
      else
	{
	  /* Since we have just move the popped value into the frame
	     pointer, the popping register is available for reuse, and
	     we know that we still have the stack pointer left to pop.  */
	  regs_available_for_popping |= (1 << frame_pointer);
	}
    }
  
  /* If we still have registers left on the stack, but we no longer have
     any registers into which we can pop them, then we must move the return
     address into the link register and make available the register that
     contained it.  */
  if (regs_available_for_popping == 0 && pops_needed > 0)
    {
      regs_available_for_popping |= 1 << reg_containing_return_addr;
      
      asm_fprintf (f, "\tmov\t%r, %r\n", LR_REGNUM, reg_containing_return_addr);
      
      reg_containing_return_addr = LR_REGNUM;
    }

  /* If we have registers left on the stack then pop some more.
     We know that at most we will want to pop FP and SP.  */
  if (pops_needed > 0)
    {
      int  popped_into;
      int  move_to;
      
      thumb_pushpop (f, regs_available_for_popping, FALSE);

      /* We have popped either FP or SP.
	 Move whichever one it is into the correct register.  */
      popped_into = number_of_first_bit_set (regs_available_for_popping);
      move_to     = number_of_first_bit_set (regs_to_pop);

      asm_fprintf (f, "\tmov\t%r, %r\n", move_to, popped_into);

      regs_to_pop &= ~ (1 << move_to);

      -- pops_needed;
    }
  
  /* If we still have not popped everything then we must have only
     had one register available to us and we are now popping the SP.  */
  if (pops_needed > 0)
    {
      int  popped_into;
      
      thumb_pushpop (f, regs_available_for_popping, FALSE);

      popped_into = number_of_first_bit_set (regs_available_for_popping);

      asm_fprintf (f, "\tmov\t%r, %r\n", SP_REGNUM, popped_into);
      /*
	assert (regs_to_pop == (1 << STACK_POINTER))
	assert (pops_needed == 1)
      */
    }

  /* If necessary restore the a4 register.  */
  if (restore_a4)
    {
      if (reg_containing_return_addr != LR_REGNUM)
	{
	  asm_fprintf (f, "\tmov\t%r, %r\n", LR_REGNUM, LAST_ARG_REGNUM);
	  reg_containing_return_addr = LR_REGNUM;
	}
    
      asm_fprintf (f, "\tmov\t%r, %r\n", LAST_ARG_REGNUM, IP_REGNUM);
    }
  
  /* Return to caller.  */
  asm_fprintf (f, "\tbx\t%r\n", reg_containing_return_addr);
}

/* Emit code to push or pop registers to or from the stack.  */
static void
thumb_pushpop (f, mask, push)
     FILE * f;
     int mask;
     int push;
{
  int regno;
  int lo_mask = mask & 0xFF;

  if (lo_mask == 0 && ! push && (mask & (1 << 15)))
    {
      /* Special case.  Do not generate a POP PC statement here, do it in
	 thumb_exit() */
      thumb_exit (f, -1);
      return;
    }
      
  fprintf (f, "\t%s\t{", push ? "push" : "pop");

  /* Look at the low registers first.  */
  for (regno = 0; regno <= LAST_LO_REGNUM; regno ++, lo_mask >>= 1)
    {
      if (lo_mask & 1)
	{
	  asm_fprintf (f, "%r", regno);
	  
	  if ((lo_mask & ~1) != 0)
	    fprintf (f, ", ");
	}
    }
  
  if (push && (mask & (1 << LR_REGNUM)))
    {
      /* Catch pushing the LR.  */
      if (mask & 0xFF)
	fprintf (f, ", ");
      
      asm_fprintf (f, "%r", LR_REGNUM);
    }
  else if (!push && (mask & (1 << PC_REGNUM)))
    {
      /* Catch popping the PC.  */
      if (TARGET_INTERWORK || TARGET_BACKTRACE)
	{
	  /* The PC is never poped directly, instead
	     it is popped into r3 and then BX is used. */
	  fprintf (f, "}\n");

	  thumb_exit (f, -1);

	  return;
	}
      else
	{
	  if (mask & 0xFF)
	    fprintf (f, ", ");
	  
	  asm_fprintf (f, "%r", PC_REGNUM);
	}
    }
       
  fprintf (f, "}\n");
}

void
thumb_final_prescan_insn (insn)
     rtx insn;
{
  extern int * insn_addresses;

  if (flag_print_asm_name)
    asm_fprintf (asm_out_file, "%@ 0x%04x\n", insn_addresses[INSN_UID (insn)]);
}

int
thumb_shiftable_const (val)
     unsigned HOST_WIDE_INT val;
{
  unsigned HOST_WIDE_INT mask = 0xff;
  int i;

  if (val == 0) /* XXX */
    return 0;
  
  for (i = 0; i < 25; i++)
    if ((val & (mask << i)) == val)
      return 1;

  return 0;
}

/* Returns non-zero if the current function contains a far jump */
int
thumb_far_jump_used_p (void)
{
  rtx insn;
  
  for (insn = get_insns (); insn; insn = NEXT_INSN (insn))
    {
      if (GET_CODE (insn) == JUMP_INSN
	  /* Ignore tablejump patterns.  */
	  && GET_CODE (PATTERN (insn)) != ADDR_VEC
	  && GET_CODE (PATTERN (insn)) != ADDR_DIFF_VEC
	  && get_attr_far_jump (insn) == FAR_JUMP_YES
	  )
	return 1;
    }

  return 0;
}

/* Return non-zero if FUNC must be entered in ARM mode.  */
int
is_called_in_ARM_mode (func)
     tree func;
{
  if (TREE_CODE (func) != FUNCTION_DECL)
    abort ();

  /* Ignore the problem about functions whoes address is taken.  */
  if (TARGET_CALLEE_INTERWORKING && TREE_PUBLIC (func))
    return TRUE;

  return FALSE;
}

/* The bits which aren't usefully expanded as rtl. */
char *
thumb_unexpanded_epilogue ()
{
  int regno;
  int live_regs_mask = 0;
  int high_regs_pushed = 0;
  int leaf_function = leaf_function_p ();
  int had_to_push_lr;

  if (return_used_this_function)
    return "";

  for (regno = 0; regno <= LAST_LO_REGNUM; regno++)
    if (regs_ever_live[regno] && ! call_used_regs[regno]
	&& ! (TARGET_SINGLE_PIC_BASE && (regno == arm_pic_register)))
      live_regs_mask |= 1 << regno;

  for (regno = 8; regno < 13; regno++)
    {
      if (regs_ever_live[regno] && ! call_used_regs[regno]
	  && ! (TARGET_SINGLE_PIC_BASE && (regno == arm_pic_register)))
	high_regs_pushed ++;
    }

  /* The prolog may have pushed some high registers to use as
     work registers.  eg the testuite file:
     gcc/testsuite/gcc/gcc.c-torture/execute/complex-2.c
     compiles to produce:
	push	{r4, r5, r6, r7, lr}
	mov	r7, r9
	mov	r6, r8
	push	{r6, r7}
     as part of the prolog.  We have to undo that pushing here.  */
  
  if (high_regs_pushed)
    {
      int mask = live_regs_mask;
      int next_hi_reg;
      int size;
      int mode;
       
#ifdef RTX_CODE
      /* If we can deduce the registers used from the function's return value.
	 This is more reliable that examining regs_ever_live[] because that
	 will be set if the register is ever used in the function, not just if
	 the register is used to hold a return value.  */

      if (current_function_return_rtx != 0)
	mode = GET_MODE (current_function_return_rtx);
      else
#endif
	mode = DECL_MODE (DECL_RESULT (current_function_decl));

      size = GET_MODE_SIZE (mode);

      /* Unless we are returning a type of size > 12 register r3 is available.  */
      if (size < 13)
	mask |=  1 << 3;

      if (mask == 0)
	/* Oh dear!  We have no low registers into which we can pop high registers!  */
	fatal ("No low registers available for popping high registers");
      
      for (next_hi_reg = 8; next_hi_reg < 13; next_hi_reg++)
	if (regs_ever_live[next_hi_reg] && ! call_used_regs[next_hi_reg]
	    && ! (TARGET_SINGLE_PIC_BASE && (next_hi_reg == arm_pic_register)))
	  break;

      while (high_regs_pushed)
	{
	  /* Find lo register(s) into which the high register(s) can be popped.  */
	  for (regno = 0; regno <= LAST_LO_REGNUM; regno++)
	    {
	      if (mask & (1 << regno))
		high_regs_pushed--;
	      if (high_regs_pushed == 0)
		break;
	    }

	  mask &= (2 << regno) - 1;	/* A noop if regno == 8 */

	  /* Pop the values into the low register(s). */
	  thumb_pushpop (asm_out_file, mask, 0);

	  /* Move the value(s) into the high registers.  */
	  for (regno = 0; regno <= LAST_LO_REGNUM; regno++)
	    {
	      if (mask & (1 << regno))
		{
		  asm_fprintf (asm_out_file, "\tmov\t%r, %r\n", next_hi_reg, regno);
		  
		  for (next_hi_reg++; next_hi_reg < 13; next_hi_reg++)
		    if (regs_ever_live[next_hi_reg] && 
			! call_used_regs[next_hi_reg]
			&& ! (TARGET_SINGLE_PIC_BASE 
			      && (next_hi_reg == arm_pic_register)))
		      break;
		}
	    }
	}
    }

  had_to_push_lr = (live_regs_mask || ! leaf_function || thumb_far_jump_used_p ());
  
  if (TARGET_BACKTRACE && ((live_regs_mask & 0xFF) == 0) && regs_ever_live [LAST_ARG_REGNUM] != 0)
    {
      /* The stack backtrace structure creation code had to
	 push R7 in order to get a work register, so we pop
	 it now.   */
      live_regs_mask |= (1 << LAST_LO_REGNUM);
    }
  
  if (current_function_pretend_args_size == 0 || TARGET_BACKTRACE)
    {
      if (had_to_push_lr
	  && ! is_called_in_ARM_mode (current_function_decl))
	live_regs_mask |= 1 << PC_REGNUM;

      /* Either no argument registers were pushed or a backtrace
	 structure was created which includes an adjusted stack
	 pointer, so just pop everything.  */
      
      if (live_regs_mask)
	thumb_pushpop (asm_out_file, live_regs_mask, FALSE);
      
      /* We have either just popped the return address into the
	 PC or it is was kept in LR for the entire function or
	 it is still on the stack because we do not want to
	 return by doing a pop {pc}.  */
      
      if ((live_regs_mask & (1 << PC_REGNUM)) == 0)
	thumb_exit (asm_out_file,
		    (had_to_push_lr
		     && is_called_in_ARM_mode (current_function_decl)) ?
		    -1 : LR_REGNUM);
    }
  else
    {
      /* Pop everything but the return address.  */
      live_regs_mask &= ~ (1 << PC_REGNUM);
      
      if (live_regs_mask)
	thumb_pushpop (asm_out_file, live_regs_mask, FALSE);

      if (had_to_push_lr)
	{
	  /* Get the return address into a temporary register.  */
	  thumb_pushpop (asm_out_file, 1 << LAST_ARG_REGNUM, 0);
	}
      
      /* Remove the argument registers that were pushed onto the stack.  */
      asm_fprintf (asm_out_file, "\tadd\t%r, %r, #%d\n",
		   SP_REGNUM, SP_REGNUM,
		   current_function_pretend_args_size);
      
      thumb_exit (asm_out_file, had_to_push_lr ? LAST_ARG_REGNUM : LR_REGNUM);
    }

  return "";
}

/* Functions to save and restore thumb_return_addr_rtx.  */

static rtx thumb_return_addr_rtx = NULL_RTX;
struct machine_function
{
  rtx ra_rtx;
};

static void
arm_save_machine_status (p)
     struct function * p;
{
  struct machine_function * machine =
    (struct machine_function *) xmalloc (sizeof (* machine));

  p->machine = machine;
  machine->ra_rtx = thumb_return_addr_rtx;
}

static void
arm_restore_machine_status (p)
     struct function * p;
{
  struct machine_function * machine = p->machine;

  thumb_return_addr_rtx = machine->ra_rtx;

  free (machine);
  
  p->machine = (struct machine_function *) NULL;
}



/* Return an RTX indicating where the return address to the
   calling function can be found.  */
rtx
arm_return_addr_rtx (count, frame)
     int count;
     rtx frame;
{
  if (count != 0)
    return NULL_RTX;

  if (TARGET_ARM)
    return gen_rtx_MEM (Pmode, plus_constant (frame, -4));

  if (thumb_return_addr_rtx == NULL_RTX)
    thumb_return_addr_rtx = gen_reg_rtx (Pmode);

  return thumb_return_addr_rtx;
}

/* Do anything needed before RTL is emitted for each function.  */
void
arm_init_expanders ()
{
  thumb_return_addr_rtx = NULL_RTX;

  /* Arrange to save and restore machine status around nested functions.  */
  save_machine_status    = arm_save_machine_status;
  restore_machine_status = arm_restore_machine_status;
}

/* Generate the rest of a function's prologue.  */
void
thumb_expand_prologue ()
{
  HOST_WIDE_INT amount = (get_frame_size ()
			  + current_function_outgoing_args_size);

#ifdef THUMB_PE
  /* Naked functions don't have prologues.  */
  if (arm_naked_function_p (current_function_decl))
    return;
#endif

  if (frame_pointer_needed)
    emit_insn (gen_movsi (hard_frame_pointer_rtx, stack_pointer_rtx));

  if (thumb_return_addr_rtx)
    emit_insn (gen_movsi (thumb_return_addr_rtx, gen_rtx_REG (SImode, LR_REGNUM)));
  
  if (amount)
    {
      if (amount < 512)
	emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx,
			       GEN_INT (-amount)));
      else
	{
	  rtx reg;
	  rtx spare;
	  int live_regs_mask;
	  int regno;
      
	  live_regs_mask = 0;
      
	  for (regno = 0; regno <= LAST_LO_REGNUM; regno ++)
	    if (regs_ever_live[regno] && ! call_used_regs[regno]
		&& ! (TARGET_SINGLE_PIC_BASE && (regno == arm_pic_register)))
	      live_regs_mask |= 1 << regno;

	  if ((live_regs_mask & 0xff) == 0) /* Very unlikely.  */
	    emit_insn (gen_movsi (spare = gen_rtx (REG, SImode, IP_REGNUM),
				  reg = gen_rtx (REG, SImode, 4)));
	  else
	    {
	      for (regno = 0; regno <= LAST_LO_REGNUM; regno ++)
		if (live_regs_mask & (1 << regno))
		  break;
	      reg = gen_rtx (REG, SImode, regno);
	    }

	  emit_insn (gen_movsi (reg, GEN_INT (-amount)));
	  emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx, reg));
	  
	  if ((live_regs_mask & 0xff) == 0)
	    emit_insn (gen_movsi (reg, spare));
	}
    }
  
  if (profile_flag || profile_block_flag || TARGET_NO_SCHED_PRO)
    emit_insn (gen_blockage ());
}

void
thumb_expand_epilogue ()
{
  HOST_WIDE_INT amount = (get_frame_size ()
			  + current_function_outgoing_args_size);
#ifdef THUMB_PE
  /* Naked functions don't have epilogues.  */
  if (arm_naked_function_p (current_function_decl))
    return;
#endif

  if (frame_pointer_needed)
    emit_insn (gen_movsi (stack_pointer_rtx, hard_frame_pointer_rtx));
  else if (amount)
    {
      if (amount < 512)
	emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx,
			       GEN_INT (amount)));
      else
	{
	  rtx reg = gen_rtx (REG, SImode, LAST_ARG_REGNUM); /* Always free in the epilogue.  */

	  emit_insn (gen_movsi (reg, GEN_INT (amount)));
	  emit_insn (gen_addsi3 (stack_pointer_rtx, stack_pointer_rtx, reg));
	}
    }
      
  if (profile_flag || profile_block_flag || TARGET_NO_SCHED_PRO)
    emit_insn (gen_blockage ());
}

void
output_thumb_prologue (f)
     FILE * f;
{
  int live_regs_mask = 0;
  int high_regs_pushed = 0;
  int store_arg_regs = 0;
  int regno;

  if (is_called_in_ARM_mode (current_function_decl))
    {
      char * name;

      if (GET_CODE (DECL_RTL (current_function_decl)) != MEM)
	abort();
      if (GET_CODE (XEXP (DECL_RTL (current_function_decl), 0)) != SYMBOL_REF)
	abort();
      name = XSTR  (XEXP (DECL_RTL (current_function_decl), 0), 0);
      
      /* Generate code sequence to switch us into Thumb mode.  */
      /* The .code 32 directive has already been emitted by
	 ASM_DECLARE_FUNCITON_NAME */
      asm_fprintf (f, "\torr\t%r, %r, #1\n", IP_REGNUM, PC_REGNUM);
      asm_fprintf (f, "\tbx\t%r\n", IP_REGNUM);

      /* Generate a label, so that the debugger will notice the
	 change in instruction sets.  This label is also used by
	 the assembler to bypass the ARM code when this function
	 is called from a Thumb encoded function elsewhere in the
	 same file.  Hence the definition of STUB_NAME here must
	 agree with the definition in gas/config/tc-arm.c  */
      
#define STUB_NAME ".real_start_of"
      
      asm_fprintf (f, "\t.code\t16\n");
      asm_fprintf (f, "\t.globl %s%U%s\n", STUB_NAME, name);
      asm_fprintf (f, "\t.thumb_func\n");
      asm_fprintf (f, "%s%U%s:\n", STUB_NAME, name);
    }
    
  if (current_function_anonymous_args && current_function_pretend_args_size)
    store_arg_regs = 1;

  if (current_function_pretend_args_size)
    {
      if (store_arg_regs)
	{
	  int num_pushes;
	  
	  asm_fprintf (f, "\tpush\t{");

	  num_pushes = NUM_INTS (current_function_pretend_args_size);
	  
	  for (regno = LAST_ARG_REGNUM + 1 - num_pushes; regno <= LAST_ARG_REGNUM;
	       regno ++)
	    asm_fprintf (f, "%r%s", regno, regno == LAST_ARG_REGNUM ? "" : ", ");
	  asm_fprintf (f, "}\n");
	}
      else
	asm_fprintf (f, "\tsub\t%r, %r, #%d\n", 
		     SP_REGNUM, SP_REGNUM,
		     current_function_pretend_args_size);
    }

  for (regno = 0; regno <= LAST_LO_REGNUM; regno ++)
    if (regs_ever_live[regno] && ! call_used_regs[regno]
	&& ! (TARGET_SINGLE_PIC_BASE && (regno == arm_pic_register)))
      live_regs_mask |= 1 << regno;

  if (live_regs_mask || ! leaf_function_p () || thumb_far_jump_used_p ())
    live_regs_mask |= 1 << LR_REGNUM;

  if (TARGET_BACKTRACE)
    {
      int    offset;
      int    work_register = 0;
      
      /* We have been asked to create a stack backtrace structure.
         The code looks like this:
	 
	 0   .align 2
	 0   func:
         0     sub   SP, #16         Reserve space for 4 registers.
	 2     push  {R7}            Get a work register.
         4     add   R7, SP, #20     Get the stack pointer before the push.
         6     str   R7, [SP, #8]    Store the stack pointer (before reserving the space).
         8     mov   R7, PC          Get hold of the start of this code plus 12.
        10     str   R7, [SP, #16]   Store it.
        12     mov   R7, FP          Get hold of the current frame pointer.
        14     str   R7, [SP, #4]    Store it.
        16     mov   R7, LR          Get hold of the current return address.
        18     str   R7, [SP, #12]   Store it.
        20     add   R7, SP, #16     Point at the start of the backtrace structure.
        22     mov   FP, R7          Put this value into the frame pointer.  */

      if ((live_regs_mask & 0xFF) == 0)
	{
	  /* See if the a4 register is free.  */

	  if (regs_ever_live [LAST_ARG_REGNUM] == 0)
	    work_register = LAST_ARG_REGNUM;
	  else	  /* We must push a register of our own */
	    live_regs_mask |= (1 << LAST_LO_REGNUM);
	}

      if (work_register == 0)
	{
	  /* Select a register from the list that will be pushed to use as our work register. */

	  for (work_register = (LAST_LO_REGNUM + 1); work_register--;)
	    if ((1 << work_register) & live_regs_mask)
	      break;
	}
      
      asm_fprintf (f, "\tsub\t%r, %r, #16\t%@ Create stack backtrace structure\n",
		   SP_REGNUM, SP_REGNUM);
      
      if (live_regs_mask)
	thumb_pushpop (f, live_regs_mask, 1);
      
      for (offset = 0, work_register = 1 << 15; work_register; work_register >>= 1)
	if (work_register & live_regs_mask)
	  offset += 4;
      
      asm_fprintf (f, "\tadd\t%r, %r, #%d\n",
		   work_register, SP_REGNUM, offset + 16 + current_function_pretend_args_size);
      
      asm_fprintf (f, "\tstr\t%r, [%r, #%d]\n", work_register, SP_REGNUM, offset + 4);

      /* Make sure that the instruction fetching the PC is in the right place
	 to calculate "start of backtrace creation code + 12".  */
      
      if (live_regs_mask)
	{
	  asm_fprintf (f, "\tmov\t%r, %r\n", work_register, PC_REGNUM);
	  asm_fprintf (f, "\tstr\t%r, [%r, #%d]\n", work_register, SP_REGNUM, offset + 12);
	  asm_fprintf (f, "\tmov\t%r, %r\n", work_register, ARM_HARD_FRAME_POINTER_REGNUM);
	  asm_fprintf (f, "\tstr\t%r, [%r, #%d]\n", work_register, SP_REGNUM, offset);
	}
      else
	{
	  asm_fprintf (f, "\tmov\t%r, %r\n", work_register, ARM_HARD_FRAME_POINTER_REGNUM);
	  asm_fprintf (f, "\tstr\t%r, [%r, #%d]\n", work_register, SP_REGNUM, offset);
	  asm_fprintf (f, "\tmov\t%r, %r\n", work_register, PC_REGNUM);
	  asm_fprintf (f, "\tstr\t%r, [%r, #%d]\n", work_register, SP_REGNUM, offset + 12);
	}
      
      asm_fprintf (f, "\tmov\t%r, %r\n", work_register, LR_REGNUM);
      asm_fprintf (f, "\tstr\t%r, [%r, #%d]\n", work_register, SP_REGNUM, offset + 8);
      asm_fprintf (f, "\tadd\t%r, %r, #%d\n", work_register, SP_REGNUM, offset + 12);
      asm_fprintf (f, "\tmov\t%r, %r\t\t%@ Backtrace structure created\n",
		   ARM_HARD_FRAME_POINTER_REGNUM, work_register);
    }
  else if (live_regs_mask)
    thumb_pushpop (f, live_regs_mask, 1);

  for (regno = 8; regno < 13; regno++)
    {
      if (regs_ever_live[regno] && ! call_used_regs[regno]
	  && ! (TARGET_SINGLE_PIC_BASE && (regno == arm_pic_register)))
	high_regs_pushed ++;
    }

  if (high_regs_pushed)
    {
      int pushable_regs = 0;
      int mask = live_regs_mask & 0xff;
      int next_hi_reg;

      for (next_hi_reg = 12; next_hi_reg > LAST_LO_REGNUM; next_hi_reg--)
	{
	  if (regs_ever_live[next_hi_reg] && ! call_used_regs[next_hi_reg]
	      && ! (TARGET_SINGLE_PIC_BASE && (next_hi_reg == arm_pic_register)))
	    break;
	}

      pushable_regs = mask;

      if (pushable_regs == 0)
	{
	  /* Desperation time -- this probably will never happen.  */
	  if (regs_ever_live[LAST_ARG_REGNUM] || ! call_used_regs[LAST_ARG_REGNUM])
	    asm_fprintf (f, "\tmov\t%r, %r\n", IP_REGNUM, LAST_ARG_REGNUM);
	  mask = 1 << LAST_ARG_REGNUM;
	}

      while (high_regs_pushed > 0)
	{
	  for (regno = LAST_LO_REGNUM; regno >= 0; regno--)
	    {
	      if (mask & (1 << regno))
		{
		  asm_fprintf (f, "\tmov\t%r, %r\n", regno, next_hi_reg);
		  
		  high_regs_pushed --;
		  
		  if (high_regs_pushed)
		    for (next_hi_reg--; next_hi_reg > LAST_LO_REGNUM; next_hi_reg --)
		      {
			if (regs_ever_live[next_hi_reg]
			    && ! call_used_regs[next_hi_reg]
			    && ! (TARGET_SINGLE_PIC_BASE 
				  && (next_hi_reg == arm_pic_register)))
			  break;
		      }
		  else
		    {
		      mask &= ~ ((1 << regno) - 1);
		      break;
		    }
		}
	    }
	  
	  thumb_pushpop (f, mask, 1);
	}

      if (pushable_regs == 0
	  && (regs_ever_live[LAST_ARG_REGNUM] || ! call_used_regs[LAST_ARG_REGNUM]))
	asm_fprintf (f, "\tmov\t%r, %r\n", LAST_ARG_REGNUM, IP_REGNUM);
    }
}

/* Handle the case of a double word load into a low register from
   a computed memory address.  The computed address may involve a
   register which is overwritten by the load.  */

char *
thumb_load_double_from_address (operands)
     rtx * operands;
{
  rtx addr;
  rtx base;
  rtx offset;
  rtx arg1;
  rtx arg2;
  
  if (GET_CODE (operands[0]) != REG)
    fatal ("thumb_load_double_from_address: destination is not a register");
  
  if (GET_CODE (operands[1]) != MEM)
    {
      debug_rtx (operands[1]);
      fatal ("thumb_load_double_from_address: source is not a computed memory address");
    }

  /* Get the memory address.  */
  addr = XEXP (operands[1], 0);
      
  /* Work out how the memory address is computed.  */
  switch (GET_CODE (addr))
    {
    case REG:
      operands[2] = gen_rtx (MEM, SImode, plus_constant (XEXP (operands[1], 0), 4));

      if (REGNO (operands[0]) == REGNO (addr))
	{
	  output_asm_insn ("ldr\t%H0, %2", operands);
	  output_asm_insn ("ldr\t%0, %1", operands);
	}
      else
	{
	  output_asm_insn ("ldr\t%0, %1", operands);
	  output_asm_insn ("ldr\t%H0, %2", operands);
	}
      break;
      
    case CONST:
      /* Compute <address> + 4 for the high order load.  */
      operands[2] = gen_rtx (MEM, SImode, plus_constant (XEXP (operands[1], 0), 4));
	  
      output_asm_insn ("ldr\t%0, %1", operands);
      output_asm_insn ("ldr\t%H0, %2", operands);
      break;
	  
    case PLUS:
      arg1   = XEXP (addr, 0);
      arg2   = XEXP (addr, 1);
	    
      if (CONSTANT_P (arg1))
	base = arg2, offset = arg1;
      else
	base = arg1, offset = arg2;
  
      if (GET_CODE (base) != REG)
	fatal ("thumb_load_double_from_address: base is not a register");

      /* Catch the case of <address> = <reg> + <reg> */
      if (GET_CODE (offset) == REG)
	{
	  int reg_offset = REGNO (offset);
	  int reg_base   = REGNO (base);
	  int reg_dest   = REGNO (operands[0]);
	  
	  /* Add the base and offset registers together into the higher destination register.  */
	  asm_fprintf (asm_out_file, "\tadd\t%r, %r, %r",
		       reg_dest + 1, reg_base, reg_offset);
	  
	  /* Load the lower destination register from the address in the higher destination register.  */
	  asm_fprintf (asm_out_file, "\tldr\t%r, [%r, #0]",
		       reg_dest, reg_dest + 1);
	  
	  /* Load the higher destination register from its own address plus 4.  */
	  asm_fprintf (asm_out_file, "\tldr\t%r, [%r, #4]",
		       reg_dest + 1, reg_dest + 1);
	}
      else
	{
	  /* Compute <address> + 4 for the high order load.  */
	  operands[2] = gen_rtx (MEM, SImode, plus_constant (XEXP (operands[1], 0), 4));
	  
	  /* If the computed address is held in the low order register
	     then load the high order register first, otherwise always
	     load the low order register first.  */
	  if (REGNO (operands[0]) == REGNO (base))
	    {
	      output_asm_insn ("ldr\t%H0, %2", operands);
	      output_asm_insn ("ldr\t%0, %1", operands);
	    }
	  else
	    {
	      output_asm_insn ("ldr\t%0, %1", operands);
	      output_asm_insn ("ldr\t%H0, %2", operands);
	    }
	}
      break;

    case LABEL_REF:
      /* With no registers to worry about we can just load the value directly.  */
      operands[2] = gen_rtx (MEM, SImode, plus_constant (XEXP (operands[1], 0), 4));
	  
      output_asm_insn ("ldr\t%H0, %2", operands);
      output_asm_insn ("ldr\t%0, %1", operands);
      break;
      
    default:
      debug_rtx (operands[1]);
      fatal ("thumb_load_double_from_address: Unhandled address calculation");
      break;
    }
  
  return "";
}


char *
thumb_output_move_mem_multiple (n, operands)
     int n;
     rtx * operands;
{
  rtx tmp;

  switch (n)
    {
    case 2:
      if (REGNO (operands[2]) > REGNO (operands[3]))
	{
	  tmp = operands[2];
	  operands[2] = operands[3];
	  operands[3] = tmp;
	}
      output_asm_insn ("ldmia\t%1!, {%2, %3}", operands);
      output_asm_insn ("stmia\t%0!, {%2, %3}", operands);
      break;

    case 3:
      if (REGNO (operands[2]) > REGNO (operands[3]))
	{
	  tmp = operands[2];
	  operands[2] = operands[3];
	  operands[3] = tmp;
	}
      if (REGNO (operands[3]) > REGNO (operands[4]))
	{
	  tmp = operands[3];
	  operands[3] = operands[4];
	  operands[4] = tmp;
	}
      if (REGNO (operands[2]) > REGNO (operands[3]))
	{
	  tmp = operands[2];
	  operands[2] = operands[3];
	  operands[3] = tmp;
	}
      
      output_asm_insn ("ldmia\t%1!, {%2, %3, %4}", operands);
      output_asm_insn ("stmia\t%0!, {%2, %3, %4}", operands);
      break;

    default:
      abort ();
    }

  return "";
}

/* Routines for generating rtl */

void
thumb_expand_movstrqi (operands)
     rtx * operands;
{
  rtx out = copy_to_mode_reg (SImode, XEXP (operands[0], 0));
  rtx in  = copy_to_mode_reg (SImode, XEXP (operands[1], 0));
  HOST_WIDE_INT len = INTVAL (operands[2]);
  HOST_WIDE_INT offset = 0;

  while (len >= 12)
    {
      emit_insn (gen_movmem12b (out, in));
      len -= 12;
    }
  
  if (len >= 8)
    {
      emit_insn (gen_movmem8b (out, in));
      len -= 8;
    }
  
  if (len >= 4)
    {
      rtx reg = gen_reg_rtx (SImode);
      emit_insn (gen_movsi (reg, gen_rtx (MEM, SImode, in)));
      emit_insn (gen_movsi (gen_rtx (MEM, SImode, out), reg));
      len -= 4;
      offset += 4;
    }
  
  if (len >= 2)
    {
      rtx reg = gen_reg_rtx (HImode);
      emit_insn (gen_movhi (reg, gen_rtx (MEM, HImode, 
					  plus_constant (in, offset))));
      emit_insn (gen_movhi (gen_rtx (MEM, HImode, plus_constant (out, offset)),
			    reg));
      len -= 2;
      offset += 2;
    }
  
  if (len)
    {
      rtx reg = gen_reg_rtx (QImode);
      emit_insn (gen_movqi (reg, gen_rtx (MEM, QImode,
					  plus_constant (in, offset))));
      emit_insn (gen_movqi (gen_rtx (MEM, QImode, plus_constant (out, offset)),
			    reg));
    }
}

int
thumb_cmp_operand (op, mode)
     rtx op;
     enum machine_mode mode;
{
  return ((GET_CODE (op) == CONST_INT
	   && (unsigned HOST_WIDE_INT) (INTVAL (op)) < 256)
	  || register_operand (op, mode));
}

char *
thumb_condition_code (x, invert)
     rtx x;
     int invert;
{
  static char * conds[] =
  {
    "eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", 
    "hi", "ls", "ge", "lt", "gt", "le"
  };
  int val;

  switch (GET_CODE (x))
    {
    case EQ: val = 0; break;
    case NE: val = 1; break;
    case GEU: val = 2; break;
    case LTU: val = 3; break;
    case GTU: val = 8; break;
    case LEU: val = 9; break;
    case GE: val = 10; break;
    case LT: val = 11; break;
    case GT: val = 12; break;
    case LE: val = 13; break;
    default:
      abort ();
    }

  return conds[val ^ invert];
}

/* Handle storing a half-word to memory during reload.  */ 
void
thumb_reload_out_hi (operands)
     rtx * operands;
{
  emit_insn (gen_thumb_movhi_clobber (operands[0], operands[1], operands[2]));
}

/* Handle storing a half-word to memory during reload.  */ 
void
thumb_reload_in_hi (operands)
     rtx * operands ATTRIBUTE_UNUSED;
{
  abort ();
}
