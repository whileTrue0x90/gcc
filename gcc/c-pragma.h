/* Pragma related interfaces.
   Copyright (C) 1995, 1998, 1999, 2000, 2001, 2002, 2003
   Free Software Foundation, Inc.

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

#ifndef GCC_C_PRAGMA_H
#define GCC_C_PRAGMA_H

/* Cause the `yydebug' variable to be defined.  */
#define YYDEBUG 1
extern int yydebug;

extern struct cpp_reader* parse_in;

#ifdef HANDLE_SYSV_PRAGMA
#if ((defined (ASM_WEAKEN_LABEL) && defined (ASM_OUTPUT_WEAK_ALIAS)) \
     || defined (ASM_WEAKEN_DECL))
#define HANDLE_PRAGMA_WEAK SUPPORTS_WEAK
#endif

/* We always support #pragma pack for SYSV pragmas.  */
#ifndef HANDLE_PRAGMA_PACK
#define HANDLE_PRAGMA_PACK 1
#endif
#endif /* HANDLE_SYSV_PRAGMA */


#ifdef HANDLE_PRAGMA_PACK_PUSH_POP
/* If we are supporting #pragma pack(push... then we automatically
   support #pragma pack(<n>)  */
#define HANDLE_PRAGMA_PACK 1
#endif /* HANDLE_PRAGMA_PACK_PUSH_POP */

extern void init_pragma PARAMS ((void));

/* Front-end wrapper for pragma registration to avoid dragging
   cpplib.h in almost everywhere.  */
extern void c_register_pragma
	PARAMS ((const char *, const char *,
		 void (*) PARAMS ((struct cpp_reader *))));
extern void maybe_apply_pragma_weak PARAMS ((tree));
extern tree maybe_apply_renaming_pragma PARAMS ((tree, tree));
extern void add_to_renaming_pragma_list PARAMS ((tree, tree));

extern int c_lex PARAMS ((tree *));

#endif /* GCC_C_PRAGMA_H */
