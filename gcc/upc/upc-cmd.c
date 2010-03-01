/* upc - The UPC compiler driver program.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Written by Gary Funck <gary@intrepid.com>
   and Nenad Vukicevic <nenad@intrepid.com>.

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

#include "config.h"
#include "system.h"
#include "coretypes.h"
#define USED_FOR_TARGET 1	/* disable inclusion of code gen defs. */
#include "tm.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/stat.h> 
#include <sys/param.h>
#include <sys/types.h>

/* The UPC driver program invokes the GCC compiler, passing along
   the swtiches on the command line. If the user does not supply
   switches (such -c, -S, or -E) that would disable linking,
   then add the additional link switches that are required to
   link a UPC program. */

#ifndef COMPILER
 #error "-DCOMPILER must be supplied when compiling upc.c"
#endif

#define MULTI_DIR_SWITCH "-print-multi-directory"
#define FIND_LIBUPC_SWITCH "-print-file-name=libupc.a"

#ifndef UPC_LINKER_SCRIPT
# define UPC_LINKER_SCRIPT "gccupc.ld"
#endif

#define DEFAULT_SWITCH_TAKES_ARG(CHAR) \
  ((CHAR) == 'D' || (CHAR) == 'U' || (CHAR) == 'o' \
   || (CHAR) == 'e' || (CHAR) == 'T' || (CHAR) == 'u' \
   || (CHAR) == 'I' || (CHAR) == 'm' || (CHAR) == 'x' \
   || (CHAR) == 'L' || (CHAR) == 'A' || (CHAR) == 'V' \
   || (CHAR) == 'B' || (CHAR) == 'b')

#ifndef SWITCH_TAKES_ARG
#define SWITCH_TAKES_ARG DEFAULT_SWITCH_TAKES_ARG
#endif

/* This defines which multi-letter switches take arguments.  */
#define DEFAULT_WORD_SWITCH_TAKES_ARG(STR)		\
 (!strcmp (STR, "Tdata") || !strcmp (STR, "Ttext")	\
  || !strcmp (STR, "Tbss") || !strcmp (STR, "include")	\
  || !strcmp (STR, "imacros") || !strcmp (STR, "aux-info") \
  || !strcmp (STR, "idirafter") || !strcmp (STR, "iprefix") \
  || !strcmp (STR, "iwithprefix") || !strcmp (STR, "iwithprefixbefore") \
  || !strcmp (STR, "isystem") || !strcmp (STR, "specs") \
  || !strcmp (STR, "Xlinker"))

#ifndef WORD_SWITCH_TAKES_ARG
#define WORD_SWITCH_TAKES_ARG DEFAULT_WORD_SWITCH_TAKES_ARG
#endif

#define NO_LINK_SWITCHES(STR) \
  (!strcmp (STR, "-fsyntax-only") || !strcmp (STR, "-c") \
  || !strcmp (STR, "-M") || !strcmp (STR, "-MM") \
  || !strcmp (STR, "-E") || !strcmp (STR, "-S"))

#define INFO_ONLY_SWITCHES(STR) \
  (!strcmp (STR, "-v") || !strcmp(STR, "--verbose") \
  || !strcmp (STR, "--version") \
  || !strcmp (STR, "--help") \
  || !strncmp(STR, "-print-", 7) || !strncmp(STR, "--print-", 8))

#define LIBNUMA "-lnuma" 
#define LIBUPC "-lupc" 
#define LIBUPC_PT "-lupc_pt"

#ifndef ARG_MAX
#define ARG_MAX 4096
#endif

#ifndef GET_ENV_PATH_LIST
#define GET_ENV_PATH_LIST(VAR,NAME)	do { (VAR) = getenv (NAME); } while (0)
#endif

#define BINSUFFIX "/bin/"
#define GCCLIBSUFFIX "/lib/gcc/"

static const char *const standard_bindir_prefix = STANDARD_BINDIR_PREFIX;
static const char *const standard_exec_prefix = STANDARD_EXEC_PREFIX;

static char *progname;

static int debug;

static char *substr PARAMS((const char *s, int len));
static int match_suffix PARAMS((const char *s, const char *suffix));

#define END_ARGS ((char *) 0)

/* Concatenate a sequence of strings, returning the result.

   This function is based on the one in libiberty.  */

