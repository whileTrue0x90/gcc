/* Common hooks for AArch64.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published
   by the Free Software Foundation; either version 3, or (at your
   option) any later version.

   GCC is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with GCC; see the file COPYING3.  If not see
   <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "tm_p.h"
#include "common/common-target.h"
#include "common/common-target-def.h"
#include "opts.h"
#include "flags.h"
#include "diagnostic.h"

#ifdef  TARGET_BIG_ENDIAN_DEFAULT
#undef  TARGET_DEFAULT_TARGET_FLAGS
#define TARGET_DEFAULT_TARGET_FLAGS (MASK_BIG_END)
#endif

#undef  TARGET_HANDLE_OPTION
#define TARGET_HANDLE_OPTION aarch64_handle_option

#undef	TARGET_OPTION_OPTIMIZATION_TABLE
#define TARGET_OPTION_OPTIMIZATION_TABLE aarch_option_optimization_table

/* Set default optimization options.  */
static const struct default_options aarch_option_optimization_table[] =
  {
    /* Enable section anchors by default at -O1 or higher.  */
    { OPT_LEVELS_1_PLUS, OPT_fsection_anchors, NULL, 1 },
    /* Enable -fsched-pressure by default when optimizing.  */
    { OPT_LEVELS_1_PLUS, OPT_fsched_pressure, NULL, 1 },
    /* Enable redundant extension instructions removal at -O2 and higher.  */
    { OPT_LEVELS_2_PLUS, OPT_free, NULL, 1 },
    { OPT_LEVELS_NONE, 0, NULL, 0 }
  };

/* Implement TARGET_HANDLE_OPTION.
   This function handles the target specific options for CPU/target selection.

   -mcpu=CPU is shorthand for -march=ARCH_FOR_CPU, -mtune=CPU.
   If either of -march or -mtune is given, they override their
   respective component of -mcpu.  This logic is implemented
   in config/aarch64/aarch64.c:aarch64_override_options.  */

bool
aarch64_handle_option (struct gcc_options *opts,
		       struct gcc_options *opts_set ATTRIBUTE_UNUSED,
		       const struct cl_decoded_option *decoded,
		       location_t loc ATTRIBUTE_UNUSED)
{
  size_t code = decoded->opt_index;
  const char *arg = decoded->arg;
  int val = decoded->value;

  switch (code)
    {
    case OPT_march_:
      opts->x_aarch64_arch_string = arg;
      return true;

    case OPT_mcpu_:
      opts->x_aarch64_cpu_string = arg;
      return true;

    case OPT_mtune_:
      opts->x_aarch64_tune_string = arg;
      return true;

    case OPT_mgeneral_regs_only:
      opts->x_target_flags |= MASK_GENERAL_REGS_ONLY;
      return true;

    case OPT_mfix_cortex_a53_835769:
      opts->x_aarch64_fix_a53_err835769 = val;
      return true;

    case OPT_mstrict_align:
      opts->x_target_flags |= MASK_STRICT_ALIGN;
      return true;

    case OPT_momit_leaf_frame_pointer:
      opts->x_flag_omit_frame_pointer = val;
      return true;

    default:
      return true;
    }
}

struct gcc_targetm_common targetm_common = TARGETM_COMMON_INITIALIZER;

/* An ISA extension in the co-processor and main instruction set space.  */
struct aarch64_option_extension
{
  const char *const name;
  const unsigned long flags_on;
  const unsigned long flags_off;
};

/* ISA extensions in AArch64.  */
static const struct aarch64_option_extension all_extensions[] =
{
#define AARCH64_OPT_EXTENSION(NAME, FLAGS_ON, FLAGS_OFF, FEATURE_STRING) \
  {NAME, FLAGS_ON, FLAGS_OFF},
#include "config/aarch64/aarch64-option-extensions.def"
#undef AARCH64_OPT_EXTENSION
  {NULL, 0, 0}
};

struct processor_name_to_arch
{
  const std::string processor_name;
  const enum aarch64_arch arch;
  const unsigned long flags;
};

