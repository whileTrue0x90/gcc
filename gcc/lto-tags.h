/* Declarations and definitions of codes relating to the
   encoding of gimple into the object files.

   Copyright (C) 2006 Free Software Foundation, Inc.
   Contributed by Kenneth Zadeck <zadeck@naturalbridge.com>

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING.  If not, write to the Free
   Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.  */

#ifndef GCC_LTO_TAGS_H
#define GCC_LTO_TAGS_H

#include "sbitmap.h"

#define LTO_major_version 0
#define LTO_minor_version 2

/* This file is one header in a collection of files that write the
   gimple intermediate code for a function into the assembly stream
   and read it back.

   This comment describes, in rough terms the methods used to encode
   that gimple stream.  This does not describe gimple itself.

   The encoding for a function consists of 8 (9 in debugging mode),
   sections of information.

   1) The header.
   2) FIELD_DECLS.
   3) FUNCTION_DECLS.
   4) global VAR_DECLS.
   5) types.
   6) Gimple for local decls.
   7) Gimple for the function.
   8) Strings.
   9) Redundant information to aid in debugging the stream.
      This is only present if the compiler is built with
      LTO_STREAM_DEBUGGING defined.

   Sections 1-5 are in plain text that can easily be read in the
   assembly file.  Sections 6-8 will be zlib encoded in the future.

   1) THE HEADER.
*/

/* The is the first part of the record for a function in the .o file.  */
struct lto_function_header
{
  int16_t major_version;     /* LTO_major_version. */
  int16_t minor_version;     /* LTO_minor_version. */
  int32_t num_field_decls;   /* Number of FIELD_DECLS.  */
  int32_t num_fn_decls;      /* Number of FUNCTION_DECLS.  */
  int32_t num_var_decls;     /* Number of non local VAR_DECLS.  */
  int32_t num_types;         /* Number of types.  */
  int32_t compressed_size;   /* Size compressed or 0 if not compressed.  */
  int32_t local_decls_size;  /* Size of local par and var decl region. */
  int32_t main_size;         /* Size of main gimple body of function.  */
  int32_t string_size;       /* Size of the string table.  */
}; 

/* 2-5) THE GLOBAL DECLS AND TYPES.

     The global decls and types are encoded in the same way.  For each
     entry, there is a pair of words.  The first is the debugging
     section number and the second is the offset within the section to
     the entry.

     FIXME: This encoding may change change so that the first word is
     a label for the debugging section.  This will cause more work for
     the linker but will make ln -r work properly.

   6-7) GIMPLE FOR THE LOCAL DECLS AND THE FUNCTION BODY.

     The gimple consists of a set of records.

       Each record starts with a "LTO_" tag defined in this file.  The
       mapping between gimple tree codes and LTO code described in
       lto-tree-tags.def along with useful macros that allow the
       simple cases to be processed mechanically.

       For the tree types that can be processed mechanically, the form
       of the output is:
         tag          - The LTO_ tag.

	 type         - If the tree code has a bit set in
                        lto_types_needed_for, a reference to the type
			is generated.  This reference is an index into
                        (6).  The index is encoded in uleb128 form.

	 flags        - The set of flags defined for this tree code
		        packed into a word where the low order 2 bits
		        are used to encode the presence or absense of
		        a file and line number respectively. The word
		        is output in uleb128 form. The encoding and
		        decoding of the flags is controlled by
		        lto-tree-flags.def which is designed so that
		        it is useful on both sides.
			
	 file         - If the file of this gimple record is different
                        from the file of the previous record, the file
                        is encoded into the string table and the
                        offset of that entry in uleb128 is output in
                        the stream.  The flags are ored with 0x2 to
                        indicate the presence of this.

	 line number  - If the line number of this gimple record is
                        different from the line number of the previous
			record, the line number in uleb128 is output
			in the stream.  The flags are ored with 0x1 to
			indicate the presence of this.

	 children     - If this gimple record has children defined,
                        they follow here.  For automaticly generated
			gimple forms, TREE_CODE_LENGTH sepcifies the
                        number of children.

     Many gimple forms cannot be automatically processed.  There are
     two reasons for this: either extra fields must be processed
     beyond those defined in the gimple core or some of the children
     are not always there.

     For these cases, the best reference is the code.  The the
     greatest extent possible, the automatic mechanisms are used to
     processes the pieces of such trees.  But the spec is really the
     code and will change as gimple evolves.

     THE FUNCTION
	
     At the top level of (7) is the function. It consists of five
     pieces:

     LTO_function     - The tag.
     eh tree          - This is all of the exception handling regions
                        put out in a post order traversial of the
                        tree.  Siblings are output as lists terminated
			by a 0.  The set of fields matches the fields
			defined in except.c.

     last_basic_block - in uleb128 form.

     basic blocks     - This is the set of basic blocks.

     zero             - The termination of the basic blocks.

     FIXME: I am sure that there is a lot of random stuff for a
     function that needs to be put out and is not here.  This will be
     fixed if someone steps up and enumerates what is necessary or
     when we really try to make this work.  I do not know where to
     look for everything.

     BASIC BLOCKS

     There are two forms of basic blocks depending on if they are
     empty or not.

     The basic block consists of:

     LTO_bb1 or LTO_bb0 - The tag.

     bb->index          - the index in uleb128 form.

     #succs             - The number of successors un uleb128 form.

     the successors     - For each edge, a pair.  The first of the
                          pair is the index of the successor in
                          uleb128 form and the second are the flags in
                          uleb128 form.

     the statements     - A gimple tree, as described above.
                          These are only present for LTO_BB1.
                          Following each statement is an optional
                          exception handling record LTO_set_eh1 or
                          LTO_set_eh0 if the exception handling for
                          this statement differed from the last region
                          output.  LTO_set_eh0 is a special case that
                          sets the region to 0. LTO_set_eh1 contains
			  the region number in sleb128 form.
			
     zero               - This is only present for LTO_BB1 and is used
			  to terminate the statements and exception
			  regions within this block.

   8) STRINGS

     String are represented in the table as pairs, a length in ULEB128
     form followed by the data for the string.

   9) STREAM DEBUGGING

     tbd
*/