char *
concat (const char *first, ...)
{
  int length;
  char *newstr;
  char *end;
  const char *arg;
  va_list args;

  /* First compute the size of the result and get sufficient memory.  */

  va_start (args, first);
  arg = first;
  length = 0;

  while (arg != END_ARGS)
    {
      length += strlen (arg);
      arg = va_arg (args, const char *);
    }

  newstr = (char *) xmalloc (length + 1);
  va_end (args);

  /* Now copy the individual pieces to the result string.  */

  va_start (args, first);
  end = newstr;
  arg = first;
  while (arg != END_ARGS)
    {
      while (*arg)
	*end++ = *arg++;
      arg = va_arg (args, const char *);
    }
  *end = '\000';
  va_end (args);

  return newstr;
}

static
char *
substr (const char *s, int len)
{
  char *sub = (char *)xmalloc (len + 1);
  strncpy (sub, s, len);
  sub[len] = '\0';
  return sub;
}

static
int
match_suffix (const char *s, const char *suffix)
{
  int slen = strlen (s);
  int xlen = strlen (suffix);
  const char *start = (xlen <= slen) ? s + slen - xlen : 0;
  return start && !strncmp (start, suffix, xlen);
}

/* Escape characters that might be harmful to the shell.  */

static
char *
shell_escape (const char *s)
{
  const char *meta = "&;`'\\\"|*?~<>^()[]{}$\n\r\f\t ";
  int needs_escape, needs_quote;
  char *r;
  const char *ps;
  char *result = xstrdup (s);
  for (needs_quote = 0, needs_escape = 0, ps = s; *ps; ++ps)
    {
      if (strchr (meta, (int)*ps))
         needs_quote = 1;
      needs_escape += (*ps == '\'');
    }
  if (needs_quote)
    {
      result = (char *) xmalloc (strlen(s) + 1 + 2 + needs_escape * 5);
      for (ps = s, r = result, *r++ = '\''; *ps; ++ps)
        if (*ps == '\'')
	  { memcpy(r, "'\"'\"'", 5); r += 5; }
        else
	  *r++ = *ps;
      *r++ = '\'';
      *r = '\0';
    }
  return result;
}

/* Using the arg. list we've built up so far, tack on
   the PRINT_CMD argument, and return the result.  */

static
const char *
get_print_cmd (const char *exec_args[], int n_args, const char *print_cmd)
{
  int i, len;
  char *cmd;
  char *s;
  const char *result = NULL;
  FILE *pipe;
  for (i = 0, len = strlen(print_cmd) + 1; i < n_args; ++i)
    len += strlen (shell_escape(exec_args[i])) + 1;
  cmd = (char *)xmalloc (len);
  for (i = 0, s = cmd; i < n_args; ++i)
    {
      char *p = shell_escape(exec_args[i]);
      while (*p) *s++ = *p++;
      *s++ = ' ';
    }
  strcpy (s, print_cmd);
  pipe = popen (cmd, "r");
  if (pipe)
    {
      char buf[256];
      int slen;
      (void) fgets (buf, sizeof (buf), pipe);
      slen = strlen (buf);
      if (buf[slen-1] == '\n') buf[slen-1] = '\0';
      (void) pclose (pipe);
      result = (const char *) xstrdup (buf);
    }
  return result;
}

/* Test whether the file named by FILENAME exists.  */
static
int
file_exists (const char *filename)
{
  struct stat statbuf;
  return stat(filename, &statbuf) == 0;
}

/* Return the path of the library directory,
   where libupc can be found.  LIB_PATH will be defined
   when the development version of the upc command is
   being built; use that path, and add the multilib
   suffix if required.  Otherwise, for the installed
   upc command, use -print-file-name to find the "libupc.a"
   library file, and return the containing directory.  */

