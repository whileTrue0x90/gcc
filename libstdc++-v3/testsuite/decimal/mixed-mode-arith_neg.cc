// Copyright (C) 2009-2016 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// { dg-do compile }
// { dg-require-effective-target dfp }

// Test that binary arithmetic operators do not accept mixed decimal
// and generic floating-point operands.  This isn't explicity
// prohibited in ISO/IEC TR 24733 but it is prohibited in C, and in C++
// there should not be an implicit conversion from a decimal
// floating-point type to a generic floating-point type.

#include <decimal/decimal>

using namespace std::decimal;

decimal32 a32, b32, c32;
decimal64 a64, b64, c64;
decimal128 a128, b128, c128;
float f;
double d;
long double ld;
bool b1, b2, b3, b4, b5, b6;

void
bad_add (void)
{
  a32 = b32 + f;	// { dg-error "error" }
  a32 = ld + b32;	// { dg-error "error" }
  a64 = b64 + d;	// { dg-error "error" }
  a64 = ld + b64;	// { dg-error "error" }
  a128 = b128 + ld;	// { dg-error "error" }
  a128 = d + b128;	// { dg-error "error" }
}

void
bad_subtract (void)
{
  a32 = b32 - f;	// { dg-error "error" }
  a32 = ld - b32;	// { dg-error "error" }
  a64 = b64 - d;	// { dg-error "error" }
  a64 = ld - b64;	// { dg-error "error" }
  a128 = b128 - ld;	// { dg-error "error" }
  a128 = d - b128;	// { dg-error "error" }
}

void
bad_multiply (void)
{
  a32 = b32 * f;	// { dg-error "error" }
  a32 = ld * b32;	// { dg-error "error" }
  a64 = b64 * d;	// { dg-error "error" }
  a64 = ld * b64;	// { dg-error "error" }
  a128 = b128 * ld;	// { dg-error "error" }
  a128 = d * b128;	// { dg-error "error" }
}

void
bad_divide (void)
{
  a32 = b32 / f;	// { dg-error "error" }
  a32 = ld / b32;	// { dg-error "error" }
  a64 = b64 / d;	// { dg-error "error" }
  a64 = ld / b64;	// { dg-error "error" }
  a128 = b128 / ld;	// { dg-error "error" }
  a128 = d / b128;	// { dg-error "error" }
}

void
bad_pluseq (void)
{
  a32 += f;		// { dg-error "error" }
  a32 += d;		// { dg-error "error" }
  a32 += ld;		// { dg-error "error" }
  a64 += f;		// { dg-error "error" }
  a64 += d;		// { dg-error "error" }
  a64 += ld;		// { dg-error "error" }
  a128 += f;		// { dg-error "error" }
  a128 += d;		// { dg-error "error" }
  a128 += ld;		// { dg-error "error" }
}

void
bad_minuseq (void)
{
  a32 -= f;		// { dg-error "error" }
  a32 -= d;		// { dg-error "error" }
  a32 -= ld;		// { dg-error "error" }
  a64 -= f;		// { dg-error "error" }
  a64 -= d;		// { dg-error "error" }
  a64 -= ld;		// { dg-error "error" }
  a128 -= f;		// { dg-error "error" }
  a128 -= d;		// { dg-error "error" }
  a128 -= ld;		// { dg-error "error" }
}

void
bad_timeseq (void)
{
  a32 *= f;		// { dg-error "error" }
  a32 *= d;		// { dg-error "error" }
  a32 *= ld;		// { dg-error "error" }
  a64 *= f;		// { dg-error "error" }
  a64 *= d;		// { dg-error "error" }
  a64 *= ld;		// { dg-error "error" }
  a128 *= f;		// { dg-error "error" }
  a128 *= d;		// { dg-error "error" }
  a128 *= ld;		// { dg-error "error" }
}

void
bad_divideeq (void)
{
  a32 /= f;		// { dg-error "error" }
  a32 /= d;		// { dg-error "error" }
  a32 /= ld;		// { dg-error "error" }
  a64 /= f;		// { dg-error "error" }
  a64 /= d;		// { dg-error "error" }
  a64 /= ld;		// { dg-error "error" }
  a128 /= f;		// { dg-error "error" }
  a128 /= d;		// { dg-error "error" }
  a128 /= ld;		// { dg-error "error" }
}

