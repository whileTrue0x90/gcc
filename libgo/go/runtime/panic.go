// Copyright 2014 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package runtime

import (
	"runtime/internal/atomic"
	"unsafe"
)

// For gccgo, use go:linkname to export compiler-called functions.
//
//go:linkname deferproc
//go:linkname deferprocStack
//go:linkname deferreturn
//go:linkname setdeferretaddr
//go:linkname checkdefer
//go:linkname gopanic
//go:linkname canrecover
//go:linkname makefuncfficanrecover
//go:linkname makefuncreturning
//go:linkname gorecover
//go:linkname deferredrecover
//go:linkname goPanicIndex
//go:linkname goPanicIndexU
//go:linkname goPanicSliceAlen
//go:linkname goPanicSliceAlenU
//go:linkname goPanicSliceAcap
//go:linkname goPanicSliceAcapU
//go:linkname goPanicSliceB
//go:linkname goPanicSliceBU
//go:linkname goPanicSlice3Alen
//go:linkname goPanicSlice3AlenU
//go:linkname goPanicSlice3Acap
//go:linkname goPanicSlice3AcapU
//go:linkname goPanicSlice3B
//go:linkname goPanicSlice3BU
//go:linkname goPanicSlice3C
//go:linkname goPanicSlice3CU
//go:linkname panicshift
//go:linkname panicdivide
//go:linkname panicmem
// Temporary for C code to call:
//go:linkname throw

// Check to make sure we can really generate a panic. If the panic
// was generated from the runtime, or from inside malloc, then convert
// to a throw of msg.
// pc should be the program counter of the compiler-generated code that
// triggered this panic.
func panicCheck1(pc uintptr, msg string) {
	name, _, _, _ := funcfileline(pc-1, -1)
	if hasPrefix(name, "runtime.") {
		throw(msg)
	}
	// TODO: is this redundant? How could we be in malloc
	// but not in the runtime? runtime/internal/*, maybe?
	gp := getg()
	if gp != nil && gp.m != nil && gp.m.mallocing != 0 {
		throw(msg)
	}
}

// Same as above, but calling from the runtime is allowed.
//
// Using this function is necessary for any panic that may be
// generated by runtime.sigpanic, since those are always called by the
// runtime.
func panicCheck2(err string) {
	// panic allocates, so to avoid recursive malloc, turn panics
	// during malloc into throws.
	gp := getg()
	if gp != nil && gp.m != nil && gp.m.mallocing != 0 {
		throw(err)
	}
}

// Many of the following panic entry-points turn into throws when they
// happen in various runtime contexts. These should never happen in
// the runtime, and if they do, they indicate a serious issue and
// should not be caught by user code.
//
// The panic{Index,Slice,divide,shift} functions are called by
// code generated by the compiler for out of bounds index expressions,
// out of bounds slice expressions, division by zero, and shift by negative.
// The panicdivide (again), panicoverflow, panicfloat, and panicmem
// functions are called by the signal handler when a signal occurs
// indicating the respective problem.
//
// Since panic{Index,Slice,shift} are never called directly, and
// since the runtime package should never have an out of bounds slice
// or array reference or negative shift, if we see those functions called from the
// runtime package we turn the panic into a throw. That will dump the
// entire runtime stack for easier debugging.
//
// The entry points called by the signal handler will be called from
// runtime.sigpanic, so we can't disallow calls from the runtime to
// these (they always look like they're called from the runtime).
// Hence, for these, we just check for clearly bad runtime conditions.

// failures in the comparisons for s[x], 0 <= x < y (y == len(s))
func goPanicIndex(x int, y int) {
	panicCheck1(getcallerpc(), "index out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsIndex})
}
func goPanicIndexU(x uint, y int) {
	panicCheck1(getcallerpc(), "index out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsIndex})
}

// failures in the comparisons for s[:x], 0 <= x <= y (y == len(s) or cap(s))
func goPanicSliceAlen(x int, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsSliceAlen})
}
func goPanicSliceAlenU(x uint, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsSliceAlen})
}
func goPanicSliceAcap(x int, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsSliceAcap})
}
func goPanicSliceAcapU(x uint, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsSliceAcap})
}

// failures in the comparisons for s[x:y], 0 <= x <= y
func goPanicSliceB(x int, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsSliceB})
}
func goPanicSliceBU(x uint, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsSliceB})
}

