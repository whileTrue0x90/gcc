/* Definitions of target machine GNU compiler.  IA-64 version.
   Copyright (C) 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
   Contributed by Steve Ellcey <sje@cup.hp.com> and
                  Reva Cuthbertson <reva@cup.hp.com>

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

/* This macro is a C statement to print on `stderr' a string describing the
   particular machine description choice.  */

#define TARGET_VERSION fprintf (stderr, " (IA-64) HP-UX");

/* Target OS builtins.  */
/* -D__fpreg=long double is needed to compensate for
   the lack of __fpreg which is a primative type in
   HP C but does not exist in GNU C.  */
#define TARGET_OS_CPP_BUILTINS()			\
do {							\
	builtin_assert("system=hpux");			\
	builtin_assert("system=posix");			\
	builtin_assert("system=unix");			\
	builtin_define_std("hpux");			\
	builtin_define_std("unix");			\
	builtin_define("__IA64__");			\
	builtin_define("_LONGLONG");			\
	builtin_define("_UINT128_T");			\
	builtin_define("__fpreg=long double");		\
	builtin_define("__float80=long double");	\
	builtin_define("__float128=long double");	\
	if (c_language == clk_cplusplus || !flag_iso)	\
	  {						\
	    builtin_define("_HPUX_SOURCE");		\
	    builtin_define("__STDC_EXT__");		\
	  }						\
} while (0)

#undef  ASM_EXTRA_SPEC
#define ASM_EXTRA_SPEC "%{milp32:-milp32} %{mlp64:-mlp64}"

#undef ENDFILE_SPEC

#undef STARTFILE_SPEC
#define STARTFILE_SPEC "%{!shared:%{static:crt0%O%s}}"

#ifndef CROSS_COMPILE
#define STARTFILE_PREFIX_SPEC \
  "%{mlp64: /usr/ccs/lib/hpux64/} \
   %{!mlp64: /usr/ccs/lib/hpux32/}"
#endif

#undef LINK_SPEC
#define LINK_SPEC \
  "+Accept TypeMismatch \
   %{shared:-b} \
   %{!shared: \
     -u main \
     %{static:-noshared}}"

#undef  LIB_SPEC
#define LIB_SPEC \
  "%{!shared: \
     %{p:%{!mlp64:-L/usr/lib/hpux32/libp} \
	 %{mlp64:-L/usr/lib/hpux64/libp} -lprof} \
     %{pg:%{!mlp64:-L/usr/lib/hpux32/libp} \
	  %{mlp64:-L/usr/lib/hpux64/libp} -lgprof} \
     %{!symbolic:-lc}}"

#ifndef CROSS_COMPILE
#undef LIBGCC_SPEC
#define LIBGCC_SPEC \
  "%{shared-libgcc:%{!mlp64:-lgcc_s_hpux32}%{mlp64:-lgcc_s_hpux64} -lgcc} \
   %{!shared-libgcc:-lgcc}"
#endif

#undef SUBTARGET_SWITCHES
#define SUBTARGET_SWITCHES \
  { "ilp32",    MASK_ILP32,     "Generate ILP32 code" }, \
  { "lp64",    -MASK_ILP32,     "Generate LP64 code" },

/* A C expression whose value is zero if pointers that need to be extended
   from being `POINTER_SIZE' bits wide to `Pmode' are sign-extended and
   greater then zero if they are zero-extended and less then zero if the
   ptr_extend instruction should be used.  */

#define POINTERS_EXTEND_UNSIGNED -1

#define JMP_BUF_SIZE  (8 * 76)

#undef TARGET_DEFAULT
#define TARGET_DEFAULT (MASK_DWARF2_ASM | MASK_BIG_ENDIAN | MASK_ILP32)

/* This needs to be set to force structure arguments with a single
   field to be treated as structures and not as the type of their
   field.  Without this a structure with a single char will be
   returned just like a char variable and that is wrong on HP-UX
   IA64.  TARGET_STRUCT_ARG_REG_LITTLE_ENDIAN triggers the special
   structure handling, this macro simply ensures that single field
   structures are always treated like structures.  */

/* ASM_OUTPUT_EXTERNAL_LIBCALL defaults to just a globalize_label call,
   but that doesn't put out the @function type information which causes
   shared library problems.  */

#undef ASM_OUTPUT_EXTERNAL_LIBCALL
#define ASM_OUTPUT_EXTERNAL_LIBCALL(FILE, FUN)			\
do {								\
  (*targetm.asm_out.globalize_label) (FILE, XSTR (FUN, 0));	\
  ASM_OUTPUT_TYPE_DIRECTIVE (FILE, XSTR (FUN, 0), "function");	\
} while (0)

#define MEMBER_TYPE_FORCES_BLK(FIELD, MODE) (TREE_CODE (TREE_TYPE (FIELD)) != REAL_TYPE || (MODE == TFmode && !INTEL_EXTENDED_IEEE_FORMAT))

/* Override the setting of FUNCTION_ARG_REG_LITTLE_ENDIAN in
   defaults.h.  Setting this to true means that we are not passing
   structures in registers in the "normal" big-endian way.  See
   See section 8.5 of the "Itanium Software Conventions and Runtime
   Architecture", specifically Table 8-1 and the explanation of Byte 0
   alignment and LSB alignment and a description of how structures
   are passed.  */

#define FUNCTION_ARG_REG_LITTLE_ENDIAN 1

#undef FUNCTION_ARG_PADDING
#define FUNCTION_ARG_PADDING(MODE, TYPE) \
	ia64_hpux_function_arg_padding ((MODE), (TYPE))

#undef PAD_VARARGS_DOWN
#define PAD_VARARGS_DOWN (!AGGREGATE_TYPE_P (type))

#define REGISTER_TARGET_PRAGMAS(PFILE) \
  cpp_register_pragma (PFILE, 0, "builtin", ia64_hpux_handle_builtin_pragma)

/* Tell ia64.c that we are using the HP linker and we should delay output of
   function extern declarations so that we don't output them for functions
   which are never used (and may not be defined).  */

#undef TARGET_HPUX_LD
#define TARGET_HPUX_LD	1

/* Put out the needed function declarations at the end.  */

#define ASM_FILE_END(STREAM) ia64_hpux_asm_file_end(STREAM)

#undef CTORS_SECTION_ASM_OP
#define CTORS_SECTION_ASM_OP  "\t.section\t.init_array,\t\"aw\",\"init_array\""

#undef DTORS_SECTION_ASM_OP
#define DTORS_SECTION_ASM_OP  "\t.section\t.fini_array,\t\"aw\",\"fini_array\""

#undef READONLY_DATA_SECTION_ASM_OP
#define READONLY_DATA_SECTION_ASM_OP "\t.section\t.rodata,\t\"a\",\t\"progbits\""

#undef DATA_SECTION_ASM_OP
#define DATA_SECTION_ASM_OP "\t.section\t.data,\t\"aw\",\t\"progbits\""

#undef SDATA_SECTION_ASM_OP
#define SDATA_SECTION_ASM_OP "\t.section\t.sdata,\t\"asw\",\t\"progbits\""

#undef BSS_SECTION_ASM_OP
#define BSS_SECTION_ASM_OP "\t.section\t.bss,\t\"aw\",\t\"nobits\""

#undef SBSS_SECTION_ASM_OP
#define SBSS_SECTION_ASM_OP "\t.section\t.sbss,\t\"asw\",\t\"nobits\""

#undef TEXT_SECTION_ASM_OP
#define TEXT_SECTION_ASM_OP "\t.section\t.text,\t\"ax\",\t\"progbits\""
