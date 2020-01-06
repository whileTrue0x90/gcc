/* Common hooks for Blackfin.
   Copyright (C) 2005-2020 Free Software Foundation, Inc.

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
#include "diagnostic-core.h"
#include "tm.h"
#include "memmodel.h"
#include "tm_p.h"
#include "common/common-target.h"
#include "common/common-target-def.h"
#include "opts.h"
#include "flags.h"

EXPORTED_CONST struct bfin_cpu bfin_cpus[] =
{

  {"bf512", BFIN_CPU_BF512, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf512", BFIN_CPU_BF512, 0x0001,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf512", BFIN_CPU_BF512, 0x0000,
   WA_SPECULATIVE_LOADS | WA_05000074},

  {"bf514", BFIN_CPU_BF514, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf514", BFIN_CPU_BF514, 0x0001,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf514", BFIN_CPU_BF514, 0x0000,
   WA_SPECULATIVE_LOADS | WA_05000074},

  {"bf516", BFIN_CPU_BF516, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf516", BFIN_CPU_BF516, 0x0001,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf516", BFIN_CPU_BF516, 0x0000,
   WA_SPECULATIVE_LOADS | WA_05000074},

  {"bf518", BFIN_CPU_BF518, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf518", BFIN_CPU_BF518, 0x0001,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf518", BFIN_CPU_BF518, 0x0000,
   WA_SPECULATIVE_LOADS | WA_05000074},

  {"bf522", BFIN_CPU_BF522, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf522", BFIN_CPU_BF522, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},
  {"bf522", BFIN_CPU_BF522, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},

  {"bf523", BFIN_CPU_BF523, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf523", BFIN_CPU_BF523, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},
  {"bf523", BFIN_CPU_BF523, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},

  {"bf524", BFIN_CPU_BF524, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf524", BFIN_CPU_BF524, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},
  {"bf524", BFIN_CPU_BF524, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},

  {"bf525", BFIN_CPU_BF525, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf525", BFIN_CPU_BF525, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},
  {"bf525", BFIN_CPU_BF525, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},

  {"bf526", BFIN_CPU_BF526, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf526", BFIN_CPU_BF526, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},
  {"bf526", BFIN_CPU_BF526, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},

  {"bf527", BFIN_CPU_BF527, 0x0002,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf527", BFIN_CPU_BF527, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},
  {"bf527", BFIN_CPU_BF527, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000074},

  {"bf531", BFIN_CPU_BF531, 0x0006,
   WA_SPECULATIVE_LOADS | WA_LOAD_LCREGS | WA_05000074},
  {"bf531", BFIN_CPU_BF531, 0x0005,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000283 | WA_05000315
   | WA_LOAD_LCREGS | WA_05000074},
  {"bf531", BFIN_CPU_BF531, 0x0004,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},
  {"bf531", BFIN_CPU_BF531, 0x0003,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf532", BFIN_CPU_BF532, 0x0006,
   WA_SPECULATIVE_LOADS | WA_LOAD_LCREGS | WA_05000074},
  {"bf532", BFIN_CPU_BF532, 0x0005,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000283 | WA_05000315
   | WA_LOAD_LCREGS | WA_05000074},
  {"bf532", BFIN_CPU_BF532, 0x0004,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},
  {"bf532", BFIN_CPU_BF532, 0x0003,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf533", BFIN_CPU_BF533, 0x0006,
   WA_SPECULATIVE_LOADS | WA_LOAD_LCREGS | WA_05000074},
  {"bf533", BFIN_CPU_BF533, 0x0005,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_05000283 | WA_05000315
   | WA_LOAD_LCREGS | WA_05000074},
  {"bf533", BFIN_CPU_BF533, 0x0004,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},
  {"bf533", BFIN_CPU_BF533, 0x0003,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf534", BFIN_CPU_BF534, 0x0003,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_LOAD_LCREGS | WA_05000074},
  {"bf534", BFIN_CPU_BF534, 0x0002,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},
  {"bf534", BFIN_CPU_BF534, 0x0001,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf536", BFIN_CPU_BF536, 0x0003,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_LOAD_LCREGS | WA_05000074},
  {"bf536", BFIN_CPU_BF536, 0x0002,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},
  {"bf536", BFIN_CPU_BF536, 0x0001,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf537", BFIN_CPU_BF537, 0x0003,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_LOAD_LCREGS | WA_05000074},
  {"bf537", BFIN_CPU_BF537, 0x0002,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},
  {"bf537", BFIN_CPU_BF537, 0x0001,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf538", BFIN_CPU_BF538, 0x0005,
   WA_SPECULATIVE_LOADS | WA_LOAD_LCREGS | WA_05000074},
  {"bf538", BFIN_CPU_BF538, 0x0004,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_LOAD_LCREGS | WA_05000074},
  {"bf538", BFIN_CPU_BF538, 0x0003,
   WA_SPECULATIVE_LOADS | WA_RETS
   | WA_05000283 | WA_05000315 | WA_LOAD_LCREGS | WA_05000074},
  {"bf538", BFIN_CPU_BF538, 0x0002,
   WA_SPECULATIVE_LOADS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf539", BFIN_CPU_BF539, 0x0005,
   WA_SPECULATIVE_LOADS | WA_LOAD_LCREGS | WA_05000074},
  {"bf539", BFIN_CPU_BF539, 0x0004,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_LOAD_LCREGS | WA_05000074},
  {"bf539", BFIN_CPU_BF539, 0x0003,
   WA_SPECULATIVE_LOADS | WA_RETS
   | WA_05000283 | WA_05000315 | WA_LOAD_LCREGS | WA_05000074},
  {"bf539", BFIN_CPU_BF539, 0x0002,
   WA_SPECULATIVE_LOADS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf542m", BFIN_CPU_BF542M, 0x0003,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},

  {"bf542", BFIN_CPU_BF542, 0x0004,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf542", BFIN_CPU_BF542, 0x0002,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf542", BFIN_CPU_BF542, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf542", BFIN_CPU_BF542, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf544m", BFIN_CPU_BF544M, 0x0003,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},

  {"bf544", BFIN_CPU_BF544, 0x0004,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf544", BFIN_CPU_BF544, 0x0002,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf544", BFIN_CPU_BF544, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf544", BFIN_CPU_BF544, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf547m", BFIN_CPU_BF547M, 0x0003,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},

  {"bf547", BFIN_CPU_BF547, 0x0004,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf547", BFIN_CPU_BF547, 0x0002,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf547", BFIN_CPU_BF547, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf547", BFIN_CPU_BF547, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf548m", BFIN_CPU_BF548M, 0x0003,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},

  {"bf548", BFIN_CPU_BF548, 0x0004,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf548", BFIN_CPU_BF548, 0x0002,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf548", BFIN_CPU_BF548, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf548", BFIN_CPU_BF548, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf549m", BFIN_CPU_BF549M, 0x0003,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},

  {"bf549", BFIN_CPU_BF549, 0x0004,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf549", BFIN_CPU_BF549, 0x0002,
   WA_SPECULATIVE_LOADS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf549", BFIN_CPU_BF549, 0x0001,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_05000074},
  {"bf549", BFIN_CPU_BF549, 0x0000,
   WA_SPECULATIVE_LOADS | WA_RETS | WA_INDIRECT_CALLS | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf561", BFIN_CPU_BF561, 0x0005, WA_RETS
   | WA_05000283 | WA_05000315 | WA_LOAD_LCREGS | WA_05000074},
  {"bf561", BFIN_CPU_BF561, 0x0003,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},
  {"bf561", BFIN_CPU_BF561, 0x0002,
   WA_SPECULATIVE_LOADS | WA_SPECULATIVE_SYNCS | WA_RETS
   | WA_05000283 | WA_05000257 | WA_05000315 | WA_LOAD_LCREGS
   | WA_05000074},

  {"bf592", BFIN_CPU_BF592, 0x0001,
   WA_SPECULATIVE_LOADS | WA_05000074},
  {"bf592", BFIN_CPU_BF592, 0x0000,
   WA_SPECULATIVE_LOADS | WA_05000074},

  {NULL, BFIN_CPU_UNKNOWN, 0, 0}
};

/* Implement TARGET_HANDLE_OPTION.  */

