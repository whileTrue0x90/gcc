#! /bin/sh

# Generate gcc's various configuration headers: config.h, tconfig.h,
# the misnamed hconfig.h, tm.h, and tm_p.h.
# $1 is the file to generate.  DEFINES, HEADERS, and possibly
# TARGET_CPU_DEFAULT are expected to be set in the environment.

if [ -z "$1" ]; then
    echo "Usage: DEFINES='list' HEADERS='list' \\" >&2
    echo "  [TARGET_CPU_DEFAULT='default'] mkconfig.sh FILE" >&2
    exit 1
fi

output=$1
rm -f ${output}T

# Define TARGET_CPU_DEFAULT if the system wants one.
# This substitutes for lots of *.h files.
if [ "$TARGET_CPU_DEFAULT" != "" ]; then
    echo "#define TARGET_CPU_DEFAULT ($TARGET_CPU_DEFAULT)" >> ${output}T
fi

# Provide defines for other macros set in config.gcc for this file.
for def in $DEFINES; do
    echo "#ifndef $def" | sed 's/=.*//' >> ${output}T
    echo "# define $def" | sed 's/=/ /' >> ${output}T
    echo "#endif" >> ${output}T
done

# The first entry in HEADERS may be auto-host.h or auto-build.h;
# it wants to be included even when not -DIN_GCC.
if [ -n "$HEADERS" ]; then
    set $HEADERS
    case "$1" in auto-* )
	echo "#include \"$1\"" >> ${output}T
	shift
	;;
    esac
    if [ $# -ge 1 ]; then
	echo '#ifdef IN_GCC' >> ${output}T
	for file in "$@"; do
	    echo "# include \"$file\"" >> ${output}T
	done
	echo '#endif' >> ${output}T
    fi
fi

# If this is tconfig.h, now define USED_FOR_TARGET.  If this is tm.h,
# now include insn-constants.h and insn-flags.h only if IN_GCC is
# defined but neither GENERATOR_FILE nor USED_FOR_TARGET is defined.
# (Much of this is temporary.)

case $output in
    tconfig.h )
	cat >> ${output}T <<EOF
#define USED_FOR_TARGET
EOF
    ;;
    tm.h )
        cat >> ${output}T <<EOF
#if defined IN_GCC && !defined GENERATOR_FILE && !defined USED_FOR_TARGET
# include "insn-constants.h"
# include "insn-flags.h"
#endif
EOF
    ;;
esac

# Avoid changing the actual file if possible.
if [ -f $output ] && cmp ${output}T $output >/dev/null 2>&1; then
    echo $output is unchanged >&2
    rm -f ${output}T
else
    mv -f ${output}T $output
fi

# Touch a stamp file for Make's benefit.
rm -f cs-$output
echo timestamp > cs-$output
