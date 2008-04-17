/* Structure for saving state for a nested function.
   Copyright (C) 1989, 1992, 1993, 1994, 1995, 1996, 1997, 1998,
   1999, 2000, 2003, 2004, 2005, 2006, 2007, 2008
   Free Software Foundation, Inc.

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

#ifndef GCC_FUNCTION_H
#define GCC_FUNCTION_H

#include "tree.h"
#include "hashtab.h"
#include "varray.h"

/* Stack of pending (incomplete) sequences saved by `start_sequence'.
   Each element describes one pending sequence.
   The main insn-chain is saved in the last element of the chain,
   unless the chain is empty.  */

struct sequence_stack GTY(())
{
  /* First and last insns in the chain of the saved sequence.  */
  rtx first;
  rtx last;
  struct sequence_stack *next;
};

struct emit_status GTY(())
{
  /* This is reset to LAST_VIRTUAL_REGISTER + 1 at the start of each function.
     After rtl generation, it is 1 plus the largest register number used.  */
  int x_reg_rtx_no;

  /* Lowest label number in current function.  */
  int x_first_label_num;

  /* The ends of the doubly-linked chain of rtl for the current function.
     Both are reset to null at the start of rtl generation for the function.

     start_sequence saves both of these on `sequence_stack' and then starts
     a new, nested sequence of insns.  */
  rtx x_first_insn;
  rtx x_last_insn;

  /* Stack of pending (incomplete) sequences saved by `start_sequence'.
     Each element describes one pending sequence.
     The main insn-chain is saved in the last element of the chain,
     unless the chain is empty.  */
  struct sequence_stack *sequence_stack;

  /* INSN_UID for next insn emitted.
     Reset to 1 for each function compiled.  */
  int x_cur_insn_uid;

  /* Location the last line-number NOTE emitted.
     This is used to avoid generating duplicates.  */
  location_t x_last_location;

  /* The length of the regno_pointer_align, regno_decl, and x_regno_reg_rtx
     vectors.  Since these vectors are needed during the expansion phase when
     the total number of registers in the function is not yet known, the
     vectors are copied and made bigger when necessary.  */
  int regno_pointer_align_length;

  /* Indexed by pseudo register number, if nonzero gives the known alignment
     for that pseudo (if REG_POINTER is set in x_regno_reg_rtx).
     Allocated in parallel with x_regno_reg_rtx.  */
  unsigned char * GTY((skip)) regno_pointer_align;
};


/* Indexed by pseudo register number, gives the rtx for that pseudo.
   Allocated in parallel with regno_pointer_align.  
   FIXME: We could put it into emit_status struct, but gengtype is not able to deal
   with length attribute nested in top level structures.  */

extern GTY ((length ("crtl->emit.x_reg_rtx_no"))) rtx * regno_reg_rtx;

/* For backward compatibility... eventually these should all go away.  */
#define reg_rtx_no (crtl->emit.x_reg_rtx_no)
#define seq_stack (crtl->emit.sequence_stack)

#define REGNO_POINTER_ALIGN(REGNO) (crtl->emit.regno_pointer_align[REGNO])

struct expr_status GTY(())
{
  /* Number of units that we should eventually pop off the stack.
     These are the arguments to function calls that have already returned.  */
  int x_pending_stack_adjust;

  /* Under some ABIs, it is the caller's responsibility to pop arguments
     pushed for function calls.  A naive implementation would simply pop
     the arguments immediately after each call.  However, if several
     function calls are made in a row, it is typically cheaper to pop
     all the arguments after all of the calls are complete since a
     single pop instruction can be used.  Therefore, GCC attempts to
     defer popping the arguments until absolutely necessary.  (For
     example, at the end of a conditional, the arguments must be popped,
     since code outside the conditional won't know whether or not the
     arguments need to be popped.)

     When INHIBIT_DEFER_POP is nonzero, however, the compiler does not
     attempt to defer pops.  Instead, the stack is popped immediately
     after each call.  Rather then setting this variable directly, use
     NO_DEFER_POP and OK_DEFER_POP.  */
  int x_inhibit_defer_pop;

