/* C/ObjC/C++ command line option handling.
   Copyright (C) 2002 Free Software Foundation, Inc.
   Contributed by Neil Booth.

This file is part of GCC.

GCC is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2, or (at your option) any later
version.

GCC is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.  */

#include "config.h"
#include "system.h"
#include "tree.h"
#include "c-common.h"
#include "c-pragma.h"
#include "flags.h"
#include "toplev.h"
#include "langhooks.h"
#include "tree-inline.h"
#include "diagnostic.h"

static void missing_arg PARAMS ((size_t));
static size_t parse_option PARAMS ((const char *, int));
static void set_Wimplicit PARAMS ((int));
static void complain_wrong_lang PARAMS ((size_t));
static void write_langs PARAMS ((char *, int));

#define CL_C_ONLY	(1 << 0) /* Only C.  */
#define CL_OBJC_ONLY	(1 << 1) /* Only ObjC.  */
#define CL_CXX_ONLY	(1 << 2) /* Only C++.  */
#define CL_OBJCXX_ONLY	(1 << 3) /* Only ObjC++.  */
#define CL_JOINED	(1 << 4) /* If takes joined argument.  */
#define CL_SEPARATE	(1 << 5) /* If takes a separate argument.  */

#define CL_ARG		(CL_JOINED | CL_SEPARATE)
#define CL_C		(CL_C_ONLY | CL_OBJC_ONLY)
#define CL_OBJC		(CL_OBJC_ONLY | CL_OBJCXX_ONLY)
#define CL_CXX		(CL_CXX_ONLY | CL_OBJCXX_ONLY)
#define CL_ALL		(CL_C | CL_CXX)

/* This is the list of all command line options, with the leading "-"
   removed.  It must be sorted in ASCII collating order.  All options
   beginning with "f" or "W" are implicitly assumed to take a "no-"
   form; this form should not be listed.  If you don't want a "no-"
   form, your handler should check and reject it.

   If the user gives an option to a front end that doesn't support it,
   an error is output, mentioning which front ends the option is valid
   for.  If you don't want this, you must accept it for all front
   ends, and test for the front end in the option handler.  */
