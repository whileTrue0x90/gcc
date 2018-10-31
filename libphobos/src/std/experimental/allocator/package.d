// Written in the D programming language.
/**

High-level interface for allocators. Implements bundled allocation/creation
and destruction/deallocation of data including `struct`s and `class`es,
and also array primitives related to allocation. This module is the entry point
for both making use of allocators and for their documentation.

$(SCRIPT inhibitQuickIndex = 1;)
$(BOOKTABLE,
$(TR $(TH Category) $(TH Functions))
$(TR $(TD Make) $(TD
    $(LREF make)
    $(LREF makeArray)
    $(LREF makeMultidimensionalArray)
))
$(TR $(TD Dispose) $(TD
    $(LREF dispose)
    $(LREF disposeMultidimensionalArray)
))
$(TR $(TD Modify) $(TD
    $(LREF expandArray)
    $(LREF shrinkArray)
))
$(TR $(TD Global) $(TD
    $(LREF processAllocator)
    $(LREF theAllocator)
))
$(TR $(TD Class interface) $(TD
    $(LREF allocatorObject)
    $(LREF CAllocatorImpl)
    $(LREF IAllocator)
))
)

Synopsis:
---
// Allocate an int, initialize it with 42
int* p = theAllocator.make!int(42);
assert(*p == 42);
// Destroy and deallocate it
theAllocator.dispose(p);

// Allocate using the global process allocator
p = processAllocator.make!int(100);
assert(*p == 100);
// Destroy and deallocate
processAllocator.dispose(p);

// Create an array of 50 doubles initialized to -1.0
double[] arr = theAllocator.makeArray!double(50, -1.0);
// Append two zeros to it
theAllocator.expandArray(arr, 2, 0.0);
// On second thought, take that back
theAllocator.shrinkArray(arr, 2);
// Destroy and deallocate
theAllocator.dispose(arr);
---

$(H2 Layered Structure)

D's allocators have a layered structure in both implementation and documentation:

$(OL
$(LI A high-level, dynamically-typed layer (described further down in this
module). It consists of an interface called $(LREF IAllocator), which concret;
allocators need to implement. The interface primitives themselves are oblivious
to the type of the objects being allocated; they only deal in `void[]`, by
necessity of the interface being dynamic (as opposed to type-parameterized).
Each thread has a current allocator it uses by default, which is a thread-local
variable $(LREF theAllocator) of type $(LREF IAllocator). The process has a
global _allocator called $(LREF processAllocator), also of type $(LREF
IAllocator). When a new thread is created, $(LREF processAllocator) is copied
into $(LREF theAllocator). An application can change the objects to which these
references point. By default, at application startup, $(LREF processAllocator)
refers to an object that uses D's garbage collected heap. This layer also
include high-level functions such as $(LREF make) and $(LREF dispose) that
comfortably allocate/create and respectively destroy/deallocate objects. This
layer is all needed for most casual uses of allocation primitives.)

$(LI A mid-level, statically-typed layer for assembling several allocators into
one. It uses properties of the type of the objects being created to route
allocation requests to possibly specialized allocators. This layer is relatively
thin and implemented and documented in the $(MREF
std,experimental,_allocator,typed) module. It allows an interested user to e.g.
use different allocators for arrays versus fixed-sized objects, to the end of
better overall performance.)

$(LI A low-level collection of highly generic $(I heap building blocks)$(MDASH)
Lego-like pieces that can be used to assemble application-specific allocators.
The real allocation smarts are occurring at this level. This layer is of
interest to advanced applications that want to configure their own allocators.
A good illustration of typical uses of these building blocks is module $(MREF
std,experimental,_allocator,showcase) which defines a collection of frequently-
used preassembled allocator objects. The implementation and documentation entry
point is $(MREF std,experimental,_allocator,building_blocks). By design, the
primitives of the static interface have the same signatures as the $(LREF
IAllocator) primitives but are for the most part optional and driven by static
introspection. The parameterized class $(LREF CAllocatorImpl) offers an
immediate and useful means to package a static low-level _allocator into an
implementation of $(LREF IAllocator).)

$(LI Core _allocator objects that interface with D's garbage collected heap
($(MREF std,experimental,_allocator,gc_allocator)), the C `malloc` family
($(MREF std,experimental,_allocator,mallocator)), and the OS ($(MREF
std,experimental,_allocator,mmap_allocator)). Most custom allocators would
ultimately obtain memory from one of these core allocators.)
)

$(H2 Idiomatic Use of $(D std.experimental._allocator))

As of this time, $(D std.experimental._allocator) is not integrated with D's
built-in operators that allocate memory, such as `new`, array literals, or
array concatenation operators. That means $(D std.experimental._allocator) is
opt-in$(MDASH)applications need to make explicit use of it.

For casual creation and disposal of dynamically-allocated objects, use $(LREF
make), $(LREF dispose), and the array-specific functions $(LREF makeArray),
$(LREF expandArray), and $(LREF shrinkArray). These use by default D's garbage
collected heap, but open the application to better configuration options. These
primitives work either with `theAllocator` but also with any allocator obtained
by combining heap building blocks. For example:

----
void fun(size_t n)
{
    // Use the current allocator
    int[] a1 = theAllocator.makeArray!int(n);
    scope(exit) theAllocator.dispose(a1);
    ...
}
----

To experiment with alternative allocators, set $(LREF theAllocator) for the
current thread. For example, consider an application that allocates many 8-byte
objects. These are not well supported by the default _allocator, so a
$(MREF_ALTTEXT free list _allocator,
std,experimental,_allocator,building_blocks,free_list) would be recommended.
To install one in `main`, the application would use:

----
void main()
{
    import std.experimental.allocator.building_blocks.free_list
        : FreeList;
    theAllocator = allocatorObject(FreeList!8());
    ...
}
----

$(H3 Saving the `IAllocator` Reference For Later Use)

As with any global resource, setting `theAllocator` and `processAllocator`
should not be done often and casually. In particular, allocating memory with
one allocator and deallocating with another causes undefined behavior.
Typically, these variables are set during application initialization phase and
last through the application.

To avoid this, long-lived objects that need to perform allocations,
reallocations, and deallocations relatively often may want to store a reference
to the _allocator object they use throughout their lifetime. Then, instead of
using `theAllocator` for internal allocation-related tasks, they'd use the
internally held reference. For example, consider a user-defined hash table:

----
struct HashTable
{
    private IAllocator _allocator;
    this(size_t buckets, IAllocator allocator = theAllocator) {
        this._allocator = allocator;
        ...
    }
    // Getter and setter
    IAllocator allocator() { return _allocator; }
    void allocator(IAllocator a) { assert(empty); _allocator = a; }
}
----

Following initialization, the `HashTable` object would consistently use its
$(D _allocator) object for acquiring memory. Furthermore, setting
$(D HashTable._allocator) to point to a different _allocator should be legal but
only if the object is empty; otherwise, the object wouldn't be able to
deallocate its existing state.

$(H3 Using Allocators without `IAllocator`)

Allocators assembled from the heap building blocks don't need to go through
`IAllocator` to be usable. They have the same primitives as `IAllocator` and
they work with $(LREF make), $(LREF makeArray), $(LREF dispose) etc. So it
suffice to create allocator objects wherever fit and use them appropriately:

----
void fun(size_t n)
{
    // Use a stack-installed allocator for up to 64KB
    StackFront!65536 myAllocator;
    int[] a2 = myAllocator.makeArray!int(n);
    scope(exit) myAllocator.dispose(a2);
    ...
}
----

In this case, `myAllocator` does not obey the `IAllocator` interface, but
implements its primitives so it can work with `makeArray` by means of duck
typing.

One important thing to note about this setup is that statically-typed assembled
allocators are almost always faster than allocators that go through
`IAllocator`. An important rule of thumb is: "assemble allocator first, adapt
to `IAllocator` after". A good allocator implements intricate logic by means of
template assembly, and gets wrapped with `IAllocator` (usually by means of
$(LREF allocatorObject)) only once, at client level.

Copyright: Andrei Alexandrescu 2013-.

License: $(HTTP boost.org/LICENSE_1_0.txt, Boost License 1.0).

Authors: $(HTTP erdani.com, Andrei Alexandrescu)

Source: $(PHOBOSSRC std/experimental/_allocator)

*/

module std.experimental.allocator;

public import std.experimental.allocator.common,
    std.experimental.allocator.typed;

// Example in the synopsis above
@system unittest
{
    import std.algorithm.comparison : min, max;
    import std.experimental.allocator.building_blocks.allocator_list
        : AllocatorList;
    import std.experimental.allocator.building_blocks.bitmapped_block
        : BitmappedBlock;
    import std.experimental.allocator.building_blocks.bucketizer : Bucketizer;
    import std.experimental.allocator.building_blocks.free_list : FreeList;
    import std.experimental.allocator.building_blocks.segregator : Segregator;
    import std.experimental.allocator.gc_allocator : GCAllocator;

    alias FList = FreeList!(GCAllocator, 0, unbounded);
    alias A = Segregator!(
        8, FreeList!(GCAllocator, 0, 8),
        128, Bucketizer!(FList, 1, 128, 16),
        256, Bucketizer!(FList, 129, 256, 32),
        512, Bucketizer!(FList, 257, 512, 64),
        1024, Bucketizer!(FList, 513, 1024, 128),
        2048, Bucketizer!(FList, 1025, 2048, 256),
        3584, Bucketizer!(FList, 2049, 3584, 512),
        4072 * 1024, AllocatorList!(
            (n) => BitmappedBlock!(4096)(
                    cast(ubyte[])(GCAllocator.instance.allocate(
                        max(n, 4072 * 1024))))),
        GCAllocator
    );
    A tuMalloc;
    auto b = tuMalloc.allocate(500);
    assert(b.length == 500);
    auto c = tuMalloc.allocate(113);
    assert(c.length == 113);
    assert(tuMalloc.expand(c, 14));
    tuMalloc.deallocate(b);
    tuMalloc.deallocate(c);
}

import std.range.primitives;
import std.traits;
import std.typecons;

/**
Dynamic allocator interface. Code that defines allocators ultimately implements
this interface. This should be used wherever a uniform type is required for
encapsulating various allocator implementations.

Composition of allocators is not recommended at this level due to
inflexibility of dynamic interfaces and inefficiencies caused by cascaded
multiple calls. Instead, compose allocators using the static interface defined
in $(A std_experimental_allocator_building_blocks.html,
`std.experimental.allocator.building_blocks`), then adapt the composed
allocator to `IAllocator` (possibly by using $(LREF CAllocatorImpl) below).

Methods returning $(D Ternary) return $(D Ternary.yes) upon success,
$(D Ternary.no) upon failure, and $(D Ternary.unknown) if the primitive is not
implemented by the allocator instance.
*/
interface IAllocator
{
    /**
    Returns the alignment offered.
    */
    @property uint alignment();