  /* If PREFERRED_STACK_BOUNDARY and PUSH_ROUNDING are defined, the stack
     boundary can be momentarily unaligned while pushing the arguments.
     Record the delta since last aligned boundary here in order to get
     stack alignment in the nested function calls working right.  */
  int x_stack_pointer_delta;

  /* Nonzero means __builtin_saveregs has already been done in this function.
     The value is the pseudoreg containing the value __builtin_saveregs
     returned.  */
  rtx x_saveregs_value;

  /* Similarly for __builtin_apply_args.  */
  rtx x_apply_args_value;

  /* List of labels that must never be deleted.  */
  rtx x_forced_labels;
};

typedef struct call_site_record *call_site_record;
DEF_VEC_P(call_site_record);
DEF_VEC_ALLOC_P(call_site_record, gc);

/* RTL representation of exception handling.  */
struct rtl_eh GTY(())
{
  rtx filter;
  rtx exc_ptr;

  int built_landing_pads;

  rtx ehr_stackadj;
  rtx ehr_handler;
  rtx ehr_label;

  rtx sjlj_fc;
  rtx sjlj_exit_after;

  htab_t GTY ((param_is (struct ehl_map_entry))) exception_handler_label_map;

  VEC(tree,gc) *ttype_data;
  varray_type ehspec_data;
  varray_type action_record_data;

  VEC(call_site_record,gc) *call_site_record;
};

#define pending_stack_adjust (crtl->expr.x_pending_stack_adjust)
#define inhibit_defer_pop (crtl->expr.x_inhibit_defer_pop)
#define saveregs_value (crtl->expr.x_saveregs_value)
#define apply_args_value (crtl->expr.x_apply_args_value)
#define forced_labels (crtl->expr.x_forced_labels)
#define stack_pointer_delta (crtl->expr.x_stack_pointer_delta)

struct gimple_df;
struct temp_slot;
typedef struct temp_slot *temp_slot_p;
struct call_site_record;

DEF_VEC_P(temp_slot_p);
DEF_VEC_ALLOC_P(temp_slot_p,gc);

enum function_frequency {
  /* This function most likely won't be executed at all.
     (set only when profile feedback is available or via function attribute). */
  FUNCTION_FREQUENCY_UNLIKELY_EXECUTED,
  /* The default value.  */
  FUNCTION_FREQUENCY_NORMAL,
  /* Optimize this function hard
     (set only when profile feedback is available or via function attribute). */
  FUNCTION_FREQUENCY_HOT
};

struct varasm_status GTY(())
{
  /* If we're using a per-function constant pool, this is it.  */
  struct rtx_constant_pool *pool;

  /* Number of tree-constants deferred during the expansion of this
     function.  */
  unsigned int deferred_constants;
};

/* Information mainlined about RTL representation of incoming arguments.  */
struct incoming_args GTY(())
{
  /* Number of bytes of args popped by function being compiled on its return.
     Zero if no bytes are to be popped.
     May affect compilation of return insn or of function epilogue.  */
  int pops_args;

  /* If function's args have a fixed size, this is that size, in bytes.
     Otherwise, it is -1.
     May affect compilation of return insn or of function epilogue.  */
  int size;

  /* # bytes the prologue should push and pretend that the caller pushed them.
     The prologue must do this, but only if parms can be passed in
     registers.  */
  int pretend_args_size;

  /* This is the offset from the arg pointer to the place where the first
     anonymous arg can be found, if there is one.  */
  rtx arg_offset_rtx;

  /* Quantities of various kinds of registers
     used for the current function's args.  */
  CUMULATIVE_ARGS info;

  /* The arg pointer hard register, or the pseudo into which it was copied.  */
  rtx internal_arg_pointer;
};

/* Data for function partitioning.  */
struct function_subsections GTY(())
{
  /* Assembly labels for the hot and cold text sections, to
     be used by debugger functions for determining the size of text
     sections.  */

  const char *hot_section_label;
  const char *cold_section_label;
  const char *hot_section_end_label;
  const char *cold_section_end_label;

  /* String to be used for name of cold text sections, via
     targetm.asm_out.named_section.  */

  const char *unlikely_text_section_name;
};

