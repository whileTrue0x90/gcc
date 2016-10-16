/* Copyright (C) 1998-2016 Free Software Foundation, Inc.
   Contributed by Joern Rennecke

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"

#define IN_GEN_AVR_MMCU_TEXI

#include "avr-devices.c"

// Get rid of "defaults.h".  We just need tm.h for `WITH_AVRLIBC' and
// and `WITH_RTEMS'.  */
#define GCC_DEFAULTS_H

#include "tm.h"

// Mimic the include order as specified in config.gcc::tm_file.

#include "specs.h"

#if defined (WITH_AVRLIBC)
#include "avrlibc.h"
#endif

#if defined (WITH_RTEMS)
#include "../rtems.h"
#include "rtems.h"
#endif


#define SPECFILE_DOC_URL                                \
  "https://gcc.gnu.org/onlinedocs/gcc/Spec-Files.html"

#define SPECFILE_USAGE_URL                              \
  "https://gcc.gnu.org/gcc-5/changes.html"

/* Return true iff STR starts with PREFIX.  */

static bool
str_prefix_p (const char *str, const char *prefix)
{
  return 0 == strncmp (str, prefix, strlen (prefix));
}


static const char header[] =
  "#\n"
  "# Generated by   : ./gcc/config/avr/gen-avr-mmcu-specs.c\n"
  "# Generated from : ./gcc/config/gcc.c\n"
  "#                  ./gcc/config/avr/specs.h\n"
#if defined (WITH_RTEMS)
  "#                  ./gcc/config/rtems.h\n"
  "#                  ./gcc/config/avr/rtems.h\n"
#endif
#if defined (WITH_AVRLIBC)
  "#                  ./gcc/config/avr/avrlibc.h\n"
#endif
  "# Used by        : avr-gcc compiler driver\n"
  "# Used for       : building command options for sub-processes\n"
  "#\n"
  "# See <" SPECFILE_DOC_URL ">\n"
  "# for a documentation of spec files.\n"
  "\n";

static const char help_copy_paste[] =
  "# If you intend to use an existing device specs file as a starting point\n"
  "# for a new device spec file, make sure you are copying from a specs\n"
  "# file for a device from the same core architecture and SP width.\n"
  "# See <" SPECFILE_USAGE_URL "> for a description\n"
  "# of how to use such own spec files.\n";

#if defined (WITH_AVRLIBC)
static const char help_dev_lib_name[] =
  "# AVR-LibC's avr/io.h uses the device specifying macro to determine\n"
  "# the name of the device header.  For example, -mmcu=atmega8a triggers\n"
  "# the definition of __AVR_ATmega8A__ and avr/io.h includes the device\n"
  "# header 'iom8a.h' by means of:\n"
  "#\n"
  "#     ...\n"
  "#     #elif defined (__AVR_ATmega8A__)\n"
  "#     #  include <avr/iom8a.h>\n"
  "#     #elif ...\n"
  "# \n"
  "# If no device macro is defined, AVR-LibC uses __AVR_DEV_LIB_NAME__\n"
  "# as fallback to determine the name of the device header as\n"
  "#\n"
  "#     \"avr/io\" + __AVR_DEV_LIB_NAME__ + \".h\"\n"
  "#\n"
  "# If you provide your own specs file for a device not yet known to\n"
  "# AVR-LibC, you can now define the hook macro __AVR_DEV_LIB_NAME__\n"
  "# as needed so that\n"
  "#\n"
  "#     #include <avr/io.h>\n"
  "#\n"
  "# will include the desired device header.  For ATmega8A the supplement\n"
  "# to *cpp would read\n"
  "#\n"
  "#     -D__AVR_DEV_LIB_NAME__=m8a\n"
  "\n";
#endif // WITH_AVRLIBC

