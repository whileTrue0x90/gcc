/* Exception Handling interface routines.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
   Contributed by Mike Stump <mrs@cygnus.com>.

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

#if !defined(NULL_RTX) && !defined(rtx)
typedef struct rtx_def *_except_rtx;
#define rtx _except_rtx
#endif

#ifdef TREE_CODE

/* A stack of labels. CHAIN points to the next entry in the stack.  */

struct label_node {
  union {
    rtx rlabel;
    tree tlabel;
  } u;
  struct label_node *chain;
};

/* An eh_entry is used to describe one exception handling region.

   OUTER_CONTEXT is the label used for rethrowing into the outer context.

   EXCEPTION_HANDLER_LABEL is the label corresponding to the handler
   for this region.

   LABEL_USED indicates whether a CATCH block has already used this
   label or not. New ones are needed for additional catch blocks if
   it has.

   FINALIZATION is the tree codes for the handler, or is NULL_TREE if
   one hasn't been generated yet, or is integer_zero_node to mark the
   end of a group of try blocks.  */

struct eh_entry {
  rtx outer_context;
  rtx exception_handler_label;
  tree finalization;
  int label_used;
};

/* A list of EH_ENTRYs. ENTRY is the entry; CHAIN points to the next
   entry in the list, or is NULL if this is the last entry.  */

struct eh_node {
  struct eh_entry *entry;
  struct eh_node *chain;
};

/* A stack of EH_ENTRYs. TOP is the topmost entry on the stack. TOP is
   NULL if the stack is empty.  */

struct eh_stack {
  struct eh_node *top;
};

/* A queue of EH_ENTRYs. HEAD is the front of the queue; TAIL is the
   end (the latest entry). HEAD and TAIL are NULL if the queue is
   empty.  */

struct eh_queue {
  struct eh_node *head;
  struct eh_node *tail;
};


/* Start an exception handling region.  All instructions emitted after
   this point are considered to be part of the region until
   expand_eh_region_end () is invoked.  */

extern void expand_eh_region_start		PROTO((void));

/* Just like expand_eh_region_start, except if a cleanup action is
   entered on the cleanup chain, the TREE_PURPOSE of the element put
   on the chain is DECL.  DECL should be the associated VAR_DECL, if
   any, otherwise it should be NULL_TREE.  */

extern void expand_eh_region_start_for_decl	PROTO((tree));

/* Start an exception handling region for the given cleanup action.
   All instructions emitted after this point are considered to be part
   of the region until expand_eh_region_end () is invoked.  CLEANUP is
   the cleanup action to perform.  The return value is true if the
   exception region was optimized away.  If that case,
   expand_eh_region_end does not need to be called for this cleanup,
   nor should it be.

   This routine notices one particular common case in C++ code
   generation, and optimizes it so as to not need the exception
   region.  */

extern int expand_eh_region_start_tree		PROTO((tree, tree));

/* End an exception handling region.  The information about the region
   is found on the top of ehstack.

   HANDLER is either the cleanup for the exception region, or if we're
   marking the end of a try block, HANDLER is integer_zero_node.

   HANDLER will be transformed to rtl when expand_leftover_cleanups ()
   is invoked.  */

extern void expand_eh_region_end		PROTO((tree));

/* Push RLABEL or TLABEL onto LABELSTACK. Only one of RLABEL or TLABEL
   should be set; the other must be NULL.  */

extern void push_label_entry			PROTO((struct label_node **labelstack, rtx rlabel, tree tlabel));

/* Pop the topmost entry from LABELSTACK and return its value as an
   rtx node. If LABELSTACK is empty, return NULL.  */

extern rtx pop_label_entry			PROTO((struct label_node **labelstack));

/* Return the topmost entry of LABELSTACK as a tree node, or return
   NULL_TREE if LABELSTACK is empty.  */

extern tree top_label_entry			PROTO((struct label_node **labelstack));

/* A set of insns for the catch clauses in the current function. They
   will be emitted at the end of the current function.  */

extern rtx catch_clauses;

#endif

/* Test: is exception handling turned on? */

extern int doing_eh				       PROTO ((int));

/* Toplevel initialization for EH.  */

void set_exception_lang_code                    PROTO((short));
void set_exception_version_code                 PROTO((short));

/* A list of handlers asocciated with an exception region. HANDLER_LABEL
   is the the label that control should be transfered to if the data
   in TYPE_INFO matches an exception. a value of NULL_TREE for TYPE_INFO
   means This is a cleanup, and must always be called. A value of
   CATCH_ALL_TYPE works like a cleanup, but a call to the runtime matcher
   is still performed to avoid being caught by a different language
   exception. NEXT is a pointer to the next handler for this region. 
   NULL means there are no more. */

#define CATCH_ALL_TYPE   (tree *) -1

typedef struct handler_info 
{
  rtx  handler_label;
  void *type_info;
  struct handler_info *next;
} handler_info;


/* Add a new eh_entry for this function, The parameter specifies what
   exception region number NOTE insns use to delimit this range. 
   The integer returned is uniquely identifies this exception range
   within an internal table. */

int new_eh_region_entry                         PROTO((int));

/* Add new handler information to an exception range. The  first parameter
   specifies the range number (returned from new_eh_entry()). The second
   parameter specifies the handler.  By default the handler is inserted at
   the end of the list. A handler list may contain only ONE NULL_TREE
   typeinfo entry. Regardless where it is positioned, a NULL_TREE entry
   is always output as the LAST handler in the exception table for a region. */

void add_new_handler                       PROTO((int, struct handler_info *));

/* Remove a handler label. The handler label is being deleted, so all
   regions which reference this handler should have it removed from their
   list of possible handlers. Any region which has the final handler
   removed can be deleted. */

void remove_handler                        PROTO((rtx));