#define COMMAND_LINE_OPTIONS						     \
  OPT("E",			CL_ALL,   OPT_E)			     \
  OPT("Wall",			CL_ALL,   OPT_Wall)			     \
  OPT("Wbad-function-cast",	CL_C,     OPT_Wbad_function_cast)	     \
  OPT("Wcast-qual",		CL_ALL,   OPT_Wcast_qual)		     \
  OPT("Wchar-subscripts",	CL_ALL,   OPT_Wchar_subscripts)		     \
  OPT("Wconversion",		CL_ALL,   OPT_Wconversion)		     \
  OPT("Wctor-dtor-privacy",	CL_CXX,   OPT_Wctor_dtor_privacy)	     \
  OPT("Wdeprecated",		CL_CXX,   OPT_Wdeprecated)		     \
  OPT("Wdiv-by-zero",		CL_C,     OPT_Wdiv_by_zero)		     \
  OPT("Weffc++",		CL_CXX,   OPT_Weffcxx)			     \
  OPT("Werror-implicit-function-declaration",				     \
	     			CL_C,     OPT_Werror_implicit_function_decl) \
  OPT("Wfloat-equal",		CL_ALL,   OPT_Wfloat_equal)		     \
  OPT("Wformat",		CL_ALL,   OPT_Wformat)			     \
  OPT("Wformat-extra-args",	CL_ALL,   OPT_Wformat_extra_args)	     \
  OPT("Wformat-nonliteral",	CL_ALL,   OPT_Wformat_nonliteral)	     \
  OPT("Wformat-security",	CL_ALL,   OPT_Wformat_security)		     \
  OPT("Wformat-y2k",		CL_ALL,   OPT_Wformat_y2k)		     \
  OPT("Wformat-zero-length",	CL_C,     OPT_Wformat_zero_length)	     \
  OPT("Wformat=",		CL_ALL | CL_JOINED, OPT_Wformat_eq)	     \
  OPT("Wimplicit",		CL_CXX,   OPT_Wimplicit)		     \
  OPT("Wimplicit-function-declaration",	CL_C, OPT_Wimplicit_function_decl)   \
  OPT("Wimplicit-int",		CL_C,	  OPT_Wimplicit_int)		     \
  OPT("Wlong-long",		CL_ALL,   OPT_Wlong_long)		     \
  OPT("Wmain",			CL_C,     OPT_Wmain)			     \
  OPT("Wmissing-braces",	CL_ALL,   OPT_Wmissing_braces)		     \
  OPT("Wmissing-declarations",	CL_C,     OPT_Wmissing_declarations)	     \
  OPT("Wmissing-format-attribute",CL_ALL, OPT_Wmissing_format_attribute)     \
  OPT("Wmissing-prototypes",	CL_ALL,   OPT_Wmissing_prototypes)	     \
  OPT("Wmultichar",		CL_ALL,   OPT_Wmultichar)		     \
  OPT("Wnested-externs",	CL_C,     OPT_Wnested_externs)		     \
  OPT("Wnon-template-friend",	CL_CXX,   OPT_Wnon_template_friend)	     \
  OPT("Wnon-virtual-dtor",	CL_CXX,   OPT_Wnon_virtual_dtor)	     \
  OPT("Wnonnull",		CL_C,     OPT_Wnonnull)			     \
  OPT("Wold-style-cast",	CL_CXX,   OPT_Wold_style_cast)		     \
  OPT("Woverloaded-virtual",	CL_CXX,   OPT_Woverloaded_virtual)	     \
  OPT("Wparentheses",		CL_ALL,   OPT_Wparentheses)		     \
  OPT("Wpmf-conversions",	CL_CXX,   OPT_Wpmf_conversions)		     \
  OPT("Wpointer-arith",		CL_ALL,   OPT_Wpointer_arith)		     \
  OPT("Wprotocol",		CL_OBJC,  OPT_Wprotocol)		     \
  OPT("Wredundant-decls",	CL_ALL,   OPT_Wredundant_decls)		     \
  OPT("Wreorder",		CL_CXX,   OPT_Wreorder)			     \
  OPT("Wreturn-type",		CL_ALL,   OPT_Wreturn_type)		     \
  OPT("Wselector",		CL_OBJC,  OPT_Wselector)		     \
  OPT("Wsequence-point",	CL_C,     OPT_Wsequence_point)		     \
  OPT("Wsign-compare",		CL_ALL,   OPT_Wsign_compare)		     \
  OPT("Wsign-promo",		CL_CXX,   OPT_Wsign_promo)		     \
  OPT("Wstrict-prototypes",	CL_ALL,   OPT_Wstrict_prototypes)	     \
  OPT("Wsynth",			CL_CXX,   OPT_Wsynth)			     \
  OPT("Wtraditional",		CL_C,     OPT_Wtraditional)		     \
  OPT("Wunknown-pragmas",	CL_ALL,   OPT_Wunknown_pragmas)		     \
  OPT("Wwrite-strings",		CL_ALL,   OPT_Wwrite_strings)		     \
  OPT("ansi",			CL_ALL,   OPT_ansi)			     \
  OPT("faccess-control",	CL_CXX,   OPT_faccess_control)		     \
  OPT("fall-virtual",		CL_CXX,   OPT_fall_virtual)		     \
  OPT("falt-external-templates",CL_CXX,   OPT_falt_external_templates)	     \
  OPT("fasm",			CL_ALL,   OPT_fasm)			     \
  OPT("fbuiltin",		CL_ALL,   OPT_fbuiltin)			     \
  OPT("fbuiltin-",		CL_ALL | CL_JOINED, OPT_fbuiltin_)	     \
  OPT("fcheck-new",		CL_CXX,   OPT_fcheck_new)		     \
  OPT("fcond-mismatch",		CL_ALL,   OPT_fcond_mismatch)		     \
  OPT("fconserve-space",	CL_CXX,   OPT_fconserve_space)		     \
  OPT("fconst-strings",		CL_CXX,   OPT_fconst_strings)		     \
  OPT("fconstant-string-class=", CL_OBJC | CL_JOINED,			     \
					  OPT_fconstant_string_class)	     \
  OPT("fdefault-inline",	CL_CXX,   OPT_fdefault_inline)		     \
  OPT("fdollars-in-identifiers",CL_ALL,   OPT_fdollars_in_identifiers)	     \
  OPT("fdump-",			CL_ALL | CL_JOINED, OPT_fdump)		     \
  OPT("felide-constructors",	CL_CXX,   OPT_felide_constructors)	     \
  OPT("fenforce-eh-specs",	CL_CXX,   OPT_fenforce_eh_specs)	     \
  OPT("fenum-int-equiv",	CL_CXX,   OPT_fenum_int_equiv)		     \
  OPT("fexternal-templates",	CL_CXX,   OPT_fexternal_templates)	     \
  OPT("ffor-scope",		CL_CXX,   OPT_ffor_scope)		     \
  OPT("ffreestanding",		CL_C,     OPT_ffreestanding)		     \
  OPT("fgnu-keywords",		CL_CXX,   OPT_fgnu_keywords)		     \
  OPT("fgnu-runtime",		CL_OBJC,  OPT_fgnu_runtime)		     \
  OPT("fguiding-decls",		CL_CXX,   OPT_fguiding_decls)		     \
  OPT("fhandle-exceptions",	CL_CXX,   OPT_fhandle_exceptions)	     \
  OPT("fhonor-std",		CL_CXX,   OPT_fhonor_std)		     \
  OPT("fhosted",		CL_C,     OPT_fhosted)			     \
  OPT("fhuge-objects",		CL_CXX,   OPT_fhuge_objects)		     \
  OPT("fimplement-inlines",	CL_CXX,   OPT_fimplement_inlines)	     \
  OPT("fimplicit-inline-templates", CL_CXX, OPT_fimplicit_inline_templates)  \
  OPT("fimplicit-templates",	CL_CXX,   OPT_fimplicit_templates)	     \
  OPT("flabels-ok",		CL_CXX,   OPT_flabels_ok)		     \
  OPT("fms-extensions",		CL_ALL,   OPT_fms_extensions)		     \
  OPT("fname-mangling-version-",CL_CXX | CL_JOINED, OPT_fname_mangling)	     \
  OPT("fnew-abi",		CL_CXX,   OPT_fnew_abi)			     \
  OPT("fnext-runtime",		CL_OBJC,  OPT_fnext_runtime)		     \
  OPT("fnonansi-builtins",	CL_CXX,   OPT_fnonansi_builtins)	     \
  OPT("fnonnull-objects",	CL_CXX,   OPT_fnonnull_objects)		     \
  OPT("foptional-diags",	CL_CXX,   OPT_foptional_diags)		     \
  OPT("fpermissive",		CL_CXX,   OPT_fpermissive)		     \
  OPT("frepo",			CL_CXX,   OPT_frepo)			     \
  OPT("frtti",			CL_CXX,   OPT_frtti)			     \
  OPT("fshort-double",		CL_ALL,   OPT_fshort_double)		     \
  OPT("fshort-enums",		CL_ALL,   OPT_fshort_enums)		     \
  OPT("fshort-wchar",		CL_ALL,   OPT_fshort_wchar)		     \
  OPT("fsigned-bitfields",	CL_ALL,   OPT_fsigned_bitfields)	     \
  OPT("fsigned-char",		CL_ALL,   OPT_fsigned_char)		     \
  OPT("fsquangle",		CL_CXX,   OPT_fsquangle)		     \
  OPT("fstats",			CL_CXX,   OPT_fstats)			     \
  OPT("fstrict-prototype",	CL_CXX,   OPT_fstrict_prototype)	     \
  OPT("ftemplate-depth-",	CL_CXX | CL_JOINED, OPT_ftemplate_depth)     \
  OPT("fthis-is-variable",	CL_CXX,   OPT_fthis_is_variable)	     \
  OPT("funsigned-bitfields",	CL_ALL,   OPT_funsigned_bitfields)	     \
  OPT("funsigned-char",		CL_ALL,   OPT_funsigned_char)		     \
  OPT("fuse-cxa-atexit",	CL_CXX,   OPT_fuse_cxa_atexit)		     \
  OPT("fvtable-gc",		CL_CXX,   OPT_fvtable_gc)		     \
  OPT("fvtable-thunks",		CL_CXX,   OPT_fvtable_thunks)		     \
  OPT("fweak",			CL_CXX,   OPT_fweak)			     \
  OPT("fxref",			CL_CXX,   OPT_fxref)			     \
  OPT("gen-decls",		CL_OBJC,  OPT_gen_decls)		     \
  OPT("print-objc-runtime-info", CL_OBJC, OPT_print_objc_runtime_info)	     \
  OPT("std=",			CL_ALL | CL_JOINED, OPT_std_bad)	     \
  OPT("std=c++98",		CL_CXX,	  OPT_std_cplusplus98)		     \
  OPT("std=c89",		CL_C,     OPT_std_c89)			     \
  OPT("std=c99",		CL_C,     OPT_std_c99)			     \
  OPT("std=c9x",		CL_C,     OPT_std_c9x)			     \
  OPT("std=gnu89",		CL_C,     OPT_std_gnu89)		     \
  OPT("std=gnu99",		CL_C,     OPT_std_gnu99)		     \
  OPT("std=gnu9x",		CL_C,     OPT_std_gnu9x)		     \
  OPT("std=iso9899:1990",	CL_C,     OPT_std_iso9899_1990)		     \
  OPT("std=iso9899:199409",	CL_C,     OPT_std_iso9899_199409)	     \
  OPT("std=iso9899:1999",	CL_C,     OPT_std_iso9899_1999)		     \
  OPT("std=iso9899:199x",	CL_C,     OPT_std_iso9899_199x)		     \
  OPT("undef",			CL_ALL,   OPT_undef)