    /**
    Returns the good allocation size that guarantees zero internal
    fragmentation.
    */
    size_t goodAllocSize(size_t s);

    /**
    Allocates `n` bytes of memory.
    */
    void[] allocate(size_t, TypeInfo ti = null);

    /**
    Allocates `n` bytes of memory with specified alignment `a`. Implementations
    that do not support this primitive should always return `null`.
    */
    void[] alignedAllocate(size_t n, uint a);

    /**
    Allocates and returns all memory available to this allocator.
    Implementations that do not support this primitive should always return
    `null`.
    */
    void[] allocateAll();

    /**
    Expands a memory block in place and returns `true` if successful.
    Implementations that don't support this primitive should always return
    `false`.
    */
    bool expand(ref void[], size_t);

    /// Reallocates a memory block.
    bool reallocate(ref void[], size_t);

    /// Reallocates a memory block with specified alignment.
    bool alignedReallocate(ref void[] b, size_t size, uint alignment);

    /**
    Returns $(D Ternary.yes) if the allocator owns $(D b), $(D Ternary.no) if
    the allocator doesn't own $(D b), and $(D Ternary.unknown) if ownership
    cannot be determined. Implementations that don't support this primitive
    should always return `Ternary.unknown`.
    */
    Ternary owns(void[] b);

    /**
    Resolves an internal pointer to the full block allocated. Implementations
    that don't support this primitive should always return `Ternary.unknown`.
    */
    Ternary resolveInternalPointer(const void* p, ref void[] result);

    /**
    Deallocates a memory block. Implementations that don't support this
    primitive should always return `false`. A simple way to check that an
    allocator supports deallocation is to call $(D deallocate(null)).
    */
    bool deallocate(void[] b);

    /**
    Deallocates all memory. Implementations that don't support this primitive
    should always return `false`.
    */
    bool deallocateAll();

    /**
    Returns $(D Ternary.yes) if no memory is currently allocated from this
    allocator, $(D Ternary.no) if some allocations are currently active, or
    $(D Ternary.unknown) if not supported.
    */
    Ternary empty();
}

/**
Dynamic shared allocator interface. Code that defines allocators shareable
across threads ultimately implements this interface. This should be used
wherever a uniform type is required for encapsulating various allocator
implementations.

Composition of allocators is not recommended at this level due to
inflexibility of dynamic interfaces and inefficiencies caused by cascaded
multiple calls. Instead, compose allocators using the static interface defined
in $(A std_experimental_allocator_building_blocks.html,
`std.experimental.allocator.building_blocks`), then adapt the composed
allocator to `ISharedAllocator` (possibly by using $(LREF CSharedAllocatorImpl) below).

Methods returning $(D Ternary) return $(D Ternary.yes) upon success,
$(D Ternary.no) upon failure, and $(D Ternary.unknown) if the primitive is not
implemented by the allocator instance.
*/
interface ISharedAllocator
{
    /**
    Returns the alignment offered.
    */
    @property uint alignment() shared;

    /**
    Returns the good allocation size that guarantees zero internal
    fragmentation.
    */
    size_t goodAllocSize(size_t s) shared;

    /**
    Allocates `n` bytes of memory.
    */
    void[] allocate(size_t, TypeInfo ti = null) shared;

    /**
    Allocates `n` bytes of memory with specified alignment `a`. Implementations
    that do not support this primitive should always return `null`.
    */
    void[] alignedAllocate(size_t n, uint a) shared;

    /**
    Allocates and returns all memory available to this allocator.
    Implementations that do not support this primitive should always return
    `null`.
    */
    void[] allocateAll() shared;

    /**
    Expands a memory block in place and returns `true` if successful.
    Implementations that don't support this primitive should always return
    `false`.
    */
    bool expand(ref void[], size_t) shared;

    /// Reallocates a memory block.
    bool reallocate(ref void[], size_t) shared;

    /// Reallocates a memory block with specified alignment.
    bool alignedReallocate(ref void[] b, size_t size, uint alignment) shared;

    /**
    Returns $(D Ternary.yes) if the allocator owns $(D b), $(D Ternary.no) if
    the allocator doesn't own $(D b), and $(D Ternary.unknown) if ownership
    cannot be determined. Implementations that don't support this primitive
    should always return `Ternary.unknown`.
    */
    Ternary owns(void[] b) shared;

    /**
    Resolves an internal pointer to the full block allocated. Implementations
    that don't support this primitive should always return `Ternary.unknown`.
    */
    Ternary resolveInternalPointer(const void* p, ref void[] result) shared;

    /**
    Deallocates a memory block. Implementations that don't support this
    primitive should always return `false`. A simple way to check that an
    allocator supports deallocation is to call $(D deallocate(null)).
    */
    bool deallocate(void[] b) shared;

    /**
    Deallocates all memory. Implementations that don't support this primitive
    should always return `false`.
    */
    bool deallocateAll() shared;

    /**
    Returns $(D Ternary.yes) if no memory is currently allocated from this
    allocator, $(D Ternary.no) if some allocations are currently active, or
    $(D Ternary.unknown) if not supported.
    */
    Ternary empty() shared;
}

private shared ISharedAllocator _processAllocator;
private IAllocator _threadAllocator;

private IAllocator setupThreadAllocator() nothrow @nogc @safe
{
    /*
    Forwards the `_threadAllocator` calls to the `processAllocator`
    */
    static class ThreadAllocator : IAllocator
    {
        override @property uint alignment()
        {
            return processAllocator.alignment();
        }

        override size_t goodAllocSize(size_t s)
        {
            return processAllocator.goodAllocSize(s);
        }

        override void[] allocate(size_t n, TypeInfo ti = null)
        {
            return processAllocator.allocate(n, ti);
        }

        override void[] alignedAllocate(size_t n, uint a)
        {
            return processAllocator.alignedAllocate(n, a);
        }

        override void[] allocateAll()
        {
            return processAllocator.allocateAll();
        }

        override bool expand(ref void[] b, size_t size)
        {
            return processAllocator.expand(b, size);
        }

        override bool reallocate(ref void[] b, size_t size)
        {
            return processAllocator.reallocate(b, size);
        }

        override bool alignedReallocate(ref void[] b, size_t size, uint alignment)
        {
            return processAllocator.alignedReallocate(b, size, alignment);
        }

        override Ternary owns(void[] b)
        {
            return processAllocator.owns(b);
        }

        override Ternary resolveInternalPointer(const void* p, ref void[] result)
        {
            return processAllocator.resolveInternalPointer(p, result);
        }

        override bool deallocate(void[] b)
        {
            return processAllocator.deallocate(b);
        }

        override bool deallocateAll()
        {
            return processAllocator.deallocateAll();
        }

        override Ternary empty()
        {
            return processAllocator.empty();
        }
    }

    assert(!_threadAllocator);
    import std.conv : emplace;
    static ulong[stateSize!(ThreadAllocator).divideRoundUp(ulong.sizeof)] _threadAllocatorState;
    _threadAllocator = () @trusted { return emplace!(ThreadAllocator)(_threadAllocatorState[]); } ();
    return _threadAllocator;
}

/**
Gets/sets the allocator for the current thread. This is the default allocator
that should be used for allocating thread-local memory. For allocating memory
to be shared across threads, use $(D processAllocator) (below). By default,
$(D theAllocator) ultimately fetches memory from $(D processAllocator), which
in turn uses the garbage collected heap.
*/
nothrow @safe @nogc @property IAllocator theAllocator()
{
    auto p = _threadAllocator;
    return p !is null ? p : setupThreadAllocator();
}

/// Ditto
nothrow @safe @nogc @property void theAllocator(IAllocator a)
{
    assert(a);
    _threadAllocator = a;
}

///
@system unittest
{
    // Install a new allocator that is faster for 128-byte allocations.
    import std.experimental.allocator.building_blocks.free_list : FreeList;
    import std.experimental.allocator.gc_allocator : GCAllocator;
    auto oldAllocator = theAllocator;
    scope(exit) theAllocator = oldAllocator;
    theAllocator = allocatorObject(FreeList!(GCAllocator, 128)());
    // Use the now changed allocator to allocate an array
    const ubyte[] arr = theAllocator.makeArray!ubyte(128);
    assert(arr.ptr);
    //...
}

/**
Gets/sets the allocator for the current process. This allocator must be used
for allocating memory shared across threads. Objects created using this
allocator can be cast to $(D shared).
*/
@property shared(ISharedAllocator) processAllocator()
{
    import std.experimental.allocator.gc_allocator : GCAllocator;
    import std.concurrency : initOnce;
    return initOnce!_processAllocator(
        sharedAllocatorObject(GCAllocator.instance));
}

/// Ditto
@property void processAllocator(shared ISharedAllocator a)
{
    assert(a);
    _processAllocator = a;
}

@system unittest
{
    import core.exception : AssertError;
    import std.exception : assertThrown;
    import std.experimental.allocator.building_blocks.free_list : SharedFreeList;
    import std.experimental.allocator.mallocator : Mallocator;

    assert(processAllocator);
    assert(theAllocator);

    testAllocatorObject(processAllocator);
    testAllocatorObject(theAllocator);

    shared SharedFreeList!(Mallocator, chooseAtRuntime, chooseAtRuntime) sharedFL;
    shared ISharedAllocator sharedFLObj = sharedAllocatorObject(sharedFL);
    assert(sharedFLObj);
    testAllocatorObject(sharedFLObj);

    // Test processAllocator setter
    shared ISharedAllocator oldProcessAllocator = processAllocator;
    processAllocator = sharedFLObj;
    assert(processAllocator is sharedFLObj);

    testAllocatorObject(processAllocator);
    testAllocatorObject(theAllocator);
    assertThrown!AssertError(processAllocator = null);

    // Restore initial processAllocator state
    processAllocator = oldProcessAllocator;
    assert(processAllocator is oldProcessAllocator);

    shared ISharedAllocator indirectShFLObj = sharedAllocatorObject(&sharedFL);
    testAllocatorObject(indirectShFLObj);

    IAllocator indirectMallocator = allocatorObject(&Mallocator.instance);
    testAllocatorObject(indirectMallocator);
}

