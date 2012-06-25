[+ AutoGen5 template -*- Mode: Makefile -*-
mk
+][+COMMENT use 'autogen --trace=everything melt-newbuild.def' to debug this
  See http://www.gnu.org/software/autogen/
+]
[+ (. (dne "#@#@# " "#@! ")) +]
##@@ melt-newbuild.mk is generated from melt-newbuild.tpl by 'autogen melt-newbuild.def'
#
# Makefile fragment for MELT modules and MELT translator bootstrap.
#   Copyright (C) 2010,2011,2012  Free Software Foundation
#
# This file is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
#@ [+ (. (tpl-file-line))+] generated by Autogen [+ (. autogen-version)+] using [+ (.(def-file))+]
## See http://stackoverflow.com/q/8727896/841108
## and http://lists.gnu.org/archive/html/help-make/2012-01/msg00017.html
## and http://gcc.gnu.org/ml/gcc/2012-01/msg00089.html
## and Ian Taylor's explanation http://gcc.gnu.org/ml/gcc/2012-01/msg00090.html
## Using remake http://bashdb.sourceforge.net/remake/ may help debugging this.
## Conventionally, all our timestamp files lie in the current directory.

#@ default MELT variant [+ (. (tpl-file-line))+]
melt_default_variant ?= optimized

## the following Makefile variables are expected to be set [+ (. (tpl-file-line))+] 
[+FOR meltmac IN 
	"melt_source_dir: directory containing *.melt & *.c files" 
	"melt_module_dir: directory containing *.so MELT module files" 
	"melt_make_module_makefile: our melt-module.mk Makefile  when making MELT"
	"melt_make_source_dir: directory containing the *.melt files when making MELT"
	"melt_make_module_dir: directory containing the *.so files when making MELT"
	"melt_default_modules_list: basename of the default module list"
	"melt_make_cc1: gcc -fplugin=melt or cc1-melt program within MELT branch"
	"melt_make_cc1_dependency: the make dependency for above"
	"melt_make_gencdeps: extra make dependency of generated C files -often empty-"
	"melt_move_if_change: a move if change command for MELT"
	"melt_cflags: the CFLAGS for compiling MELT generated C code"
	"melt_xtra_cflags: th CFLAGS for compiling extra applicative C code - often empty"
	"melt_default_variant: quicklybuilt | optimized | debugnoline"
+]
[+
	(define meltmac-name "?name")
	(define meltmac-comment "?comment")
	(let* ( 
	(meltmacstr (get "meltmac"))
	(colonpos (string-index meltmacstr #\: 
	))	
	(beforecolonstr (string-take meltmacstr colonpos))
	(aftercolonstr (string-drop meltmacstr (+ colonpos 1)))
	)
	(set! meltmac-name beforecolonstr)
	(set! meltmac-comment aftercolonstr)
	 ) +]
#@ [+ (. (tpl-file-line))+] name= [+(. meltmac-name)+] comment=  [+(. meltmac-comment)+]
ifndef [+(. meltmac-name)+]
$(warning unknown [+(. meltmac-name)+] ::: [+(. meltmac-comment)+])
endif

[+ENDFOR meltmac+]



## LN_S might not be defined, e.g. in MELT-Plugin-Makefile [+ (. (tpl-file-line))+]
LN_S ?= ln -sv

## GAWK is needed, the GNU awk [+ (. (tpl-file-line))+]
GAWK ?= gawk

## the md5sum utility is needed  [+ (. (tpl-file-line))+]
MD5SUM ?= md5sum