#define OPT(text, flags, code) code,
enum opt_code
{
  COMMAND_LINE_OPTIONS
  N_OPTS
};
#undef OPT

struct cl_option
{
  const char *opt_text;
  unsigned char opt_len;
  unsigned char flags;
  ENUM_BITFIELD (opt_code) opt_code : 2 * CHAR_BIT;
};

#define OPT(text, flags, code) { text, sizeof(text) - 1, flags, code },
#ifdef HOST_EBCDIC
static struct cl_option cl_options[] =
#else
static const struct cl_option cl_options[] =
#endif
{
  COMMAND_LINE_OPTIONS
};
#undef OPT
#undef COMMAND_LINE_OPTIONS

#ifdef HOST_EBCDIC
static int opt_comp PARAMS ((const void *, const void *));

/* Run-time sorting of options array.  */
static int
opt_comp (p1, p2)
     const void *p1, *p2;
{
  return strcmp (((struct cl_option *) p1)->opt_text,
		 ((struct cl_option *) p2)->opt_text);
}
#endif

/* Perform a binary search to find which option the command-line INPUT
   matches.  Returns its index in the option array, and N_OPTS on
   failure.

   Complications arise since some options can be suffixed with an
   argument, and multiple complete matches can occur, e.g. -pedantic
   and -pedantic-errors.  Also, some options are only accepted by some
   languages.  */