/**
Dynamically allocates (using $(D alloc)) and then creates in the memory
allocated an object of type $(D T), using $(D args) (if any) for its
initialization. Initialization occurs in the memory allocated and is otherwise
semantically the same as $(D T(args)).
(Note that using $(D alloc.make!(T[])) creates a pointer to an (empty) array
of $(D T)s, not an array. To use an allocator to allocate and initialize an
array, use $(D alloc.makeArray!T) described below.)

Params:
T = Type of the object being created.
alloc = The allocator used for getting the needed memory. It may be an object
implementing the static interface for allocators, or an $(D IAllocator)
reference.
args = Optional arguments used for initializing the created object. If not
present, the object is default constructed.

Returns: If $(D T) is a class type, returns a reference to the created $(D T)
object. Otherwise, returns a $(D T*) pointing to the created object. In all
cases, returns $(D null) if allocation failed.

Throws: If $(D T)'s constructor throws, deallocates the allocated memory and
propagates the exception.
*/
auto make(T, Allocator, A...)(auto ref Allocator alloc, auto ref A args)
{
    import std.algorithm.comparison : max;
    import std.conv : emplace, emplaceRef;
    auto m = alloc.allocate(max(stateSize!T, 1));
    if (!m.ptr) return null;

    // make can only be @safe if emplace or emplaceRef is `pure`
    auto construct()
    {
        static if (is(T == class)) return emplace!T(m, args);
        else
        {
            // Assume cast is safe as allocation succeeded for `stateSize!T`
            auto p = () @trusted { return cast(T*) m.ptr; }();
            emplaceRef(*p, args);
            return p;
        }
    }

    scope(failure)
    {
        static if (is(typeof(() pure { return construct(); })))
        {
            // Assume deallocation is safe because:
            // 1) in case of failure, `m` is the only reference to this memory
            // 2) `m` is known to originate from `alloc`
            () @trusted { alloc.deallocate(m); }();
        }
        else
        {
            alloc.deallocate(m);
        }
    }

    return construct();
}

///
@system unittest
{
    // Dynamically allocate one integer
    const int* p1 = theAllocator.make!int;
    // It's implicitly initialized with its .init value
    assert(*p1 == 0);
    // Dynamically allocate one double, initialize to 42.5
    const double* p2 = theAllocator.make!double(42.5);
    assert(*p2 == 42.5);

    // Dynamically allocate a struct
    static struct Point
    {
        int x, y, z;
    }
    // Use the generated constructor taking field values in order
    const Point* p = theAllocator.make!Point(1, 2);
    assert(p.x == 1 && p.y == 2 && p.z == 0);

    // Dynamically allocate a class object
    static class Customer
    {
        uint id = uint.max;
        this() {}
        this(uint id) { this.id = id; }
        // ...
    }
    Customer cust = theAllocator.make!Customer;
    assert(cust.id == uint.max); // default initialized
    cust = theAllocator.make!Customer(42);
    assert(cust.id == 42);

    // explicit passing of outer pointer
    static class Outer
    {
        int x = 3;
        class Inner
        {
            auto getX() { return x; }
        }
    }
    auto outer = theAllocator.make!Outer();
    auto inner = theAllocator.make!(Outer.Inner)(outer);
    assert(outer.x == inner.getX);
}

@system unittest // bugzilla 15639 & 15772
{
    abstract class Foo {}
    class Bar: Foo {}
    static assert(!is(typeof(theAllocator.make!Foo)));
    static assert( is(typeof(theAllocator.make!Bar)));
}

@system unittest
{
    void test(Allocator)(auto ref Allocator alloc)
    {
        const int* a = alloc.make!int(10);
        assert(*a == 10);

        struct A
        {
            int x;
            string y;
            double z;
        }

        A* b = alloc.make!A(42);
        assert(b.x == 42);
        assert(b.y is null);
        import std.math : isNaN;
        assert(b.z.isNaN);

        b = alloc.make!A(43, "44", 45);
        assert(b.x == 43);
        assert(b.y == "44");
        assert(b.z == 45);

        static class B
        {
            int x;
            string y;
            double z;
            this(int _x, string _y = null, double _z = double.init)
            {
                x = _x;
                y = _y;
                z = _z;
            }
        }

        B c = alloc.make!B(42);
        assert(c.x == 42);
        assert(c.y is null);
        assert(c.z.isNaN);

        c = alloc.make!B(43, "44", 45);
        assert(c.x == 43);
        assert(c.y == "44");
        assert(c.z == 45);

        const parray = alloc.make!(int[]);
        assert((*parray).empty);
    }

    import std.experimental.allocator.gc_allocator : GCAllocator;
    test(GCAllocator.instance);
    test(theAllocator);
}

// Attribute propagation
nothrow @safe @nogc unittest
{
    import std.experimental.allocator.mallocator : Mallocator;
    alias alloc = Mallocator.instance;

    void test(T, Args...)(auto ref Args args)
    {
        auto k = alloc.make!T(args);
        () @trusted { alloc.dispose(k); }();
    }

    test!int;
    test!(int*);
    test!int(0);
    test!(int*)(null);
}

// should be pure with the GCAllocator
/*pure nothrow*/ @safe unittest
{
    import std.experimental.allocator.gc_allocator : GCAllocator;

    alias alloc = GCAllocator.instance;

    void test(T, Args...)(auto ref Args args)
    {
        auto k = alloc.make!T(args);
        (a) @trusted { a.dispose(k); }(alloc);
    }

    test!int();
    test!(int*);
    test!int(0);
    test!(int*)(null);
}

// Verify that making an object by calling an impure constructor is not @safe
nothrow @safe @nogc unittest
{
    import std.experimental.allocator.mallocator : Mallocator;
    static struct Pure { this(int) pure nothrow @nogc @safe {} }

    cast(void) Mallocator.instance.make!Pure(0);

    static int g = 0;
    static struct Impure { this(int) nothrow @nogc @safe {
        g++;
    } }
    static assert(!__traits(compiles, cast(void) Mallocator.instance.make!Impure(0)));
}

// test failure with a pure, failing struct
@safe unittest
{
    import std.exception : assertThrown, enforce;

    // this struct can't be initialized
    struct InvalidStruct
    {
        this(int b)
        {
            enforce(1 == 2);
        }
    }
    import std.experimental.allocator.mallocator : Mallocator;
    assertThrown(make!InvalidStruct(Mallocator.instance, 42));
}

// test failure with an impure, failing struct
@system unittest
{
    import std.exception : assertThrown, enforce;
    static int g;
    struct InvalidImpureStruct
    {
        this(int b)
        {
            g++;
            enforce(1 == 2);
        }
    }
    import std.experimental.allocator.mallocator : Mallocator;
    assertThrown(make!InvalidImpureStruct(Mallocator.instance, 42));
}

private void fillWithMemcpy(T)(void[] array, auto ref T filler) nothrow
{
    import core.stdc.string : memcpy;
    import std.algorithm.comparison : min;
    if (!array.length) return;
    memcpy(array.ptr, &filler, T.sizeof);
    // Fill the array from the initialized portion of itself exponentially.
    for (size_t offset = T.sizeof; offset < array.length; )
    {
        size_t extent = min(offset, array.length - offset);
        memcpy(array.ptr + offset, array.ptr, extent);
        offset += extent;
    }
}

@system unittest
{
    int[] a;
    fillWithMemcpy(a, 42);
    assert(a.length == 0);
    a = [ 1, 2, 3, 4, 5 ];
    fillWithMemcpy(a, 42);
    assert(a == [ 42, 42, 42, 42, 42]);
}

private T[] uninitializedFillDefault(T)(T[] array) nothrow
{
    T t = T.init;
    fillWithMemcpy(array, t);
    return array;
}

pure nothrow @nogc
@system unittest
{
    static struct S { int x = 42; @disable this(this); }

    int[5] expected = [42, 42, 42, 42, 42];
    S[5] arr = void;
    uninitializedFillDefault(arr);
    assert((cast(int*) arr.ptr)[0 .. arr.length] == expected);
}

@system unittest
{
    int[] a = [1, 2, 4];
    uninitializedFillDefault(a);
    assert(a == [0, 0, 0]);
}

/**
Create an array of $(D T) with $(D length) elements using $(D alloc). The array is either default-initialized, filled with copies of $(D init), or initialized with values fetched from `range`.

Params:
T = element type of the array being created
alloc = the allocator used for getting memory
length = length of the newly created array
init = element used for filling the array
range = range used for initializing the array elements

Returns:
The newly-created array, or $(D null) if either $(D length) was $(D 0) or
allocation failed.

Throws:
The first two overloads throw only if `alloc`'s primitives do. The
overloads that involve copy initialization deallocate memory and propagate the
exception if the copy operation throws.
*/
T[] makeArray(T, Allocator)(auto ref Allocator alloc, size_t length)
{
    if (!length) return null;
    auto m = alloc.allocate(T.sizeof * length);
    if (!m.ptr) return null;
    alias U = Unqual!T;
    return () @trusted { return cast(T[]) uninitializedFillDefault(cast(U[]) m); }();
}

@system unittest
{
    void test1(A)(auto ref A alloc)
    {
        int[] a = alloc.makeArray!int(0);
        assert(a.length == 0 && a.ptr is null);
        a = alloc.makeArray!int(5);
        assert(a.length == 5);
        static immutable cheatsheet = [0, 0, 0, 0, 0];
        assert(a == cheatsheet);
    }

    void test2(A)(auto ref A alloc)
    {
        static struct S { int x = 42; @disable this(this); }
        S[] arr = alloc.makeArray!S(5);
        assert(arr.length == 5);
        int[] arrInt = () @trusted { return (cast(int*) arr.ptr)[0 .. 5]; }();
        static immutable res = [42, 42, 42, 42, 42];
        assert(arrInt == res);
    }

    import std.experimental.allocator.gc_allocator : GCAllocator;
    import std.experimental.allocator.mallocator : Mallocator;
    (alloc) /*pure nothrow*/ @safe { test1(alloc); test2(alloc);} (GCAllocator.instance);
    (alloc) nothrow @safe @nogc { test1(alloc); test2(alloc);} (Mallocator.instance);
    test2(theAllocator);
}

