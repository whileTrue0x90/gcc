/* Out-of-line libgcc versions of __sync_* builtins.  */
/* Copyright (C) 2008  Free Software Foundation, Inc.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file into combinations with other programs,
and to distribute those combinations without any restriction coming
from the use of this file.  (The General Public License restrictions
do apply in other respects; for example, they cover modification of
the file, and distribution when not linked into a combine
executable.)

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301, USA.  */

/* This file is used by targets whose makefiles define LIBGCC_SYNC
   to "yes".  It is compiled with LIBGCC_SYNC_CFLAGS and provides
   out-of-line versions of all relevant __sync_* primitives.

   These routines are intended for targets like MIPS that have two
   ISA encodings (the "normal" ISA and the MIPS16 ISA).  The normal
   ISA provides full synchronization capabilities but the MIPS16 ISA
   has no encoding for them.  MIPS16 code must therefore call external
   non-MIPS16 implementations of the __sync_* routines.

   The file is compiled once for each routine.  The following __foo
   routines are selected by defining a macro called L<foo>:

       __sync_synchronize

   The following __foo_N routines are selected by defining FN=foo
   and SIZE=N:

       __sync_fetch_and_add_N
       __sync_fetch_and_sub_N
       __sync_fetch_and_or_N
       __sync_fetch_and_and_N
       __sync_fetch_and_xor_N
       __sync_fetch_and_nand_N
       __sync_add_and_fetch_N
       __sync_sub_and_fetch_N
       __sync_or_and_fetch_N
       __sync_and_and_fetch_N
       __sync_xor_and_fetch_N
       __sync_nand_and_fetch_N
       __sync_bool_compare_and_swap_N
       __sync_val_compare_and_swap_N
       __sync_lock_test_and_set_N

   SIZE can be 1, 2, 4, 8 or 16.  __foo_N is omitted if the target does
   not provide __sync_compare_and_swap_N.

   Note that __sync_lock_release does not fall back on external
   __sync_lock_release_N functions.  The default implementation
   of __sync_lock_release is a call to __sync_synchronize followed
   by a store of zero, so we don't need separate library functions
   for it.  */

#if defined FN

/* Define macros for each __sync_* function type.  Each macro defines a
   local function called <NAME>_<UNITS> that acts like __<NAME>_<UNITS>.
   TYPE is a type that has UNITS bytes.  */

#define DEFINE_V_PV(NAME, UNITS, TYPE)					\
  static TYPE								\
  NAME##_##UNITS (TYPE *ptr, TYPE value)				\
  {									\
    return __##NAME (ptr, value);					\
  }

#define DEFINE_V_PVV(NAME, UNITS, TYPE)				\
  static TYPE								\
  NAME##_##UNITS (TYPE *ptr, TYPE value1, TYPE value2)			\
  {									\
    return __##NAME (ptr, value1, value2);				\
  }

#define DEFINE_BOOL_PVV(NAME, UNITS, TYPE)				\
  static _Bool								\
  NAME##_##UNITS (TYPE *ptr, TYPE value1, TYPE value2)			\
  {									\
    return __##NAME (ptr, value1, value2);				\
  }

/* Map function names to the appropriate DEFINE_* macro.  */

#define local_sync_fetch_and_add DEFINE_V_PV
#define local_sync_fetch_and_sub DEFINE_V_PV
#define local_sync_fetch_and_or DEFINE_V_PV
#define local_sync_fetch_and_and DEFINE_V_PV
#define local_sync_fetch_and_xor DEFINE_V_PV
#define local_sync_fetch_and_nand DEFINE_V_PV

#define local_sync_add_and_fetch DEFINE_V_PV
#define local_sync_sub_and_fetch DEFINE_V_PV
#define local_sync_or_and_fetch DEFINE_V_PV
#define local_sync_and_and_fetch DEFINE_V_PV
#define local_sync_xor_and_fetch DEFINE_V_PV
#define local_sync_nand_and_fetch DEFINE_V_PV

#define local_sync_bool_compare_and_swap DEFINE_BOOL_PVV
#define local_sync_val_compare_and_swap DEFINE_V_PVV

#define local_sync_lock_test_and_set DEFINE_V_PV

/* Define the function __<NAME>_<UNITS>, given that TYPE is a type with
   UNITS bytes.  */
#define DEFINE1(NAME, UNITS, TYPE) \
  static int unused[sizeof (TYPE) == UNITS ? 1 : -1]	\
    __attribute__((unused));				\
  local_##NAME (NAME, UNITS, TYPE);			\
  typeof (NAME##_##UNITS) __##NAME##_##UNITS		\
    __attribute__((alias (#NAME "_" #UNITS)));

/* As above, but performing macro expansion on the arguments.  */
#define DEFINE(NAME, UNITS, TYPE) DEFINE1 (NAME, UNITS, TYPE)

/* Find an appropriate type TYPE for SIZE and invoke DEFINE (FN, SIZE, TYPE).

   The types chosen here may be incorrect for some targets.
   For example, targets with 16-byte atomicity support might not
   support OImode.  We would need some kind of target-specific
   override if that becomes a problem.  */

#if SIZE == 1 && __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1

typedef unsigned int UQItype __attribute__((mode (QI)));
DEFINE (FN, 1, UQItype)

#elif SIZE == 2 && __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2

typedef unsigned int UHItype __attribute__((mode (HI)));
DEFINE (FN, 2, UHItype)

#elif SIZE == 4 && __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4

typedef unsigned int USItype __attribute__((mode (SI)));
DEFINE (FN, 4, USItype)

#elif SIZE == 8 && __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8

typedef unsigned int UDItype __attribute__((mode (DI)));
DEFINE (FN, 8, UDItype)

#elif SIZE == 16 && __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16

typedef unsigned int UOItype __attribute__((mode (OI)));
DEFINE (FN, 8, UOItype)

#endif

#elif __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 \
      || __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 \
      || __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 \
      || __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 \
      || __GCC_HAVE_SYNC_COMPARE_AND_SWAP_16

#if defined Lsync_synchronize

static void
sync_synchronize (void)
{
  __sync_synchronize ();
}
typeof (sync_synchronize) __sync_synchronize \
  __attribute__((alias ("sync_synchronize")));

#endif

#endif