static size_t
parse_option (input, lang_flag)
     const char *input;
     int lang_flag;
{
  size_t md, mn, mx;
  size_t opt_len;
  size_t wrong_lang = N_OPTS;
  int comp;

  mn = 0;
  mx = N_OPTS;

  while (mx > mn)
    {
      md = (mn + mx) / 2;

      opt_len = cl_options[md].opt_len;
      comp = memcmp (input, cl_options[md].opt_text, opt_len);

      if (comp < 0)
	mx = md;
      else if (comp > 0)
	mn = md + 1;
      else
	{
	  /* The switch matches.  It it an exact match?  */
	  if (input[opt_len] == '\0')
	    {
	    exact_match:
	      if (cl_options[md].flags & lang_flag)
		return md;
	      wrong_lang = md;
	      break;
	    }
	  else
	    {
	      mn = md + 1;

	      /* If the switch takes no arguments this is not a proper
		 match, so we continue the search (e.g. input="stdc++"
		 match was "stdc").  */
	      if (!(cl_options[md].flags & CL_JOINED))
		continue;

	      /* Is this switch valid for this front end?  */
	      if (!(cl_options[md].flags & lang_flag))
		{
		  /* If subsequently we don't find a good match,
		     report this as a bad match.  */
		  wrong_lang = md;
		  continue;
		}

	      /* Two scenarios remain: we have the switch's argument,
		 or we match a longer option.  This can happen with
		 -iwithprefix and -withprefixbefore.  The longest
		 possible option match succeeds.

		 Scan forwards, and return an exact match.  Otherwise
		 return the longest valid option-accepting match (mx).
		 This loops at most twice with current options.  */
	      mx = md;
	      for (md = md + 1; md < (size_t) N_OPTS; md++)
		{
		  opt_len = cl_options[md].opt_len;
		  if (memcmp (input, cl_options[md].opt_text, opt_len))
		    break;
		  if (input[opt_len] == '\0')
		    goto exact_match;
		  if (cl_options[md].flags & lang_flag
		      && cl_options[md].flags & CL_JOINED)
		    mx = md;
		}

	      return mx;
	    }
	}
    }

  if (wrong_lang != N_OPTS)
    complain_wrong_lang (wrong_lang);

  return N_OPTS;
}

/* Common initialization before parsing options.  */
void
c_common_init_options (lang)
     enum c_language_kind lang;
{
#ifdef HOST_EBCDIC
  /* For non-ASCII hosts, the cl_options array needs to be sorted at
     runtime.  */
  qsort (cl_options, N_OPTS, sizeof (struct cl_option), opt_comp);
#endif

  c_language = lang;
  parse_in = cpp_create_reader (lang == clk_c || lang == clk_objective_c
				? CLK_GNUC89 : CLK_GNUCXX);
  if (lang == clk_objective_c)
    cpp_get_options (parse_in)->objc = 1;

  flag_const_strings = (lang == clk_cplusplus);
  warn_pointer_arith = (lang == clk_cplusplus);
  if (lang == clk_c)
    warn_sign_compare = -1;

  /* Mark as "unspecified" (see c_common_post_options).  */
  flag_bounds_check = -1;
}

/* Handle one command-line option in (argc, argv).
   Can be called multiple times, to handle multiple sets of options.
   Returns number of strings consumed.  */