// failures in the comparisons for s[::x], 0 <= x <= y (y == len(s) or cap(s))
func goPanicSlice3Alen(x int, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsSlice3Alen})
}
func goPanicSlice3AlenU(x uint, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsSlice3Alen})
}
func goPanicSlice3Acap(x int, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsSlice3Acap})
}
func goPanicSlice3AcapU(x uint, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsSlice3Acap})
}

// failures in the comparisons for s[:x:y], 0 <= x <= y
func goPanicSlice3B(x int, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsSlice3B})
}
func goPanicSlice3BU(x uint, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsSlice3B})
}

// failures in the comparisons for s[x:y:], 0 <= x <= y
func goPanicSlice3C(x int, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: true, y: y, code: boundsSlice3C})
}
func goPanicSlice3CU(x uint, y int) {
	panicCheck1(getcallerpc(), "slice bounds out of range")
	panic(boundsError{x: int64(x), signed: false, y: y, code: boundsSlice3C})
}

var shiftError = error(errorString("negative shift amount"))

func panicshift() {
	panicCheck1(getcallerpc(), "negative shift amount")
	panic(shiftError)
}

var divideError = error(errorString("integer divide by zero"))

func panicdivide() {
	panicCheck2("integer divide by zero")
	panic(divideError)
}

var overflowError = error(errorString("integer overflow"))

func panicoverflow() {
	panicCheck2("integer overflow")
	panic(overflowError)
}

var floatError = error(errorString("floating point error"))

func panicfloat() {
	panicCheck2("floating point error")
	panic(floatError)
}

var memoryError = error(errorString("invalid memory address or nil pointer dereference"))

func panicmem() {
	panicCheck2("invalid memory address or nil pointer dereference")
	panic(memoryError)
}

// deferproc creates a new deferred function.
// The compiler turns a defer statement into a call to this.
// frame points into the stack frame; it is used to determine which
// deferred functions are for the current stack frame, and whether we
// have already deferred functions for this frame.
// pfn is a C function pointer.
// arg is a value to pass to pfn.
func deferproc(frame *bool, pfn uintptr, arg unsafe.Pointer) {
	d := newdefer()
	if d._panic != nil {
		throw("deferproc: d.panic != nil after newdefer")
	}
	d.frame = frame
	d.panicStack = getg()._panic
	d.pfn = pfn
	d.arg = arg
	d.retaddr = 0
	d.makefunccanrecover = false
}

// deferprocStack queues a new deferred function with a defer record on the stack.
// The defer record, d, does not need to be initialized.
// Other arguments are the same as in deferproc.
//go:nosplit
func deferprocStack(d *_defer, frame *bool, pfn uintptr, arg unsafe.Pointer) {
	gp := getg()
	if gp.m.curg != gp {
		// go code on the system stack can't defer
		throw("defer on system stack")
	}
	d.pfn = pfn
	d.retaddr = 0
	d.makefunccanrecover = false
	d.heap = false
	// The lines below implement:
	//   d.frame = frame
	//   d.arg = arg
	//   d._panic = nil
	//   d.panicStack = gp._panic
	//   d.link = gp._defer
	// But without write barriers. They are writes to the stack so they
	// don't need a write barrier, and furthermore are to uninitialized
	// memory, so they must not use a write barrier.
	*(*uintptr)(unsafe.Pointer(&d.frame)) = uintptr(unsafe.Pointer(frame))
	*(*uintptr)(unsafe.Pointer(&d.arg)) = uintptr(unsafe.Pointer(arg))
	*(*uintptr)(unsafe.Pointer(&d._panic)) = 0
	*(*uintptr)(unsafe.Pointer(&d.panicStack)) = uintptr(unsafe.Pointer(gp._panic))
	*(*uintptr)(unsafe.Pointer(&d.link)) = uintptr(unsafe.Pointer(gp._defer))

	gp._defer = d
}