/* Datastructures maintained for currently processed function in RTL form.  */
struct rtl_data GTY(())
{
  struct expr_status expr;
  struct emit_status emit;
  struct varasm_status varasm;
  struct incoming_args args;
  struct function_subsections subsections;
  struct rtl_eh eh;

  /* For function.c  */

  /* # of bytes of outgoing arguments.  If ACCUMULATE_OUTGOING_ARGS is
     defined, the needed space is pushed by the prologue.  */
  int outgoing_args_size;

  /* If nonzero, an RTL expression for the location at which the current
     function returns its result.  If the current function returns its
     result in a register, current_function_return_rtx will always be
     the hard register containing the result.  */
  rtx return_rtx;

  /* Opaque pointer used by get_hard_reg_initial_val and
     has_hard_reg_initial_val (see integrate.[hc]).  */
  struct initial_value_struct *hard_reg_initial_vals;

  /* List (chain of EXPR_LIST) of labels heading the current handlers for
     nonlocal gotos.  */
  rtx x_nonlocal_goto_handler_labels;

  /* Label that will go on function epilogue.
     Jumping to this label serves as a "return" instruction
     on machines which require execution of the epilogue on all returns.  */
  rtx x_return_label;

  /* Label that will go on the end of function epilogue.
     Jumping to this label serves as a "naked return" instruction
     on machines which require execution of the epilogue on all returns.  */
  rtx x_naked_return_label;

  /* List (chain of EXPR_LISTs) of all stack slots in this function.
     Made for the sake of unshare_all_crtl->  */
  rtx x_stack_slot_list;

  /* Place after which to insert the tail_recursion_label if we need one.  */
  rtx x_stack_check_probe_note;

  /* Location at which to save the argument pointer if it will need to be
     referenced.  There are two cases where this is done: if nonlocal gotos
     exist, or if vars stored at an offset from the argument pointer will be
     needed by inner routines.  */
  rtx x_arg_pointer_save_area;

  /* Dynamic Realign Argument Pointer used for realigning stack.  */
  rtx drap_reg;

  /* Offset to end of allocated area of stack frame.
     If stack grows down, this is the address of the last stack slot allocated.
     If stack grows up, this is the address for the next slot.  */
  HOST_WIDE_INT x_frame_offset;

  /* Insn after which register parms and SAVE_EXPRs are born, if nonopt.  */
  rtx x_parm_birth_insn;

  /* List of all used temporaries allocated, by level.  */
  VEC(temp_slot_p,gc) *x_used_temp_slots;

  /* List of available temp slots.  */
  struct temp_slot *x_avail_temp_slots;

  /* Current nesting level for temporaries.  */
  int x_temp_slot_level;

  /* Nonzero if current function must be given a frame pointer.
     Set in global.c if anything is allocated on the stack there.  */
  unsigned int need_frame_pointer : 1;

  /* Nonzero if need_frame_pointer has been set.  */
  unsigned int need_frame_pointer_set : 1;
};

#define return_label (crtl->x_return_label)
#define naked_return_label (crtl->x_naked_return_label)
#define stack_slot_list (crtl->x_stack_slot_list)
#define parm_birth_insn (crtl->x_parm_birth_insn)
#define frame_offset (crtl->x_frame_offset)
#define stack_check_probe_note (crtl->x_stack_check_probe_note)
#define arg_pointer_save_area (crtl->x_arg_pointer_save_area)
#define used_temp_slots (crtl->x_used_temp_slots)
#define avail_temp_slots (crtl->x_avail_temp_slots)
#define temp_slot_level (crtl->x_temp_slot_level)
#define nonlocal_goto_handler_labels (crtl->x_nonlocal_goto_handler_labels)
#define frame_pointer_needed (crtl->need_frame_pointer)

extern GTY(()) struct rtl_data x_rtl;

/* Accestor to RTL datastructures.  We keep them statically allocated now since
   we never keep multiple functions.  For threaded compiler we might however
   want to do differntly.  */
#define crtl (&x_rtl)

/* This structure can save all the important global and static variables
   describing the status of the current function.  */

struct function GTY(())
{
  struct eh_status *eh;