@system unittest
{
    import std.algorithm.comparison : equal;
    auto a = theAllocator.makeArray!(shared int)(5);
    static assert(is(typeof(a) == shared(int)[]));
    assert(a.length == 5);
    assert(a.equal([0, 0, 0, 0, 0]));

    auto b = theAllocator.makeArray!(const int)(5);
    static assert(is(typeof(b) == const(int)[]));
    assert(b.length == 5);
    assert(b.equal([0, 0, 0, 0, 0]));

    auto c = theAllocator.makeArray!(immutable int)(5);
    static assert(is(typeof(c) == immutable(int)[]));
    assert(c.length == 5);
    assert(c.equal([0, 0, 0, 0, 0]));
}

private enum hasPurePostblit(T) = !hasElaborateCopyConstructor!T ||
    is(typeof(() pure { T.init.__xpostblit(); }));

private enum hasPureDtor(T) = !hasElaborateDestructor!T ||
    is(typeof(() pure { T.init.__xdtor(); }));

// `true` when postblit and destructor of T cannot escape references to itself
private enum canSafelyDeallocPostRewind(T) = hasPurePostblit!T && hasPureDtor!T;

/// Ditto
T[] makeArray(T, Allocator)(auto ref Allocator alloc, size_t length,
    auto ref T init)
{
    if (!length) return null;
    auto m = alloc.allocate(T.sizeof * length);
    if (!m.ptr) return null;
    auto result = () @trusted { return cast(T[]) m; } ();
    import std.traits : hasElaborateCopyConstructor;
    static if (hasElaborateCopyConstructor!T)
    {
        scope(failure)
        {
            static if (canSafelyDeallocPostRewind!T)
                () @trusted { alloc.deallocate(m); } ();
            else
                alloc.deallocate(m);
        }

        size_t i = 0;
        static if (hasElaborateDestructor!T)
        {
            scope (failure)
            {
                foreach (j; 0 .. i)
                {
                    destroy(result[j]);
                }
            }
        }
        import std.conv : emplace;
        for (; i < length; ++i)
        {
            emplace!T(&result[i], init);
        }
    }
    else
    {
        alias U = Unqual!T;
        () @trusted { fillWithMemcpy(cast(U[]) result, *(cast(U*) &init)); }();
    }
    return result;
}

///
@system unittest
{
    import std.algorithm.comparison : equal;
    static void test(T)()
    {
        T[] a = theAllocator.makeArray!T(2);
        assert(a.equal([0, 0]));
        a = theAllocator.makeArray!T(3, 42);
        assert(a.equal([42, 42, 42]));
        import std.range : only;
        a = theAllocator.makeArray!T(only(42, 43, 44));
        assert(a.equal([42, 43, 44]));
    }
    test!int();
    test!(shared int)();
    test!(const int)();
    test!(immutable int)();
}

@system unittest
{
    void test(A)(auto ref A alloc)
    {
        long[] a = alloc.makeArray!long(0, 42);
        assert(a.length == 0 && a.ptr is null);
        a = alloc.makeArray!long(5, 42);
        assert(a.length == 5);
        assert(a == [ 42, 42, 42, 42, 42 ]);
    }
    import std.experimental.allocator.gc_allocator : GCAllocator;
    (alloc) /*pure nothrow*/ @safe { test(alloc); } (GCAllocator.instance);
    test(theAllocator);
}

// test failure with a pure, failing struct
@safe unittest
{
    import std.exception : assertThrown, enforce;

    struct NoCopy
    {
        @disable this();

        this(int b){}

        // can't be copied
        this(this)
        {
            enforce(1 == 2);
        }
    }
    import std.experimental.allocator.mallocator : Mallocator;
    assertThrown(makeArray!NoCopy(Mallocator.instance, 10, NoCopy(42)));
}

// test failure with an impure, failing struct
@system unittest
{
    import std.exception : assertThrown, enforce;

    static int i = 0;
    struct Singleton
    {
        @disable this();

        this(int b){}

        // can't be copied
        this(this)
        {
            enforce(i++ == 0);
        }

        ~this()
        {
            i--;
        }
    }
    import std.experimental.allocator.mallocator : Mallocator;
    assertThrown(makeArray!Singleton(Mallocator.instance, 10, Singleton(42)));
}

/// Ditto
Unqual!(ElementEncodingType!R)[] makeArray(Allocator, R)(auto ref Allocator alloc, R range)
if (isInputRange!R && !isInfinite!R)
{
    alias T = Unqual!(ElementEncodingType!R);
    return makeArray!(T, Allocator, R)(alloc, range);
}

/// Ditto
T[] makeArray(T, Allocator, R)(auto ref Allocator alloc, R range)
if (isInputRange!R && !isInfinite!R)
{
    static if (isForwardRange!R || hasLength!R)
    {
        static if (hasLength!R || isNarrowString!R)
            immutable length = range.length;
        else
            immutable length = range.save.walkLength;

        if (!length) return null;
        auto m = alloc.allocate(T.sizeof * length);
        if (!m.ptr) return null;
        auto result = () @trusted { return cast(T[]) m; } ();

        size_t i = 0;
        scope (failure)
        {
            foreach (j; 0 .. i)
            {
                auto p = () @trusted { return cast(Unqual!T*) &result[j]; }();
                destroy(p);
            }

            static if (canSafelyDeallocPostRewind!T)
                () @trusted { alloc.deallocate(m); } ();
            else
                alloc.deallocate(m);
        }

        import std.conv : emplaceRef;
        static if (isNarrowString!R || isRandomAccessRange!R)
        {
            foreach (j; 0 .. range.length)
            {
                emplaceRef!T(result[i++], range[j]);
            }
        }
        else
        {
            for (; !range.empty; range.popFront, ++i)
            {
                emplaceRef!T(result[i], range.front);
            }
        }

        return result;
    }
    else
    {
        // Estimated size
        size_t estimated = 8;
        auto m = alloc.allocate(T.sizeof * estimated);
        if (!m.ptr) return null;
        auto result = () @trusted { return cast(T[]) m; } ();

        size_t initialized = 0;
        void bailout()
        {
            foreach (i; 0 .. initialized + 1)
            {
                destroy(result[i]);
            }

            static if (canSafelyDeallocPostRewind!T)
                () @trusted { alloc.deallocate(m); } ();
            else
                alloc.deallocate(m);
        }
        scope (failure) bailout;

        for (; !range.empty; range.popFront, ++initialized)
        {
            if (initialized == estimated)
            {
                // Need to reallocate
                static if (hasPurePostblit!T)
                    auto success = () @trusted { return alloc.reallocate(m, T.sizeof * (estimated *= 2)); } ();
                else
                    auto success = alloc.reallocate(m, T.sizeof * (estimated *= 2));
                if (!success)
                {
                    bailout;
                    return null;
                }
                result = () @trusted { return cast(T[]) m; } ();
            }
            import std.conv : emplaceRef;
            emplaceRef(result[initialized], range.front);
        }

        if (initialized < estimated)
        {
            // Try to shrink memory, no harm if not possible
            static if (hasPurePostblit!T)
                auto success = () @trusted { return alloc.reallocate(m, T.sizeof * initialized); } ();
            else
                auto success = alloc.reallocate(m, T.sizeof * initialized);
            if (success)
                result = () @trusted { return cast(T[]) m; } ();
        }

        return result[0 .. initialized];
    }
}

@system unittest
{
    void test(A)(auto ref A alloc)
    {
        long[] a = alloc.makeArray!long((int[]).init);
        assert(a.length == 0 && a.ptr is null);
        a = alloc.makeArray!long([5, 42]);
        assert(a.length == 2);
        assert(a == [ 5, 42]);

        // we can also infer the type
        auto b = alloc.makeArray([4.0, 2.0]);
        static assert(is(typeof(b) == double[]));
        assert(b == [4.0, 2.0]);
    }
    import std.experimental.allocator.gc_allocator : GCAllocator;
    (alloc) pure nothrow @safe { test(alloc); } (GCAllocator.instance);
    test(theAllocator);
}

// infer types for strings
@system unittest
{
    void test(A)(auto ref A alloc)
    {
        auto c = alloc.makeArray("fooπ😜");
        static assert(is(typeof(c) == char[]));
        assert(c == "fooπ😜");

        auto d = alloc.makeArray("fooπ😜"d);
        static assert(is(typeof(d) == dchar[]));
        assert(d == "fooπ😜");

        auto w = alloc.makeArray("fooπ😜"w);
        static assert(is(typeof(w) == wchar[]));
        assert(w == "fooπ😜");
    }

    import std.experimental.allocator.gc_allocator : GCAllocator;
    (alloc) pure nothrow @safe { test(alloc); } (GCAllocator.instance);
    test(theAllocator);
}

/*pure*/ nothrow @safe unittest
{
    import std.algorithm.comparison : equal;
    import std.experimental.allocator.gc_allocator : GCAllocator;
    import std.internal.test.dummyrange;
    import std.range : iota;
    foreach (DummyType; AllDummyRanges)
    {
        (alloc) pure nothrow @safe
        {
            DummyType d;
            auto arr = alloc.makeArray(d);
            assert(arr.length == 10);
            assert(arr.equal(iota(1, 11)));
        } (GCAllocator.instance);
    }
}

// test failure with a pure, failing struct
@safe unittest
{
    import std.exception : assertThrown, enforce;

    struct NoCopy
    {
        int b;

        @disable this();

        this(int b)
        {
            this.b = b;
        }

        // can't be copied
        this(this)
        {
            enforce(b < 3, "there can only be three elements");
        }
    }
    import std.experimental.allocator.mallocator : Mallocator;
    auto arr = [NoCopy(1), NoCopy(2), NoCopy(3)];
    assertThrown(makeArray!NoCopy(Mallocator.instance, arr));

    struct NoCopyRange
    {
        static j = 0;
        bool empty()
        {
            return j > 5;
        }

        auto front()
        {
            return NoCopy(j);
        }

        void popFront()
        {
            j++;
        }
    }
    assertThrown(makeArray!NoCopy(Mallocator.instance, NoCopyRange()));
}