static bool
bfin_handle_option (struct gcc_options *opts,
		    struct gcc_options *opts_set ATTRIBUTE_UNUSED,
		    const struct cl_decoded_option *decoded,
		    location_t loc)
{
  size_t code = decoded->opt_index;
  const char *arg = decoded->arg;
  int value = decoded->value;

  switch (code)
    {
    case OPT_mshared_library_id_:
      if (value > MAX_LIBRARY_ID)
	error_at (loc, "%<-mshared-library-id=%s%> is not between 0 and %d",
		  arg, MAX_LIBRARY_ID);
      return true;

    case OPT_mcpu_:
      {
	const char *p, *q;
	int i;

	i = 0;
	while ((p = bfin_cpus[i].name) != NULL)
	  {
	    if (strncmp (arg, p, strlen (p)) == 0)
	      break;
	    i++;
	  }

	if (p == NULL)
	  {
	    error_at (loc, "%<-mcpu=%s%> is not valid", arg);
	    return false;
	  }

	opts->x_bfin_cpu_type = bfin_cpus[i].type;

	q = arg + strlen (p);

	if (*q == '\0')
	  {
	    opts->x_bfin_si_revision = bfin_cpus[i].si_revision;
	    opts->x_bfin_workarounds |= bfin_cpus[i].workarounds;
	  }
	else if (strcmp (q, "-none") == 0)
	  opts->x_bfin_si_revision = -1;
      	else if (strcmp (q, "-any") == 0)
	  {
	    opts->x_bfin_si_revision = 0xffff;
	    while (bfin_cpus[i].type == opts->x_bfin_cpu_type)
	      {
		opts->x_bfin_workarounds |= bfin_cpus[i].workarounds;
		i++;
	      }
	  }
	else
	  {
	    unsigned int si_major, si_minor;
	    int rev_len, n;

	    rev_len = strlen (q);

	    if (sscanf (q, "-%u.%u%n", &si_major, &si_minor, &n) != 2
		|| n != rev_len
		|| si_major > 0xff || si_minor > 0xff)
	      {
	      invalid_silicon_revision:
		error_at (loc, "%<-mcpu=%s%> has invalid silicon revision",
			  arg);
		return false;
	      }

	    opts->x_bfin_si_revision = (si_major << 8) | si_minor;

	    while (bfin_cpus[i].type == opts->x_bfin_cpu_type
		   && bfin_cpus[i].si_revision != opts->x_bfin_si_revision)
	      i++;

	    if (bfin_cpus[i].type != opts->x_bfin_cpu_type)
	      goto invalid_silicon_revision;

	    opts->x_bfin_workarounds |= bfin_cpus[i].workarounds;
	  }

	return true;
      }

    default:
      return true;
    }
}

#undef TARGET_HANDLE_OPTION
#define TARGET_HANDLE_OPTION bfin_handle_option

#undef TARGET_DEFAULT_TARGET_FLAGS
#define TARGET_DEFAULT_TARGET_FLAGS TARGET_DEFAULT

struct gcc_targetm_common targetm_common = TARGETM_COMMON_INITIALIZER;
