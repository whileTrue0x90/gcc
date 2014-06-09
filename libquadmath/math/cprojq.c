/* Compute projection of complex __float128 value to Riemann sphere.
   Copyright (C) 1997, 1999, 2010 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "quadmath-imp.h"


__complex128
cprojq (__complex128 x)
{
  if (isnanq (__real__ x) && isnanq (__imag__ x))
    return x;
  else if (!finiteq (__real__ x) || !finiteq (__imag__ x))
    {
      __complex128 res;

      __real__ res = __builtin_inf ();
      __imag__ res = copysignq (0.0, __imag__ x);

      return res;
    }

  return x;
}