// test failure with an impure, failing struct
@system unittest
{
    import std.exception : assertThrown, enforce;

    static i = 0;
    static maxElements = 2;
    struct NoCopy
    {
        int val;
        @disable this();

        this(int b){
            this.val = i++;
        }

        // can't be copied
        this(this)
        {
            enforce(i++ < maxElements, "there can only be four elements");
        }
    }

    import std.experimental.allocator.mallocator : Mallocator;
    auto arr = [NoCopy(1), NoCopy(2)];
    assertThrown(makeArray!NoCopy(Mallocator.instance, arr));

    // allow more copies and thus force reallocation
    i = 0;
    maxElements = 30;
    static j = 0;

    struct NoCopyRange
    {
        bool empty()
        {
            return j > 100;
        }

        auto front()
        {
            return NoCopy(1);
        }

        void popFront()
        {
            j++;
        }
    }
    assertThrown(makeArray!NoCopy(Mallocator.instance, NoCopyRange()));

    maxElements = 300;
    auto arr2 = makeArray!NoCopy(Mallocator.instance, NoCopyRange());

    import std.algorithm.comparison : equal;
    import std.algorithm.iteration : map;
    import std.range : iota;
    assert(arr2.map!`a.val`.equal(iota(32, 204, 2)));
}

version (unittest)
{
    private struct ForcedInputRange
    {
        int[]* array;
        pure nothrow @safe @nogc:
        bool empty() { return !array || (*array).empty; }
        ref int front() { return (*array)[0]; }
        void popFront() { *array = (*array)[1 .. $]; }
    }
}

@system unittest
{
    import std.array : array;
    import std.range : iota;
    int[] arr = iota(10).array;

    void test(A)(auto ref A alloc)
    {
        ForcedInputRange r;
        long[] a = alloc.makeArray!long(r);
        assert(a.length == 0 && a.ptr is null);
        auto arr2 = arr;
        r.array = () @trusted { return &arr2; } ();
        a = alloc.makeArray!long(r);
        assert(a.length == 10);
        assert(a == iota(10).array);
    }
    import std.experimental.allocator.gc_allocator : GCAllocator;
    (alloc) pure nothrow @safe { test(alloc); } (GCAllocator.instance);
    test(theAllocator);
}

/**
Grows $(D array) by appending $(D delta) more elements. The needed memory is
allocated using $(D alloc). The extra elements added are either default-
initialized, filled with copies of $(D init), or initialized with values
fetched from `range`.

Params:
T = element type of the array being created
alloc = the allocator used for getting memory
array = a reference to the array being grown
delta = number of elements to add (upon success the new length of $(D array) is
$(D array.length + delta))
init = element used for filling the array
range = range used for initializing the array elements

Returns:
$(D true) upon success, $(D false) if memory could not be allocated. In the
latter case $(D array) is left unaffected.

Throws:
The first two overloads throw only if `alloc`'s primitives do. The
overloads that involve copy initialization deallocate memory and propagate the
exception if the copy operation throws.
*/
bool expandArray(T, Allocator)(auto ref Allocator alloc, ref T[] array,
        size_t delta)
{
    if (!delta) return true;
    if (array is null) return false;
    immutable oldLength = array.length;
    void[] buf = array;
    if (!alloc.reallocate(buf, buf.length + T.sizeof * delta)) return false;
    array = cast(T[]) buf;
    array[oldLength .. $].uninitializedFillDefault;
    return true;
}

@system unittest
{
    void test(A)(auto ref A alloc)
    {
        auto arr = alloc.makeArray!int([1, 2, 3]);
        assert(alloc.expandArray(arr, 3));
        assert(arr == [1, 2, 3, 0, 0, 0]);
    }
    import std.experimental.allocator.gc_allocator : GCAllocator;
    test(GCAllocator.instance);
    test(theAllocator);
}

/// Ditto
bool expandArray(T, Allocator)(auto ref Allocator alloc, ref T[] array,
    size_t delta, auto ref T init)
{
    if (!delta) return true;
    if (array is null) return false;
    void[] buf = array;
    if (!alloc.reallocate(buf, buf.length + T.sizeof * delta)) return false;
    immutable oldLength = array.length;
    array = cast(T[]) buf;
    scope(failure) array[oldLength .. $].uninitializedFillDefault;
    import std.algorithm.mutation : uninitializedFill;
    array[oldLength .. $].uninitializedFill(init);
    return true;
}

@system unittest
{
    void test(A)(auto ref A alloc)
    {
        auto arr = alloc.makeArray!int([1, 2, 3]);
        assert(alloc.expandArray(arr, 3, 1));
        assert(arr == [1, 2, 3, 1, 1, 1]);
    }
    import std.experimental.allocator.gc_allocator : GCAllocator;
    test(GCAllocator.instance);
    test(theAllocator);
}

/// Ditto
bool expandArray(T, Allocator, R)(auto ref Allocator alloc, ref T[] array,
        R range)
if (isInputRange!R)
{
    if (array is null) return false;
    static if (isForwardRange!R)
    {
        immutable delta = walkLength(range.save);
        if (!delta) return true;
        immutable oldLength = array.length;

        // Reallocate support memory
        void[] buf = array;
        if (!alloc.reallocate(buf, buf.length + T.sizeof * delta))
        {
            return false;
        }
        array = cast(T[]) buf;
        // At this point we're committed to the new length.

        auto toFill = array[oldLength .. $];
        scope (failure)
        {
            // Fill the remainder with default-constructed data
            toFill.uninitializedFillDefault;
        }

        for (; !range.empty; range.popFront, toFill.popFront)
        {
            assert(!toFill.empty);
            import std.conv : emplace;
            emplace!T(&toFill.front, range.front);
        }
        assert(toFill.empty);
    }
    else
    {
        scope(failure)
        {
            // The last element didn't make it, fill with default
            array[$ - 1 .. $].uninitializedFillDefault;
        }
        void[] buf = array;
        for (; !range.empty; range.popFront)
        {
            if (!alloc.reallocate(buf, buf.length + T.sizeof))
            {
                array = cast(T[]) buf;
                return false;
            }
            import std.conv : emplace;
            emplace!T(buf[$ - T.sizeof .. $], range.front);
        }

        array = cast(T[]) buf;
    }
    return true;
}

///
@system unittest
{
    auto arr = theAllocator.makeArray!int([1, 2, 3]);
    assert(theAllocator.expandArray(arr, 2));
    assert(arr == [1, 2, 3, 0, 0]);
    import std.range : only;
    assert(theAllocator.expandArray(arr, only(4, 5)));
    assert(arr == [1, 2, 3, 0, 0, 4, 5]);
}

@system unittest
{
    auto arr = theAllocator.makeArray!int([1, 2, 3]);
    ForcedInputRange r;
    int[] b = [ 1, 2, 3, 4 ];
    auto temp = b;
    r.array = &temp;
    assert(theAllocator.expandArray(arr, r));
    assert(arr == [1, 2, 3, 1, 2, 3, 4]);
}

/**
Shrinks an array by $(D delta) elements.

If $(D array.length < delta), does nothing and returns `false`. Otherwise,
destroys the last $(D array.length - delta) elements in the array and then
reallocates the array's buffer. If reallocation fails, fills the array with
default-initialized data.

Params:
T = element type of the array being created
alloc = the allocator used for getting memory
array = a reference to the array being shrunk
delta = number of elements to remove (upon success the new length of $(D array) is $(D array.length - delta))

Returns:
`true` upon success, `false` if memory could not be reallocated. In the latter
case, the slice $(D array[$ - delta .. $]) is left with default-initialized
elements.

Throws:
The first two overloads throw only if `alloc`'s primitives do. The
overloads that involve copy initialization deallocate memory and propagate the
exception if the copy operation throws.
*/
bool shrinkArray(T, Allocator)(auto ref Allocator alloc,
        ref T[] array, size_t delta)
{
    if (delta > array.length) return false;

    // Destroy elements. If a destructor throws, fill the already destroyed
    // stuff with the default initializer.
    {
        size_t destroyed;
        scope(failure)
        {
            array[$ - delta .. $][0 .. destroyed].uninitializedFillDefault;
        }
        foreach (ref e; array[$ - delta .. $])
        {
            e.destroy;
            ++destroyed;
        }
    }

    if (delta == array.length)
    {
        alloc.deallocate(array);
        array = null;
        return true;
    }

    void[] buf = array;
    if (!alloc.reallocate(buf, buf.length - T.sizeof * delta))
    {
        // urgh, at least fill back with default
        array[$ - delta .. $].uninitializedFillDefault;
        return false;
    }
    array = cast(T[]) buf;
    return true;
}

///
@system unittest
{
    int[] a = theAllocator.makeArray!int(100, 42);
    assert(a.length == 100);
    assert(theAllocator.shrinkArray(a, 98));
    assert(a.length == 2);
    assert(a == [42, 42]);
}

@system unittest
{
    void test(A)(auto ref A alloc)
    {
        long[] a = alloc.makeArray!long((int[]).init);
        assert(a.length == 0 && a.ptr is null);
        a = alloc.makeArray!long(100, 42);
        assert(alloc.shrinkArray(a, 98));
        assert(a.length == 2);
        assert(a == [ 42, 42]);
    }
    import std.experimental.allocator.gc_allocator : GCAllocator;
    test(GCAllocator.instance);
    test(theAllocator);
}

/**

Destroys and then deallocates (using $(D alloc)) the object pointed to by a
pointer, the class object referred to by a $(D class) or $(D interface)
reference, or an entire array. It is assumed the respective entities had been
allocated with the same allocator.

*/
void dispose(A, T)(auto ref A alloc, T* p)
{
    static if (hasElaborateDestructor!T)
    {
        destroy(*p);
    }
    alloc.deallocate((cast(void*) p)[0 .. T.sizeof]);
}

/// Ditto
void dispose(A, T)(auto ref A alloc, T p)
if (is(T == class) || is(T == interface))
{
    if (!p) return;
    static if (is(T == interface))
    {
        version (Windows)
        {
            import core.sys.windows.unknwn : IUnknown;
            static assert(!is(T: IUnknown), "COM interfaces can't be destroyed in "
                ~ __PRETTY_FUNCTION__);
        }
        auto ob = cast(Object) p;
    }
    else
        alias ob = p;
    auto support = (cast(void*) ob)[0 .. typeid(ob).initializer.length];
    destroy(p);
    alloc.deallocate(support);
}

/// Ditto
void dispose(A, T)(auto ref A alloc, T[] array)
{
    static if (hasElaborateDestructor!(typeof(array[0])))
    {
        foreach (ref e; array)
        {
            destroy(e);
        }
    }
    alloc.deallocate(array);
}

@system unittest
{
    static int x;
    static interface I
    {
        void method();
    }
    static class A : I
    {
        int y;
        override void method() { x = 21; }
        ~this() { x = 42; }
    }
    static class B : A
    {
    }
    auto a = theAllocator.make!A;
    a.method();
    assert(x == 21);
    theAllocator.dispose(a);
    assert(x == 42);

    B b = theAllocator.make!B;
    b.method();
    assert(x == 21);
    theAllocator.dispose(b);
    assert(x == 42);

    I i = theAllocator.make!B;
    i.method();
    assert(x == 21);
    theAllocator.dispose(i);
    assert(x == 42);

    int[] arr = theAllocator.makeArray!int(43);
    theAllocator.dispose(arr);
}

