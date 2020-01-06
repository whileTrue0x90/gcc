#  Copyright (C) 2003-2020 Free Software Foundation, Inc.
#  Contributed by Kelley Cook, June 2004.
#  Original code from Neil Booth, May 2003.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3, or (at your option) any
# later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; see the file COPYING3.  If not see
# <http://www.gnu.org/licenses/>.

# This Awk script reads in the option records generated from 
# opt-gather.awk, combines the flags of duplicate options and generates a
# C header file.
#
# This program uses functions from opt-functions.awk and code from
# opt-read.awk.
# Usage: awk -f opt-functions.awk -f opt-read.awk -f opth-gen.awk \
#            < inputfile > options.h

# Dump out an enumeration into a .h file.
# Combine the flags of duplicate options.
END {
print "/* This file is auto-generated by opth-gen.awk.  */"
print ""
print "#ifndef OPTIONS_H"
print "#define OPTIONS_H"
print ""
print "#include \"flag-types.h\""
print ""

if (n_extra_h_includes > 0) {
	for (i = 0; i < n_extra_h_includes; i++) {
		print "#include " quote extra_h_includes[i] quote
	}
	print ""
}

print "#if !defined(IN_LIBGCC2) && !defined(IN_TARGET_LIBS) && !defined(IN_RTS)"
print "#ifndef GENERATOR_FILE"
print "#if !defined(IN_LIBGCC2) && !defined(IN_TARGET_LIBS)"
print "struct GTY(()) gcc_options"
print "#else"
print "struct gcc_options"
print "#endif"
print "{"
print "#endif"

for (i = 0; i < n_extra_vars; i++) {
	var = extra_vars[i]
	sub(" *=.*", "", var)
	orig_var = var
	name = var
	type = var
	type_after = var
	sub("^.*[ *]", "", name)
	sub("\\[.*\\]$", "", name)
	sub("\\[.*\\]$", "", type)
	sub(" *" name "$", "", type)
	sub("^.*" name, "", type_after)
	var_seen[name] = 1
	print "#ifdef GENERATOR_FILE"
	print "extern " orig_var ";"
	print "#else"
	print "  " type " x_" name type_after ";"
	print "#define " name " global_options.x_" name
	print "#endif"
}

for (i = 0; i < n_opts; i++) {
	if (flag_set_p("Save", flags[i]))
		have_save = 1;

	name = var_name(flags[i]);
	if (name == "")
		continue;

	if (name in var_seen)
		continue;

	var_seen[name] = 1;
	print "#ifdef GENERATOR_FILE"
	print "extern " var_type(flags[i]) name ";"
	print "#else"
	print "  " var_type(flags[i]) "x_" name ";"
	print "#define " name " global_options.x_" name
	print "#endif"
}
for (i = 0; i < n_opts; i++) {
	name = static_var(opts[i], flags[i]);
	if (name != "") {
		print "#ifndef GENERATOR_FILE"
		print "  " var_type(flags[i]) "x_" name ";"
		print "#define x_" name " do_not_use"
		print "#endif"
	}
}
for (i = 0; i < n_opts; i++) {
	if (flag_set_p("SetByCombined", flags[i])) {
		print "#ifndef GENERATOR_FILE"
		print "  bool frontend_set_" var_name(flags[i]) ";"
		print "#endif"
	}
}
print "#ifndef GENERATOR_FILE"
print "};"
print "extern struct gcc_options global_options;"
print "extern const struct gcc_options global_options_init;"
print "extern struct gcc_options global_options_set;"
print "#define target_flags_explicit global_options_set.x_target_flags"
print "#endif"
print "#endif"
print ""

# All of the optimization switches gathered together so they can be saved and restored.
# This will allow attribute((cold)) to turn on space optimization.

# Change the type of normal switches from int to unsigned char to save space.
# Also, order the structure so that pointer fields occur first, then int
# fields, and then char fields to provide the best packing.

print "#if !defined(IN_LIBGCC2) && !defined(IN_TARGET_LIBS) && !defined(IN_RTS)"
print ""
print "/* Structure to save/restore optimization and target specific options.  */";
print "struct GTY(()) cl_optimization";
print "{";

n_opt_char = 4;
n_opt_short = 0;
n_opt_int = 0;
n_opt_enum = 0;
n_opt_other = 0;
var_opt_char[0] = "unsigned char x_optimize";
var_opt_char[1] = "unsigned char x_optimize_size";
var_opt_char[2] = "unsigned char x_optimize_debug";
var_opt_char[3] = "unsigned char x_optimize_fast";

for (i = 0; i < n_opts; i++) {
	if (flag_set_p("(Optimization|PerFunction)", flags[i])) {
		name = var_name(flags[i])
		if(name == "")
			continue;

		if(name in var_opt_seen)
			continue;

		var_opt_seen[name]++;
		otype = var_type_struct(flags[i]);
		if (otype ~ "^((un)?signed +)?int *$")
			var_opt_int[n_opt_int++] = otype "x_" name;

		else if (otype ~ "^((un)?signed +)?short *$")
			var_opt_short[n_opt_short++] = otype "x_" name;

		else if (otype ~ "^((un)?signed +)?char *$")
			var_opt_char[n_opt_char++] = otype "x_" name;

		else if (otype ~ ("^enum +[_" alnum "]+ *$"))
			var_opt_enum[n_opt_enum++] = otype "x_" name;

		else
			var_opt_other[n_opt_other++] = otype "x_" name;
	}
}

for (i = 0; i < n_opt_other; i++) {
	print "  " var_opt_other[i] ";";
}

for (i = 0; i < n_opt_int; i++) {
	print "  " var_opt_int[i] ";";
}

for (i = 0; i < n_opt_enum; i++) {
	print "  " var_opt_enum[i] ";";
}

for (i = 0; i < n_opt_short; i++) {
	print "  " var_opt_short[i] ";";
}

for (i = 0; i < n_opt_char; i++) {
	print "  " var_opt_char[i] ";";
}

print "};";
print "";

# Target and optimization save/restore/print functions.
print "/* Structure to save/restore selected target specific options.  */";
print "struct GTY(()) cl_target_option";
print "{";

n_target_char = 0;
n_target_short = 0;
n_target_int = 0;
n_target_enum = 0;
n_target_other = 0;

for (i = 0; i < n_target_save; i++) {
	if (target_save_decl[i] ~ "^((un)?signed +)?int +[_" alnum "]+$")
		var_target_int[n_target_int++] = target_save_decl[i];

	else if (target_save_decl[i] ~ "^((un)?signed +)?short +[_" alnum "]+$")
		var_target_short[n_target_short++] = target_save_decl[i];

	else if (target_save_decl[i] ~ "^((un)?signed +)?char +[_ " alnum "]+$")
		var_target_char[n_target_char++] = target_save_decl[i];

	else if (target_save_decl[i] ~ ("^enum +[_" alnum "]+ +[_" alnum "]+$")) {
		var_target_enum[n_target_enum++] = target_save_decl[i];
	}
	else
		var_target_other[n_target_other++] = target_save_decl[i];
}

if (have_save) {
	for (i = 0; i < n_opts; i++) {
		if (flag_set_p("Save", flags[i])) {
			name = var_name(flags[i])
			if(name == "")
				name = "target_flags";

			if(name in var_save_seen)
				continue;

			var_save_seen[name]++;
			otype = var_type_struct(flags[i])
			if (otype ~ "^((un)?signed +)?int *$")
				var_target_int[n_target_int++] = otype "x_" name;

			else if (otype ~ "^((un)?signed +)?short *$")
				var_target_short[n_target_short++] = otype "x_" name;

			else if (otype ~ "^((un)?signed +)?char *$")
				var_target_char[n_target_char++] = otype "x_" name;

			else if (otype ~ ("^enum +[_" alnum "]+ +[_" alnum "]+"))
				var_target_enum[n_target_enum++] = otype "x_" name;

			else
				var_target_other[n_target_other++] = otype "x_" name;
		}
	}
} else {
	var_target_int[n_target_int++] = "int x_target_flags";
}

for (i = 0; i < n_target_other; i++) {
	print "  " var_target_other[i] ";";
}

for (i = 0; i < n_target_enum; i++) {
	print "  " var_target_enum[i] ";";
}

for (i = 0; i < n_target_int; i++) {
	print "  " var_target_int[i] ";";
}

for (i = 0; i < n_target_short; i++) {
	print "  " var_target_short[i] ";";
}

for (i = 0; i < n_target_char; i++) {
	print "  " var_target_char[i] ";";
}

print "};";
print "";
print "";
print "/* Save optimization variables into a structure.  */"
print "extern void cl_optimization_save (struct cl_optimization *, struct gcc_options *);";
print "";
print "/* Restore optimization variables from a structure.  */";
print "extern void cl_optimization_restore (struct gcc_options *, struct cl_optimization *);";
print "";
print "/* Print optimization variables from a structure.  */";
print "extern void cl_optimization_print (FILE *, int, struct cl_optimization *);";
print "";
print "/* Print different optimization variables from structures provided as arguments.  */";
print "extern void cl_optimization_print_diff (FILE *, int, cl_optimization *ptr1, cl_optimization *ptr2);";
print "";
print "/* Save selected option variables into a structure.  */"
print "extern void cl_target_option_save (struct cl_target_option *, struct gcc_options *);";
print "";
print "/* Restore selected option variables from a structure.  */"
print "extern void cl_target_option_restore (struct gcc_options *, struct cl_target_option *);";
print "";
print "/* Print target option variables from a structure.  */";
print "extern void cl_target_option_print (FILE *, int, struct cl_target_option *);";
print "";
print "/* Print different target option variables from structures provided as arguments.  */";
print "extern void cl_target_option_print_diff (FILE *, int, cl_target_option *ptr1, cl_target_option *ptr2);";
print "";
print "/* Compare two target option variables from a structure.  */";
print "extern bool cl_target_option_eq (const struct cl_target_option *, const struct cl_target_option *);";
print "";
print "/* Free heap memory used by target option variables.  */";
print "extern void cl_target_option_free (struct cl_target_option *);";
print "";
print "/* Hash option variables from a structure.  */";
print "extern hashval_t cl_target_option_hash (const struct cl_target_option *);";
print "";
print "/* Hash optimization from a structure.  */";
print "extern hashval_t cl_optimization_hash (const struct cl_optimization *);";
print "";
print "/* Compare two optimization options.  */";
print "extern bool cl_optimization_option_eq (cl_optimization const *ptr1, cl_optimization const *ptr2);"
print "";
print "/* Free heap memory used by optimization options.  */";
print "extern void cl_optimization_option_free (cl_optimization *ptr1);"
print "";
print "/* Generator files may not have access to location_t, and don't need these.  */"
print "#if defined(UNKNOWN_LOCATION)"
print "bool                                                                  "
print "common_handle_option_auto (struct gcc_options *opts,                  "
print "                           struct gcc_options *opts_set,              "
print "                           const struct cl_decoded_option *decoded,   "
print "                           unsigned int lang_mask, int kind,          "
print "                           location_t loc,                            "
print "                           const struct cl_option_handlers *handlers, "
print "                           diagnostic_context *dc);                   "
for (i = 0; i < n_langs; i++) {
    lang_name = lang_sanitized_name(langs[i]);
    print "bool"
    print lang_name "_handle_option_auto (struct gcc_options *opts,"
    print "                           struct gcc_options *opts_set,"
    print "                           size_t scode, const char *arg,"
    print "                           HOST_WIDE_INT value,"
    print "                           unsigned int lang_mask, int kind,"
    print "                           location_t loc,"
    print "                           const struct cl_option_handlers *handlers,"
    print "                           diagnostic_context *dc);"
}
print "void cpp_handle_option_auto (const struct gcc_options * opts, size_t scode,"
print "                             struct cpp_options * cpp_opts);"
print "void init_global_opts_from_cpp(struct gcc_options * opts,      "
print "                               const struct cpp_options * cpp_opts);"    
print "#endif";
print "#endif";
print "";

for (i = 0; i < n_opts; i++) {
	name = opt_args("Mask", flags[i])
	if (name == "") {
		opt = opt_args("InverseMask", flags[i])
		if (opt ~ ",")
			name = nth_arg(0, opt)
		else
			name = opt
	}
	if (name != "" && mask_bits[name] == 0) {
		mask_bits[name] = 1
		vname = var_name(flags[i])
		mask = "MASK_"
		mask_1 = "1U"
		if (vname != "") {
			mask = "OPTION_MASK_"
			if (host_wide_int[vname] == "yes")
				mask_1 = "HOST_WIDE_INT_1U"
		} else
			extra_mask_bits[name] = 1
		print "#define " mask name " (" mask_1 " << " masknum[vname]++ ")"
	}
}
for (i = 0; i < n_extra_masks; i++) {
	if (extra_mask_bits[extra_masks[i]] == 0)
		print "#define MASK_" extra_masks[i] " (1U << " masknum[""]++ ")"
}

for (var in masknum) {
	if (var != "" && host_wide_int[var] == "yes") {
		print "#if defined(HOST_BITS_PER_WIDE_INT) && " masknum[var] " > HOST_BITS_PER_WIDE_INT"
		print "#error too many masks for " var
		print "#endif"
	}
	else if (masknum[var] > 32) {
		if (var == "")
			print "#error too many target masks"
		else
			print "#error too many masks for " var
	}
}
print ""

for (i = 0; i < n_opts; i++) {
	name = opt_args("Mask", flags[i])
	if (name == "") {
		opt = opt_args("InverseMask", flags[i])
		if (opt ~ ",")
			name = nth_arg(0, opt)
		else
			name = opt
	}
	if (name != "" && mask_macros[name] == 0) {
		mask_macros[name] = 1
		vname = var_name(flags[i])
		mask = "OPTION_MASK_"
		if (vname == "") {
			vname = "target_flags"
			mask = "MASK_"
			extra_mask_macros[name] = 1
		}
		print "#define TARGET_" name \
		      " ((" vname " & " mask name ") != 0)"
		print "#define TARGET_" name "_P(" vname ")" \
		      " (((" vname ") & " mask name ") != 0)"
	}
}
for (i = 0; i < n_extra_masks; i++) {
	if (extra_mask_macros[extra_masks[i]] == 0)
		print "#define TARGET_" extra_masks[i] \
		      " ((target_flags & MASK_" extra_masks[i] ") != 0)"
}
print ""

for (i = 0; i < n_opts; i++) {
	opt = opt_args("InverseMask", flags[i])
	if (opt ~ ",") {
		vname = var_name(flags[i])
		mask = "OPTION_MASK_"
		if (vname == "") {
			vname = "target_flags"
			mask = "MASK_"
		}
		print "#define TARGET_" nth_arg(1, opt) \
		      " ((" vname " & " mask nth_arg(0, opt) ") == 0)"
	}
}
print ""

for (i = 0; i < n_langs; i++) {
        macros[i] = "CL_" lang_sanitized_name(langs[i])
	s = substr("            ", length (macros[i]))
	print "#define " macros[i] s " (1U << " i ")"
    }
print "#define CL_LANG_ALL   ((1U << " n_langs ") - 1)"

print ""
print "enum opt_code"
print "{"
	
for (i = 0; i < n_opts; i++)
	back_chain[i] = "N_OPTS";

enum_value = 0
for (i = 0; i < n_opts; i++) {
	# Combine the flags of identical switches.  Switches
	# appear many times if they are handled by many front
	# ends, for example.
	while( i + 1 != n_opts && opts[i] == opts[i + 1] ) {
		flags[i + 1] = flags[i] " " flags[i + 1];
		i++;
	}

	len = length (opts[i]);
	enum = opt_enum(opts[i])
	enum_string = enum " = " enum_value ","

	# Aliases do not get enumeration names.
	if ((flag_set_p("Alias.*", flags[i]) \
	     && !flag_set_p("SeparateAlias", flags[i])) \
	    || flag_set_p("Ignore", flags[i])) {
		enum_string = "/* " enum_string " */"
	}

	# If this switch takes joined arguments, back-chain all
	# subsequent switches to it for which it is a prefix.  If
	# a later switch S is a longer prefix of a switch T, T
	# will be back-chained to S in a later iteration of this
	# for() loop, which is what we want.
	if (flag_set_p("Joined.*", flags[i])) {
		for (j = i + 1; j < n_opts; j++) {
			if (substr (opts[j], 1, len) != opts[i])
				break;
			back_chain[j] = enum;
		}
	}

	s = substr("                                          ",
		   length (enum_string))

	if (help[i] == "")
		hlp = "0"
	else
		hlp = "N_(\"" help[i] "\")";

	print "  " enum_string s "/* -" opts[i] " */"
	enum_value++
}

print "  N_OPTS,"
print "  OPT_SPECIAL_unknown,"
print "  OPT_SPECIAL_ignore,"
print "  OPT_SPECIAL_warn_removed,"
print "  OPT_SPECIAL_program_name,"
print "  OPT_SPECIAL_input_file"
print "};"
print ""
print "#ifdef GCC_C_COMMON_C"
print "/* Mapping from cpp message reasons to the options that enable them.  */"
print "#include <cpplib.h>"
print "struct cpp_reason_option_codes_t"
print "{"
print "  /* cpplib message reason.  */"
print "  const enum cpp_warning_reason reason;"
print "  /* gcc option that controls this message.  */"
print "  const int option_code;"
print "};"
print ""
print "static const struct cpp_reason_option_codes_t cpp_reason_option_codes[] = {"
for (i = 0; i < n_opts; i++) {
    # With identical flags, pick only the last one.  The
    # earlier loop ensured that it has all flags merged,
    # and a nonempty help text if one of the texts was nonempty.
    while( i + 1 != n_opts && opts[i] == opts[i + 1] ) {
        i++;
    }
    cpp_reason = nth_arg(0, opt_args("CppReason", flags[i]));
    if (cpp_reason != "") {
        cpp_reason = cpp_reason ",";
        printf("  {%-40s %s},\n", cpp_reason, opt_enum(opts[i]))
    }
}
printf("  {%-40s 0},\n", "CPP_W_NONE,")
print "};"
print "#endif"
print ""
print "#endif /* OPTIONS_H */"
}