int
c_common_decode_option (argc, argv)
     int argc;
     char **argv;
{
  size_t opt_index;
  const char *opt, *arg = 0;
  char *dup = 0;
  bool on = true;
  int result, lang_flag;
  const struct cl_option *option;
  enum opt_code code;

  result = cpp_handle_option (parse_in, argc, argv);
  opt = argv[0];

  /* Until handling CPP stuff, ignore non-switches.  */
  if (opt[0] != '-' || opt[1] == '\0')
    return result;

  switch (c_language)
    {
    case clk_c:			lang_flag = CL_C_ONLY; break;
    case clk_cplusplus:		lang_flag = CL_CXX_ONLY; break;
    case clk_objective_c:	lang_flag = CL_OBJC_ONLY; break;
    default:			abort ();
    }

  /* Drop the "no-" from negative switches.  */
  if ((opt[1] == 'W' || opt[1] == 'f')
      && opt[2] == 'n' && opt[3] == 'o' && opt[4] == '-')
    {
      size_t len = strlen (opt) - 3;

      dup = xmalloc (len + 1);
      dup[0] = '-';
      dup[1] = opt[1];
      memcpy (dup + 2, opt + 5, len - 2 + 1);
      opt = dup;
      on = false;
    }

  /* Skip over '-'.  */
  opt_index = parse_option (opt + 1, lang_flag);
  if (opt_index == N_OPTS)
    goto done;
  option = &cl_options[opt_index];

  /* Sort out any argument the switch takes.  */
  if (option->flags & CL_ARG)
    {
      if (option->flags & CL_JOINED)
	{
	  /* Have arg point to the original switch.  This is because
	     some code, such as disable_builtin_function, expects its
	     argument to be persistent until the program exits.  */
	  arg = argv[0] + cl_options[opt_index].opt_len + 1;
	  if (!on)
	    arg += strlen ("no-");
	  if (*arg == '\0' && (option->flags & CL_SEPARATE))
	    arg = 0;
	}

      /* If arg is still 0, we can only be a CL_SEPARATE switch.  */
      if (arg == 0)
	{
	  arg = argv[1];
	  if (!arg)
	    {
	      missing_arg (opt_index);
	      result = argc;
	      goto done;
	    }
	}
    }

  switch (code = cl_options[opt_index].opt_code)
    {
    case N_OPTS: /* Shut GCC up.  */
      break;

    case OPT_E:
      flag_preprocess_only = 1;
      break;

    case OPT_Wall:
      set_Wunused (on);
      set_Wformat (on);
      set_Wimplicit (on);
      warn_char_subscripts = on;
      warn_missing_braces = on;
      warn_multichar = on;	/* Was C++ only.  */
      warn_parentheses = on;
      warn_return_type = on;
      warn_sequence_point = on;	/* Was C only.  */
      warn_sign_compare = on;	/* Was C++ only.  */
      warn_switch = on;

      /* Only warn about unknown pragmas that are not in system
	 headers.  */                                        
      warn_unknown_pragmas = on;

      /* We save the value of warn_uninitialized, since if they put
	 -Wuninitialized on the command line, we need to generate a
	 warning about not using it without also specifying -O.  */
      if (warn_uninitialized != 1)
	warn_uninitialized = (on ? 2 : 0);

      if (c_language == clk_c || c_language == clk_objective_c)
	/* We set this to 2 here, but 1 in -Wmain, so -ffreestanding
	   can turn it off only if it's not explicit.  */
	warn_main = on * 2;
      else
	{
	  /* C++-specific warnings.  */
	  warn_ctor_dtor_privacy = on;
	  warn_nonvdtor = on;
	  warn_reorder = on;
	  warn_nontemplate_friend = on;
	}
      break;

    case OPT_Wbad_function_cast:
      warn_bad_function_cast = on;
      break;

    case OPT_Wcast_qual:
      warn_cast_qual = on;
      break;

    case OPT_Wchar_subscripts:
      warn_char_subscripts = on;
      break;

    case OPT_Wconversion:
      warn_conversion = on;
      break;

    case OPT_Wctor_dtor_privacy:
      warn_ctor_dtor_privacy = on;
      break;

    case OPT_Wdeprecated:
      warn_deprecated = on;
      break;

    case OPT_Wdiv_by_zero:
      warn_div_by_zero = on;
      break;

    case OPT_Weffcxx:
      warn_ecpp = on;
      break;

    case OPT_Werror_implicit_function_decl:
      if (!on)
	{
	  result = 0;
	  goto done;
	}
      mesg_implicit_function_declaration = 2;
      break;

    case OPT_Wfloat_equal:
      warn_float_equal = on;
      break;

    case OPT_Wformat:
      set_Wformat (on);
      break;

    case OPT_Wformat_eq:
      set_Wformat (atoi (arg));
      break;

    case OPT_Wformat_extra_args:
      warn_format_extra_args = on;
      break;

    case OPT_Wformat_nonliteral:
      warn_format_nonliteral = on;
      break;

    case OPT_Wformat_security:
      warn_format_security = on;
      break;

    case OPT_Wformat_y2k:
      warn_format_y2k = on;
      break;

    case OPT_Wformat_zero_length:
      warn_format_zero_length = on;
      break;

    case OPT_Wimplicit:
      set_Wimplicit (on);
      break;

    case OPT_Wimplicit_function_decl:
      mesg_implicit_function_declaration = on;
      break;

    case OPT_Wimplicit_int:
      warn_implicit_int = on;
      break;

    case OPT_Wlong_long:
      warn_long_long = on;
      break;

    case OPT_Wmain:
      if (on)
	warn_main = 1;
      else
	warn_main = -1;
      break;

    case OPT_Wmissing_braces:
      warn_missing_braces = on;
      break;

    case OPT_Wmissing_declarations:
      warn_missing_declarations = on;
      break;

    case OPT_Wmissing_format_attribute:
      warn_missing_format_attribute = on;
      break;

    case OPT_Wmissing_prototypes:
      warn_missing_prototypes = on;
      break;

    case OPT_Wmultichar:
      warn_multichar = on;
      break;

    case OPT_Wnested_externs:
      warn_nested_externs = on;
      break;

    case OPT_Wnon_template_friend:
      warn_nontemplate_friend = on;
      break;

    case OPT_Wnon_virtual_dtor:
      warn_nonvdtor = on;
      break;

    case OPT_Wnonnull:
      warn_nonnull = on;
      break;

    case OPT_Wold_style_cast:
      warn_old_style_cast = on;
      break;

    case OPT_Woverloaded_virtual:
      warn_overloaded_virtual = on;
      break;

    case OPT_Wparentheses:
      warn_parentheses = on;
      break;

    case OPT_Wpmf_conversions:
      warn_pmf2ptr = on;
      break;

    case OPT_Wpointer_arith:
      warn_pointer_arith = on;
      break;

    case OPT_Wprotocol:
      warn_protocol = on;
      break;

    case OPT_Wselector:
      warn_selector = on;
      break;

    case OPT_Wredundant_decls:
      warn_redundant_decls = on;
      break;

    case OPT_Wreorder:
      warn_reorder = on;
      break;

    case OPT_Wreturn_type:
      warn_return_type = on;
      break;

    case OPT_Wsequence_point:
      warn_sequence_point = on;
      break;

    case OPT_Wsign_compare:
      warn_sign_compare = on;
      break;

    case OPT_Wsign_promo:
      warn_sign_promo = on;
      break;

    case OPT_Wstrict_prototypes:
      if (!on && c_language == clk_cplusplus)
	warning ("-Wno-strict-prototypes is not supported in C++");
      else
	warn_strict_prototypes = on;
      break;

    case OPT_Wsynth:
      warn_synth = on;
      break;

    case OPT_Wtraditional:
      warn_traditional = on;
      break;

    case OPT_Wunknown_pragmas:
      /* Set to greater than 1, so that even unknown pragmas in
	 system headers will be warned about.  */  
      warn_unknown_pragmas = on * 2;
      break;

    case OPT_Wwrite_strings:
      if (c_language == clk_c || c_language == clk_objective_c)
	flag_const_strings = on;
      else
	warn_write_strings = on;
      break;

    case OPT_fcond_mismatch:
      if (c_language == clk_c || c_language == clk_objective_c)
	{
	  flag_cond_mismatch = on;
	  break;
	}
      /* Fall through.  */

    case OPT_fall_virtual:
    case OPT_fenum_int_equiv:
    case OPT_fguiding_decls:
    case OPT_fhonor_std:
    case OPT_fhuge_objects:
    case OPT_flabels_ok:
    case OPT_fname_mangling:
    case OPT_fnew_abi:
    case OPT_fnonnull_objects:
    case OPT_fsquangle:
    case OPT_fstrict_prototype:
    case OPT_fthis_is_variable:
    case OPT_fvtable_thunks:
    case OPT_fxref:
      warning ("switch \"%s\" is no longer supported", argv[0]);
      break;

    case OPT_faccess_control:
      flag_access_control = on;
      break;

    case OPT_falt_external_templates:
      flag_alt_external_templates = on;
      if (on)
	flag_external_templates = true;
    cp_deprecated:
      warning ("switch \"%s\" is deprecated, please see documentation for details", argv[0]);
      break;

    case OPT_fasm:
      flag_no_asm = !on;
      break;

    case OPT_fbuiltin:
      flag_no_builtin = !on;
      break;

    case OPT_fbuiltin_:
      if (on)
	{
	  result = 0;
	  goto done;
	}
      disable_builtin_function (arg);
      break;

    case OPT_fdollars_in_identifiers:
      dollars_in_ident = on;
      break;

    case OPT_fdump:
      if (!on || !dump_switch_p (argv[0] + strlen ("-f")))
	{
	  result = 0;
	  goto done;
	}
      break;

    case OPT_ffreestanding:
      on = !on;
      /* Fall through...  */
    case OPT_fhosted:
      flag_hosted = on;
      flag_no_builtin = !on;
      /* warn_main will be 2 if set by -Wall, 1 if set by -Wmain */
      if (!on && warn_main == 2)
	warn_main = 0;
      break;

    case OPT_fshort_double:
      flag_short_double = on;
      break;

    case OPT_fshort_enums:
      flag_short_enums = on;
      break;

    case OPT_fshort_wchar:
      flag_short_wchar = on;
      break;

    case OPT_fsigned_bitfields:
      flag_signed_bitfields = on;
      explicit_flag_signed_bitfields = 1;
      break;

    case OPT_fsigned_char:
      flag_signed_char = on;
      break;

    case OPT_funsigned_bitfields:
      flag_signed_bitfields = !on;
      explicit_flag_signed_bitfields = 1;
      break;

    case OPT_funsigned_char:
      flag_signed_char = !on;
      break;

    case OPT_fcheck_new:
      flag_check_new = on;
      break;

    case OPT_fconserve_space:
      flag_conserve_space = on;
      break;

    case OPT_fconst_strings:
      flag_const_strings = on;
      break;

    case OPT_fconstant_string_class:
      if (*arg == 0)
	error ("no class name specified with -fconstant-string-class=");
      else
	constant_string_class_name = arg;
      break;

    case OPT_fdefault_inline:
      flag_default_inline = on;
      break;

    case OPT_felide_constructors:
      flag_elide_constructors = on;
      break;

    case OPT_fenforce_eh_specs:
      flag_enforce_eh_specs = on;
      break;

    case OPT_fexternal_templates:
      flag_external_templates = on;
      goto cp_deprecated;

    case OPT_ffor_scope:
      flag_new_for_scope = on;
      break;

    case OPT_fgnu_keywords:
      flag_no_gnu_keywords = !on;
      break;

    case OPT_fgnu_runtime:
      flag_next_runtime = !on;
      break;

    case OPT_fhandle_exceptions:
      warning ("-fhandle-exceptions has been renamed to -fexceptions (and is now on by default)");
      flag_exceptions = on;
      break;

    case OPT_fimplement_inlines:
      flag_implement_inlines = on;
      break;

    case OPT_fimplicit_inline_templates:
      flag_implicit_inline_templates = on;
      break;

    case OPT_fimplicit_templates:
      flag_implicit_templates = on;
      break;

    case OPT_fms_extensions:
      flag_ms_extensions = on;
      break;

    case OPT_fnext_runtime:
      flag_next_runtime = on;
      break;

    case OPT_fnonansi_builtins:
      flag_no_nonansi_builtin = !on;
      break;

    case OPT_foptional_diags:
      flag_optional_diags = on;
      break;

    case OPT_fpermissive:
      flag_permissive = on;
      break;

    case OPT_frepo:
      flag_use_repository = on;
      if (on)
	flag_implicit_templates = 0;
      break;

    case OPT_frtti:
      flag_rtti = on;
      break;

    case OPT_fstats:
      flag_detailed_statistics = on;
      break;

    case OPT_ftemplate_depth:
      max_tinst_depth = read_integral_parameter (arg, argv[0], 0);
      break;

    case OPT_fvtable_gc:
      flag_vtable_gc = on;
      break;

    case OPT_fuse_cxa_atexit:
      flag_use_cxa_atexit = on;
      break;

    case OPT_fweak:
      flag_weak = on;
      break;

    case OPT_gen_decls:
      flag_gen_declaration = 1;
      break;

    case OPT_print_objc_runtime_info:
      print_struct_values = 1;
      break;

    case OPT_std_bad:
      error ("unknown standard \"%s\"", arg);
      break;

      /* Language standards.  We currently recognize:
	 -std=iso9899:1990	same as -ansi
	 -std=iso9899:199409	ISO C as modified in amend. 1
	 -std=iso9899:1999	ISO C 99
	 -std=c89		same as -std=iso9899:1990
	 -std=c99		same as -std=iso9899:1999
	 -std=gnu89		default, iso9899:1990 + gnu extensions
	 -std=gnu99		iso9899:1999 + gnu extensions
      */

    case OPT_std_cplusplus98:
      break;

    case OPT_std_c89:
    case OPT_std_iso9899_1990:
    case OPT_std_iso9899_199409:
    case OPT_ansi:
      /* Note: -ansi is used by both the C and C++ front ends.  */
      if (c_language == clk_c || c_language == clk_objective_c)
	{
	  flag_no_asm = 1;
	  flag_writable_strings = 0;
	}
      flag_isoc94 = (code == OPT_std_iso9899_199409);
      flag_no_gnu_keywords = 1;
      flag_no_nonansi_builtin = 1;
      flag_noniso_default_format_attributes = 0;
      flag_isoc99 = 0;
      flag_iso = 1;
      break;

    case OPT_std_c99:
    case OPT_std_c9x:
    case OPT_std_iso9899_1999:
    case OPT_std_iso9899_199x:
      flag_writable_strings = 0;
      flag_no_asm = 1;
      flag_no_nonansi_builtin = 1;
      flag_noniso_default_format_attributes = 0;
      flag_isoc99 = 1;
      flag_isoc94 = 1;
      flag_iso = 1;
      break;

    case OPT_std_gnu89:
      flag_writable_strings = 0;
      flag_no_asm = 0;
      flag_no_nonansi_builtin = 0;
      flag_noniso_default_format_attributes = 1;
      flag_isoc99 = 0;
      flag_isoc94 = 0;
      break;

    case OPT_std_gnu99:
    case OPT_std_gnu9x:
      flag_writable_strings = 0;
      flag_no_asm = 0;
      flag_no_nonansi_builtin = 0;
      flag_noniso_default_format_attributes = 1;
      flag_isoc99 = 1;
      flag_isoc94 = 1;
      break;

    case OPT_undef:
      flag_undef = 1;
      break;
    }

  result = 1 + (arg == argv[1]);

 done:
  if (dup)
    free (dup);
  return result;
}