static
const char *
get_libupc_path (const char *exec_args[], int n_args)
{
  const char *libupc_path = NULL;
#ifdef LIB_PATH
  {
    const char *lib_suffix;
    libupc_path = LIB_PATH;
    lib_suffix = get_print_cmd (exec_args, n_args, MULTI_DIR_SWITCH);
    if (debug)
      fprintf (stderr, "lib suffix = %s\n",
               lib_suffix ? lib_suffix : "<none>");
    if (lib_suffix && *lib_suffix && (strcmp(lib_suffix, ".") != 0))
      libupc_path = concat (libupc_path, "/", lib_suffix, END_ARGS);
    libupc_path = concat (libupc_path, "/libupc", END_ARGS);
  }
#else
  {
    const char *libupc_archive;
    libupc_archive = get_print_cmd (exec_args, n_args, FIND_LIBUPC_SWITCH);
    if (debug)
      fprintf (stderr, "libupc.a path = %s\n",
               libupc_archive ? libupc_archive : "<none>");
    if (libupc_archive[0] == '/')
      {
        const char *s, *last_slash;
	char *path;
        size_t slen;
	for (s = libupc_archive; *s; ++s)
	  if (*s == '/') last_slash = s;
        slen = (last_slash - libupc_archive);
	path = (char *) xmalloc (slen + 1);
	memcpy (path, libupc_archive, slen);
	path[slen] = '\0';
	libupc_path = (const char *)path;
      }
  }
#endif
  if (debug)
    fprintf (stderr, "lib path = %s\n", libupc_path ? libupc_path : "<none>");
  return libupc_path;
}

#ifdef HAVE_UPC_LINK_SCRIPT
/* Search for the UPC linker script, first in the current
   directory, and then in LIB_DIR.  */
static
const char *
find_ld_script(const char *lib_dir)
{
  char *ld_script;
  ld_script = concat ("./", UPC_LINKER_SCRIPT, END_ARGS);
  if (file_exists (ld_script))
    return ld_script;
  ld_script = concat (lib_dir, "/", UPC_LINKER_SCRIPT, END_ARGS);
  if (file_exists (ld_script))
    return ld_script;
  return NULL;
}
#endif

