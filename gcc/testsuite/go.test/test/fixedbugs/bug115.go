// $G $D/$F.go || echo BUG: bug115 should compile

// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package main

func isuint(i uint) { }

func main() {
	i := ^uint(0);
	isuint(i);
}