/* Post-switch processing.  */
bool
c_common_post_options ()
{
  cpp_post_options (parse_in);

  flag_inline_trees = 1;

  /* Use tree inlining if possible.  Function instrumentation is only
     done in the RTL level, so we disable tree inlining.  */
  if (! flag_instrument_function_entry_exit)
    {
      if (!flag_no_inline)
	flag_no_inline = 1;
      if (flag_inline_functions)
	{
	  flag_inline_trees = 2;
	  flag_inline_functions = 0;
	}
    }

  /* If still "unspecified", make it match -fbounded-pointers.  */
  if (flag_bounds_check == -1)
    flag_bounds_check = flag_bounded_pointers;

  /* Special format checking options don't work without -Wformat; warn if
     they are used.  */
  if (warn_format_y2k && !warn_format)
    warning ("-Wformat-y2k ignored without -Wformat");
  if (warn_format_extra_args && !warn_format)
    warning ("-Wformat-extra-args ignored without -Wformat");
  if (warn_format_zero_length && !warn_format)
    warning ("-Wformat-zero-length ignored without -Wformat");
  if (warn_format_nonliteral && !warn_format)
    warning ("-Wformat-nonliteral ignored without -Wformat");
  if (warn_format_security && !warn_format)
    warning ("-Wformat-security ignored without -Wformat");
  if (warn_missing_format_attribute && !warn_format)
    warning ("-Wmissing-format-attribute ignored without -Wformat");

  /* If an error has occurred in cpplib, note it so we fail
     immediately.  */
  errorcount += cpp_errors (parse_in);

  return flag_preprocess_only;
}

