// Copyright 2011 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package os

const (
	PathSeparator     = '/' // OS-specific path separator
	PathListSeparator = 0   // OS-specific path list separator
)

// IsPathSeparator returns true if c is a directory separator character.
func IsPathSeparator(c uint8) bool {
	return PathSeparator == c
}
