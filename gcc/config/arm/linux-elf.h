/* Definitions for ARM running Linux-based GNU systems using ELF
   Copyright (C) 1993, 1994, 1997, 1998, 1999, 2000, 2001 
   Free Software Foundation, Inc.
   Contributed by Philip Blundell <philb@gnu.org>

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
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* Run-time Target Specification.  */
#define TARGET_VERSION  fputs (" (ARM GNU/Linux with ELF)", stderr);

/* Do not assume anything about header files.  */
#define NO_IMPLICIT_EXTERN_C

/* Default is to use APCS-32 mode.  */
#define TARGET_DEFAULT (ARM_FLAG_APCS_32 | ARM_FLAG_MMU_TRAPS)
#define SUBTARGET_EXTRA_LINK_SPEC " -m armelf_linux -p"
#define MULTILIB_DEFAULTS \
	{ "marm", "mlittle-endian", "mhard-float", "mapcs-32", "mno-thumb-interwork" }
#define CPP_APCS_PC_DEFAULT_SPEC "-D__APCS_32__"

/* Handle #pragma weak and #pragma pack.  */
#define HANDLE_SYSV_PRAGMA

/* Now we define the strings used to build the spec file.  */
#define LIB_SPEC \
  "%{shared: -lc} \
   %{!shared: %{pthread:-lpthread} \
   %{profile:-lc_p} %{!profile: -lc}}"


#define LIBGCC_SPEC "%{msoft-float:-lfloat} -lgcc"

/* Provide a STARTFILE_SPEC appropriate for GNU/Linux.  Here we add
   the GNU/Linux magical crtbegin.o file (see crtstuff.c) which
   provides part of the support for getting C++ file-scope static
   object constructed before entering `main'. */
   
#define STARTFILE_SPEC \
  "%{!shared: \
     %{pg:gcrt1.o%s} %{!pg:%{p:gcrt1.o%s} \
		       %{!p:%{profile:gcrt1.o%s} \
			 %{!profile:crt1.o%s}}}} \
   crti.o%s %{!shared:crtbegin.o%s} %{shared:crtbeginS.o%s}"

/* Provide a ENDFILE_SPEC appropriate for GNU/Linux.  Here we tack on
   the GNU/Linux magical crtend.o file (see crtstuff.c) which
   provides part of the support for getting C++ file-scope static
   object constructed before entering `main', followed by a normal
   GNU/Linux "finalizer" file, `crtn.o'.  */

#define ENDFILE_SPEC \
  "%{!shared:crtend.o%s} %{shared:crtendS.o%s} crtn.o%s"

#define LINK_SPEC "%{h*} %{version:-v} \
   %{b} %{Wl,*:%*} \
   %{static:-Bstatic} \
   %{shared:-shared} \
   %{symbolic:-Bsymbolic} \
   %{rdynamic:-export-dynamic} \
   %{!dynamic-linker:-dynamic-linker /lib/ld-linux.so.2} \
   -X \
   %{mbig-endian:-EB}" \
   SUBTARGET_EXTRA_LINK_SPEC

#undef  CPP_PREDEFINES
#define CPP_PREDEFINES \
"-Dunix -Dlinux -D__ELF__ \
-Asystem=unix -Asystem=posix -Acpu=arm -Amachine=arm"

/* Allow #sccs in preprocessor.  */
#define SCCS_DIRECTIVE

#define USER_LABEL_PREFIX 	""	/* For ELF the default is no underscores */
#define LOCAL_LABEL_PREFIX 	"."

#define IDENT_ASM_OP 	"\t.ident\t"

/* Output #ident as a .ident.  */
#define ASM_OUTPUT_IDENT(FILE, NAME) \
  fprintf (FILE, "%s\"%s\"\n", IDENT_ASM_OP, NAME);

/* Support const sections and the ctors and dtors sections for g++.
   Note that there appears to be two different ways to support const
   sections at the moment.  You can either #define the symbol
   READONLY_DATA_SECTION (giving it some code which switches to the
   readonly data section) or else you can #define the symbols
   EXTRA_SECTIONS, EXTRA_SECTION_FUNCTIONS, SELECT_SECTION, and
   SELECT_RTX_SECTION.  We do both here just to be on the safe side.  */
#define USE_CONST_SECTION	1

/* Support for Constructors and Destructors.  */
#define READONLY_DATA_SECTION() const_section ()

/* A default list of other sections which we might be "in" at any given
   time.  For targets that use additional sections (e.g. .tdesc) you
   should override this definition in the target-specific file which
   includes this file.  */
#define SUBTARGET_EXTRA_SECTIONS in_const,

/* A default list of extra section function definitions.  For targets
   that use additional sections (e.g. .tdesc) you should override this
   definition in the target-specific file which includes this file.  */
#define SUBTARGET_EXTRA_SECTION_FUNCTIONS	CONST_SECTION_FUNCTION

#define CONST_SECTION_ASM_OP	"\t.section\t.rodata"