## the various arguments to MELT - avoid spaces in them! [+ (. (tpl-file-line))+]
meltarg_mode=$(if $(melt_is_plugin),-fplugin-arg-melt-mode,-fmelt-mode)
meltarg_init=$(if $(melt_is_plugin),-fplugin-arg-melt-init,-fmelt-init)
meltarg_module_path=$(if $(melt_is_plugin),-fplugin-arg-melt-module-path,-fmelt-module-path)
meltarg_source_path=$(if $(melt_is_plugin),-fplugin-arg-melt-source-path,-fmelt-source-path)
meltarg_tempdir=$(if $(melt_is_plugin),-fplugin-arg-melt-tempdir,-fmelt-tempdir)
meltarg_workdir=$(if $(melt_is_plugin),-fplugin-arg-melt-workdir,-fmelt-workdir)
meltarg_arg=$(if $(melt_is_plugin),-fplugin-arg-melt-arg,-fmelt-arg)
meltarg_bootstrapping=$(if $(melt_is_plugin),-fplugin-arg-melt-bootstrapping,-fmelt-bootstrapping)
meltarg_genworklink=$(if $(melt_is_plugin),-fplugin-arg-melt-generate-work-link,-fmelt-generate-work-link)
meltarg_makefile=$(if $(melt_is_plugin),-fplugin-arg-melt-module-makefile,-fmelt-module-makefile)
meltarg_makecmd=$(if $(melt_is_plugin),-fplugin-arg-melt-module-make-command,-fmelt-module-make-command)
meltarg_arglist=$(if $(melt_is_plugin),-fplugin-arg-melt-arglist,-fmelt-arglist)
meltarg_output=$(if $(melt_is_plugin),-fplugin-arg-melt-output,-fmelt-output)
meltarg_modulecflags=$(if $(melt_is_plugin),-fplugin-arg-melt-module-cflags,-fmelt-module-cflags)
meltarg_inhibitautobuild=$(if $(melt_is_plugin),-fplugin-arg-melt-inhibit-auto-build,-fmelt-inhibit-auto-build)

## compiler used to compile MELT generated C (or C++ compatible) code  [+ (. (tpl-file-line))+]
ifndef GCCMELT_COMPILER
$(error GCCMELT_COMPILER should be explicitly given  [+ (. (tpl-file-line))+])
endif

## compiler flags [+ (. (tpl-file-line))+]
ifndef GCCMELT_COMPILER_FLAGS
$(error GCCMELT_COMPILER_FLAGS should be explicitly given  [+ (. (tpl-file-line))+])
endif

## linker used to link the shared object modules [+ (. (tpl-file-line))+]
ifndef GCCMELT_LINKER
$(error GCCMELT_LINKER should be explicitly given  [+ (. (tpl-file-line))+])
endif

## linker flags [+ (. (tpl-file-line))+]
ifndef GCCMELT_LINKER_FLAGS
$(error GCCMELT_LINKER_FLAGS should be explicitly given  [+ (. (tpl-file-line))+])
endif

## position independent flag to compile *.c into *pic.o [+ (. (tpl-file-line))+]
GCCMELT_PIC_FLAGS ?= -fPIC

## shared object flag to link *pic.o into *.so [+ (. (tpl-file-line))+]
GCCMELT_SHARED_FLAGS ?= -shared

## The base name of the MELT translator files [+ (. (tpl-file-line))+]
MELT_TRANSLATOR_BASE= \
  [+FOR melt_translator_file " \\\n"+]  [+base+][+ENDFOR melt_translator_file+]

## the MELT translator MELT source files [+ (. (tpl-file-line))+]
MELT_TRANSLATOR_SOURCE= $(patsubst %,$(melt_make_source_dir)/%.melt,$(MELT_TRANSLATOR_BASE))

## The base name of the MELT application files [+ (. (tpl-file-line))+]
MELT_APPLICATION_BASE= \
  [+FOR melt_application_file " \\\n"+]  [+base+][+ENDFOR melt_application_file+]

## The MELT application source files [+ (. (tpl-file-line))+]
MELT_APPLICATION_SOURCE= $(patsubst %,$(melt_make_source_dir)/%.melt,$(MELT_APPLICATION_BASE))