  /* The control flow graph for this function.  */
  struct control_flow_graph *cfg;
  /* SSA and dataflow information.  */
  struct gimple_df *gimple_df;

  /* The loops in this function.  */
  struct loops *x_current_loops;

  /* Value histograms attached to particular statements.  */
  htab_t GTY((skip)) value_histograms;

  /* For function.c.  */

  /* Points to the FUNCTION_DECL of this function.  */
  tree decl;

  /* Function containing this function, if any.  */
  struct function *outer;

  /* A PARM_DECL that should contain the static chain for this function.
     It will be initialized at the beginning of the function.  */
  tree static_chain_decl;

  /* An expression that contains the non-local goto save area.  The first
     word is the saved frame pointer and the second is the saved stack 
     pointer.  */
  tree nonlocal_goto_save_area;

  /* Function sequence number for profiling, debugging, etc.  */
  int funcdef_no;

  /* For md files.  */

  /* tm.h can use this to store whatever it likes.  */
  struct machine_function * GTY ((maybe_undef)) machine;

  /* The largest alignment needed on the stack, including requirement
     for outgoing stack alignment.  */
  unsigned int stack_alignment_needed;

  /* The largest alignment of slot allocated on the stack.  */
  unsigned int stack_alignment_used;

  /* The estimated stack alignment.  */
  unsigned int stack_alignment_estimated;

  /* Preferred alignment of the end of stack frame.  */
  unsigned int preferred_stack_boundary;

  /* Language-specific code can use this to store whatever it likes.  */
  struct language_function * language;

  /* Used types hash table.  */
  htab_t GTY ((param_is (union tree_node))) used_types_hash;

  /* For reorg.  */

  /* If some insns can be deferred to the delay slots of the epilogue, the
     delay list for them is recorded here.  */
  rtx epilogue_delay_list;

  /* Maximal number of entities in the single jumptable.  Used to estimate
     final flowgraph size.  */
  int max_jumptable_ents;

  /* UIDs for LABEL_DECLs.  */
  int last_label_uid;

  /* Line number of the end of the function.  */
  location_t function_end_locus;

  /* The variables unexpanded so far.  */
  tree unexpanded_var_list;

  /* A variable living at the top of the frame that holds a known value.
     Used for detecting stack clobbers.  */
  tree stack_protect_guard;

  /* Properties used by the pass manager.  */
  unsigned int curr_properties;
  unsigned int last_verified;

  /* Collected bit flags.  */

  /* Number of units of general registers that need saving in stdarg
     function.  What unit is depends on the backend, either it is number
     of bytes, or it can be number of registers.  */
  unsigned int va_list_gpr_size : 8;

  /* Number of units of floating point registers that need saving in stdarg
     function.  */
  unsigned int va_list_fpr_size : 8;


  /* How commonly executed the function is.  Initialized during branch
     probabilities pass.  */
  ENUM_BITFIELD (function_frequency) function_frequency : 2;

  /* Nonzero if function being compiled can call setjmp.  */
  unsigned int calls_setjmp : 1;

  /* Nonzero if function being compiled can call alloca,
     either as a subroutine or builtin.  */
  unsigned int calls_alloca : 1;

  /* Nonzero if function being compiled called builtin_return_addr or
     builtin_frame_address with nonzero count.  */
  unsigned int accesses_prior_frames : 1;

  /* Nonzero if the function calls __builtin_eh_return.  */
  unsigned int calls_eh_return : 1;


  /* Nonzero if function being compiled receives nonlocal gotos
     from nested functions.  */
  unsigned int has_nonlocal_label : 1;

  /* Nonzero if function saves all registers, e.g. if it has a nonlocal
     label that can reach the exit block via non-exceptional paths. */
  unsigned int saves_all_registers : 1;

  /* Nonzero if function being compiled has nonlocal gotos to parent
     function.  */
  unsigned int has_nonlocal_goto : 1;
  
  /* Nonzero if function being compiled has an asm statement.  */
  unsigned int has_asm_statement : 1;

  /* Nonzero if the current function is a thunk, i.e., a lightweight
     function implemented by the output_mi_thunk hook) that just
     adjusts one of its arguments and forwards to another
     function.  */
  unsigned int is_thunk : 1;