#define CONST_SECTION_FUNCTION						\
void									\
const_section ()							\
{									\
  if (!USE_CONST_SECTION)						\
    text_section ();							\
  else if (in_section != in_const)					\
    {									\
      fprintf (asm_out_file, "%s\n", CONST_SECTION_ASM_OP);		\
      in_section = in_const;						\
    }									\
}

/* Switch into a generic section.  */
#define TARGET_ASM_NAMED_SECTION  arm_elf_asm_named_section

/* A C statement or statements to switch to the appropriate
   section for output of DECL.  DECL is either a `VAR_DECL' node
   or a constant of some sort.  RELOC indicates whether forming
   the initial value of DECL requires link-time relocations.  */
#define SELECT_SECTION(DECL,RELOC)					\
{									\
  if (TREE_CODE (DECL) == STRING_CST)					\
    {									\
      if (! flag_writable_strings)					\
	const_section ();						\
      else								\
	data_section ();						\
    }									\
  else if (TREE_CODE (DECL) == VAR_DECL)				\
    {									\
      if ((flag_pic && RELOC)						\
	  || !TREE_READONLY (DECL) || TREE_SIDE_EFFECTS (DECL)		\
	  || !DECL_INITIAL (DECL)					\
	  || (DECL_INITIAL (DECL) != error_mark_node			\
	      && !TREE_CONSTANT (DECL_INITIAL (DECL))))			\
	data_section ();						\
      else								\
	const_section ();						\
    }									\
  else									\
    const_section ();							\
}

#define MAKE_DECL_ONE_ONLY(DECL) (DECL_WEAK (DECL) = 1)
#define UNIQUE_SECTION(DECL, RELOC)				\
  do								\
    {								\
      int len;							\
      char * name;						\
      char * string;						\
      char * prefix;						\
								\
      name = IDENTIFIER_POINTER (DECL_ASSEMBLER_NAME (DECL));	\
      								\
      if (! DECL_ONE_ONLY (DECL))				\
	{							\
	  prefix = ".";                                         \
	  if (TREE_CODE (DECL) == FUNCTION_DECL)		\
	    prefix = ".text.";					\
	  else if (DECL_READONLY_SECTION (DECL, RELOC))		\
	    prefix = ".rodata.";				\
	  else							\
	    prefix = ".data.";					\
	}							\
      else if (TREE_CODE (DECL) == FUNCTION_DECL)		\
	prefix = ".gnu.linkonce.t.";				\
      else if (DECL_READONLY_SECTION (DECL, RELOC))		\
	prefix = ".gnu.linkonce.r.";				\
      else							\
	prefix = ".gnu.linkonce.d.";				\
      								\
      len = strlen (name) + strlen (prefix);			\
      string = alloca (len + 1);				\
      sprintf (string, "%s%s", prefix, name);			\
      								\
      DECL_SECTION_NAME (DECL) = build_string (len, string);	\
    }								\
  while (0)

/* A C statement or statements to switch to the appropriate
   section for output of RTX in mode MODE.  RTX is some kind
   of constant in RTL.  The argument MODE is redundant except
   in the case of a `const_int' rtx.  Currently, these always
   go into the const section.  */
#define SELECT_RTX_SECTION(MODE,RTX) const_section ()

/* On svr4, we *do* have support for the .init and .fini sections, and we
   can put stuff in there to be executed before and after `main'.  We let
   crtstuff.c and other files know this by defining the following symbols.
   The definitions say how to change sections to the .init and .fini
   sections.  This is the same for all known svr4 assemblers.  */
#define INIT_SECTION_ASM_OP	"\t.section\t.init"
#define FINI_SECTION_ASM_OP	"\t.section\t.fini"


/* This is how we tell the assembler that a symbol is weak.  */
#define ASM_WEAKEN_LABEL(FILE,NAME) \
  do { fputs ("\t.weak\t", FILE); assemble_name (FILE, NAME); \
       fputc ('\n', FILE); } while (0)

/* This is how we tell the assembler that two symbols have the same value.  */

#define ASM_OUTPUT_DEF(FILE,NAME1,NAME2) \
  do { assemble_name (FILE, NAME1); 	 \
       fputs (" = ", FILE);		 \
       assemble_name (FILE, NAME2);	 \
       fputc ('\n', FILE); } while (0)

/* Make DWARF2 an option, but keep DBX as the default for now.
   Use -gdwarf-2 to turn on DWARF2.  */
#define DWARF2_DEBUGGING_INFO
#define PREFERRED_DEBUGGING_TYPE DBX_DEBUG

/* Get the standard ELF stabs definitions.  */
#include "dbxelf.h"

#include "arm/elf.h"
#include "arm/linux-gas.h"

/* NWFPE always understands FPA instructions.  */
#undef  FP_DEFAULT
#define FP_DEFAULT FP_SOFT3

/* Call the function profiler with a given profile label.  */
#undef  ARM_FUNCTION_PROFILER
#define ARM_FUNCTION_PROFILER(STREAM, LABELNO)  			\
{									\
  fprintf (STREAM, "\tbl\tmcount%s\n", NEED_PLT_RELOC ? "(PLT)" : "");	\
}

#undef  CC1_SPEC
#define CC1_SPEC "%{profile:-p}"