## The cold stage 0 of the translator [+ (. (tpl-file-line))+]
[+FOR melt_translator_file +]
#@ The C files of the stage 0 are deposited [+ (. (tpl-file-line))+]
MELT_ZERO_GENERATED_[+mkvarsuf+]_C_FILES= \
                  $(realpath $(melt_make_source_dir))/generated/[+base+].c \
                  $(wildcard $(realpath $(melt_make_source_dir))/generated/[+base+]+*.c)
# The base names of stage 0 files [+ (. (tpl-file-line))+]
MELT_ZERO_GENERATED_[+mkvarsuf+]_BASE= \
                  $(basename $(notdir $(MELT_GENERATED_[+mkvarsuf+]_C_FILES)))
# for stage 0 files, we don't compute the checksum, we extract what was deposited [+ (. (tpl-file-line))+]
## avoid spaces in MELT_GENERATED_[+mkvarsuf+]_CUMULMD5 below [+ (. (tpl-file-line))+]
MELT_ZERO_GENERATED_[+mkvarsuf+]_CUMULMD5:=$(shell $(GAWK) -F\" '/extern/{next} /melt_cumulated_hexmd5/{print $$2}' $(melt_make_source_dir)/generated/[+base+]+meltdesc.c)
[+ENDFOR melt_translator_file+]

## An empty file is needed for every MELT translation [+ (. (tpl-file-line))+]
empty-file-for-melt.c:
	date +"/* empty-file-for-melt.c %c */" > $@-tmp
	mv $@-tmp $@

.PHONY: warmelt0

## the default stage0 is [+ (. (tpl-file-line))+]
MELT_STAGE_ZERO?=melt-stage0-dynamic
MELT_ZERO_FLAVOR=$(patsubst melt-stage0-%,%,$(MELT_STAGE_ZERO))

warmelt0: $(MELT_STAGE_ZERO)/$(MELT_STAGE_ZERO).stamp

[+FOR zeroflavor IN "dynamic" "quicklybuilt" +]
## stage 0 flavor [+zeroflavor+]  [+ (. (tpl-file-line))+]

melt-stage0-[+zeroflavor+]:
	test -d  melt-stage0-[+zeroflavor+]/ || mkdir  melt-stage0-[+zeroflavor+]/

#@  [+ (. (tpl-file-line))+]
melt-stage0-[+zeroflavor+]/melt-stage0-[+zeroflavor+].stamp: \
[+FOR melt_translator_file  " \\\n" 
+]  melt-stage0-[+zeroflavor+]/[+base+].$(MELT_ZERO_GENERATED_[+mkvarsuf+]_CUMULMD5).[+zeroflavor+].meltmod.so[+ENDFOR  melt_translator_file+] \
[+FOR melt_translator_file  " \\\n" 
+]  $(addprefix melt-stage0-[+zeroflavor+]/,$(notdir $(MELT_ZERO_GENERATED_[+mkvarsuf+]_C_FILES)))[+ENDFOR  melt_translator_file+] |  melt-stage0-[+zeroflavor+]


#@ [+ (. (tpl-file-line))+] symbolic links for stage 0 sources
[+FOR melt_translator_file+]
$(addprefix melt-stage0-[+zeroflavor+],$(notdir $(MELT_ZERO_GENERATED_[+mkvarsuf+]_C_FILES)) [+base+]+meltdesc.c  [+base+]+melttime.c): | melt-stage0-[+zeroflavor+]
	$(LN_S) $(realpath $(melt_make_source_dir)/generated/$(@F)) melt-stage0-[+zeroflavor+]/
#@ [+ (. (tpl-file-line))+]
[+ENDFOR melt_translator_file+]


## end stage 0 flavor [+zeroflavor+]  [+ (. (tpl-file-line))+]
[+ENDFOR zeroflavor+]
#@ [+ (. (tpl-file-line))+] eof melt-newbuild.mk
## eof melt-newbuild.mk generated from melt-newbuild.tpl & melt-newbuild.def