// Allocate a Defer, usually using per-P pool.
// Each defer must be released with freedefer.
func newdefer() *_defer {
	var d *_defer
	gp := getg()
	pp := gp.m.p.ptr()
	if len(pp.deferpool) == 0 && sched.deferpool != nil {
		systemstack(func() {
			lock(&sched.deferlock)
			for len(pp.deferpool) < cap(pp.deferpool)/2 && sched.deferpool != nil {
				d := sched.deferpool
				sched.deferpool = d.link
				d.link = nil
				pp.deferpool = append(pp.deferpool, d)
			}
			unlock(&sched.deferlock)
		})
	}
	if n := len(pp.deferpool); n > 0 {
		d = pp.deferpool[n-1]
		pp.deferpool[n-1] = nil
		pp.deferpool = pp.deferpool[:n-1]
	}
	if d == nil {
		systemstack(func() {
			d = new(_defer)
		})
		if debugCachedWork {
			// Duplicate the tail below so if there's a
			// crash in checkPut we can tell if d was just
			// allocated or came from the pool.
			d.heap = true
			d.link = gp._defer
			gp._defer = d
			return d
		}
	}
	d.heap = true
	d.link = gp._defer
	gp._defer = d
	return d
}

// Free the given defer.
// The defer cannot be used after this call.
//
// This must not grow the stack because there may be a frame without a
// stack map when this is called.
//
//go:nosplit
func freedefer(d *_defer) {
	if d._panic != nil {
		freedeferpanic()
	}
	if d.pfn != 0 {
		freedeferfn()
	}
	if !d.heap {
		return
	}
	pp := getg().m.p.ptr()
	if len(pp.deferpool) == cap(pp.deferpool) {
		// Transfer half of local cache to the central cache.
		//
		// Take this slow path on the system stack so
		// we don't grow freedefer's stack.
		systemstack(func() {
			var first, last *_defer
			for len(pp.deferpool) > cap(pp.deferpool)/2 {
				n := len(pp.deferpool)
				d := pp.deferpool[n-1]
				pp.deferpool[n-1] = nil
				pp.deferpool = pp.deferpool[:n-1]
				if first == nil {
					first = d
				} else {
					last.link = d
				}
				last = d
			}
			lock(&sched.deferlock)
			last.link = sched.deferpool
			sched.deferpool = first
			unlock(&sched.deferlock)
		})
	}

	// These lines used to be simply `*d = _defer{}` but that
	// started causing a nosplit stack overflow via typedmemmove.
	d.link = nil
	d.frame = nil
	d.panicStack = nil
	d.arg = nil
	d.retaddr = 0
	d.makefunccanrecover = false
	// d._panic and d.pfn must be nil already.
	// If not, we would have called freedeferpanic or freedeferfn above,
	// both of which throw.

	pp.deferpool = append(pp.deferpool, d)
}

// Separate function so that it can split stack.
// Windows otherwise runs out of stack space.
func freedeferpanic() {
	// _panic must be cleared before d is unlinked from gp.
	throw("freedefer with d._panic != nil")
}

func freedeferfn() {
	// fn must be cleared before d is unlinked from gp.
	throw("freedefer with d.fn != nil")
}

// deferreturn is called to undefer the stack.
// The compiler inserts a call to this function as a finally clause
// wrapped around the body of any function that calls defer.
// The frame argument points to the stack frame of the function.
func deferreturn(frame *bool) {
	gp := getg()
	for gp._defer != nil && gp._defer.frame == frame {
		d := gp._defer
		pfn := d.pfn
		d.pfn = 0

		if pfn != 0 {
			// This is rather awkward.
			// The gc compiler does this using assembler
			// code in jmpdefer.
			var fn func(unsafe.Pointer)
			*(*uintptr)(unsafe.Pointer(&fn)) = uintptr(noescape(unsafe.Pointer(&pfn)))
			gp.deferring = true
			fn(d.arg)
			gp.deferring = false
		}

		// If that was CgocallBackDone, it will have freed the
		// defer for us, since we are no longer running as Go code.
		if getg() == nil {
			*frame = true
			return
		}
		if gp.ranCgocallBackDone {
			gp.ranCgocallBackDone = false
			*frame = true
			return
		}

		gp._defer = d.link

		freedefer(d)

		// Since we are executing a defer function now, we
		// know that we are returning from the calling
		// function. If the calling function, or one of its
		// callees, panicked, then the defer functions would
		// be executed by panic.
		*frame = true
	}
}

// __builtin_extract_return_addr is a GCC intrinsic that converts an
// address returned by __builtin_return_address(0) to a real address.
// On most architectures this is a nop.
//extern __builtin_extract_return_addr
func __builtin_extract_return_addr(uintptr) uintptr

// setdeferretaddr records the address to which the deferred function
// returns.  This is check by canrecover.  The frontend relies on this
// function returning false.
func setdeferretaddr(retaddr uintptr) bool {
	gp := getg()
	if gp._defer != nil {
		gp._defer.retaddr = __builtin_extract_return_addr(retaddr)
	}
	return false
}

