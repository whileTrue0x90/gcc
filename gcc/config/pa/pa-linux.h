/* Definitions for PA_RISC with ELF format
   Copyright 1999, 2000, 2001, 2002 Free Software Foundation, Inc.

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

/* Use DWARF2 debugging info and unwind.  */
#undef PREFERRED_DEBUGGING_TYPE
#define PREFERRED_DEBUGGING_TYPE DWARF2_DEBUG
#define DWARF2_ASM_LINE_DEBUG_INFO 1

/* A C expression whose value is RTL representing the location of the
   incoming return address at the beginning of any function, before the
   prologue.  You only need to define this macro if you want to support
   call frame debugging information like that provided by DWARF 2.  */
#define INCOMING_RETURN_ADDR_RTX (gen_rtx_REG (word_mode, 2))
#define DWARF_FRAME_RETURN_COLUMN (DWARF_FRAME_REGNUM (2))

/* This macro chooses the encoding of pointers embedded in the exception
   handling sections.  If at all possible, this should be defined such
   that the exception handling section will not require dynamic relocations,
   and so may be read-only.

   FIXME: We use DW_EH_PE_aligned to output a PLABEL constructor for
   global function pointers.  */
#define ASM_PREFERRED_EH_DATA_FORMAT(CODE,GLOBAL)			\
  (CODE == 2 && GLOBAL ? DW_EH_PE_aligned : DW_EH_PE_absptr)

/* Handle special EH pointer encodings.  Absolute, pc-relative, and
   indirect are handled automatically.  Since pc-relative encoding is
   not possible on the PA and we don't have the infrastructure for
   data relative encoding, we use aligned plabels for global function
   pointers.  */
#define ASM_MAYBE_OUTPUT_ENCODED_ADDR_RTX(FILE, ENCODING, SIZE, ADDR, DONE) \
  do {									\
    if (((ENCODING) & 0x0F) == DW_EH_PE_aligned)			\
      {									\
	fputs (integer_asm_op (SIZE, FALSE), FILE);			\
	fputs ("P%", FILE);						\
	assemble_name (FILE, XSTR (ADDR, 0));				\
	goto DONE;							\
      }									\
    } while (0)

#undef CPP_PREDEFINES
#define CPP_PREDEFINES "-D__ELF__ -Dunix -D__hppa__ -D__gnu_linux__ -Dlinux -Asystem=unix -Asystem=posix -Acpu=hppa -Amachine=hppa -Amachine=bigendian"

#undef	LIB_SPEC
#define LIB_SPEC \
  "%{shared: -lgcc -lc} \
   %{!shared: %{mieee-fp:-lieee} %{pthread:-lpthread} \
	%{shared-libgcc: -lgcc} %{profile:-lc_p} %{!profile: -lc}}"

#undef ASM_SPEC
#define ASM_SPEC \
  "%{v:-V} %{n} %{T} %{Ym,*} %{Yd,*} %{Wa,*:%*}"

/* Define this for shared library support because it isn't in the main
   linux.h file.  */

#undef LINK_SPEC
#define LINK_SPEC "\
  %{shared:-shared} \
  %{!shared: \
    %{!static: \
      %{rdynamic:-export-dynamic} \
      %{!dynamic-linker:-dynamic-linker /lib/ld.so.1}} \
      %{static:-static}}"

/* Sibcalls, stubs, and elf sections don't play well.  */
#undef FUNCTION_OK_FOR_SIBCALL
#define FUNCTION_OK_FOR_SIBCALL(x) 0

/* glibc's profiling functions don't need gcc to allocate counters.  */
#define NO_PROFILE_COUNTERS 1

/* Define the strings used for the special svr4 .type and .size directives.
   These strings generally do not vary from one system running svr4 to
   another, but if a given system (e.g. m88k running svr) needs to use
   different pseudo-op names for these, they may be overridden in the
   file which includes this one.  */

#undef STRING_ASM_OP
#define STRING_ASM_OP   ".stringz"

#define TEXT_SECTION_ASM_OP "\t.text"
#define DATA_SECTION_ASM_OP "\t.data"
#define BSS_SECTION_ASM_OP "\t.section\t.bss"