@system unittest //bugzilla 15721
{
    import std.experimental.allocator.mallocator : Mallocator;

    interface Foo {}
    class Bar: Foo {}

    Bar bar;
    Foo foo;
    bar = Mallocator.instance.make!Bar;
    foo = cast(Foo) bar;
    Mallocator.instance.dispose(foo);
}

/**
Allocates a multidimensional array of elements of type T.

Params:
N = number of dimensions
T = element type of an element of the multidimensional arrat
alloc = the allocator used for getting memory
lengths = static array containing the size of each dimension

Returns:
An N-dimensional array with individual elements of type T.
*/
auto makeMultidimensionalArray(T, Allocator, size_t N)(auto ref Allocator alloc, size_t[N] lengths...)
{
    static if (N == 1)
    {
        return makeArray!T(alloc, lengths[0]);
    }
    else
    {
        alias E = typeof(makeMultidimensionalArray!(T, Allocator, N - 1)(alloc, lengths[1 .. $]));
        auto ret = makeArray!E(alloc, lengths[0]);
        foreach (ref e; ret)
            e = makeMultidimensionalArray!(T, Allocator, N - 1)(alloc, lengths[1 .. $]);
        return ret;
    }
}

///
@system unittest
{
    import std.experimental.allocator.mallocator : Mallocator;

    auto mArray = Mallocator.instance.makeMultidimensionalArray!int(2, 3, 6);

    // deallocate when exiting scope
    scope(exit)
    {
        Mallocator.instance.disposeMultidimensionalArray(mArray);
    }

    assert(mArray.length == 2);
    foreach (lvl2Array; mArray)
    {
        assert(lvl2Array.length == 3);
        foreach (lvl3Array; lvl2Array)
            assert(lvl3Array.length == 6);
    }
}

/**
Destroys and then deallocates a multidimensional array, assuming it was
created with makeMultidimensionalArray and the same allocator was used.

Params:
T = element type of an element of the multidimensional array
alloc = the allocator used for getting memory
array = the multidimensional array that is to be deallocated
*/
void disposeMultidimensionalArray(T, Allocator)(auto ref Allocator alloc, T[] array)
{
    static if (isArray!T)
    {
        foreach (ref e; array)
            disposeMultidimensionalArray(alloc, e);
    }

    dispose(alloc, array);
}

///
@system unittest
{
    struct TestAllocator
    {
        import std.experimental.allocator.common : platformAlignment;
        import std.experimental.allocator.mallocator : Mallocator;

        alias allocator = Mallocator.instance;

        private static struct ByteRange
        {
            void* ptr;
            size_t length;
        }

        private ByteRange[] _allocations;

        enum uint alignment = platformAlignment;

        void[] allocate(size_t numBytes)
        {
             auto ret = allocator.allocate(numBytes);
             _allocations ~= ByteRange(ret.ptr, ret.length);
             return ret;
        }

        bool deallocate(void[] bytes)
        {
            import std.algorithm.mutation : remove;
            import std.algorithm.searching : canFind;

            bool pred(ByteRange other)
            { return other.ptr == bytes.ptr && other.length == bytes.length; }

            assert(_allocations.canFind!pred);

             _allocations = _allocations.remove!pred;
             return allocator.deallocate(bytes);
        }

        ~this()
        {
            assert(!_allocations.length);
        }
    }

    TestAllocator allocator;

    auto mArray = allocator.makeMultidimensionalArray!int(2, 3, 5, 6, 7, 2);

    allocator.disposeMultidimensionalArray(mArray);
}

/**

Returns a dynamically-typed $(D CAllocator) built around a given statically-
typed allocator $(D a) of type $(D A). Passing a pointer to the allocator
creates a dynamic allocator around the allocator pointed to by the pointer,
without attempting to copy or move it. Passing the allocator by value or
reference behaves as follows.

$(UL
$(LI If $(D A) has no state, the resulting object is allocated in static
shared storage.)
$(LI If $(D A) has state and is copyable, the result will store a copy of it
within. The result itself is allocated in its own statically-typed allocator.)
$(LI If $(D A) has state and is not copyable, the result will move the
passed-in argument into the result. The result itself is allocated in its own
statically-typed allocator.)
)

*/
CAllocatorImpl!A allocatorObject(A)(auto ref A a)
if (!isPointer!A)
{
    import std.conv : emplace;
    static if (stateSize!A == 0)
    {
        enum s = stateSize!(CAllocatorImpl!A).divideRoundUp(ulong.sizeof);
        static __gshared ulong[s] state;
        static __gshared CAllocatorImpl!A result;
        if (!result)
        {
            // Don't care about a few races
            result = emplace!(CAllocatorImpl!A)(state[]);
        }
        assert(result);
        return result;
    }
    else static if (is(typeof({ A b = a; A c = b; }))) // copyable
    {
        auto state = a.allocate(stateSize!(CAllocatorImpl!A));
        import std.traits : hasMember;
        static if (hasMember!(A, "deallocate"))
        {
            scope(failure) a.deallocate(state);
        }
        return cast(CAllocatorImpl!A) emplace!(CAllocatorImpl!A)(state);
    }
    else // the allocator object is not copyable
    {
        // This is sensitive... create on the stack and then move
        enum s = stateSize!(CAllocatorImpl!A).divideRoundUp(ulong.sizeof);
        ulong[s] state;
        import std.algorithm.mutation : move;
        emplace!(CAllocatorImpl!A)(state[], move(a));
        auto dynState = a.allocate(stateSize!(CAllocatorImpl!A));
        // Bitblast the object in its final destination
        dynState[] = state[];
        return cast(CAllocatorImpl!A) dynState.ptr;
    }
}

/// Ditto
CAllocatorImpl!(A, Yes.indirect) allocatorObject(A)(A* pa)
{
    assert(pa);
    import std.conv : emplace;
    auto state = pa.allocate(stateSize!(CAllocatorImpl!(A, Yes.indirect)));
    import std.traits : hasMember;
    static if (hasMember!(A, "deallocate"))
    {
        scope(failure) pa.deallocate(state);
    }
    return emplace!(CAllocatorImpl!(A, Yes.indirect))
        (state, pa);
}

///
@system unittest
{
    import std.experimental.allocator.mallocator : Mallocator;
    IAllocator a = allocatorObject(Mallocator.instance);
    auto b = a.allocate(100);
    assert(b.length == 100);
    assert(a.deallocate(b));

    // The in-situ region must be used by pointer
    import std.experimental.allocator.building_blocks.region : InSituRegion;
    auto r = InSituRegion!1024();
    a = allocatorObject(&r);
    b = a.allocate(200);
    assert(b.length == 200);
    // In-situ regions can deallocate the last allocation
    assert(a.deallocate(b));
}

/**

Returns a dynamically-typed $(D CSharedAllocator) built around a given statically-
typed allocator $(D a) of type $(D A). Passing a pointer to the allocator
creates a dynamic allocator around the allocator pointed to by the pointer,
without attempting to copy or move it. Passing the allocator by value or
reference behaves as follows.

$(UL
$(LI If $(D A) has no state, the resulting object is allocated in static
shared storage.)
$(LI If $(D A) has state and is copyable, the result will store a copy of it
within. The result itself is allocated in its own statically-typed allocator.)
$(LI If $(D A) has state and is not copyable, the result will move the
passed-in argument into the result. The result itself is allocated in its own
statically-typed allocator.)
)

*/
shared(CSharedAllocatorImpl!A) sharedAllocatorObject(A)(auto ref A a)
if (!isPointer!A)
{
    import std.conv : emplace;
    static if (stateSize!A == 0)
    {
        enum s = stateSize!(CSharedAllocatorImpl!A).divideRoundUp(ulong.sizeof);
        static __gshared ulong[s] state;
        static shared CSharedAllocatorImpl!A result;
        if (!result)
        {
            // Don't care about a few races
            result = cast(shared
                    CSharedAllocatorImpl!A)(emplace!(CSharedAllocatorImpl!A)(state[]));
        }
        assert(result);
        return result;
    }
    else static if (is(typeof({ shared A b = a; shared A c = b; }))) // copyable
    {
        auto state = a.allocate(stateSize!(CSharedAllocatorImpl!A));
        import std.traits : hasMember;
        static if (hasMember!(A, "deallocate"))
        {
            scope(failure) a.deallocate(state);
        }
        return emplace!(shared CSharedAllocatorImpl!A)(state);
    }
    else // the allocator object is not copyable
    {
        assert(0, "Not yet implemented");
    }
}

/// Ditto
shared(CSharedAllocatorImpl!(A, Yes.indirect)) sharedAllocatorObject(A)(A* pa)
{
    assert(pa);
    import std.conv : emplace;
    auto state = pa.allocate(stateSize!(CSharedAllocatorImpl!(A, Yes.indirect)));
    import std.traits : hasMember;
    static if (hasMember!(A, "deallocate"))
    {
        scope(failure) pa.deallocate(state);
    }
    return emplace!(shared CSharedAllocatorImpl!(A, Yes.indirect))(state, pa);
}


