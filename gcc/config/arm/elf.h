/* Definitions of target machine for GNU compiler.
   For ARM with ELF obj format.
   Copyright (C) 1995, 1996, 1997, 1998, 1999, 2000, 2001
   Free Software Foundation, Inc.
   Contributed by Philip Blundell <philb@gnu.org> and
   Catherine Moore <clm@cygnus.com>
   
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

#ifndef OBJECT_FORMAT_ELF
 #error elf.h included before elfos.h
#endif

#ifndef LOCAL_LABEL_PREFIX
#define LOCAL_LABEL_PREFIX "."
#endif

#ifndef SUBTARGET_CPP_SPEC
#define SUBTARGET_CPP_SPEC  "-D__ELF__"
#endif

#ifndef SUBTARGET_EXTRA_SPECS
#define SUBTARGET_EXTRA_SPECS \
  { "subtarget_extra_asm_spec",	SUBTARGET_EXTRA_ASM_SPEC },
#endif

#ifndef SUBTARGET_EXTRA_ASM_SPEC
#define SUBTARGET_EXTRA_ASM_SPEC ""
#endif

#ifndef ASM_SPEC
#define ASM_SPEC "\
%{mbig-endian:-EB} \
%{mcpu=*:-mcpu=%*} \
%{march=*:-march=%*} \
%{mapcs-*:-mapcs-%*} \
%{mapcs-float:-mfloat} \
%{msoft-float:-mno-fpu} \
%{mthumb-interwork:-mthumb-interwork} \
%(subtarget_extra_asm_spec)"
#endif

/* The ARM uses @ are a comment character so we need to redefine
   TYPE_OPERAND_FMT.  */
#undef  TYPE_OPERAND_FMT
#define TYPE_OPERAND_FMT	"%s"

/* We might need a ARM specific header to function declarations.  */
#undef  ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL)		\
  do								\
    {								\
      ARM_DECLARE_FUNCTION_NAME (FILE, NAME, DECL);		\
      ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "function");	\
      ASM_DECLARE_RESULT (FILE, DECL_RESULT (DECL));		\
      ASM_OUTPUT_LABEL(FILE, NAME);				\
    }								\
  while (0)

/* We might need an ARM specific trailer for function declarations.  */
#undef  ASM_DECLARE_FUNCTION_SIZE
#define ASM_DECLARE_FUNCTION_SIZE(FILE, FNAME, DECL)		\
  do								\
    {								\
      ARM_DECLARE_FUNCTION_SIZE (FILE, FNAME, DECL);		\
      if (!flag_inhibit_size_directive)				\
	ASM_OUTPUT_MEASURED_SIZE (FILE, FNAME);			\
    }								\
  while (0)

/* Define this macro if jump tables (for `tablejump' insns) should be
   output in the text section, along with the assembler instructions.
   Otherwise, the readonly data section is used.  */
/* We put ARM jump tables in the text section, because it makes the code
   more efficient, but for Thumb it's better to put them out of band.  */
#define JUMP_TABLES_IN_TEXT_SECTION (TARGET_ARM)

#ifndef LINK_SPEC
#define LINK_SPEC "%{mbig-endian:-EB} -X"
#endif
  
/* Run-time Target Specification.  */
#ifndef TARGET_VERSION
#define TARGET_VERSION fputs (" (ARM/elf)", stderr)
#endif

#ifndef TARGET_DEFAULT
#define TARGET_DEFAULT (ARM_FLAG_SOFT_FLOAT | ARM_FLAG_APCS_32 | ARM_FLAG_APCS_FRAME)
#endif

#ifndef MULTILIB_DEFAULTS
#define MULTILIB_DEFAULTS \
  { "marm", "mlittle-endian", "msoft-float", "mapcs-32", "mno-thumb-interwork", "fno-leading-underscore" }
#endif


/* This outputs a lot of .req's to define alias for various registers.
   Let's try to avoid this.  */
#ifndef ASM_FILE_START
#define ASM_FILE_START(STREAM)					\
  do								\
    {								\
      fprintf (STREAM, "%s Generated by gcc %s for ARM/elf\n",	\
	       ASM_COMMENT_START, version_string);		\
      output_file_directive (STREAM, main_input_filename);	\
      fprintf (STREAM, ASM_APP_OFF);				\
    }								\
  while (0)
#endif

/* Output an internal label definition.  */
#undef  ASM_OUTPUT_INTERNAL_LABEL
#define ASM_OUTPUT_INTERNAL_LABEL(STREAM, PREFIX, NUM)  	\
  do								\
    {								\
      char * s = (char *) alloca (40 + strlen (PREFIX));	\
      extern int arm_target_label, arm_ccfsm_state;		\
      extern rtx arm_target_insn;				\
								\
      if (arm_ccfsm_state == 3 && arm_target_label == (NUM)	\
	  && !strcmp (PREFIX, "L"))				\
	{							\
	  arm_ccfsm_state = 0;					\
	  arm_target_insn = NULL;				\
	}							\
      ASM_GENERATE_INTERNAL_LABEL (s, (PREFIX), (NUM));		\
      ASM_OUTPUT_LABEL (STREAM, s);		                \
    }								\
  while (0)

#undef  TARGET_ASM_NAMED_SECTION
#define TARGET_ASM_NAMED_SECTION  arm_elf_asm_named_section

#undef  ASM_OUTPUT_ALIGNED_COMMON
#define ASM_OUTPUT_ALIGNED_COMMON(STREAM, NAME, SIZE, ALIGN)	\
  do								\
    {								\
      fprintf (STREAM, "\t.comm\t");				\
      assemble_name (STREAM, NAME);				\
      fprintf (STREAM, ", %d, %d\n", SIZE, ALIGN);		\
    }								\
  while (0)

/* For PIC code we need to explicitly specify (PLT) and (GOT) relocs.  */
#define NEED_PLT_RELOC	flag_pic
#define NEED_GOT_RELOC	flag_pic

/* The ELF assembler handles GOT addressing differently to NetBSD.  */
#define GOT_PCREL	0

/* Biggest alignment supported by the object file format of this
   machine.  Use this macro to limit the alignment which can be
   specified using the `__attribute__ ((aligned (N)))' construct.  If
   not defined, the default value is `BIGGEST_ALIGNMENT'.  */
#define MAX_OFILE_ALIGNMENT (32768 * 8)

/* Align output to a power of two.  Note ".align 0" is redundant,
   and also GAS will treat it as ".align 2" which we do not want.  */
#define ASM_OUTPUT_ALIGN(STREAM, POWER)			\
  do							\
    {							\
      if ((POWER) > 0)					\
	fprintf (STREAM, "\t.align\t%d\n", POWER);	\
    }							\
  while (0)

#define SUPPORTS_INIT_PRIORITY 1