/* Handle setting implicit to ON.  */
static void
set_Wimplicit (on)
     int on;
{
  warn_implicit = on;
  warn_implicit_int = on;
  if (on)
    {
      if (mesg_implicit_function_declaration != 2)
	mesg_implicit_function_declaration = 1;
    }
  else
    mesg_implicit_function_declaration = 0;
}

/* Complain that switch OPT_INDEX expects an argument but none was
   provided.  This is currenlty unused, as the C front ends have no
   switches that take separate arguments.  Will be used when cpplib's
   switches are integrated.  */
static void
missing_arg (opt_index)
     size_t opt_index ATTRIBUTE_UNUSED;
{
  abort ();
}

/* Write a slash-separated list of languages in FLAGS to BUF.  */
static void
write_langs (buf, flags)
     char *buf;
     int flags;
{
  *buf = '\0';
  if (flags & CL_C_ONLY)
    strcat (buf, "C");
  if (flags & CL_OBJC_ONLY)
    {
      if (*buf)
	strcat (buf, "/");
      strcat (buf, "ObjC");
    }
  if (flags & CL_CXX_ONLY)
    {
      if (*buf)
	strcat (buf, "/");
      strcat (buf, "C++");
    }
}

/* Complain that switch OPT_INDEX does not apply to this front end.  */
static void
complain_wrong_lang (opt_index)
     size_t opt_index;
{
  char ok_langs[60], bad_langs[60];
  int ok_flags = cl_options[opt_index].flags;

  write_langs (ok_langs, ok_flags);
  write_langs (bad_langs, ~ok_flags);
  warning ("\"-%s\" is valid for %s but not for %s",
	   cl_options[opt_index].opt_text, ok_langs, bad_langs);
}
