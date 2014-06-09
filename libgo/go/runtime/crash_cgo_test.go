// Copyright 2012 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// +build cgo

package runtime_test

import (
	"runtime"
	"testing"
)

func TestCgoCrashHandler(t *testing.T) {
	testCrashHandler(t, true)
}

func TestCgoSignalDeadlock(t *testing.T) {
	if testing.Short() && runtime.GOOS == "windows" {
		t.Skip("Skipping in short mode") // takes up to 64 seconds
	}
	t.Skip("gccgo does not have a go command")
	got := executeTest(t, cgoSignalDeadlockSource, nil)
	want := "OK\n"
	if got != want {
		t.Fatalf("expected %q, but got %q", want, got)
	}
}

func TestCgoTraceback(t *testing.T) {
	got := executeTest(t, cgoTracebackSource, nil)
	want := "OK\n"
	if got != want {
		t.Fatalf("expected %q, but got %q", want, got)
	}
}

const cgoSignalDeadlockSource = `
package main

import "C"

import (
	"fmt"
	"runtime"
	"time"
)

func main() {
	runtime.GOMAXPROCS(100)
	ping := make(chan bool)
	go func() {
		for i := 0; ; i++ {
			runtime.Gosched()
			select {
			case done := <-ping:
				if done {
					ping <- true
					return
				}
				ping <- true
			default:
			}
			func() {
				defer func() {
					recover()
				}()
				var s *string
				*s = ""
			}()
		}
	}()
	time.Sleep(time.Millisecond)
	for i := 0; i < 64; i++ {
		go func() {
			runtime.LockOSThread()
			select {}
		}()
		go func() {
			runtime.LockOSThread()
			select {}
		}()
		time.Sleep(time.Millisecond)
		ping <- false
		select {
		case <-ping:
		case <-time.After(time.Second):
			fmt.Printf("HANG\n")
			return
		}
	}
	ping <- true
	select {
	case <-ping:
	case <-time.After(time.Second):
		fmt.Printf("HANG\n")
		return
	}
	fmt.Printf("OK\n")
}
`

const cgoTracebackSource = `
package main

/* void foo(void) {} */
import "C"

import (
	"fmt"
	"runtime"
)

func main() {
	C.foo()
	buf := make([]byte, 1)
	runtime.Stack(buf, true)
	fmt.Printf("OK\n")
}
`
