/* Prototypes for ft32.c functions used in the md file & elsewhere.
   Copyright (C) 2015-2019 Free Software Foundation, Inc.

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

extern void        ft32_expand_prologue (void);
extern void        ft32_expand_epilogue (void);
extern int         ft32_initial_elimination_offset (int, int);
extern void        ft32_print_operand (FILE *, rtx, int);
extern void        ft32_print_operand_address (FILE *, rtx);
extern const char* ft32_load_immediate(rtx, int32_t i);
extern int         ft32_as_bitfield(unsigned int x);