// checkdefer is called by exception handlers used when unwinding the
// stack after a recovered panic. The exception handler is simply
//   checkdefer(frame)
//   return;
// If we have not yet reached the frame we are looking for, we
// continue unwinding.
func checkdefer(frame *bool) {
	gp := getg()
	if gp == nil {
		// We should never wind up here. Even if some other
		// language throws an exception, the cgo code
		// should ensure that g is set.
		throw("no g in checkdefer")
	} else if gp.isforeign {
		// Some other language has thrown an exception.
		// We need to run the local defer handlers.
		// If they call recover, we stop unwinding here.
		var p _panic
		p.isforeign = true
		p.link = gp._panic
		gp._panic = (*_panic)(noescape(unsafe.Pointer(&p)))
		for {
			d := gp._defer
			if d == nil || d.frame != frame || d.pfn == 0 {
				break
			}

			pfn := d.pfn
			gp._defer = d.link

			var fn func(unsafe.Pointer)
			*(*uintptr)(unsafe.Pointer(&fn)) = uintptr(noescape(unsafe.Pointer(&pfn)))
			gp.deferring = true
			fn(d.arg)
			gp.deferring = false

			freedefer(d)

			if p.recovered {
				// The recover function caught the panic
				// thrown by some other language.
				break
			}
		}

		recovered := p.recovered
		gp._panic = p.link

		if recovered {
			// Just return and continue executing Go code.
			*frame = true
			return
		}

		// We are panicking through this function.
		*frame = false
	} else if gp._defer != nil && gp._defer.pfn == 0 && gp._defer.frame == frame {
		// This is the defer function that called recover.
		// Simply return to stop the stack unwind, and let the
		// Go code continue to execute.
		d := gp._defer
		gp._defer = d.link
		freedefer(d)

		// We are returning from this function.
		*frame = true

		return
	}

	// This is some other defer function. It was already run by
	// the call to panic, or just above. Rethrow the exception.
	rethrowException()
	throw("rethrowException returned")
}

// unwindStack starts unwinding the stack for a panic. We unwind
// function calls until we reach the one which used a defer function
// which called recover. Each function which uses a defer statement
// will have an exception handler, as shown above for checkdefer.
func unwindStack() {
	// Allocate the exception type used by the unwind ABI.
	// It would be nice to define it in runtime_sysinfo.go,
	// but current definitions don't work because the required
	// alignment is larger than can be represented in Go.
	// The type never contains any Go pointers.
	size := unwindExceptionSize()
	usize := uintptr(unsafe.Sizeof(uintptr(0)))
	c := (size + usize - 1) / usize
	s := make([]uintptr, c)
	getg().exception = unsafe.Pointer(&s[0])
	throwException()
}

// Goexit terminates the goroutine that calls it. No other goroutine is affected.
// Goexit runs all deferred calls before terminating the goroutine. Because Goexit
// is not a panic, any recover calls in those deferred functions will return nil.
//
// Calling Goexit from the main goroutine terminates that goroutine
// without func main returning. Since func main has not returned,
// the program continues execution of other goroutines.
// If all other goroutines exit, the program crashes.
func Goexit() {
	// Run all deferred functions for the current goroutine.
	// This code is similar to gopanic, see that implementation
	// for detailed comments.
	gp := getg()
	gp.goexiting = true
	for {
		d := gp._defer
		if d == nil {
			break
		}

		pfn := d.pfn
		if pfn == 0 {
			if d._panic != nil {
				d._panic.aborted = true
				d._panic = nil
			}
			gp._defer = d.link
			freedefer(d)
			continue
		}
		d.pfn = 0

		var fn func(unsafe.Pointer)
		*(*uintptr)(unsafe.Pointer(&fn)) = uintptr(noescape(unsafe.Pointer(&pfn)))
		gp.deferring = true
		fn(d.arg)
		gp.deferring = false

		if gp._defer != d {
			throw("bad defer entry in Goexit")
		}
		d._panic = nil
		gp._defer = d.link
		freedefer(d)
		// Note: we ignore recovers here because Goexit isn't a panic
	}
	gp.goexiting = false
	goexit1()
}