/**

Implementation of `IAllocator` using `Allocator`. This adapts a
statically-built allocator type to `IAllocator` that is directly usable by
non-templated code.

Usually `CAllocatorImpl` is used indirectly by calling $(LREF theAllocator).
*/
class CAllocatorImpl(Allocator, Flag!"indirect" indirect = No.indirect)
    : IAllocator
{
    import std.traits : hasMember;

    /**
    The implementation is available as a public member.
    */
    static if (indirect)
    {
        private Allocator* pimpl;
        ref Allocator impl()
        {
            return *pimpl;
        }
        this(Allocator* pa)
        {
            pimpl = pa;
        }
    }
    else
    {
        static if (stateSize!Allocator) Allocator impl;
        else alias impl = Allocator.instance;
    }

    /// Returns `impl.alignment`.
    override @property uint alignment()
    {
        return impl.alignment;
    }

    /**
    Returns `impl.goodAllocSize(s)`.
    */
    override size_t goodAllocSize(size_t s)
    {
        return impl.goodAllocSize(s);
    }

    /**
    Returns `impl.allocate(s)`.
    */
    override void[] allocate(size_t s, TypeInfo ti = null)
    {
        return impl.allocate(s);
    }

    /**
    If `impl.alignedAllocate` exists, calls it and returns the result.
    Otherwise, always returns `null`.
    */
    override void[] alignedAllocate(size_t s, uint a)
    {
        static if (hasMember!(Allocator, "alignedAllocate"))
            return impl.alignedAllocate(s, a);
        else
            return null;
    }

    /**
    If `Allocator` implements `owns`, forwards to it. Otherwise, returns
    `Ternary.unknown`.
    */
    override Ternary owns(void[] b)
    {
        static if (hasMember!(Allocator, "owns")) return impl.owns(b);
        else return Ternary.unknown;
    }

    /// Returns $(D impl.expand(b, s)) if defined, `false` otherwise.
    override bool expand(ref void[] b, size_t s)
    {
        static if (hasMember!(Allocator, "expand"))
            return impl.expand(b, s);
        else
            return s == 0;
    }

    /// Returns $(D impl.reallocate(b, s)).
    override bool reallocate(ref void[] b, size_t s)
    {
        return impl.reallocate(b, s);
    }

    /// Forwards to `impl.alignedReallocate` if defined, `false` otherwise.
    bool alignedReallocate(ref void[] b, size_t s, uint a)
    {
        static if (!hasMember!(Allocator, "alignedAllocate"))
        {
            return false;
        }
        else
        {
            return impl.alignedReallocate(b, s, a);
        }
    }

    // Undocumented for now
    Ternary resolveInternalPointer(const void* p, ref void[] result)
    {
        static if (hasMember!(Allocator, "resolveInternalPointer"))
        {
            return impl.resolveInternalPointer(p, result);
        }
        else
        {
            return Ternary.unknown;
        }
    }

    /**
    If `impl.deallocate` is not defined, returns `false`. Otherwise it forwards
    the call.
    */
    override bool deallocate(void[] b)
    {
        static if (hasMember!(Allocator, "deallocate"))
        {
            return impl.deallocate(b);
        }
        else
        {
            return false;
        }
    }

    /**
    Calls `impl.deallocateAll()` and returns the result if defined,
    otherwise returns `false`.
    */
    override bool deallocateAll()
    {
        static if (hasMember!(Allocator, "deallocateAll"))
        {
            return impl.deallocateAll();
        }
        else
        {
            return false;
        }
    }

    /**
    Forwards to `impl.empty()` if defined, otherwise returns `Ternary.unknown`.
    */
    override Ternary empty()
    {
        static if (hasMember!(Allocator, "empty"))
        {
            return Ternary(impl.empty);
        }
        else
        {
            return Ternary.unknown;
        }
    }

    /**
    Returns `impl.allocateAll()` if present, `null` otherwise.
    */
    override void[] allocateAll()
    {
        static if (hasMember!(Allocator, "allocateAll"))
        {
            return impl.allocateAll();
        }
        else
        {
            return null;
        }
    }
}

/**

Implementation of `ISharedAllocator` using `Allocator`. This adapts a
statically-built, shareable across threads, allocator type to `ISharedAllocator`
that is directly usable by non-templated code.

Usually `CSharedAllocatorImpl` is used indirectly by calling
$(LREF processAllocator).
*/
class CSharedAllocatorImpl(Allocator, Flag!"indirect" indirect = No.indirect)
    : ISharedAllocator
{
    import std.traits : hasMember;

    /**
    The implementation is available as a public member.
    */
    static if (indirect)
    {
        private shared Allocator* pimpl;
        ref Allocator impl() shared
        {
            return *pimpl;
        }
        this(Allocator* pa) shared
        {
            pimpl = pa;
        }
    }
    else
    {
        static if (stateSize!Allocator) shared Allocator impl;
        else alias impl = Allocator.instance;
    }

    /// Returns `impl.alignment`.
    override @property uint alignment() shared
    {
        return impl.alignment;
    }

    /**
    Returns `impl.goodAllocSize(s)`.
    */
    override size_t goodAllocSize(size_t s) shared
    {
        return impl.goodAllocSize(s);
    }

    /**
    Returns `impl.allocate(s)`.
    */
    override void[] allocate(size_t s, TypeInfo ti = null) shared
    {
        return impl.allocate(s);
    }

    /**
    If `impl.alignedAllocate` exists, calls it and returns the result.
    Otherwise, always returns `null`.
    */
    override void[] alignedAllocate(size_t s, uint a) shared
    {
        static if (hasMember!(Allocator, "alignedAllocate"))
            return impl.alignedAllocate(s, a);
        else
            return null;
    }

    /**
    If `Allocator` implements `owns`, forwards to it. Otherwise, returns
    `Ternary.unknown`.
    */
    override Ternary owns(void[] b) shared
    {
        static if (hasMember!(Allocator, "owns")) return impl.owns(b);
        else return Ternary.unknown;
    }

    /// Returns $(D impl.expand(b, s)) if defined, `false` otherwise.
    override bool expand(ref void[] b, size_t s) shared
    {
        static if (hasMember!(Allocator, "expand"))
            return impl.expand(b, s);
        else
            return s == 0;
    }

    /// Returns $(D impl.reallocate(b, s)).
    override bool reallocate(ref void[] b, size_t s) shared
    {
        return impl.reallocate(b, s);
    }

    /// Forwards to `impl.alignedReallocate` if defined, `false` otherwise.
    bool alignedReallocate(ref void[] b, size_t s, uint a) shared
    {
        static if (!hasMember!(Allocator, "alignedAllocate"))
        {
            return false;
        }
        else
        {
            return impl.alignedReallocate(b, s, a);
        }
    }

    // Undocumented for now
    Ternary resolveInternalPointer(const void* p, ref void[] result) shared
    {
        static if (hasMember!(Allocator, "resolveInternalPointer"))
        {
            return impl.resolveInternalPointer(p, result);
        }
        else
        {
            return Ternary.unknown;
        }
    }

    /**
    If `impl.deallocate` is not defined, returns `false`. Otherwise it forwards
    the call.
    */
    override bool deallocate(void[] b) shared
    {
        static if (hasMember!(Allocator, "deallocate"))
        {
            return impl.deallocate(b);
        }
        else
        {
            return false;
        }
    }

    /**
    Calls `impl.deallocateAll()` and returns the result if defined,
    otherwise returns `false`.
    */
    override bool deallocateAll() shared
    {
        static if (hasMember!(Allocator, "deallocateAll"))
        {
            return impl.deallocateAll();
        }
        else
        {
            return false;
        }
    }

    /**
    Forwards to `impl.empty()` if defined, otherwise returns `Ternary.unknown`.
    */
    override Ternary empty() shared
    {
        static if (hasMember!(Allocator, "empty"))
        {
            return Ternary(impl.empty);
        }
        else
        {
            return Ternary.unknown;
        }
    }

    /**
    Returns `impl.allocateAll()` if present, `null` otherwise.
    */
    override void[] allocateAll() shared
    {
        static if (hasMember!(Allocator, "allocateAll"))
        {
            return impl.allocateAll();
        }
        else
        {
            return null;
        }
    }
}


// Example in intro above
@system unittest
{
    // Allocate an int, initialize it with 42
    int* p = theAllocator.make!int(42);
    assert(*p == 42);

    // Destroy and deallocate it
    theAllocator.dispose(p);

    // Allocate using the global process allocator
    p = processAllocator.make!int(100);
    assert(*p == 100);

    // Destroy and deallocate
    processAllocator.dispose(p);

    // Create an array of 50 doubles initialized to -1.0
    double[] arr = theAllocator.makeArray!double(50, -1.0);

    // Check internal pointer
    void[] result;
    assert(theAllocator.resolveInternalPointer(null, result) == Ternary.no);
    Ternary r = theAllocator.resolveInternalPointer(arr.ptr, result);
    assert(result.ptr is arr.ptr && result.length >= arr.length);

    // Append two zeros to it
    theAllocator.expandArray(arr, 2, 0.0);
    // On second thought, take that back
    theAllocator.shrinkArray(arr, 2);
    // Destroy and deallocate
    theAllocator.dispose(arr);
}

__EOF__

/**

Stores an allocator object in thread-local storage (i.e. non-$(D shared) D
global). $(D ThreadLocal!A) is a subtype of $(D A) so it appears to implement
$(D A)'s allocator primitives.

$(D A) must hold state, otherwise $(D ThreadLocal!A) refuses instantiation. This
means e.g. $(D ThreadLocal!Mallocator) does not work because $(D Mallocator)'s
state is not stored as members of $(D Mallocator), but instead is hidden in the
C library implementation.

*/
struct ThreadLocal(A)
{
    static assert(stateSize!A,
        typeof(A).stringof
        ~ " does not have state so it cannot be used with ThreadLocal");

    /**
    The allocator instance.
    */
    static A instance;

    /**
    `ThreadLocal!A` is a subtype of `A` so it appears to implement `A`'s
    allocator primitives.
    */
    alias instance this;

    /**
    `ThreadLocal` disables all constructors. The intended usage is
    `ThreadLocal!A.instance`.
    */
    @disable this();
    /// Ditto
    @disable this(this);
}

///
unittest
{
    static assert(!is(ThreadLocal!Mallocator));
    static assert(!is(ThreadLocal!GCAllocator));
    alias ThreadLocal!(FreeList!(GCAllocator, 0, 8)) Allocator;
    auto b = Allocator.instance.allocate(5);
    static assert(hasMember!(Allocator, "allocate"));
}

/*
(Not public.)

A binary search tree that uses no allocation of its own. Instead, it relies on
user code to allocate nodes externally. Then $(D EmbeddedTree)'s primitives wire
the nodes appropriately.

Warning: currently $(D EmbeddedTree) is not using rebalancing, so it may
degenerate. A red-black tree implementation storing the color with one of the
pointers is planned for the future.
*/
private struct EmbeddedTree(T, alias less)
{
    static struct Node
    {
        T payload;
        Node* left, right;
    }

    private Node* root;

    private Node* insert(Node* n, ref Node* backref)
    {
        backref = n;
        n.left = n.right = null;
        return n;
    }

    Node* find(Node* data)
    {
        for (auto n = root; n; )
        {
            if (less(data, n))
            {
                n = n.left;
            }
            else if (less(n, data))
            {
                n = n.right;
            }
            else
            {
                return n;
            }
        }
        return null;
    }