int
main (int argc, char *argv[])
{
  int i, nargs;
  int info_only = 1;
  int invoke_linker = 1;
  int explicit_linker_script = 0;
  int no_start_files = 0;
  int no_default_libs = 0;
  int no_std_inc = 0;
  int is_x_upc_in_effect = 0;
  int flag_upc_pthreads = 0;
  const int is_dev_compiler = !strcmp (COMPILER, "xgcc");
  const char *cp;
  const char *compiler = 0;
  const char *compiler_dir = 0;
  const char *bin_dir = 0;
  const char *lib_dir = 0;
  const char *inc_dir = 0;
  const char *upc_exec_prefix = 0;
  char *all_exec_args[ARG_MAX];
  const char **exec_args = (const char **)all_exec_args;

#ifdef DEBUG
  debug = 1;
#endif 

  /* Parse command line early for instances of -debug.  This allows
     the debug flag to be set before functions like find_a_file()
     are called.  */
  for (i = 1; i < argc; ++i)
    if (!strcmp (argv[i], "-debug"))
      debug = 1;

  /* extract the program's name from the command line. */
  for (cp = argv[0] + strlen(argv[0]) - 1;
       cp != argv [0] && *cp != '/'; --cp) /* loop */;
  progname = (char *) xmalloc (strlen (cp + 1) + 1);
  strcpy (progname, cp + 1);


#ifdef COMPILER_DIR
  compiler_dir = concat (COMPILER_DIR, END_ARGS);
#endif
#ifdef BIN_PATH
  bin_dir = BIN_PATH;
#endif
#ifdef INC_PATH
  inc_dir = INC_PATH;
#endif
  
  /* Check to see if any switches are asserted that inhibit linking
     and record the presence of other switches that may require
     special handling. */
  for (i = 1; i < argc; ++i)
    {
      char *arg = argv[i];
      if (arg[0] == '-')
	{
          /* skip upc's '-debug' switch */
	  if (!strcmp(arg, "-debug"))
	    continue;
	  else if (!strcmp(arg, "-nostartfiles"))
	    { 
	       no_start_files = 1;
	    }
	  else if (!strcmp(arg, "-nodefaultlibs"))
	    {
	       no_default_libs = 1;
	    }
	  else if (!strcmp(arg, "-nostdinc"))
	    { 
	       no_std_inc = 1;
	    }
	  else if (!strcmp(arg, "-nostdlib"))
	    { 
	       no_start_files = 1;
	       no_default_libs = 1;
	    }
	  else if (!strcmp(arg, "-fupc-pthreads-model-tls"))
	    {
	       flag_upc_pthreads = 1;
	    }
	  else if (!strcmp(arg, "-Xlinker") && (i < (argc - 1))
	            && !strncmp (argv[i+1], "-T", 2))
	    {
	       explicit_linker_script = 1;
	    }
	  invoke_linker = invoke_linker && !NO_LINK_SWITCHES (arg);
	  info_only = info_only && INFO_ONLY_SWITCHES (arg);
	  if (((arg[2] == '\0') && SWITCH_TAKES_ARG (arg[1]))
	      || WORD_SWITCH_TAKES_ARG (&arg[1])
	      || ((arg[1] == '-') && WORD_SWITCH_TAKES_ARG (&arg[2])))
	    /* skip the following arg */
	    ++i;
	}
      else
	/* an arg. that is not a switch implies that we'll do something. */
	info_only = FALSE;
    }
  invoke_linker = invoke_linker && !info_only;
  nargs = 0;
  /* The COMPILER preprocessor variable is passed on the command
     line to the C compiler when 'upc' is built. It usually has
     the form "xgcc" for builds in the development directory,
     and "gcc" for installed upc command. The COMPILER_DIR
     directory gives the location of where the gcc (or xgcc)
     binary lives, usually with a "/" appended to the end, so
     that the result can be passed directly to the "gcc" command,
     yield an invocation of the form:
       <full_pathname_of_gcc_or_xgcc> -B<compiler_dir>/
     
     If the UPC_EXEC_PREFIX env. variable is set, this value overrides
     the compiled-in COMPILER_DIR setting. */

  GET_ENV_PATH_LIST (upc_exec_prefix, "UPC_EXEC_PREFIX");
  if (!(compiler_dir || upc_exec_prefix))
    {
      upc_exec_prefix = make_relative_prefix (argv[0], standard_bindir_prefix, standard_exec_prefix);
    }
  if (upc_exec_prefix && strcmp(upc_exec_prefix, standard_exec_prefix) != 0)
    {
      int len = strlen (upc_exec_prefix);

      if (debug)
        {
	  fprintf (stderr, "using UPC_EXEC_PREFIX=%s\n", upc_exec_prefix);
	}
      if (match_suffix (upc_exec_prefix, GCCLIBSUFFIX))
        {
	  bin_dir = concat (substr (upc_exec_prefix,
			            len - (sizeof(GCCLIBSUFFIX) - 1)),
			    "/bin", END_ARGS);
	  compiler_dir = upc_exec_prefix;
	}
      else if (match_suffix(upc_exec_prefix, BINSUFFIX))
        {
	  bin_dir = substr (upc_exec_prefix, len - 1);
          compiler_dir = concat (substr (upc_exec_prefix,
					 len - (sizeof(BINSUFFIX) - 1)), 
				 GCCLIBSUFFIX, END_ARGS);
	}
      else
	{
	  bin_dir = concat (upc_exec_prefix, "bin", END_ARGS);
	  compiler_dir = concat (upc_exec_prefix, "lib/gcc-lib/", END_ARGS);
	}

      inc_dir = concat (compiler_dir,
			DEFAULT_TARGET_MACHINE, "/",
			DEFAULT_TARGET_VERSION,
			"/include", END_ARGS);
    }

  compiler = concat (bin_dir, "/", COMPILER, END_ARGS);
  exec_args[nargs++] = compiler;

  if (compiler_dir)
    {
      exec_args[nargs++] = xstrdup ("-B");
      exec_args[nargs++] = compiler_dir;
    }

  if (!info_only)
    {
      if (inc_dir && !no_std_inc)
	{
          /* Copy in the -isystem <path> argument */
	  exec_args[nargs++] = xstrdup ("-isystem");
	  exec_args[nargs++] = inc_dir;
	}
    }

  /* Copy in the arguments as passed to 'upc' */
  for (i = 1, is_x_upc_in_effect = 0; i < argc; ++i)
    {
      int is_c_file = match_suffix (argv[i], ".c");
      int num;
      /* skip upc's '-debug' switch */
      if (!strcmp(argv[i], "-debug"))
        continue;
      if (!strcmp(argv[i], "-n") && ((i + 1) < argc))
        {
	  /* rewrite "-n <num>" into "-fupc-threads-<num>" */
          exec_args[nargs++] = concat ("-fupc-threads-", argv[++i], END_ARGS);
	}
      else if (!strncmp(argv[i], "-n", 2)
               && (sscanf (argv[i] + 2, "%d", &num) == 1))
        {
	  /* rewrite "-n<num>" into "-fupc-threads-<num>" */
          exec_args[nargs++] = concat ("-fupc-threads-", argv[i] + 2, END_ARGS);
	}
      else if (!strcmp(argv[i], "-inst") || !strcmp(argv[i], "--inst"))
        {
	  /* rewrite "-inst" or "--inst" into "-fupc-instrument" */
          exec_args[nargs++] = "-fupc-instrument";
	}
      else if (!strcmp(argv[i], "-inst-functions") || !strcmp(argv[i], "--inst-functions"))
        {
	  /* rewrite "-inst-functions" or "--inst-functions"
	     into "-fupc-instrument-functions" */
          exec_args[nargs++] = "-fupc-instrument-functions";
	}
      else if (is_c_file)
        {
	  /* Assume that .c files are in fact UPC source files */
	  if (!is_x_upc_in_effect)
	    {
	      is_x_upc_in_effect = 1;
	      exec_args[nargs++] = "-x";
	      exec_args[nargs++] = "upc";
	    }
          exec_args[nargs++] = argv[i];
        }
      else
        {
	  if (is_x_upc_in_effect)
	    {
	      is_x_upc_in_effect = 0;
              exec_args[nargs++] = "-x";
              exec_args[nargs++] = "none";
	    }
	  exec_args[nargs++] = argv[i];
        }
    }

  if (!info_only)
    {
      lib_dir = get_libupc_path (exec_args, nargs);
      if (!lib_dir)
	{
	  fprintf (stderr, "Cannot find UPC library directory.\n");
	  exit (2);
	}
      if (!no_std_inc)
        {
	  /* Place libdir first so that we can find gcc-upc-lib.h. */
	  exec_args[nargs++] = xstrdup ("-isystem");
	  exec_args[nargs++] = lib_dir;
        }
    }

  if (invoke_linker)
    {
#ifdef HAVE_UPC_LINK_SCRIPT
      if (!explicit_linker_script)
        {
	  /* Look for special linker script.  If found, add it
	     to the argument list.  */
	  const char *ld_script = find_ld_script (lib_dir);
	  if (ld_script)
	    {
	      exec_args[nargs++] = "-Xlinker";
	      exec_args[nargs++] = concat("-T", ld_script, END_ARGS);
	    }
        }
#endif
#ifdef UPC_LINKER_SWITCHES
      /* add additional linker switches as required */
      cp = UPC_LINKER_SWITCHES;
      while (*cp)
	{
	  char *ap;
	  char arg[1024];
	  for (ap = arg; *cp && !ISSPACE(*cp); )
	    *ap++ = *cp++;
	  *ap = '\0';
	  exec_args[nargs++] = xstrdup (arg);
	  while (*cp && ISSPACE(*cp)) ++cp;
	}
#endif
      /* always link with -lm */
      exec_args[nargs++] = "-lm";
      if (!no_default_libs && lib_dir)
	{
	  const char *link_lib_dir = lib_dir;
	  /* If we're building the development version of the upc
	     driver ("xupc"), then we need to add the .libs suffix
	     because that's where libtool hides libupc.a */
	  if (is_dev_compiler)
	    link_lib_dir = concat (link_lib_dir, "/.libs", END_ARGS);
	  /* Add the link library path where libupc.a is located.  */
          exec_args[nargs++] = concat (xstrdup ("-L"), link_lib_dir, END_ARGS);
        }
      if (!flag_upc_pthreads)
	{
	  exec_args[nargs++] = LIBUPC;
	}
      else
	{
	  exec_args[nargs++] = LIBUPC_PT;
	  exec_args[nargs++] = "-lpthread";
	}
#ifdef HAVE_UPC_NUMA_SUPPORT
      exec_args[nargs++] = LIBNUMA;
#endif
    }

  if (debug)
    {
      fprintf(stderr, "upc exec args: ");
      for (i = 0; i < nargs; ++i)
	{
	    if (i != 0) fprintf(stderr, " ");
	    fprintf(stderr, "%s", exec_args[i]);
	}
      fprintf(stderr, "\n");
    }
  exec_args[nargs++] = 0;

  if (execv(exec_args[0], all_exec_args) < 0)
    {
      perror (exec_args[0]);
      exit (255);
    }
  /* no return */
  exit (255);
}
