// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package math

/*
	Floating-point arcsine and arccosine.

	They are implemented by computing the arctangent
	after appropriate range reduction.
*/

// Asin returns the arcsine of x.
//
// Special cases are:
//	Asin(±0) = ±0
//	Asin(x) = NaN if x < -1 or x > 1

//extern asin
func libc_asin(float64) float64

func Asin(x float64) float64 {
	return libc_asin(x)
}

func asin(x float64) float64 {
	if x == 0 {
		return x // special case
	}
	sign := false
	if x < 0 {
		x = -x
		sign = true
	}
	if x > 1 {
		return NaN() // special case
	}

	temp := Sqrt(1 - x*x)
	if x > 0.7 {
		temp = Pi/2 - satan(temp/x)
	} else {
		temp = satan(x / temp)
	}

	if sign {
		temp = -temp
	}
	return temp
}

// Acos returns the arccosine of x.
//
// Special case is:
//	Acos(x) = NaN if x < -1 or x > 1

//extern acos
func libc_acos(float64) float64

func Acos(x float64) float64 {
	return libc_acos(x)
}

func acos(x float64) float64 {
	return Pi/2 - Asin(x)
}
