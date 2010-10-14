/* Definitions of target machine for GNU compiler. Matsushita MN10300 series
   Copyright (C) 2000, 2003, 2004, 2005, 2007, 2009, 2010
   Free Software Foundation, Inc.
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

#define Mmode enum machine_mode
#define Cstar const char *
#define Rclas enum reg_class

#ifdef RTX_CODE
extern rtx   legitimize_pic_address (rtx, rtx);
extern int   legitimate_pic_operand_p (rtx);
extern bool  mn10300_function_value_regno_p (const unsigned int);
extern void  mn10300_gen_multiple_store (int);
extern int   mn10300_get_live_callee_saved_regs (void);
extern bool  mn10300_hard_regno_mode_ok (unsigned int, Mmode);
extern bool  mn10300_legitimate_constant_p (rtx);
extern bool  mn10300_modes_tieable (Mmode, Mmode);
extern Cstar mn10300_output_cmp (rtx, rtx);
extern void  mn10300_print_reg_list (FILE *, int);
extern Rclas mn10300_secondary_reload_class (Rclas, Mmode, rtx);
extern Mmode mn10300_select_cc_mode (rtx);
extern bool  mn10300_wide_const_load_uses_clr (rtx operands[2]);
extern void  print_operand (FILE *, rtx, int);
extern void  print_operand_address (FILE *, rtx);
extern int   store_multiple_operation (rtx, Mmode);
extern int   symbolic_operand (rtx, Mmode);
#endif /* RTX_CODE */

#ifdef TREE_CODE
extern struct rtx_def *function_arg (CUMULATIVE_ARGS *, Mmode, tree, int);
#endif /* TREE_CODE */

extern int   can_use_return_insn (void);
extern void  expand_prologue (void);
extern void  expand_epilogue (void);
extern int   initial_offset (int, int);
extern int   mask_ok_for_mem_btst (int, int);

#undef Mmode
#undef Cstar
#undef Rclas