// Call all Error and String methods before freezing the world.
// Used when crashing with panicking.
func preprintpanics(p *_panic) {
	defer func() {
		if recover() != nil {
			throw("panic while printing panic value")
		}
	}()
	for p != nil {
		switch v := p.arg.(type) {
		case error:
			p.arg = v.Error()
		case stringer:
			p.arg = v.String()
		}
		p = p.link
	}
}

// Print all currently active panics. Used when crashing.
// Should only be called after preprintpanics.
func printpanics(p *_panic) {
	if p.link != nil {
		printpanics(p.link)
		print("\t")
	}
	print("panic: ")
	printany(p.arg)
	if p.recovered {
		print(" [recovered]")
	}
	print("\n")
}

// The implementation of the predeclared function panic.
func gopanic(e interface{}) {
	gp := getg()
	if gp.m.curg != gp {
		print("panic: ")
		printany(e)
		print("\n")
		throw("panic on system stack")
	}

	if gp.m.mallocing != 0 {
		print("panic: ")
		printany(e)
		print("\n")
		throw("panic during malloc")
	}
	if gp.m.preemptoff != "" {
		print("panic: ")
		printany(e)
		print("\n")
		print("preempt off reason: ")
		print(gp.m.preemptoff)
		print("\n")
		throw("panic during preemptoff")
	}
	if gp.m.locks != 0 {
		print("panic: ")
		printany(e)
		print("\n")
		throw("panic holding locks")
	}

	// The gc compiler allocates this new _panic struct on the
	// stack. We can't do that, because when a deferred function
	// recovers the panic we unwind the stack. We unlink this
	// entry before unwinding the stack, but that doesn't help in
	// the case where we panic, a deferred function recovers and
	// then panics itself, that panic is in turn recovered, and
	// unwinds the stack past this stack frame.

	p := &_panic{
		arg:  e,
		link: gp._panic,
	}
	gp._panic = p

	atomic.Xadd(&runningPanicDefers, 1)

	for {
		d := gp._defer
		if d == nil {
			break
		}

		pfn := d.pfn

		// If defer was started by earlier panic or Goexit (and, since we're back here, that triggered a new panic),
		// take defer off list. The earlier panic or Goexit will not continue running.
		if pfn == 0 {
			if d._panic != nil {
				d._panic.aborted = true
			}
			d._panic = nil
			gp._defer = d.link
			freedefer(d)
			continue
		}
		d.pfn = 0

		// Record the panic that is running the defer.
		// If there is a new panic during the deferred call, that panic
		// will find d in the list and will mark d._panic (this panic) aborted.
		d._panic = p

		var fn func(unsafe.Pointer)
		*(*uintptr)(unsafe.Pointer(&fn)) = uintptr(noescape(unsafe.Pointer(&pfn)))
		gp.deferring = true
		fn(d.arg)
		gp.deferring = false

		if gp._defer != d {
			throw("bad defer entry in panic")
		}
		d._panic = nil

		if p.recovered {
			atomic.Xadd(&runningPanicDefers, -1)

			gp._panic = p.link

			// Aborted panics are marked but remain on the g.panic list.
			// Remove them from the list.
			for gp._panic != nil && gp._panic.aborted {
				gp._panic = gp._panic.link
			}
			if gp._panic == nil { // must be done with signal
				gp.sig = 0
			}

			// Unwind the stack by throwing an exception.
			// The compiler has arranged to create
			// exception handlers in each function
			// that uses a defer statement.  These
			// exception handlers will check whether
			// the entry on the top of the defer stack
			// is from the current function.  If it is,
			// we have unwound the stack far enough.
			unwindStack()

			throw("unwindStack returned")
		}

		// Because we executed that defer function by a panic,
		// and it did not call recover, we know that we are
		// not returning from the calling function--we are
		// panicking through it.
		*d.frame = false

		// Deferred function did not panic. Remove d.
		// In the p.recovered case, d will be removed by checkdefer.
		gp._defer = d.link

		freedefer(d)
	}

	// ran out of deferred calls - old-school panic now
	// Because it is unsafe to call arbitrary user code after freezing
	// the world, we call preprintpanics to invoke all necessary Error
	// and String methods to prepare the panic strings before startpanic.
	preprintpanics(gp._panic)

	fatalpanic(gp._panic) // should not return
	*(*int)(nil) = 0      // not reached
}

