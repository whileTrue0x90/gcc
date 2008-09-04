// picoChip ASM file
//
//   Support for 32-bit arithmetic shift right.
//
//   Copyright (C) 2003, 2004, 2005  Free Software Foundation, Inc.
//   Contributed by picoChip Designs Ltd.
//   Maintained by Hariharan Sandanagobalane (hariharan@picochip.com)
//
//   This file is free software; you can redistribute it and/or modify it
//   under the terms of the GNU General Public License as published by the
//   Free Software Foundation; either version 2, or (at your option) any
//   later version.
//
//   In addition to the permissions in the GNU General Public License, the
//   Free Software Foundation gives you unlimited permission to link the
//   compiled version of this file into combinations with other programs,
//   and to distribute those combinations without any restriction coming
//   from the use of this file.  (The General Public License restrictions
//   do apply in other respects; for example, they cover modification of
//   the file, and distribution when not linked into a combine
//   executable.)
//
//   This file is distributed in the hope that it will be useful, but
//   WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public License
//   along with this program; see the file COPYING.  If not, write to
//   the Free Software Foundation, 51 Franklin Street, Fifth Floor,
//   Boston, MA 02110-1301, USA.

.section .text

.global ___ashrsi3
___ashrsi3:
_picoMark_FUNCTION_BEGIN=

// picoChip Function Prologue : &___ashrsi3 = 0 bytes

	// if (R2 > 15) goto _L2
	SUB.0 15,R2,r15
	JMPLT _L2
=->	COPY.0 R1,R3

	LSR.0 R1,R2,R1 // R1 := R1 >> R2
	// if (R2 == 0) goto _L4
	SUB.0 R2,0,r15
	JMPEQ _L4
=->	LSR.0 R0,R2,R0 // R2 := R0 >> R2

	SUB.0 16,R2,R4 // R4 := R4 - R2 (HI)
	ASR.0 R3,15,R5	// R5 = R1 >>{arith} 15
	LSL.0 R5,R4,R5 // R5 := R5 << R4
	LSL.0 R3,R4,R4 // R4 := R1 << R4
	OR.0 R5,R1,R1 // R3 := R5 IOR R3 (HI)
	BRA _L4
	=->	OR.0 R4,R0,R0 // R2 := R4 IOR R0 (HI)
_L2:
	ASR.0 R1,15,R1	// R4 = R1 >>{arith} 15
	SUB.0 16,R2,R5  // R5 := R5 - R2 (HI)
	LSR.0 R3,R2,R0 // R2 := R1 >> R2
	LSL.0 R1,R5,R5 // R5 := R4 << R5
	OR.0 R5,R0,R5 // R2 := R5 IOR R2 (HI)
	SUB.0 R2,16,r15  // R5 := R5 - R2 (HI)
	COPYNE R5,R0
_L4:
	JR (R12)	// Return to caller 

_picoMark_FUNCTION_END=

// picoChip Function Epilogue : __ashrsi3
//============================================================================
// All DWARF information between this marker, and the END OF DWARF
// marker should be included in the source file. Search for
// FUNCTION_STACK_SIZE_GOES_HERE and FUNCTION NAME GOES HERE, and
// provide the relevent information. Add markers called
// _picoMark_FUNCTION_BEGIN and _picoMark_FUNCTION_END around the
// function in question.
//============================================================================

//============================================================================
// Frame information. 
//============================================================================

.section .debug_frame
_picoMark_DebugFrame=

// Common CIE header.
.unalignedInitLong _picoMark_CieEnd-_picoMark_CieBegin
_picoMark_CieBegin=
.unalignedInitLong 0xffffffff
.initByte 0x1	// CIE Version
.ascii 16#0#	// CIE Augmentation
.uleb128 0x1	// CIE Code Alignment Factor
.sleb128 2	// CIE Data Alignment Factor
.initByte 0xc	// CIE RA Column
.initByte 0xc	// DW_CFA_def_cfa
.uleb128 0xd
.uleb128 0x0
.align 2
_picoMark_CieEnd=