  /* This bit is used by the exception handling logic.  It is set if all
     calls (if any) are sibling calls.  Such functions do not have to
     have EH tables generated, as they cannot throw.  A call to such a
     function, however, should be treated as throwing if any of its callees
     can throw.  */
  unsigned int all_throwers_are_sibcalls : 1;

  /* Nonzero if profiling code should be generated.  */
  unsigned int profile : 1;

  /* Nonzero if stack limit checking should be enabled in the current
     function.  */
  unsigned int limit_stack : 1;


  /* Nonzero if current function uses stdarg.h or equivalent.  */
  unsigned int stdarg : 1;

  /* Nonzero if the back-end should not keep track of expressions that
     determine the size of variable-sized objects.  Normally, such
     expressions are saved away, and then expanded when the next
     function is started.  For example, if a parameter has a
     variable-sized type, then the size of the parameter is computed
     when the function body is entered.  However, some front-ends do
     not desire this behavior.  */
  unsigned int x_dont_save_pending_sizes_p : 1;

  /* Nonzero if the current function uses the constant pool.  */
  unsigned int uses_const_pool : 1;

  /* Nonzero if the current function uses pic_offset_table_rtx.  */
  unsigned int uses_pic_offset_table : 1;

  /* Nonzero if the current function needs an lsda for exception handling.  */
  unsigned int uses_eh_lsda : 1;

  /* Nonzero if code to initialize arg_pointer_save_area has been emitted.  */
  unsigned int arg_pointer_save_area_init : 1;

  unsigned int after_inlining : 1;

  /* Set when the call to function itself has been emit.  */
  unsigned int recursive_call_emit : 1;


  /* Set when the tail call has been produced.  */
  unsigned int tail_call_emit : 1;

  /* FIXME tuples: This bit is temporarily here to mark when a
     function has been gimplified, so we can make sure we're not
     creating non GIMPLE tuples after gimplification.  */
  unsigned int gimplified : 1;

  /* Fields below this point are not set for abstract functions; see
     allocate_struct_function.  */

  /* Nonzero if function being compiled needs to be given an address
     where the value should be stored.  */
  unsigned int returns_struct : 1;

  /* Nonzero if function being compiled needs to
     return the address of where it has put a structure value.  */
  unsigned int returns_pcc_struct : 1;

  /* Nonzero if pass_tree_profile was run on this function.  */
  unsigned int after_tree_profile : 1;

  /* Nonzero if, by estimation, current function stack needs realignment. */
  unsigned int stack_realign_needed : 1;

  /* Nonzero if function stack realignment is really needed. This flag
     will be set after reload if by then criteria of stack realignment
     is still true. Its value may be contridition to stack_realign_needed
     since the latter was set before reload. This flag is more accurate
     than stack_realign_needed so prologue/epilogue should be generated
     according to both flags  */
  unsigned int stack_realign_really : 1;

  /* Nonzero if function being compiled needs dynamic realigned
     argument pointer (drap) if stack needs realigning.  */
  unsigned int need_drap : 1;

  /* Nonzero if current function needs to save/restore parameter
     pointer register in prolog, because it is a callee save reg.  */
  unsigned int save_param_ptr_reg : 1;

  /* Nonzero if function stack realignment estimatoin is done.  */
  unsigned int stack_realign_processed : 1;

  /* Nonzero if function stack realignment has been finalized.  */
  unsigned int stack_realign_finalized : 1;
};

/* If va_list_[gf]pr_size is set to this, it means we don't know how
   many units need to be saved.  */
#define VA_LIST_MAX_GPR_SIZE	255
#define VA_LIST_MAX_FPR_SIZE	255

/* The function currently being compiled.  */
extern GTY(()) struct function *cfun;

/* In order to ensure that cfun is not set directly, we redefine it so
   that it is not an lvalue.  Rather than assign to cfun, use
   push_cfun or set_cfun.  */
#define cfun (cfun + 0)

/* Pointer to chain of `struct function' for containing functions.  */
extern GTY(()) struct function *outer_function_chain;