    Node* insert(Node* data)
    {
        if (!root)
        {
            root = data;
            data.left = data.right = null;
            return root;
        }
        auto n = root;
        for (;;)
        {
            if (less(data, n))
            {
                if (!n.left)
                {
                    // Found insertion point
                    return insert(data, n.left);
                }
                n = n.left;
            }
            else if (less(n, data))
            {
                if (!n.right)
                {
                    // Found insertion point
                    return insert(data, n.right);
                }
                n = n.right;
            }
            else
            {
                // Found
                return n;
            }
            if (!n) return null;
        }
    }

    Node* remove(Node* data)
    {
        auto n = root;
        Node* parent = null;
        for (;;)
        {
            if (!n) return null;
            if (less(data, n))
            {
                parent = n;
                n = n.left;
            }
            else if (less(n, data))
            {
                parent = n;
                n = n.right;
            }
            else
            {
                // Found
                remove(n, parent);
                return n;
            }
        }
    }

    private void remove(Node* n, Node* parent)
    {
        assert(n);
        assert(!parent || parent.left == n || parent.right == n);
        Node** referrer = parent
            ? (parent.left == n ? &parent.left : &parent.right)
            : &root;
        if (!n.left)
        {
            *referrer = n.right;
        }
        else if (!n.right)
        {
            *referrer = n.left;
        }
        else
        {
            // Find the leftmost child in the right subtree
            auto leftmost = n.right;
            Node** leftmostReferrer = &n.right;
            while (leftmost.left)
            {
                leftmostReferrer = &leftmost.left;
                leftmost = leftmost.left;
            }
            // Unlink leftmost from there
            *leftmostReferrer = leftmost.right;
            // Link leftmost in lieu of n
            leftmost.left = n.left;
            leftmost.right = n.right;
            *referrer = leftmost;
        }
    }

    Ternary empty() const
    {
        return Ternary(!root);
    }

    void dump()
    {
        writeln(typeid(this), " @ ", cast(void*) &this);
        dump(root, 3);
    }

    void dump(Node* r, uint indent)
    {
        write(repeat(' ', indent).array);
        if (!r)
        {
            writeln("(null)");
            return;
        }
        writeln(r.payload, " @ ", cast(void*) r);
        dump(r.left, indent + 3);
        dump(r.right, indent + 3);
    }

    void assertSane()
    {
        static bool isBST(Node* r, Node* lb, Node* ub)
        {
            if (!r) return true;
            if (lb && !less(lb, r)) return false;
            if (ub && !less(r, ub)) return false;
            return isBST(r.left, lb, r) &&
                isBST(r.right, r, ub);
        }
        if (isBST(root, null, null)) return;
        dump;
        assert(0);
    }
}

unittest
{
    alias a = GCAllocator.instance;
    alias Tree = EmbeddedTree!(int, (a, b) => a.payload < b.payload);
    Tree t;
    assert(t.empty);
    int[] vals = [ 6, 3, 9, 1, 0, 2, 8, 11 ];
    foreach (v; vals)
    {
        auto n = new Tree.Node(v, null, null);
        assert(t.insert(n));
        assert(n);
        t.assertSane;
    }
    assert(!t.empty);
    foreach (v; vals)
    {
        Tree.Node n = { v };
        assert(t.remove(&n));
        t.assertSane;
    }
    assert(t.empty);
}

/*

$(D InternalPointersTree) adds a primitive on top of another allocator: calling
$(D resolveInternalPointer(p)) returns the block within which the internal
pointer $(D p) lies. Pointers right after the end of allocated blocks are also
considered internal.

The implementation stores three additional words with each allocation (one for
the block size and two for search management).

*/
private struct InternalPointersTree(Allocator)
{
    alias Tree = EmbeddedTree!(size_t,
        (a, b) => cast(void*) a + a.payload < cast(void*) b);
    alias Parent = AffixAllocator!(Allocator, Tree.Node);

    // Own state
    private Tree blockMap;

    alias alignment = Parent.alignment;

    /**
    The implementation is available as a public member.
    */
    static if (stateSize!Parent) Parent parent;
    else alias parent = Parent.instance;

    /// Allocator API.
    void[] allocate(size_t bytes)
    {
        auto r = parent.allocate(bytes);
        if (!r.ptr) return r;
        Tree.Node* n = &parent.prefix(r);
        n.payload = bytes;
        blockMap.insert(n) || assert(0);
        return r;
    }

    /// Ditto
    bool deallocate(void[] b)
    {
        if (!b.ptr) return;
        Tree.Node* n = &parent.prefix(b);
        blockMap.remove(n) || assert(false);
        parent.deallocate(b);
        return true;
    }

    /// Ditto
    static if (hasMember!(Allocator, "reallocate"))
    bool reallocate(ref void[] b, size_t s)
    {
        auto n = &parent.prefix(b);
        assert(n.payload == b.length);
        blockMap.remove(n) || assert(0);
        if (!parent.reallocate(b, s))
        {
            // Failed, must reinsert the same node in the tree
            assert(n.payload == b.length);
            blockMap.insert(n) || assert(0);
            return false;
        }
        // Insert the new node
        n = &parent.prefix(b);
        n.payload = s;
        blockMap.insert(n) || assert(0);
        return true;
    }

    /// Ditto
    Ternary owns(void[] b)
    {
        void[] result;
        return resolveInternalPointer(b.ptr, result);
    }

    /// Ditto
    Ternary empty()
    {
        return Ternary(blockMap.empty);
    }

    /** Returns the block inside which $(D p) resides, or $(D null) if the
    pointer does not belong.
    */
    Ternary resolveInternalPointer(const void* p, ref void[] result)
    {
        // Must define a custom find
        Tree.Node* find()
        {
            for (auto n = blockMap.root; n; )
            {
                if (p < n)
                {
                    n = n.left;
                }
                else if (p > (cast(void*) (n + 1)) + n.payload)
                {
                    n = n.right;
                }
                else
                {
                    return n;
                }
            }
            return null;
        }

        auto n = find();
        if (!n) return Ternary.no;
        result = (cast(void*) (n + 1))[0 .. n.payload];
        return Ternary.yes;
    }
}

unittest
{
    InternalPointersTree!(Mallocator) a;
    int[] vals = [ 6, 3, 9, 1, 2, 8, 11 ];
    void[][] allox;
    foreach (v; vals)
    {
        allox ~= a.allocate(v);
    }
    a.blockMap.assertSane;

    foreach (b; allox)
    {
        void[] p;
        Ternary r = a.resolveInternalPointer(b.ptr, p);
        assert(p.ptr is b.ptr && p.length >= b.length);
        r = a.resolveInternalPointer(b.ptr + b.length, p);
        assert(p.ptr is b.ptr && p.length >= b.length);
        r = a.resolveInternalPointer(b.ptr + b.length / 2, p);
        assert(p.ptr is b.ptr && p.length >= b.length);
        auto bogus = new void[b.length];
        assert(a.resolveInternalPointer(bogus.ptr, p) == Ternary.no);
    }

    foreach (b; allox.randomCover)
    {
        a.deallocate(b);
    }

    assert(a.empty);
}

//version (std_allocator_benchmark)
unittest
{
    static void testSpeed(A)()
    {
        static if (stateSize!A) A a;
        else alias a = A.instance;

        void[][128] bufs;

        import std.random;
        foreach (i; 0 .. 100_000)
        {
            auto j = uniform(0, bufs.length);
            switch (uniform(0, 2))
            {
            case 0:
                a.deallocate(bufs[j]);
                bufs[j] = a.allocate(uniform(0, 4096));
                break;
            case 1:
                a.deallocate(bufs[j]);
                bufs[j] = null;
                break;
            default:
                assert(0);
            }
        }
    }

    alias FList = FreeList!(GCAllocator, 0, unbounded);
    alias A = Segregator!(
        8, FreeList!(GCAllocator, 0, 8),
        128, Bucketizer!(FList, 1, 128, 16),
        256, Bucketizer!(FList, 129, 256, 32),
        512, Bucketizer!(FList, 257, 512, 64),
        1024, Bucketizer!(FList, 513, 1024, 128),
        2048, Bucketizer!(FList, 1025, 2048, 256),
        3584, Bucketizer!(FList, 2049, 3584, 512),
        4072 * 1024, AllocatorList!(
            (size_t n) => BitmappedBlock!(4096)(GCAllocator.instance.allocate(
                max(n, 4072 * 1024)))),
        GCAllocator
    );

    import std.datetime, std.experimental.allocator.null_allocator;
    if (false) writeln(benchmark!(
        testSpeed!NullAllocator,
        testSpeed!Mallocator,
        testSpeed!GCAllocator,
        testSpeed!(ThreadLocal!A),
        testSpeed!(A),
    )(20)[].map!(t => t.to!("seconds", double)));
}

unittest
{
    auto a = allocatorObject(Mallocator.instance);
    auto b = a.allocate(100);
    assert(b.length == 100);

    FreeList!(GCAllocator, 0, 8) fl;
    auto sa = allocatorObject(fl);
    b = a.allocate(101);
    assert(b.length == 101);

    FallbackAllocator!(InSituRegion!(10240, 64), GCAllocator) fb;
    // Doesn't work yet...
    //a = allocatorObject(fb);
    //b = a.allocate(102);
    //assert(b.length == 102);
}

///
unittest
{
    /// Define an allocator bound to the built-in GC.
    IAllocator alloc = allocatorObject(GCAllocator.instance);
    auto b = alloc.allocate(42);
    assert(b.length == 42);
    assert(alloc.deallocate(b) == Ternary.yes);

    // Define an elaborate allocator and bind it to the class API.
    // Note that the same variable "alloc" is used.
    alias FList = FreeList!(GCAllocator, 0, unbounded);
    alias A = ThreadLocal!(
        Segregator!(
            8, FreeList!(GCAllocator, 0, 8),
            128, Bucketizer!(FList, 1, 128, 16),
            256, Bucketizer!(FList, 129, 256, 32),
            512, Bucketizer!(FList, 257, 512, 64),
            1024, Bucketizer!(FList, 513, 1024, 128),
            2048, Bucketizer!(FList, 1025, 2048, 256),
            3584, Bucketizer!(FList, 2049, 3584, 512),
            4072 * 1024, AllocatorList!(
                (n) => BitmappedBlock!(4096)(GCAllocator.instance.allocate(
                    max(n, 4072 * 1024)))),
            GCAllocator
        )
    );

    auto alloc2 = allocatorObject(A.instance);
    b = alloc.allocate(101);
    assert(alloc.deallocate(b) == Ternary.yes);
}