static void
print_mcu (const avr_mcu_t *mcu)
{
  const char *sp8_spec;
  const avr_mcu_t *arch_mcu;
  const avr_arch_t *arch;
  enum avr_arch_id arch_id = mcu->arch_id;

  for (arch_mcu = mcu; arch_mcu->macro; )
    arch_mcu--;
  if (arch_mcu->arch_id != arch_id)
    exit (EXIT_FAILURE);

  arch = &avr_arch_types[arch_id];

  char name[100];
  if (snprintf (name, sizeof name, "specs-%s", mcu->name) >= (int) sizeof name)
   exit (EXIT_FAILURE);

  FILE *f = fopen (name ,"w");

  bool errata_skip = 0 != (mcu->dev_attribute & AVR_ERRATA_SKIP);
  bool rmw = 0 != (mcu->dev_attribute & AVR_ISA_RMW);
  bool sp8 = 0 != (mcu->dev_attribute & AVR_SHORT_SP);
  bool is_arch = NULL == mcu->macro;
  bool is_device = ! is_arch;

  if (is_arch
      && (ARCH_AVR2 == arch_id
          || ARCH_AVR25 == arch_id))
    {
      // Leave "avr2" and "avr25" alone.  These two architectures are
      // the only ones that mix devices with 8-bit SP and 16-bit SP.
      sp8_spec = "";
    }
  else
    {
      sp8_spec = sp8 ? "-msp8" :"%<msp8";
    }

  fprintf (f, "#\n"
           "# Auto-generated specs for AVR ");
  if (is_arch)
    fprintf (f, "core architecture %s\n", arch->name);
  else
    fprintf (f, "device %s (core %s, %d-bit SP)\n",
             mcu->name, arch->name, sp8 ? 8 : 16);
  fprintf (f, "%s\n", header);

  if (is_device)
    fprintf (f, "%s\n", help_copy_paste);

#if defined (WITH_AVRLIBC)
  // AVR-LibC specific.  See avrlibc.h for the specs using them as subspecs.

  if (is_device)
    {
      fprintf (f, "*avrlibc_startfile:\n");
      fprintf (f, "\tcrt%s.o%%s", mcu->name);
      fprintf (f, "\n\n");

      fprintf (f, "*avrlibc_devicelib:\n");
      fprintf (f, "\t%%{!nodevicelib:-l%s}", mcu->name);
      fprintf (f, "\n\n");
    }
#endif  // WITH_AVRLIBC

  // avr-gcc specific specs for the compilation / the compiler proper.

  fprintf (f, "*cc1_n_flash:\n"
           "\t%%{!mn-flash=*:-mn-flash=%d}\n\n", mcu->n_flash);

  fprintf (f, "*cc1_rmw:\n%s\n\n", rmw
           ? "\t%{!mno-rmw: -mrmw}"
           : "\t%{mrmw}");

  fprintf (f, "*cc1_errata_skip:\n%s\n\n", errata_skip
           ? "\t%{!mno-skip-bug: -mskip-bug}"
           : "\t%{!mskip-bug: -mno-skip-bug}");

  // avr-gcc specific specs for assembling / the assembler.

  fprintf (f, "*asm_arch:\n\t-mmcu=%s\n\n", arch->name);

#ifdef HAVE_AS_AVR_MLINK_RELAX_OPTION
  fprintf (f, "*asm_relax:\n\t%s\n\n", ASM_RELAX_SPEC);
#endif // have avr-as --mlink-relax

#ifdef HAVE_AS_AVR_MRMW_OPTION
  fprintf (f, "*asm_rmw:\n%s\n\n", rmw
           ? "\t%{!mno-rmw: -mrmw}"
           : "\t%{mrmw}");
#endif // have avr-as -mrmw

  fprintf (f, "*asm_errata_skip:\n%s\n\n", errata_skip
           ? "\t%{mno-skip-bug}"
           : "\t%{!mskip-bug: -mno-skip-bug}");

  // avr-specific specs for linking / the linker.

  int wrap_k =
    str_prefix_p (mcu->name, "at90usb8") ? 8
    : str_prefix_p (mcu->name, "atmega16") ? 16
    : (str_prefix_p (mcu->name, "atmega32")
       || str_prefix_p (mcu->name, "at90can32")) ? 32
    : (str_prefix_p (mcu->name, "atmega64")
       || str_prefix_p (mcu->name, "at90can64")
       || str_prefix_p (mcu->name, "at90usb64")) ? 64
    : 0;

  fprintf (f, "*link_pmem_wrap:\n");
  if (wrap_k)
    fprintf (f, "\t%%{mpmem-wrap-around: --pmem-wrap-around=%dk}", wrap_k);
  fprintf (f, "\n\n");

  fprintf (f, "*link_relax:\n\t%s\n\n", LINK_RELAX_SPEC);

  fprintf (f, "*link_arch:\n\t%s\n\n", LINK_ARCH_SPEC);

  if (is_device)
    {
      fprintf (f, "*link_data_start:\n");
      if (mcu->data_section_start
          != arch->default_data_section_start)
        fprintf (f, "\t%%{!Tdata:-Tdata 0x%lX}",
                 0x800000UL + mcu->data_section_start);
      fprintf (f, "\n\n");

      fprintf (f, "*link_text_start:\n");
      if (mcu->text_section_start != 0x0)
        fprintf (f, "\t%%{!Ttext:-Ttext 0x%lX}", 0UL + mcu->text_section_start);
      fprintf (f, "\n\n");
    }

  // Specs known to GCC.

  if (is_device)
    {
      fprintf (f, "*self_spec:\n");
      fprintf (f, "\t%%{!mmcu=avr*: %%<mmcu=* -mmcu=%s} ", arch->name);
      fprintf (f, "%s\n\n", sp8_spec);

#if defined (WITH_AVRLIBC)
      fprintf (f, "%s\n", help_dev_lib_name);
#endif // WITH_AVRLIBC

      fprintf (f, "*cpp:\n");
      fprintf (f, "\t-D%s -D__AVR_DEVICE_NAME__=%s", mcu->macro, mcu->name);
      fprintf (f, "\n\n");
    }

  fprintf (f, "# End of file\n");

  fclose (f);
}


int main (void)
{
  for (const avr_mcu_t *mcu = avr_mcu_types; mcu->name; mcu++)
    print_mcu (mcu);

  return EXIT_SUCCESS;
}
