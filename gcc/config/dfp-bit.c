/* This is a software decimal floating point library.
   Copyright (C) 2005 Free Software Foundation, Inc.

This file is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2, or (at your option) any
later version.

In addition to the permissions in the GNU General Public License, the
Free Software Foundation gives you unlimited permission to link the
compiled version of this file with other programs, and to distribute
those programs without any restriction coming from the use of this
file.  (The General Public License restrictions do apply in other
respects; for example, they cover modification of the file, and
distribution when not linked into another program.)

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

/* As a special exception, if you link this library with other files,
   some of which are compiled with GCC, to produce an executable,
   this library does not by itself cause the resulting executable
   to be covered by the GNU General Public License.
   This exception does not however invalidate any other reasons why
   the executable file might be covered by the GNU General Public License.  */

/* This implements IEEE 754R decimal floating point arithmetic, but
   does not provide a mechanism for setting the rounding mode, or for
   generating or handling exceptions.

   Contributed by Ben Elliston  <bje@au.ibm.com>.  */

/* The intended way to use this file is to make two copies, add `#define '
   to one copy, then compile both copies and add them to libgcc.a.  */

#include "tconfig.h"
#include "coretypes.h"
#include "tm.h"
#include "config/dfp-bit.h"

typedef decNumber* (*dfp_func_ptr)
     (decNumber *, decNumber *, decNumber *, decContext *);

static inline DFP_TYPE
dfp_op (dfp_func_ptr op, DFP_TYPE arg_a, DFP_TYPE arg_b)
{
  decContext context;
  decNumber a, b, result;
  DFP_TYPE encoded_result;

  decContextDefault (&context, DEC_INIT_BASE);
  context.digits = DECNUMDIGITS;
  TO_INTERNAL (&arg_a, &a);
  TO_INTERNAL (&arg_b, &b);

  /* Perform the operation.  */
  op (&result, &a, &b, &context);

  TO_ENCODED (&encoded_result, &result, &context);
  return encoded_result;
}

#if defined(L_mul_sd) | defined(L_mul_dd) | defined(L_mul_td)
DFP_TYPE
DFP_MULTIPLY (DFP_TYPE arg_a, DFP_TYPE arg_b)
{
  return dfp_op (decNumberMultiply, arg_a, arg_b);
}
#endif /* defined(L_mul_sd) | defined(L_mul_dd) | defined(L_mul_td) */
