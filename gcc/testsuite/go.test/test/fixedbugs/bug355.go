// $G $D/$F.go && $L $F.$A && ./$A.out

// Copyright 2011 The Go Authors.  All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

var f = func() int {
	type S int
	return 42
}

func main() {
	if f() != 42 {
		panic("BUG: bug355")
	}
}
