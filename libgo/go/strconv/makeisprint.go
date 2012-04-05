// Copyright 2012 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// +build ignore

// makeisprint generates the tables for strconv's compact isPrint.
package main

import (
	"fmt"
	"os"
	"unicode"
)

var (
	range16  []uint16
	except16 []uint16
	range32  []uint32
	except32 []uint32
)

// bsearch16 returns the smallest i such that a[i] >= x.
// If there is no such i, bsearch16 returns len(a).
func bsearch16(a []uint16, x uint16) int {
	i, j := 0, len(a)
	for i < j {
		h := i + (j-i)/2
		if a[h] < x {
			i = h + 1
		} else {
			j = h
		}
	}
	return i
}

// bsearch32 returns the smallest i such that a[i] >= x.
// If there is no such i, bsearch32 returns len(a).
func bsearch32(a []uint32, x uint32) int {
	i, j := 0, len(a)
	for i < j {
		h := i + (j-i)/2
		if a[h] < x {
			i = h + 1
		} else {
			j = h
		}
	}
	return i
}

func isPrint(r rune) bool {
	// Same algorithm, either on uint16 or uint32 value.
	// First, find first i such that rang[i] >= x.
	// This is the index of either the start or end of a pair that might span x.
	// The start is even (rang[i&^1]) and the end is odd (rang[i|1]).
	// If we find x in a range, make sure x is not in exception list.

	if 0 <= r && r < 1<<16 {
		rr, rang, except := uint16(r), range16, except16
		i := bsearch16(rang, rr)
		if i >= len(rang) || rr < rang[i&^1] || rang[i|1] < rr {
			return false
		}
		j := bsearch16(except, rr)
		return j >= len(except) || except[j] != rr
	}

	rr, rang, except := uint32(r), range32, except32
	i := bsearch32(rang, rr)
	if i >= len(rang) || rr < rang[i&^1] || rang[i|1] < rr {
		return false
	}
	j := bsearch32(except, rr)
	return j >= len(except) || except[j] != rr
}

func scan(min, max rune) (rang, except []uint32) {
	lo := rune(-1)
	for i := min; ; i++ {
		if (i > max || !unicode.IsPrint(i)) && lo >= 0 {
			// End range, but avoid flip flop.
			if i+1 <= max && unicode.IsPrint(i+1) {
				except = append(except, uint32(i))
				continue
			}
			rang = append(rang, uint32(lo), uint32(i-1))
			lo = -1
		}
		if i > max {
			break
		}
		if lo < 0 && unicode.IsPrint(i) {
			lo = i
		}
	}
	return
}

func to16(x []uint32) []uint16 {
	var y []uint16
	for _, v := range x {
		if uint32(uint16(v)) != v {
			panic("bad 32->16 conversion")
		}
		y = append(y, uint16(v))
	}
	return y
}

func main() {
	rang, except := scan(0, 0xFFFF)
	range16 = to16(rang)
	except16 = to16(except)
	range32, except32 = scan(0x10000, unicode.MaxRune)

	for i := rune(0); i <= unicode.MaxRune; i++ {
		if isPrint(i) != unicode.IsPrint(i) {
			fmt.Fprintf(os.Stderr, "%U: isPrint=%v, want %v\n", i, isPrint(i), unicode.IsPrint(i))
			return
		}
	}

	fmt.Printf("// DO NOT EDIT.  GENERATED BY\n")
	fmt.Printf("//     go run makeisprint.go >x && mv x isprint.go\n\n")
	fmt.Printf("package strconv\n\n")

	fmt.Printf("// (%d+%d+%d)*2 + (%d)*4 = %d bytes\n\n",
		len(range16), len(except16), len(except32),
		len(range32),
		(len(range16)+len(except16)+len(except32))*2+
			(len(range32))*4)

	fmt.Printf("var isPrint16 = []uint16{\n")
	for i := 0; i < len(range16); i += 2 {
		fmt.Printf("\t%#04x, %#04x,\n", range16[i], range16[i+1])
	}
	fmt.Printf("}\n\n")

	fmt.Printf("var isNotPrint16 = []uint16{\n")
	for _, r := range except16 {
		fmt.Printf("\t%#04x,\n", r)
	}
	fmt.Printf("}\n\n")

	fmt.Printf("var isPrint32 = []uint32{\n")
	for i := 0; i < len(range32); i += 2 {
		fmt.Printf("\t%#06x, %#06x,\n", range32[i], range32[i+1])
	}
	fmt.Printf("}\n\n")

	fmt.Printf("var isNotPrint32 = []uint16{ // add 0x10000 to each entry\n")
	for _, r := range except32 {
		if r >= 0x20000 {
			fmt.Fprintf(os.Stderr, "%U too big for isNotPrint32\n", r)
			return
		}
		fmt.Printf("\t%#04x,\n", r-0x10000)
	}
	fmt.Printf("}\n")
}