// currentDefer returns the top of the defer stack if it can be recovered.
// Otherwise it returns nil.
func currentDefer() *_defer {
	gp := getg()
	d := gp._defer
	if d == nil {
		return nil
	}

	// The panic that would be recovered is the one on the top of
	// the panic stack. We do not want to recover it if that panic
	// was on the top of the panic stack when this function was
	// deferred.
	if d.panicStack == gp._panic {
		return nil
	}

	// The deferred thunk will call setdeferretaddr. If this has
	// not happened, then we have not been called via defer, and
	// we can not recover.
	if d.retaddr == 0 {
		return nil
	}

	return d
}

// canrecover is called by a thunk to see if the real function would
// be permitted to recover a panic value. Recovering a value is
// permitted if the thunk was called directly by defer. retaddr is the
// return address of the function that is calling canrecover--that is,
// the thunk.
func canrecover(retaddr uintptr) bool {
	d := currentDefer()
	if d == nil {
		return false
	}

	ret := __builtin_extract_return_addr(retaddr)
	dret := d.retaddr
	if ret <= dret && ret+16 >= dret {
		return true
	}

	// On some systems, in some cases, the return address does not
	// work reliably. See http://gcc.gnu.org/PR60406. If we are
	// permitted to call recover, the call stack will look like this:
	//     runtime.gopanic, runtime.deferreturn, etc.
	//     thunk to call deferred function (calls __go_set_defer_retaddr)
	//     function that calls __go_can_recover (passing return address)
	//     runtime.canrecover
	// Calling callers will skip the thunks. So if our caller's
	// caller starts with "runtime.", then we are permitted to
	// call recover.
	var locs [16]location
	if callers(1, locs[:2]) < 2 {
		return false
	}

	name := locs[1].function
	if hasPrefix(name, "runtime.") {
		return true
	}

	// If the function calling recover was created by reflect.MakeFunc,
	// then makefuncfficanrecover will have set makefunccanrecover.
	if !d.makefunccanrecover {
		return false
	}

	// We look up the stack, ignoring libffi functions and
	// functions in the reflect package, until we find
	// reflect.makeFuncStub or reflect.ffi_callback called by FFI
	// functions.  Then we check the caller of that function.

	n := callers(2, locs[:])
	foundFFICallback := false
	i := 0
	for ; i < n; i++ {
		name = locs[i].function
		if name == "" {
			// No function name means this caller isn't Go code.
			// Assume that this is libffi.
			continue
		}

		// Ignore function in libffi.
		if hasPrefix(name, "ffi_") {
			continue
		}

		if foundFFICallback {
			break
		}

		if name == "reflect.ffi_callback" {
			foundFFICallback = true
			continue
		}

		// Ignore other functions in the reflect package.
		if hasPrefix(name, "reflect.") || hasPrefix(name, ".1reflect.") {
			continue
		}

		// We should now be looking at the real caller.
		break
	}

	if i < n {
		name = locs[i].function
		if hasPrefix(name, "runtime.") {
			return true
		}
	}

	return false
}

// This function is called when code is about to enter a function
// created by the libffi version of reflect.MakeFunc. This function is
// passed the names of the callers of the libffi code that called the
// stub. It uses them to decide whether it is permitted to call
// recover, and sets d.makefunccanrecover so that gorecover can make
// the same decision.
func makefuncfficanrecover(loc []location) {
	d := currentDefer()
	if d == nil {
		return
	}

	// If we are already in a call stack of MakeFunc functions,
	// there is nothing we can usefully check here.
	if d.makefunccanrecover {
		return
	}

	// loc starts with the caller of our caller. That will be a thunk.
	// If its caller was a function function, then it was called
	// directly by defer.
	if len(loc) < 2 {
		return
	}

	name := loc[1].function
	if hasPrefix(name, "runtime.") {
		d.makefunccanrecover = true
	}
}

// makefuncreturning is called when code is about to exit a function
// created by reflect.MakeFunc. It is called by the function stub used
// by reflect.MakeFunc. It clears the makefunccanrecover field. It's
// OK to always clear this field, because canrecover will only be
// called by a stub created for a function that calls recover. That
// stub will not call a function created by reflect.MakeFunc, so by
// the time we get here any caller higher up on the call stack no
// longer needs the information.
func makefuncreturning() {
	d := getg()._defer
	if d != nil {
		d.makefunccanrecover = false
	}
}

// The implementation of the predeclared function recover.
func gorecover() interface{} {
	gp := getg()
	p := gp._panic
	if p != nil && !p.recovered {
		p.recovered = true
		return p.arg
	}
	return nil
}

