// Backward-compat support -*- C++ -*-

// Copyright (C) 2001 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

/*
 *
 * Copyright (c) 1994
 * Hewlett-Packard Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Hewlett-Packard Company makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 *
 * Copyright (c) 1996,1997
 * Silicon Graphics Computer Systems, Inc.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  Silicon Graphics makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 */

#ifndef _CPP_BACKWARD_FUNCTION_H
#define _CPP_BACKWARD_FUNCTION_H 1

#include "backward_warning.h"
#include <bits/c++config.h>
#include <stddef.h>
#include <bits/stl_function.h>

// Names from stl_function.h
using std::unary_function; 
using std::binary_function; 
using std::plus; 
using std::minus; 
using std::multiplies; 
using std::divides; 
using std::identity_element; 
using std::modulus; 
using std::negate; 
using std::equal_to; 
using std::not_equal_to; 
using std::greater; 
using std::less; 
using std::greater_equal; 
using std::less_equal; 
using std::logical_and; 
using std::logical_or; 
using std::logical_not; 
using std::unary_negate; 
using std::binary_negate; 
using std::not1; 
using std::not2; 
using std::binder1st; 
using std::binder2nd; 
using std::bind1st; 
using std::bind2nd; 
using std::unary_compose; 
using std::binary_compose; 
using std::compose1; 
using std::compose2; 
using std::pointer_to_unary_function; 
using std::pointer_to_binary_function; 
using std::ptr_fun; 
using std::identity; 
using std::select1st; 
using std::select2nd; 
using std::project1st; 
using std::project2nd; 
using std::constant_void_fun; 
using std::constant_unary_fun; 
using std::constant_binary_fun; 
using std::constant0; 
using std::constant1; 
using std::constant2; 
using std::subtractive_rng; 
using std::mem_fun_t; 
using std::const_mem_fun_t; 
using std::mem_fun_ref_t; 
using std::const_mem_fun_ref_t; 
using std::mem_fun1_t; 
using std::const_mem_fun1_t; 
using std::mem_fun1_ref_t; 
using std::const_mem_fun1_ref_t; 
using std::mem_fun; 
using std::mem_fun_ref; 
using std::mem_fun1; 
using std::mem_fun1_ref; 

#endif /* _CPP_BACKWARD_FUNCTION_H */

// Local Variables:
// mode:C++
// End:
