// $G $D/$F.go && $L $F.$A && ./$A.out

// Copyright 2011 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

// Test that dynamic interface checks treat byte=uint8
// and rune=int or rune=int32.

func main() {
	var x interface{}

	x = byte(1)
	switch x.(type) {
	case uint8:
		// ok
	default:
		println("byte != uint8")
	}

	x = uint8(2)
	switch x.(type) {
	case byte:
		// ok
	default:
		println("uint8 != byte")
	}

	rune32 := false
	x = rune(3)
	switch x.(type) {
	case int:
		// ok
	case int32:
		// must be new code
		rune32 = true
	default:
		println("rune != int and rune != int32")
	}

	if rune32 {
		x = int32(4)
	} else {
		x = int(5)
	}
	switch x.(type) {
	case rune:
		// ok
	default:
		println("int (or int32) != rune")
	}
}