// FDE 
_picoMark_LSFDE0I900821033007563=
.unalignedInitLong _picoMark_FdeEnd-_picoMark_FdeBegin
_picoMark_FdeBegin=
.unalignedInitLong _picoMark_DebugFrame	// FDE CIE offset
.unalignedInitWord _picoMark_FUNCTION_BEGIN	// FDE initial location
.unalignedInitWord _picoMark_FUNCTION_END-_picoMark_FUNCTION_BEGIN
.initByte 0xe	// DW_CFA_def_cfa_offset
.uleb128 0x0	// <-- FUNCTION_STACK_SIZE_GOES_HERE
.initByte 0x4	// DW_CFA_advance_loc4
.unalignedInitLong _picoMark_FUNCTION_END-_picoMark_FUNCTION_BEGIN
.initByte 0xe	// DW_CFA_def_cfa_offset
.uleb128 0x0
.align 2
_picoMark_FdeEnd=

//============================================================================
// Abbrevation information.
//============================================================================

.section .debug_abbrev
_picoMark_ABBREVIATIONS=

.section .debug_abbrev
	.uleb128 0x1	// (abbrev code)
	.uleb128 0x11	// (TAG: DW_TAG_compile_unit)
	.initByte 0x1	// DW_children_yes
	.uleb128 0x10	// (DW_AT_stmt_list)
	.uleb128 0x6	// (DW_FORM_data4)
	.uleb128 0x12	// (DW_AT_high_pc)
	.uleb128 0x1	// (DW_FORM_addr)
	.uleb128 0x11	// (DW_AT_low_pc)
	.uleb128 0x1	// (DW_FORM_addr)
	.uleb128 0x25	// (DW_AT_producer)
	.uleb128 0x8	// (DW_FORM_string)
	.uleb128 0x13	// (DW_AT_language)
	.uleb128 0x5	// (DW_FORM_data2)
	.uleb128 0x3	// (DW_AT_name)
	.uleb128 0x8	// (DW_FORM_string)
.initByte 0x0
.initByte 0x0

	.uleb128 0x2	;# (abbrev code)
	.uleb128 0x2e	;# (TAG: DW_TAG_subprogram)
.initByte 0x0	;# DW_children_no
	.uleb128 0x3	;# (DW_AT_name)
	.uleb128 0x8	;# (DW_FORM_string)
	.uleb128 0x11	;# (DW_AT_low_pc)
	.uleb128 0x1	;# (DW_FORM_addr)
	.uleb128 0x12	;# (DW_AT_high_pc)
	.uleb128 0x1	;# (DW_FORM_addr)
.initByte 0x0
.initByte 0x0

.initByte 0x0

//============================================================================
// Line information. DwarfLib requires this to be present, but it can
// be empty.
//============================================================================

.section .debug_line
_picoMark_LINES=

//============================================================================
// Debug Information
//============================================================================
.section .debug_info

//Fixed header.
.unalignedInitLong _picoMark_DEBUG_INFO_END-_picoMark_DEBUG_INFO_BEGIN
_picoMark_DEBUG_INFO_BEGIN=
.unalignedInitWord 0x2
.unalignedInitLong _picoMark_ABBREVIATIONS
.initByte 0x2

// Compile unit information.
.uleb128 0x1	// (DIE 0xb) DW_TAG_compile_unit)
.unalignedInitLong _picoMark_LINES
.unalignedInitWord _picoMark_FUNCTION_END
.unalignedInitWord _picoMark_FUNCTION_BEGIN
// Producer is `picoChip'
.ascii 16#70# 16#69# 16#63# 16#6f# 16#43# 16#68# 16#69# 16#70# 16#00#
.unalignedInitWord 0xcafe // ASM language
.ascii 16#0# // Name. DwarfLib expects this to be present.

.uleb128 0x2	;# (DIE DW_TAG_subprogram)

// FUNCTION NAME GOES HERE. Use `echo name | od -t x1' to get the hex. Each hex
// digit is specified using the format 16#XX#
.ascii 16#5f# 16#61# 16#73# 16#68# 16#72# 16#73# 16#69# 16#33# 16#0# // Function name `_ashrsi3'
.unalignedInitWord _picoMark_FUNCTION_BEGIN	// DW_AT_low_pc
.unalignedInitWord _picoMark_FUNCTION_END	// DW_AT_high_pc

.initByte 0x0	// end of compile unit children.

_picoMark_DEBUG_INFO_END=

//============================================================================
// END OF DWARF
//============================================================================

.section .endFile
// End of picoChip ASM file
