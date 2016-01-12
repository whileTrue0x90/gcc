/* Copyright (C) 2013-2016 Free Software Foundation, Inc.

   This file is part of GCC.

   GCC is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   GCC is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   Under Section 7 of GPL version 3, you are granted additional
   permissions described in the GCC Runtime Library Exception, version
   3.1, as published by the Free Software Foundation.

   You should have received a copy of the GNU General Public License and
   a copy of the GCC Runtime Library Exception along with this program;
   see the files COPYING3 and COPYING.RUNTIME respectively.  If not, see
   <http://www.gnu.org/licenses/>.  */

#if !defined _X86INTRIN_H_INCLUDED
# error "Never use <pcommitintrin.h> directly; include <x86intrin.h> instead."
#endif

#ifndef _PCOMMITINTRIN_H_INCLUDED
#define _PCOMMITINTRIN_H_INCLUDED

#ifndef __PCOMMIT__
#pragma GCC push_options
#pragma GCC target("pcommit")
#define __DISABLE_PCOMMIT__
#endif /* __PCOMMIT__ */

extern __inline void
__attribute__((__gnu_inline__, __always_inline__, __artificial__))
_mm_pcommit (void)
{
  __builtin_ia32_pcommit ();
}

#ifdef __DISABLE_PCOMMIT__
#undef __DISABLE_PCOMMIT__
#pragma GCC pop_options
#endif /* __DISABLE_PCOMMIT__ */

#endif /* _PCOMMITINTRIN_H_INCLUDED */
