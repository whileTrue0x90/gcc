// $G $D/$F.go || echo BUG should compile

// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

func main() {
	if true {
	} else {
	L1:
	}
	if true {
	} else {
	L2:
		main()
	}
	goto L1
	goto L2
}

/*
These should be legal according to the spec.
bug140.go:6: syntax error near L1
bug140.go:7: syntax error near L2
*/