// deferredrecover is called when a call to recover is deferred.  That
// is, something like
//   defer recover()
//
// We need to handle this specially.  In gc, the recover function
// looks up the stack frame. In particular, that means that a deferred
// recover will not recover a panic thrown in the same function that
// defers the recover. It will only recover a panic thrown in a
// function that defers the deferred call to recover.
//
// In other words:
//
// func f1() {
// 	defer recover()	// does not stop panic
// 	panic(0)
// }
//
// func f2() {
// 	defer func() {
// 		defer recover()	// stops panic(0)
// 	}()
// 	panic(0)
// }
//
// func f3() {
// 	defer func() {
// 		defer recover()	// does not stop panic
// 		panic(0)
// 	}()
// 	panic(1)
// }
//
// func f4() {
// 	defer func() {
// 		defer func() {
// 			defer recover()	// stops panic(0)
// 		}()
// 		panic(0)
// 	}()
// 	panic(1)
// }
//
// The interesting case here is f3. As can be seen from f2, the
// deferred recover could pick up panic(1). However, this does not
// happen because it is blocked by the panic(0).
//
// When a function calls recover, then when we invoke it we pass a
// hidden parameter indicating whether it should recover something.
// This parameter is set based on whether the function is being
// invoked directly from defer. The parameter winds up determining
// whether __go_recover or __go_deferred_recover is called at all.
//
// In the case of a deferred recover, the hidden parameter that
// controls the call is actually the one set up for the function that
// runs the defer recover() statement. That is the right thing in all
// the cases above except for f3. In f3 the function is permitted to
// call recover, but the deferred recover call is not. We address that
// here by checking for that specific case before calling recover. If
// this function was deferred when there is already a panic on the
// panic stack, then we can only recover that panic, not any other.

// Note that we can get away with using a special function here
// because you are not permitted to take the address of a predeclared
// function like recover.
func deferredrecover() interface{} {
	gp := getg()
	if gp._defer == nil || gp._defer.panicStack != gp._panic {
		return nil
	}
	return gorecover()
}

//go:linkname sync_throw sync.throw
func sync_throw(s string) {
	throw(s)
}

//go:nosplit
func throw(s string) {
	// Everything throw does should be recursively nosplit so it
	// can be called even when it's unsafe to grow the stack.
	systemstack(func() {
		print("fatal error: ", s, "\n")
	})
	gp := getg()
	if gp.m.throwing == 0 {
		gp.m.throwing = 1
	}
	fatalthrow()
	*(*int)(nil) = 0 // not reached
}

// runningPanicDefers is non-zero while running deferred functions for panic.
// runningPanicDefers is incremented and decremented atomically.
// This is used to try hard to get a panic stack trace out when exiting.
var runningPanicDefers uint32

// panicking is non-zero when crashing the program for an unrecovered panic.
// panicking is incremented and decremented atomically.
var panicking uint32

// paniclk is held while printing the panic information and stack trace,
// so that two concurrent panics don't overlap their output.
var paniclk mutex

// fatalthrow implements an unrecoverable runtime throw. It freezes the
// system, prints stack traces starting from its caller, and terminates the
// process.
//
//go:nosplit
func fatalthrow() {
	pc := getcallerpc()
	sp := getcallersp()
	gp := getg()

	startpanic_m()

	if dopanic_m(gp, pc, sp) {
		crash()
	}

	exit(2)

	*(*int)(nil) = 0 // not reached
}

// fatalpanic implements an unrecoverable panic. It is like fatalthrow, except
// that if msgs != nil, fatalpanic also prints panic messages and decrements
// runningPanicDefers once main is blocked from exiting.
//
//go:nosplit
func fatalpanic(msgs *_panic) {
	pc := getcallerpc()
	sp := getcallersp()
	gp := getg()
	var docrash bool

	if startpanic_m() && msgs != nil {
		// There were panic messages and startpanic_m
		// says it's okay to try to print them.

		// startpanic_m set panicking, which will
		// block main from exiting, so now OK to
		// decrement runningPanicDefers.
		atomic.Xadd(&runningPanicDefers, -1)

		printpanics(msgs)
	}

	docrash = dopanic_m(gp, pc, sp)

	if docrash {
		// By crashing outside the above systemstack call, debuggers
		// will not be confused when generating a backtrace.
		// Function crash is marked nosplit to avoid stack growth.
		crash()
	}

	systemstack(func() {
		exit(2)
	})

	*(*int)(nil) = 0 // not reached
}