/* When we get a strongly typed gimple, this flag should be set to 0
   so we do not waste so much space printing out largely redundant
   type codes.  */
#define REDUNDANT_TYPE_SYSTEM 1

/* The 1 variant indicates that the basic block is not empty.  */
#define LTO_bb0                         0x001
#define LTO_bb1                         0x002
/* Variant 1 is used to set region to no zero value.  */
#define LTO_set_eh0                     0x003
#define LTO_set_eh1                     0x004

/* All of the expression types that we can see.  */
#define LTO_abs_expr                    0x005
#define LTO_addr_expr                   0x006
#define LTO_align_indirect_ref          0x007
#define LTO_array_range_ref             0x008
#define LTO_array_ref                   0x009
#define LTO_asm_expr                    0x00A
#define LTO_assert_expr                 0x00B
#define LTO_bit_and_expr                0x00C
#define LTO_bit_ior_expr                0x00D
/* Variant 1 is used if both operands 1 and 2 are constant ints.  */
#define LTO_bit_field_ref0              0x00E
#define LTO_bit_field_ref1              0x00F
#define LTO_bit_not_expr                0x010
#define LTO_bit_xor_expr                0x011
/* Call_exprs are terminated by a 0 to indicate the end of the
   parameter list.  Variant 1 indicates the presence of a call
   chain.  */
#define LTO_call_expr0                  0x012
#define LTO_call_expr1                  0x013
/* Variant 1 and 3 are if CASE_LOW exists and variant 2 and 3 are if
   CASE_HIGH exists.  */
