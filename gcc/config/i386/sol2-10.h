/* Solaris 10 configuration.
   Copyright (C) 2004 Free Software Foundation, Inc.
   Contributed by CodeSourcery, LLC.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* The Solaris 2.10 headers expect _LONGLONG_TYPE to be defined if
  "long long" is recognized by the compiler.  */
#undef TARGET_SUB_OS_CPP_BUILTINS
#define TARGET_SUB_OS_CPP_BUILTINS()		\
  do {						\
    builtin_define ("_LONGLONG_TYPE");		\
  } while (0)

#undef ASM_COMMENT_START
#define ASM_COMMENT_START "/"

#undef ASM_SPEC
#define ASM_SPEC "%{v:-V} %{Qy:} %{!Qn:-Qy} %{n} %{T} %{Ym,*} %{Yd,*} " \
		 "%{Wa,*:%*} %{m32:--32} %{m64:--64} -s %(asm_cpu)"

#undef NO_PROFILE_COUNTERS

#undef MCOUNT_NAME
#define MCOUNT_NAME "_mcount"

#undef WCHAR_TYPE
#define WCHAR_TYPE (TARGET_64BIT ? "int" : "long int")
#undef WCHAR_TYPE_SIZE
#define WCHAR_TYPE_SIZE 32

#undef WINT_TYPE
#define WINT_TYPE (TARGET_64BIT ? "int" : "long int")
#undef WINT_TYPE_SIZE
#define WINT_TYPE_SIZE 32

#define SUBTARGET_OVERRIDE_OPTIONS				\
  do								\
    {								\
      if (flag_omit_frame_pointer == 2)				\
	flag_omit_frame_pointer = 0;				\
    }								\
  while (0)

#undef TARGET_SUBTARGET_DEFAULT
#define TARGET_SUBTARGET_DEFAULT (MASK_80387 | MASK_IEEE_FP	\
				  | MASK_FLOAT_RETURNS		\
				  | MASK_OMIT_LEAF_FRAME_POINTER)

#define MULTILIB_DEFAULTS { "m32" }

#undef LINK_ARCH64_SPEC_BASE
#define LINK_ARCH64_SPEC_BASE \
  "%{G:-G} \
   %{YP,*} \
   %{R*} \
   %{compat-bsd: \
     %{!YP,*:%{p|pg:-Y P,/usr/ucblib/64:/usr/lib/libp/64:/lib/64:/usr/lib/64} \
             %{!p:%{!pg:-Y P,/usr/ucblib/64:/lib:/usr/lib/64}}} \
             -R /usr/ucblib/64} \
   %{!compat-bsd: \
     %{!YP,*:%{p|pg:-Y P,/usr/lib/libp/64:/lib/64:/usr/lib/64} \
             %{!p:%{!pg:-Y P,/lib/64:/usr/lib/64}}}}"

#undef LINK_ARCH64_SPEC
#define LINK_ARCH64_SPEC LINK_ARCH64_SPEC_BASE

#ifdef TARGET_GNU_LD
#define TARGET_LD_EMULATION "%{m64:-m elf_x86_64}%{!m64:-m elf_i386} "
#else
#define TARGET_LD_EMULATION ""
#endif

/* On, Solaris 2.10 some of the system utilities are built with GCC.
   Sun does not want these utilities to be linked with libgcc because
   a future installation of GCC in /usr/local might then result in the
   base system utilities picking up a new version of libgcc.  */
#undef LIBGCC_SPEC
#define LIBGCC_SPEC \
  "%{!nolibgcc:-lgcc}"

#undef LINK_ARCH_SPEC
#define LINK_ARCH_SPEC TARGET_LD_EMULATION \
		       "%{m64:" LINK_ARCH64_SPEC "}%{!m64:" LINK_ARCH32_SPEC "}"

#undef TARGET_ASM_NAMED_SECTION
#define TARGET_ASM_NAMED_SECTION i386_solaris_elf_named_section

/* In 32-bit mode, follow the SVR4 ABI definition; in 64-bit mode, use
   the AMD64 ABI definition.  */
#undef RETURN_IN_MEMORY
#define RETURN_IN_MEMORY(TYPE)			\
  (TARGET_64BIT 				\
   ? ix86_return_in_memory (TYPE)		\
   : (TYPE_MODE (TYPE) == BLKmode		\
      || (VECTOR_MODE_P (TYPE_MODE (TYPE)) 	\
	  && int_size_in_bytes (TYPE) == 8)))