// startpanic_m prepares for an unrecoverable panic.
//
// It returns true if panic messages should be printed, or false if
// the runtime is in bad shape and should just print stacks.
//
// It must not have write barriers even though the write barrier
// explicitly ignores writes once dying > 0. Write barriers still
// assume that g.m.p != nil, and this function may not have P
// in some contexts (e.g. a panic in a signal handler for a signal
// sent to an M with no P).
//
//go:nowritebarrierrec
func startpanic_m() bool {
	_g_ := getg()
	if mheap_.cachealloc.size == 0 { // very early
		print("runtime: panic before malloc heap initialized\n")
	}
	// Disallow malloc during an unrecoverable panic. A panic
	// could happen in a signal handler, or in a throw, or inside
	// malloc itself. We want to catch if an allocation ever does
	// happen (even if we're not in one of these situations).
	_g_.m.mallocing++

	// If we're dying because of a bad lock count, set it to a
	// good lock count so we don't recursively panic below.
	if _g_.m.locks < 0 {
		_g_.m.locks = 1
	}

	switch _g_.m.dying {
	case 0:
		// Setting dying >0 has the side-effect of disabling this G's writebuf.
		_g_.m.dying = 1
		atomic.Xadd(&panicking, 1)
		lock(&paniclk)
		if debug.schedtrace > 0 || debug.scheddetail > 0 {
			schedtrace(true)
		}
		freezetheworld()
		return true
	case 1:
		// Something failed while panicking.
		// Just print a stack trace and exit.
		_g_.m.dying = 2
		print("panic during panic\n")
		return false
	case 2:
		// This is a genuine bug in the runtime, we couldn't even
		// print the stack trace successfully.
		_g_.m.dying = 3
		print("stack trace unavailable\n")
		exit(4)
		fallthrough
	default:
		// Can't even print! Just exit.
		exit(5)
		return false // Need to return something.
	}
}

var didothers bool
var deadlock mutex

func dopanic_m(gp *g, pc, sp uintptr) bool {
	if gp.sig != 0 {
		signame := signame(gp.sig)
		if signame != "" {
			print("[signal ", signame)
		} else {
			print("[signal ", hex(gp.sig))
		}
		print(" code=", hex(gp.sigcode0), " addr=", hex(gp.sigcode1), " pc=", hex(gp.sigpc), "]\n")
	}

	level, all, docrash := gotraceback()
	_g_ := getg()
	if level > 0 {
		if gp != gp.m.curg {
			all = true
		}
		if gp != gp.m.g0 {
			print("\n")
			goroutineheader(gp)
			traceback(0)
		} else if level >= 2 || _g_.m.throwing > 0 {
			print("\nruntime stack:\n")
			traceback(0)
		}
		if !didothers && all {
			didothers = true
			tracebackothers(gp)
		}
	}
	unlock(&paniclk)

	if atomic.Xadd(&panicking, -1) != 0 {
		// Some other m is panicking too.
		// Let it print what it needs to print.
		// Wait forever without chewing up cpu.
		// It will exit when it's done.
		lock(&deadlock)
		lock(&deadlock)
	}

	printDebugLog()

	return docrash
}

// canpanic returns false if a signal should throw instead of
// panicking.
//
//go:nosplit
func canpanic(gp *g) bool {
	// Note that g is m->gsignal, different from gp.
	// Note also that g->m can change at preemption, so m can go stale
	// if this function ever makes a function call.
	_g_ := getg()
	_m_ := _g_.m

	// Is it okay for gp to panic instead of crashing the program?
	// Yes, as long as it is running Go code, not runtime code,
	// and not stuck in a system call.
	if gp == nil || gp != _m_.curg {
		return false
	}
	if _m_.locks != 0 || _m_.mallocing != 0 || _m_.throwing != 0 || _m_.preemptoff != "" || _m_.dying != 0 {
		return false
	}
	status := readgstatus(gp)
	if status&^_Gscan != _Grunning || gp.syscallsp != 0 {
		return false
	}
	return true
}

// isAbortPC reports whether pc is the program counter at which
// runtime.abort raises a signal.
//
// It is nosplit because it's part of the isgoexception
// implementation.
//
//go:nosplit
func isAbortPC(pc uintptr) bool {
	return false
}