#define LTO_case_label_expr0            0x014
#define LTO_case_label_expr1            0x015
#define LTO_case_label_expr2            0x016
#define LTO_case_label_expr3            0x017
#define LTO_ceil_div_expr               0x018
#define LTO_ceil_mod_expr               0x019
/* 1 if the elements are reals and 0 if the elements are ints.  */
#define LTO_complex_cst0                0x01A
#define LTO_complex_cst1                0x01B
#define LTO_complex_expr                0x01C
#define LTO_component_ref               0x01D
#define LTO_compound_expr               0x01E
#define LTO_cond_expr                   0x01F
#define LTO_conj_expr                   0x020
#define LTO_const_decl                  0x021
/* This form is terminated by a zero.  */
#define LTO_constructor                 0x022
#define LTO_constructor_range           0x023
#define LTO_convert_expr                0x024
#define LTO_dot_prod_expr               0x025
#define LTO_eq_expr                     0x026
#define LTO_exact_div_expr              0x027
#define LTO_exc_ptr_expr                0x028
#define LTO_field_decl                  0x029
#define LTO_filter_expr                 0x02A
#define LTO_fix_ceil_expr               0x02B
#define LTO_fix_floor_expr              0x02C
#define LTO_fix_round_expr              0x02D
#define LTO_fix_trunc_expr              0x02E
#define LTO_float_expr                  0x02F
#define LTO_floor_div_expr              0x030
#define LTO_floor_mod_expr              0x031
#define LTO_function_decl               0x032
#define LTO_ge_expr                     0x033
#define LTO_goto_expr                   0x034
#define LTO_gt_expr                     0x035
#define LTO_imagpart_expr               0x036
#define LTO_indirect_ref                0x037
#define LTO_integer_cst                 0x038
#define LTO_label_decl                  0x039
#define LTO_label_expr                  0x03A
#define LTO_le_expr                     0x03B
#define LTO_lrotate_expr                0x03C
#define LTO_lshift_expr                 0x03D
#define LTO_lt_expr                     0x03E
#define LTO_ltgt_expr                   0x03F
#define LTO_max_expr                    0x040
#define LTO_min_expr                    0x041
#define LTO_minus_expr                  0x042
#define LTO_misaligned_indirect_ref     0x043
#define LTO_modify_expr                 0x044
#define LTO_mult_expr                   0x045
#define LTO_ne_expr                     0x046
#define LTO_negate_expr                 0x047
#define LTO_non_lvalue_expr             0x048
#define LTO_nop_expr                    0x049
#define LTO_obj_type_ref                0x04A
#define LTO_ordered_expr                0x04B
#define LTO_parm_decl                   0x04C
#define LTO_plus_expr                   0x04D
#define LTO_range_expr                  0x04E
#define LTO_rdiv_expr                   0x04F
#define LTO_real_cst                    0x050
#define LTO_realign_load_expr           0x051
#define LTO_realpart_expr               0x052
#define LTO_reduc_max_expr              0x053
#define LTO_reduc_min_expr              0x054
#define LTO_reduc_plus_expr             0x055
#define LTO_result_decl                 0x056
/* Form "return;"  */
#define LTO_return_expr0                0x057
/* Form "return x;"  */
#define LTO_return_expr1                0x058
/* Form "return x=y;"  */
#define LTO_return_expr2                0x059
#define LTO_resx_expr                   0x05A
#define LTO_round_div_expr              0x05B
#define LTO_round_mod_expr              0x05C
#define LTO_rrotate_expr                0x05D
#define LTO_rshift_expr                 0x05E
#define LTO_ssa_name                    0x05F
#define LTO_string_cst                  0x060
/* Cases are terminated a zero.  */
#define LTO_switch_expr                 0x061
#define LTO_trunc_div_expr              0x062
#define LTO_trunc_mod_expr              0x063
#define LTO_truth_and_expr              0x064
#define LTO_truth_not_expr              0x065
#define LTO_truth_or_expr               0x066
#define LTO_truth_xor_expr              0x067
#define LTO_uneq_expr                   0x068
#define LTO_unge_expr                   0x069
#define LTO_ungt_expr                   0x06A
#define LTO_unle_expr                   0x06B
#define LTO_unlt_expr                   0x06C
#define LTO_unordered_expr              0x070
#define LTO_var_decl                    0x071
#define LTO_vec_cond_expr               0x072
#define LTO_vec_lshift_expr             0x073
#define LTO_vec_rshift_expr             0x074
/* 1 if the elements are reals and 0 if the elements are ints.  */
#define LTO_vector_cst0                 0x075
#define LTO_vector_cst1                 0x076
#define LTO_view_convert_expr           0x079
#define LTO_widen_mult_expr             0x07A
#define LTO_widen_sum_expr              0x07B
#define LTO_with_size_expr              0x07C


/* All of the statement types that do not also appear as
   expressions.  */
#define LTO_asm_inputs                  0x080
#define LTO_asm_outputs                 0x081
#define LTO_asm_clobbers                0x082

#define LTO_function                    0x083
#define LTO_attribute_list              0x084
#define LTO_local_var_decl              0x085
#define LTO_eh_table                    0x086

/* Each of these requires 4 variants.  1 and 3 are have_inner and 2
   and 3 are may_contain_throw.  */
#define LTO_eh_table_cleanup0           0x090
#define LTO_eh_table_cleanup1           0x091
#define LTO_eh_table_cleanup2           0x092
#define LTO_eh_table_cleanup3           0x093
#define LTO_eh_table_try0               0x094
#define LTO_eh_table_try1               0x095
#define LTO_eh_table_try2               0x096
#define LTO_eh_table_try3               0x097
#define LTO_eh_table_catch0             0x098
#define LTO_eh_table_catch1             0x099
#define LTO_eh_table_catch2             0x09A
#define LTO_eh_table_catch3             0x09B
#define LTO_eh_table_allowed0           0x09C
#define LTO_eh_table_allowed1           0x09D
#define LTO_eh_table_allowed2           0x09E
#define LTO_eh_table_allowed3           0x09F
#define LTO_eh_table_must_not_throw0    0x0A0
#define LTO_eh_table_must_not_throw1    0x0A1
#define LTO_eh_table_must_not_throw2    0x0A2
#define LTO_eh_table_must_not_throw3    0x0A3

/* There are 16 variants of the following decl bodies depending on the
   subtrees that may or may not be there in the decl_common part of
   the tree.
      variant |= DECL_ATTRIBUTES (decl) != NULL_TREE ? 0x01 : 0;
      variant |= DECL_SIZE_UNIT (decl)  != NULL_TREE ? 0x02 : 0;
      variant |= needs_backing_var                   ? 0x04 : 0;
      variant |= ABSTRACT_ORIGIN (decl) != NULL_TREE ? 0x08 : 0;
*/

#define LTO_local_var_decl_body0        0x0B0
#define LTO_parm_decl_body0             0x0C0

/* The string that is prepended on the DECL_ASSEMBLER_NAME to make the 
   section name for the function.  */
#define LTO_SECTION_NAME_PREFIX         ".lto_"

/* This bitmap is indexed by gimple type codes and contains a 1 if the 
   tree type needs to have the type written.  */
extern sbitmap lto_types_needed_for;



void lto_static_init (void);
#endif /* lto-tags.h */