struct arch_to_arch_name
{
  const enum aarch64_arch arch;
  const std::string arch_name;
};

/* Map processor names to the architecture revision they implement and
   the default set of architectural feature flags they support.  */
static const struct processor_name_to_arch all_cores[] =
{
#define AARCH64_CORE(NAME, X, IDENT, ARCH_IDENT, FLAGS, COSTS, IMP, PART) \
  {NAME, AARCH64_ARCH_##ARCH_IDENT, FLAGS},
#include "config/aarch64/aarch64-cores.def"
#undef AARCH64_CORE
  {"generic", AARCH64_ARCH_8A, AARCH64_FL_FOR_ARCH8},
  {"", aarch64_no_arch, 0}
};

/* Map architecture revisions to their string representation.  */
static const struct arch_to_arch_name all_architectures[] =
{
#define AARCH64_ARCH(NAME, CORE, ARCH_IDENT, ARCH, FLAGS) \
  {AARCH64_ARCH_##ARCH_IDENT, NAME},
#include "config/aarch64/aarch64-arches.def"
#undef AARCH64_ARCH
  {aarch64_no_arch, ""}
};

/* Return a string representation of ISA_FLAGS.  */

std::string
aarch64_get_extension_string_for_isa_flags (unsigned long isa_flags)
{
  const struct aarch64_option_extension *opt = NULL;
  std::string outstr = "";

  for (opt = all_extensions; opt->name != NULL; opt++)
    if ((isa_flags & opt->flags_on) == opt->flags_on)
      {
	outstr += "+";
	outstr += opt->name;
      }
  return outstr;
}

/* Attempt to rewrite NAME, which has been passed on the command line
   as a -mcpu option to an equivalent -march value.  If we can do so,
   return the new string, otherwise return an error.  */

const char *
aarch64_rewrite_selected_cpu (const char *name)
{
  std::string original_string (name);
  std::string extensions;
  std::string processor;
  size_t extension_pos = original_string.find_first_of ('+');

  /* Strip and save the extension string.  */
  if (extension_pos != std::string::npos)
    {
      processor = original_string.substr (0, extension_pos);
      extensions = original_string.substr (extension_pos,
					std::string::npos);
    }
  else
    {
      /* No extensions.  */
      processor = original_string;
    }

  const struct processor_name_to_arch* p_to_a;
  for (p_to_a = all_cores;
       p_to_a->arch != aarch64_no_arch;
       p_to_a++)
    {
      if (p_to_a->processor_name == processor)
	break;
    }

  const struct arch_to_arch_name* a_to_an;
  for (a_to_an = all_architectures;
       a_to_an->arch != aarch64_no_arch;
       a_to_an++)
    {
      if (a_to_an->arch == p_to_a->arch)
	break;
    }

  /* We couldn't find that proceesor name, or the processor name we
     found does not map to an architecture we understand.  */
  if (p_to_a->arch == aarch64_no_arch
      || a_to_an->arch == aarch64_no_arch)
    fatal_error (input_location, "unknown value %qs for -mcpu", name);

  std::string outstr = a_to_an->arch_name
	+ aarch64_get_extension_string_for_isa_flags (p_to_a->flags)
	+ extensions;

  /* We are going to memory leak here, nobody elsewhere
     in the callchain is going to clean up after us.  The alternative is
     to allocate a static buffer, and assert that it is big enough for our
     modified string, which seems much worse!  */
  return xstrdup (outstr.c_str ());
}

/* Called by the driver to rewrite a name passed to the -mcpu
   argument in preparation to be passed to the assembler.  The
   names passed from the commend line will be in ARGV, we want
   to use the right-most argument, which should be in
   ARGV[ARGC - 1].  ARGC should always be greater than 0.  */

const char *
aarch64_rewrite_mcpu (int argc, const char **argv)
{
  gcc_assert (argc);
  return aarch64_rewrite_selected_cpu (argv[argc - 1]);
}

#undef AARCH64_CPU_NAME_LENGTH