/* Output at beginning of assembler file.  We override the definition
   from <linux.h> so that we can get the proper .LEVEL directive.  */
#undef ASM_FILE_START
#define ASM_FILE_START(FILE) \
  do								\
    {								\
      if (write_symbols != NO_DEBUG)				\
	{							\
	  output_file_directive (FILE, main_input_filename);	\
	  fputs ("\t.version\t\"01.01\"\n", FILE);		\
	}							\
      if (TARGET_64BIT)						\
	fputs("\t.LEVEL 2.0w\n", FILE);				\
      else if (TARGET_PA_20)					\
	fputs("\t.LEVEL 2.0\n", FILE);				\
      else if (TARGET_PA_11)					\
	fputs("\t.LEVEL 1.1\n", FILE);				\
      else							\
	fputs("\t.LEVEL 1.0\n", FILE);				\
      if (profile_flag)						\
	fputs ("\t.IMPORT _mcount, CODE\n", FILE);		\
    }								\
   while (0)

/* Output a definition */
#define ASM_OUTPUT_DEF(FILE,LABEL1,LABEL2) \
  do								\
    {								\
      fprintf ((FILE), "\t%s\t", SET_ASM_OP);			\
      assemble_name (FILE, LABEL1);				\
      fprintf (FILE, ",");					\
      assemble_name (FILE, LABEL2);				\
      fprintf (FILE, "\n");					\
    }								\
  while (0)

/* We want local labels to start with period if made with asm_fprintf.  */
#undef LOCAL_LABEL_PREFIX
#define LOCAL_LABEL_PREFIX "."

/* Define these to generate the Linux/ELF/SysV style of internal
   labels all the time - i.e. to be compatible with
   ASM_GENERATE_INTERNAL_LABEL in <elfos.h>.  Compare these with the
   ones in pa.h and note the lack of dollar signs in these.  FIXME:
   shouldn't we fix pa.h to use ASM_GENERATE_INTERNAL_LABEL instead? */

#undef ASM_OUTPUT_ADDR_VEC_ELT
#define ASM_OUTPUT_ADDR_VEC_ELT(FILE, VALUE) \
  if (TARGET_BIG_SWITCH)					\
    fprintf (FILE, "\tstw %%r1,-16(%%r30)\n\tldil LR'.L%d,%%r1\n\tbe RR'.L%d(%%sr4,%%r1)\n\tldw -16(%%r30),%%r1\n", VALUE, VALUE);		\
  else								\
    fprintf (FILE, "\tb .L%d\n\tnop\n", VALUE)

#undef ASM_OUTPUT_ADDR_DIFF_ELT
#define ASM_OUTPUT_ADDR_DIFF_ELT(FILE, BODY, VALUE, REL) \
  if (TARGET_BIG_SWITCH)					\
    fprintf (FILE, "\tstw %%r1,-16(%%r30)\n\tldw T'.L%d(%%r19),%%r1\n\tbv %%r0(%%r1)\n\tldw -16(%%r30),%%r1\n", VALUE);				\
  else								\
    fprintf (FILE, "\tb .L%d\n\tnop\n", VALUE)

/* Use the default.  */
#undef ASM_OUTPUT_LABEL

/* NOTE: ASM_OUTPUT_INTERNAL_LABEL() is defined for us by elfos.h, and
   does what we want (i.e. uses colons).  It must be compatible with
   ASM_GENERATE_INTERNAL_LABEL(), so do not define it here.  */

/* Use the default.  */
#undef ASM_GLOBALIZE_LABEL
/* Globalizing directive for a label.  */
#define GLOBAL_ASM_OP ".globl "

/* FIXME: Hacked from the <elfos.h> one so that we avoid multiple
   labels in a function declaration (since pa.c seems determined to do
   it differently)  */

#undef ASM_DECLARE_FUNCTION_NAME
#define ASM_DECLARE_FUNCTION_NAME(FILE, NAME, DECL)		\
  do								\
    {								\
      ASM_OUTPUT_TYPE_DIRECTIVE (FILE, NAME, "function");	\
      ASM_DECLARE_RESULT (FILE, DECL_RESULT (DECL));		\
    }								\
  while (0)

/* Linux always uses gas.  */
#undef TARGET_GAS
#define TARGET_GAS 1