/* Create a new handler structure initialized with the handler label and
   typeinfo fields passed in. */

struct handler_info *get_new_handler            PROTO((rtx, void *));

/* Make a duplicate of an exception region by copying all the handlers
   for an exception region. Return the new handler index. */

int duplicate_handlers                          PROTO((int, int));


/* Get a pointer to the first handler in an exception region's list. */

struct handler_info *get_first_handler          PROTO((int));


extern void init_eh				PROTO((void));

/* Initialization for the per-function EH data.  */

extern void init_eh_for_function		PROTO((void));

/* Generate an exception label. Use instead of gen_label_rtx */

extern rtx gen_exception_label                  PROTO((void));

/* Adds an EH table entry for EH entry number N. Called from
   final_scan_insn for NOTE_INSN_EH_REGION_BEG.  */

extern void add_eh_table_entry			PROTO((int n));

/* Start a catch clause, triggered by runtime value paramter. */

#ifdef TREE_CODE
extern void start_catch_handler                 PROTO((tree));
#endif

/* Returns a non-zero value if we need to output an exception table.  */

extern int exception_table_p			PROTO((void));

/* Outputs the exception table if we have one.  */

extern void output_exception_table		PROTO((void));

/* Given a return address in ADDR, determine the address we should use
   to find the corresponding EH region.  */

extern rtx eh_outer_context			PROTO((rtx addr));

/* Called at the start of a block of try statements for which there is
   a supplied catch handler.  */

extern void expand_start_try_stmts 		PROTO((void));

/* Called at the start of a block of catch statements. It terminates the
   previous set of try statements.  */

extern void expand_start_all_catch		PROTO((void));

/* Called at the end of a block of catch statements.  */

extern void expand_end_all_catch		PROTO((void));

#ifdef TREE_CODE
/* Create a new exception region and add the handler for the region
   onto a list. These regions will be ended (and their handlers
   emitted) when end_protect_partials is invoked.  */

extern void add_partial_entry			PROTO((tree handler));
#endif

/* End all of the pending exception regions that have handlers added with
   push_protect_entry ().  */

extern void end_protect_partials		PROTO((void));

/* An internal throw.  */

extern void expand_internal_throw		PROTO((void));

/* Called from expand_exception_blocks and expand_end_catch_block to
   expand and pending handlers.  */

extern void expand_leftover_cleanups		PROTO((void));

/* If necessary, emit insns to get EH context for the current
   function. */

extern void emit_eh_context			PROTO((void));

/* If necessary, emit insns for the start of per-function unwinder for
   the current function.  */

extern void emit_unwinder			PROTO((void));

/* If necessary, emit insns for the end of the per-function unwinder
   for the current function.  */

extern void end_eh_unwinder			PROTO((void));

/* Builds a list of handler labels and puts them in the global
   variable exception_handler_labels.  */

extern void find_exception_handler_labels	PROTO((void));

/* Determine if an arbitrary label is an exception label */

extern int is_exception_handler_label           PROTO((int));

/* Performs sanity checking on the check_exception_handler_labels
   list.  */

extern void check_exception_handler_labels	PROTO((void));

/* A stack used to keep track of the label used to resume normal program
   flow out of the current exception handler region.  */

extern struct label_node *caught_return_label_stack;

/* Keeps track of the label used as the context of a throw to rethrow an
   exception to the outer exception region.  */

extern struct label_node *outer_context_label_stack;

/* A random area used for purposes elsewhere.  */

extern struct label_node *false_label_stack;

/* A list of labels used for exception handlers. It is created by
   find_exception_handler_labels for the optimization passes.  */

extern rtx exception_handler_labels;

/* Performs optimizations for exception handling, such as removing
   unnecessary exception regions. Invoked from jump_optimize ().  */

extern void exception_optimize			PROTO((void));

/* Return EH context (and set it up once per fn).  */
extern rtx get_eh_context			PROTO((void));

/* Get the dynamic handler chain.  */
extern rtx get_dynamic_handler_chain		PROTO((void));

/* Get the dynamic cleanup chain.  */
extern rtx get_dynamic_cleanup_chain		PROTO((void));

/* Throw an exception.  */

extern void emit_throw				PROTO((void));

/* One to use setjmp/longjmp method of generating code.  */

extern int exceptions_via_longjmp;

/* One to enable asynchronous exception support.  */

extern int asynchronous_exceptions;

/* One to protect cleanup actions with a handler that calls
   __terminate, zero otherwise.  */

extern int protect_cleanup_actions_with_terminate;

#ifdef TREE_CODE
extern tree protect_with_terminate		PROTO((tree));
#endif

extern void expand_fixup_region_start	PROTO((void));
#ifdef TREE_CODE
extern void expand_fixup_region_end	PROTO((tree));
#endif

/* Various hooks for the DWARF 2 __throw routine.  */

void expand_builtin_unwind_init		PROTO((void));
rtx expand_builtin_dwarf_fp_regnum	PROTO((void));
rtx expand_builtin_eh_stub		PROTO((void));
rtx expand_builtin_eh_stub_old          PROTO((void));
#ifdef TREE_CODE
rtx expand_builtin_frob_return_addr	PROTO((tree));
rtx expand_builtin_extract_return_addr	PROTO((tree));
void expand_builtin_set_return_addr_reg PROTO((tree));
void expand_builtin_set_eh_regs		PROTO((tree, tree));
rtx expand_builtin_dwarf_reg_size	PROTO((tree, rtx));
#endif


/* Checking whether 2 instructions are within the same exception region. */

int in_same_eh_region                   PROTO((rtx, rtx));
void free_insn_eh_region                PROTO((void));
void init_insn_eh_region                PROTO((rtx, int));

#ifdef rtx
#undef rtx
#endif