/* Nonzero if we've already converted virtual regs to hard regs.  */
extern int virtuals_instantiated;

/* Nonzero if at least one trampoline has been created.  */
extern int trampolines_created;

/* cfun shouldn't be set directly; use one of these functions instead.  */
extern void set_cfun (struct function *new_cfun);
extern void push_cfun (struct function *new_cfun);
extern void pop_cfun (void);
extern void instantiate_decl_rtl (rtx x);

/* For backward compatibility... eventually these should all go away.  */
#define current_function_returns_struct (cfun->returns_struct)
#define current_function_returns_pcc_struct (cfun->returns_pcc_struct)
#define current_function_calls_setjmp (cfun->calls_setjmp)
#define current_function_calls_alloca (cfun->calls_alloca)
#define current_function_accesses_prior_frames (cfun->accesses_prior_frames)
#define current_function_calls_eh_return (cfun->calls_eh_return)
#define current_function_is_thunk (cfun->is_thunk)
#define current_function_stdarg (cfun->stdarg)
#define current_function_profile (cfun->profile)
#define current_function_funcdef_no (cfun->funcdef_no)
#define current_function_limit_stack (cfun->limit_stack)
#define current_function_uses_pic_offset_table (cfun->uses_pic_offset_table)
#define current_function_uses_const_pool (cfun->uses_const_pool)
#define current_function_epilogue_delay_list (cfun->epilogue_delay_list)
#define current_function_has_nonlocal_label (cfun->has_nonlocal_label)
#define current_function_saves_all_registers (cfun->saves_all_registers)
#define current_function_has_nonlocal_goto (cfun->has_nonlocal_goto)
#define current_function_has_asm_statement (cfun->has_asm_statement)

#define current_loops (cfun->x_current_loops)
#define dom_computed (cfun->cfg->x_dom_computed)
#define n_bbs_in_dom_tree (cfun->cfg->x_n_bbs_in_dom_tree)
#define VALUE_HISTOGRAMS(fun) (fun)->value_histograms
#define stack_realign_fp (cfun->stack_realign_needed && !cfun->need_drap)
#define stack_realign_drap (cfun->stack_realign_needed && cfun->need_drap)

/* Given a function decl for a containing function,
   return the `struct function' for it.  */
struct function *find_function_data (tree);

/* Identify BLOCKs referenced by more than one NOTE_INSN_BLOCK_{BEG,END},
   and create duplicate blocks.  */
extern void reorder_blocks (void);

/* Set BLOCK_NUMBER for all the blocks in FN.  */
extern void number_blocks (tree);

extern void clear_block_marks (tree);
extern tree blocks_nreverse (tree);

/* Return size needed for stack frame based on slots so far allocated.
   This size counts from zero.  It is not rounded to STACK_BOUNDARY;
   the caller may have to do that.  */
extern HOST_WIDE_INT get_frame_size (void);

/* Issue an error message and return TRUE if frame OFFSET overflows in
   the signed target pointer arithmetics for function FUNC.  Otherwise
   return FALSE.  */
extern bool frame_offset_overflow (HOST_WIDE_INT, tree);

/* A pointer to a function to create target specific, per-function
   data structures.  */
extern struct machine_function * (*init_machine_status) (void);

/* Save and restore status information for a nested function.  */
extern void free_after_parsing (struct function *);
extern void free_after_compilation (struct function *);

extern void init_varasm_status (void);

#ifdef RTX_CODE
extern void diddle_return_value (void (*)(rtx, void*), void*);
extern void clobber_return_register (void);
#endif

extern rtx get_arg_pointer_save_area (void);

/* Returns the name of the current function.  */
extern const char *current_function_name (void);
/* Returns the assembler name (raw, mangled) of the current function.  */
extern const char *current_function_assembler_name (void);

extern void do_warn_unused_parameter (tree);

extern bool pass_by_reference (CUMULATIVE_ARGS *, enum machine_mode,
			       tree, bool);
extern bool reference_callee_copied (CUMULATIVE_ARGS *, enum machine_mode,
				     tree, bool);

extern void used_types_insert (tree);

extern int get_next_funcdef_no (void);
#endif  /* GCC_FUNCTION_H */
