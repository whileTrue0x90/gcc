/* Prototypes for m32r.c functions used in the md file & elsewhere.
   Copyright (C) 1999 Free Software Foundation, Inc.

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

/* Function prototypes that cannot exist in v850.h due to dependency
   compilcations.  */
#define Mmode enum machine_mode

extern void   sbss_section			PROTO ((void));
extern void   sdata_section			PROTO ((void));
extern void   m32r_init				PROTO ((void));
extern void   m32r_init_expanders		PROTO ((void));
extern unsigned m32r_compute_frame_size		PROTO ((int));
extern int    m32r_first_insn_address		PROTO ((void));
extern void   m32r_expand_prologue		PROTO ((void));
extern void   m32r_output_function_prologue	PROTO ((FILE *, int));
extern void   m32r_output_function_epilogue	PROTO ((FILE *, int));
extern void   m32r_finalize_pic			PROTO ((void));
extern void   m32r_asm_file_start		PROTO ((FILE *));
extern void   m32r_sched_init 			PROTO ((FILE *, int));
extern int    direct_return 			PROTO ((void));
#ifdef TREE_CODE
extern int    m32r_valid_machine_decl_attribute	PROTO ((tree, tree, tree, tree));
extern int    m32r_comp_type_attributes		PROTO ((tree, tree));
extern void   m32r_select_section		PROTO ((tree, int));
extern void   m32r_encode_section_info		PROTO ((tree));
extern enum m32r_function_type m32r_compute_function_type PROTO ((tree));
extern void   m32r_select_section 		PROTO ((tree, int));
extern void   m32r_set_default_type_attributes  PROTO ((tree));

#ifdef HAVE_MACHINE_MODES
extern void   m32r_setup_incoming_varargs	PROTO ((CUMULATIVE_ARGS *, Mmode, tree, int *, int));
extern int    function_arg_partial_nregs	PROTO ((CUMULATIVE_ARGS *, Mmode, tree, int));
#endif
#endif /* TREE_CODE */

#ifdef RTX_CODE
extern int    easy_di_const			PROTO ((rtx));
extern int    easy_df_const			PROTO ((rtx));
extern int    m32r_select_cc_mode		PROTO ((int, rtx, rtx));
extern rtx    gen_compare			PROTO ((enum rtx_code, rtx, rtx, int));
extern rtx    gen_split_move_double		PROTO ((rtx *));
extern int    m32r_address_code			PROTO ((rtx));
extern void   m32r_initialize_trampoline	PROTO ((rtx, rtx, rtx));
extern int    zero_and_one			PROTO ((rtx, rtx));
extern char * emit_cond_move			PROTO ((rtx *, rtx));
extern char * m32r_output_block_move 		PROTO ((rtx, rtx *));
extern void   m32r_expand_block_move 		PROTO ((rtx *));
extern void   m32r_print_operand		PROTO ((FILE *, rtx, int));
extern void   m32r_print_operand_address	PROTO ((FILE *, rtx));
extern int    m32r_address_cost 		PROTO ((rtx));
extern int    m32r_adjust_cost 			PROTO ((rtx, rtx, rtx, int));
extern int    m32r_adjust_priority 		PROTO ((rtx, int));
extern void   m32r_sched_reorder 		PROTO ((FILE *, int, rtx *, int));
extern int    m32r_sched_variable_issue 	PROTO ((FILE *, int, rtx, int));
extern int    m32r_not_same_reg 		PROTO ((rtx, rtx));

#ifdef HAVE_MACHINE_MODES
extern int    call_address_operand		PROTO ((rtx, Mmode));
extern int    call_operand			PROTO ((rtx, Mmode));
extern int    symbolic_operand			PROTO ((rtx, Mmode));
extern int    small_data_operand		PROTO ((rtx, Mmode));
extern int    addr24_operand			PROTO ((rtx, Mmode));
extern int    addr32_operand			PROTO ((rtx, Mmode));
extern int    call26_operand			PROTO ((rtx, Mmode));
extern int    seth_add3_operand			PROTO ((rtx, Mmode));
extern int    cmp_int16_operand			PROTO ((rtx, Mmode));
extern int    uint16_operand			PROTO ((rtx, Mmode));
extern int    reg_or_int16_operand		PROTO ((rtx, Mmode));
extern int    reg_or_uint16_operand		PROTO ((rtx, Mmode));
extern int    reg_or_cmp_int16_operand		PROTO ((rtx, Mmode));
extern int    two_insn_const_operand		PROTO ((rtx, Mmode));
extern int    move_src_operand			PROTO ((rtx, Mmode));
extern int    move_double_src_operand		PROTO ((rtx, Mmode));
extern int    move_dest_operand			PROTO ((rtx, Mmode));
extern int    eqne_comparison_operator		PROTO ((rtx, Mmode));
extern int    signed_comparison_operator	PROTO ((rtx, Mmode));
extern int    memreg_operand			PROTO ((rtx, Mmode));
extern int    small_insn_p			PROTO ((rtx, Mmode));
extern int    large_insn_p			PROTO ((rtx, Mmode));
extern int    conditional_move_operand		PROTO ((rtx, Mmode));
extern int    carry_compare_operand		PROTO ((rtx, Mmode));
extern int    m32r_block_immediate_operand 	PROTO ((rtx, Mmode));
extern int    extend_operand			PROTO ((rtx, Mmode));
extern int    reg_or_eq_int16_operand		PROTO ((rtx, Mmode));
extern int    int8_operand			PROTO ((rtx, Mmode));
#endif /* HAVE_MACHINE_MODES */

#ifdef TREE_CODE
extern struct rtx_def * m32r_va_arg		PROTO ((tree, tree));
#endif /* TREE_CODE */
#endif /* RTX_CODE */

#undef  Mmode
