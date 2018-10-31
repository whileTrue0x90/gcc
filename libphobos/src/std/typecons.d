// Written in the D programming language.

/**
This module implements a variety of type constructors, i.e., templates
that allow construction of new, useful general-purpose types.

$(SCRIPT inhibitQuickIndex = 1;)
$(BOOKTABLE,
$(TR $(TH Category) $(TH Functions))
$(TR $(TD Tuple) $(TD
    $(LREF isTuple)
    $(LREF Tuple)
    $(LREF tuple)
    $(LREF reverse)
))
$(TR $(TD Flags) $(TD
    $(LREF BitFlags)
    $(LREF isBitFlagEnum)
    $(LREF Flag)
    $(LREF No)
    $(LREF Yes)
))
$(TR $(TD Memory allocation) $(TD
    $(LREF RefCounted)
    $(LREF refCounted)
    $(LREF RefCountedAutoInitialize)
    $(LREF scoped)
    $(LREF Unique)
))
$(TR $(TD Code generation) $(TD
    $(LREF AutoImplement)
    $(LREF BlackHole)
    $(LREF generateAssertTrap)
    $(LREF generateEmptyFunction)
    $(LREF WhiteHole)
))
$(TR $(TD Nullable) $(TD
    $(LREF Nullable)
    $(LREF nullable)
    $(LREF NullableRef)
    $(LREF nullableRef)
))
$(TR $(TD Proxies) $(TD
    $(LREF Proxy)
    $(LREF rebindable)
    $(LREF Rebindable)
    $(LREF ReplaceType)
    $(LREF unwrap)
    $(LREF wrap)
))
$(TR $(TD Types) $(TD
    $(LREF alignForSize)
    $(LREF Ternary)
    $(LREF Typedef)
    $(LREF TypedefType)
    $(LREF UnqualRef)
))
)

Copyright: Copyright the respective authors, 2008-
License:   $(HTTP boost.org/LICENSE_1_0.txt, Boost License 1.0).
Source:    $(PHOBOSSRC std/_typecons.d)
Authors:   $(HTTP erdani.org, Andrei Alexandrescu),
           $(HTTP bartoszmilewski.wordpress.com, Bartosz Milewski),
           Don Clugston,
           Shin Fujishiro,
           Kenji Hara
 */
module std.typecons;

import core.stdc.stdint : uintptr_t;
import std.meta; // : AliasSeq, allSatisfy;
import std.traits;

///
@safe unittest
{
    // value tuples
    alias Coord = Tuple!(int, "x", int, "y", int, "z");
    Coord c;
    c[1] = 1;       // access by index
    c.z = 1;        // access by given name
    assert(c == Coord(0, 1, 1));

    // names can be omitted
    alias DicEntry = Tuple!(string, string);

    // tuples can also be constructed on instantiation
    assert(tuple(2, 3, 4)[1] == 3);
    // construction on instantiation works with names too
    assert(tuple!("x", "y", "z")(2, 3, 4).y == 3);

    // Rebindable references to const and immutable objects
    {
        class Widget { void foo() const @safe {} }
        const w1 = new Widget, w2 = new Widget;
        w1.foo();
        // w1 = w2 would not work; can't rebind const object
        auto r = Rebindable!(const Widget)(w1);
        // invoke method as if r were a Widget object
        r.foo();
        // rebind r to refer to another object
        r = w2;
    }
}

debug(Unique) import std.stdio;

/**
Encapsulates unique ownership of a resource.

When a $(D Unique!T) goes out of scope it will call $(D destroy)
on the resource $(D T) that it manages, unless it is transferred.
One important consequence of $(D destroy) is that it will call the
destructor of the resource $(D T).  GC-managed references are not
guaranteed to be valid during a destructor call, but other members of
$(D T), such as file handles or pointers to $(D malloc) memory, will
still be valid during the destructor call.  This allows the resource
$(D T) to deallocate or clean up any non-GC resources.

If it is desirable to persist a $(D Unique!T) outside of its original
scope, then it can be transferred.  The transfer can be explicit, by
calling $(D release), or implicit, when returning Unique from a
function. The resource $(D T) can be a polymorphic class object or
instance of an interface, in which case Unique behaves polymorphically
too.

If $(D T) is a value type, then $(D Unique!T) will be implemented
as a reference to a $(D T).
*/
struct Unique(T)
{
/** Represents a reference to $(D T). Resolves to $(D T*) if $(D T) is a value type. */
static if (is(T == class) || is(T == interface))
    alias RefT = T;
else
    alias RefT = T*;

public:
    // Deferred in case we get some language support for checking uniqueness.
    version (None)
    /**
    Allows safe construction of $(D Unique). It creates the resource and
    guarantees unique ownership of it (unless $(D T) publishes aliases of
    $(D this)).
    Note: Nested structs/classes cannot be created.
    Params:
    args = Arguments to pass to $(D T)'s constructor.
    ---
    static class C {}
    auto u = Unique!(C).create();
    ---
    */
    static Unique!T create(A...)(auto ref A args)
    if (__traits(compiles, new T(args)))
    {
        debug(Unique) writeln("Unique.create for ", T.stringof);
        Unique!T u;
        u._p = new T(args);
        return u;
    }

    /**
    Constructor that takes an rvalue.
    It will ensure uniqueness, as long as the rvalue
    isn't just a view on an lvalue (e.g., a cast).
    Typical usage:
    ----
    Unique!Foo f = new Foo;
    ----
    */
    this(RefT p)
    {
        debug(Unique) writeln("Unique constructor with rvalue");
        _p = p;
    }
    /**
    Constructor that takes an lvalue. It nulls its source.
    The nulling will ensure uniqueness as long as there
    are no previous aliases to the source.
    */
    this(ref RefT p)
    {
        _p = p;
        debug(Unique) writeln("Unique constructor nulling source");
        p = null;
        assert(p is null);
    }
    /**
    Constructor that takes a $(D Unique) of a type that is convertible to our type.

    Typically used to transfer a $(D Unique) rvalue of derived type to
    a $(D Unique) of base type.
    Example:
    ---
    class C : Object {}

    Unique!C uc = new C;
    Unique!Object uo = uc.release;
    ---
    */
    this(U)(Unique!U u)
    if (is(u.RefT:RefT))
    {
        debug(Unique) writeln("Unique constructor converting from ", U.stringof);
        _p = u._p;
        u._p = null;
    }

    /// Transfer ownership from a $(D Unique) of a type that is convertible to our type.
    void opAssign(U)(Unique!U u)
    if (is(u.RefT:RefT))
    {
        debug(Unique) writeln("Unique opAssign converting from ", U.stringof);
        // first delete any resource we own
        destroy(this);
        _p = u._p;
        u._p = null;
    }

    ~this()
    {
        debug(Unique) writeln("Unique destructor of ", (_p is null)? null: _p);
        if (_p !is null)
        {
            destroy(_p);
            _p = null;
        }
    }

    /** Returns whether the resource exists. */
    @property bool isEmpty() const
    {
        return _p is null;
    }
    /** Transfer ownership to a $(D Unique) rvalue. Nullifies the current contents.
    Same as calling std.algorithm.move on it.
    */
    Unique release()
    {
        debug(Unique) writeln("Unique Release");
        import std.algorithm.mutation : move;
        return this.move;
    }

    /** Forwards member access to contents. */
    mixin Proxy!_p;

    /**
    Postblit operator is undefined to prevent the cloning of $(D Unique) objects.
    */
    @disable this(this);

private:
    RefT _p;
}

///
@system unittest
{
    static struct S
    {
        int i;
        this(int i){this.i = i;}
    }
    Unique!S produce()
    {
        // Construct a unique instance of S on the heap
        Unique!S ut = new S(5);
        // Implicit transfer of ownership
        return ut;
    }
    // Borrow a unique resource by ref
    void increment(ref Unique!S ur)
    {
        ur.i++;
    }
    void consume(Unique!S u2)
    {
        assert(u2.i == 6);
        // Resource automatically deleted here
    }
    Unique!S u1;
    assert(u1.isEmpty);
    u1 = produce();
    increment(u1);
    assert(u1.i == 6);
    //consume(u1); // Error: u1 is not copyable
    // Transfer ownership of the resource
    consume(u1.release);
    assert(u1.isEmpty);
}

@system unittest
{
    // test conversion to base ref
    int deleted = 0;
    class C
    {
        ~this(){deleted++;}
    }
    // constructor conversion
    Unique!Object u = Unique!C(new C);
    static assert(!__traits(compiles, {u = new C;}));
    assert(!u.isEmpty);
    destroy(u);
    assert(deleted == 1);

    Unique!C uc = new C;
    static assert(!__traits(compiles, {Unique!Object uo = uc;}));
    Unique!Object uo = new C;
    // opAssign conversion, deleting uo resource first
    uo = uc.release;
    assert(uc.isEmpty);
    assert(!uo.isEmpty);
    assert(deleted == 2);
}

@system unittest
{
    debug(Unique) writeln("Unique class");
    class Bar
    {
        ~this() { debug(Unique) writeln("    Bar destructor"); }
        int val() const { return 4; }
    }
    alias UBar = Unique!(Bar);
    UBar g(UBar u)
    {
        debug(Unique) writeln("inside g");
        return u.release;
    }
    auto ub = UBar(new Bar);
    assert(!ub.isEmpty);
    assert(ub.val == 4);
    static assert(!__traits(compiles, {auto ub3 = g(ub);}));
    debug(Unique) writeln("Calling g");
    auto ub2 = g(ub.release);
    debug(Unique) writeln("Returned from g");
    assert(ub.isEmpty);
    assert(!ub2.isEmpty);
}

@system unittest
{
    debug(Unique) writeln("Unique interface");
    interface Bar
    {
        int val() const;
    }
    class BarImpl : Bar
    {
        static int count;
        this()
        {
            count++;
        }
        ~this()
        {
            count--;
        }
        int val() const { return 4; }
    }
    alias UBar = Unique!Bar;
    UBar g(UBar u)
    {
        debug(Unique) writeln("inside g");
        return u.release;
    }
    void consume(UBar u)
    {
        assert(u.val() == 4);
        // Resource automatically deleted here
    }
    auto ub = UBar(new BarImpl);
    assert(BarImpl.count == 1);
    assert(!ub.isEmpty);
    assert(ub.val == 4);
    static assert(!__traits(compiles, {auto ub3 = g(ub);}));
    debug(Unique) writeln("Calling g");
    auto ub2 = g(ub.release);
    debug(Unique) writeln("Returned from g");
    assert(ub.isEmpty);
    assert(!ub2.isEmpty);
    consume(ub2.release);
    assert(BarImpl.count == 0);
}

@system unittest
{
    debug(Unique) writeln("Unique struct");
    struct Foo
    {
        ~this() { debug(Unique) writeln("    Foo destructor"); }
        int val() const { return 3; }
        @disable this(this);
    }
    alias UFoo = Unique!(Foo);

    UFoo f(UFoo u)
    {
        debug(Unique) writeln("inside f");
        return u.release;
    }

    auto uf = UFoo(new Foo);
    assert(!uf.isEmpty);
    assert(uf.val == 3);
    static assert(!__traits(compiles, {auto uf3 = f(uf);}));
    debug(Unique) writeln("Unique struct: calling f");
    auto uf2 = f(uf.release);
    debug(Unique) writeln("Unique struct: returned from f");
    assert(uf.isEmpty);
    assert(!uf2.isEmpty);
}

// ensure Unique behaves correctly through const access paths
@system unittest
{
    struct Bar {int val;}
    struct Foo
    {
        Unique!Bar bar = new Bar;
    }

    Foo foo;
    foo.bar.val = 6;
    const Foo* ptr = &foo;
    static assert(is(typeof(ptr) == const(Foo*)));
    static assert(is(typeof(ptr.bar) == const(Unique!Bar)));
    static assert(is(typeof(ptr.bar.val) == const(int)));
    assert(ptr.bar.val == 6);
    foo.bar.val = 7;
    assert(ptr.bar.val == 7);
}

// Used in Tuple.toString
private template sharedToString(alias field)
    if (is(typeof(field) == shared))
{
    static immutable sharedToString = typeof(field).stringof;
}

private template sharedToString(alias field)
    if (!is(typeof(field) == shared))
{
    alias sharedToString = field;
}

/**
_Tuple of values, for example $(D Tuple!(int, string)) is a record that
stores an $(D int) and a $(D string). $(D Tuple) can be used to bundle
values together, notably when returning multiple values from a
function. If $(D obj) is a `Tuple`, the individual members are
accessible with the syntax $(D obj[0]) for the first field, $(D obj[1])
for the second, and so on.

The choice of zero-based indexing instead of one-base indexing was
motivated by the ability to use value tuples with various compile-time
loop constructs (e.g. $(REF AliasSeq, std,meta) iteration), all of which use
zero-based indexing.

See_Also: $(LREF tuple).

Params:
    Specs = A list of types (and optionally, member names) that the `Tuple` contains.
*/
template Tuple(Specs...)
{
    import std.meta : staticMap;

    // Parse (type,name) pairs (FieldSpecs) out of the specified
    // arguments. Some fields would have name, others not.
    template parseSpecs(Specs...)
    {
        static if (Specs.length == 0)
        {
            alias parseSpecs = AliasSeq!();
        }
        else static if (is(Specs[0]))
        {
            static if (is(typeof(Specs[1]) : string))
            {
                alias parseSpecs =
                    AliasSeq!(FieldSpec!(Specs[0 .. 2]),
                              parseSpecs!(Specs[2 .. $]));
            }
            else
            {
                alias parseSpecs =
                    AliasSeq!(FieldSpec!(Specs[0]),
                              parseSpecs!(Specs[1 .. $]));
            }
        }
        else
        {
            static assert(0, "Attempted to instantiate Tuple with an "
                            ~"invalid argument: "~ Specs[0].stringof);
        }
    }

    template FieldSpec(T, string s = "")
    {
        alias Type = T;
        alias name = s;
    }

    alias fieldSpecs = parseSpecs!Specs;

    // Used with staticMap.
    alias extractType(alias spec) = spec.Type;
    alias extractName(alias spec) = spec.name;

    // Generates named fields as follows:
    //    alias name_0 = Identity!(field[0]);
    //    alias name_1 = Identity!(field[1]);
    //      :
    // NOTE: field[k] is an expression (which yields a symbol of a
    //       variable) and can't be aliased directly.
    string injectNamedFields()
    {
        string decl = "";
        foreach (i, name; staticMap!(extractName, fieldSpecs))
        {
            import std.format : format;

            decl ~= format("alias _%s = Identity!(field[%s]);", i, i);
            if (name.length != 0)
            {
                decl ~= format("alias %s = _%s;", name, i);
            }
        }
        return decl;
    }

    // Returns Specs for a subtuple this[from .. to] preserving field
    // names if any.
    alias sliceSpecs(size_t from, size_t to) =
        staticMap!(expandSpec, fieldSpecs[from .. to]);

    template expandSpec(alias spec)
    {
        static if (spec.name.length == 0)
        {
            alias expandSpec = AliasSeq!(spec.Type);
        }
        else
        {
            alias expandSpec = AliasSeq!(spec.Type, spec.name);
        }
    }

    enum areCompatibleTuples(Tup1, Tup2, string op) = isTuple!Tup2 && is(typeof(
    (ref Tup1 tup1, ref Tup2 tup2)
    {
        static assert(tup1.field.length == tup2.field.length);
        foreach (i, _; Tup1.Types)
        {
            auto lhs = typeof(tup1.field[i]).init;
            auto rhs = typeof(tup2.field[i]).init;
            static if (op == "=")
                lhs = rhs;
            else
                auto result = mixin("lhs "~op~" rhs");
        }
    }));

    enum areBuildCompatibleTuples(Tup1, Tup2) = isTuple!Tup2 && is(typeof(
    {
        static assert(Tup1.Types.length == Tup2.Types.length);
        foreach (i, _; Tup1.Types)
            static assert(isBuildable!(Tup1.Types[i], Tup2.Types[i]));
    }));

    /+ Returns $(D true) iff a $(D T) can be initialized from a $(D U). +/
    enum isBuildable(T, U) =  is(typeof(
    {
        U u = U.init;
        T t = u;
    }));
    /+ Helper for partial instanciation +/
    template isBuildableFrom(U)
    {
        enum isBuildableFrom(T) = isBuildable!(T, U);
    }

    struct Tuple
    {
        /**
         * The types of the `Tuple`'s components.
         */
        alias Types = staticMap!(extractType, fieldSpecs);

        ///
        static if (Specs.length == 0) @safe unittest
        {
            alias Fields = Tuple!(int, "id", string, float);
            static assert(is(Fields.Types == AliasSeq!(int, string, float)));
        }

        /**
         * The names of the `Tuple`'s components. Unnamed fields have empty names.
         */
        alias fieldNames = staticMap!(extractName, fieldSpecs);

        ///
        static if (Specs.length == 0) @safe unittest
        {
            alias Fields = Tuple!(int, "id", string, float);
            static assert(Fields.fieldNames == AliasSeq!("id", "", ""));
        }

        /**
         * Use $(D t.expand) for a `Tuple` $(D t) to expand it into its
         * components. The result of $(D expand) acts as if the `Tuple`'s components
         * were listed as a list of values. (Ordinarily, a $(D Tuple) acts as a
         * single value.)
         */
        Types expand;
        mixin(injectNamedFields());

        ///
        static if (Specs.length == 0) @safe unittest
        {
            auto t1 = tuple(1, " hello ", 2.3);
            assert(t1.toString() == `Tuple!(int, string, double)(1, " hello ", 2.3)`);

            void takeSeveralTypes(int n, string s, bool b)
            {
                assert(n == 4 && s == "test" && b == false);
            }

            auto t2 = tuple(4, "test", false);
            //t.expand acting as a list of values
            takeSeveralTypes(t2.expand);
        }

        static if (is(Specs))
        {
            // This is mostly to make t[n] work.
            alias expand this;
        }
        else
        {
            @property
            ref inout(Tuple!Types) _Tuple_super() inout @trusted
            {
                foreach (i, _; Types)   // Rely on the field layout
                {
                    static assert(typeof(return).init.tupleof[i].offsetof ==
                                                       expand[i].offsetof);
                }
                return *cast(typeof(return)*) &(field[0]);
            }
            // This is mostly to make t[n] work.
            alias _Tuple_super this;
        }

        // backwards compatibility
        alias field = expand;

        /**
         * Constructor taking one value for each field.
         *
         * Params:
         *     values = A list of values that are either the same
         *              types as those given by the `Types` field
         *              of this `Tuple`, or can implicitly convert
         *              to those types. They must be in the same
         *              order as they appear in `Types`.
         */
        static if (Types.length > 0)
        {
            this(Types values)
            {
                field[] = values[];
            }
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            alias ISD = Tuple!(int, string, double);
            auto tup = ISD(1, "test", 3.2);
            assert(tup.toString() == `Tuple!(int, string, double)(1, "test", 3.2)`);
        }

        /**
         * Constructor taking a compatible array.
         *
         * Params:
         *     values = A compatible static array to build the `Tuple` from.
         *              Array slices are not supported.
         */
        this(U, size_t n)(U[n] values)
        if (n == Types.length && allSatisfy!(isBuildableFrom!U, Types))
        {
            foreach (i, _; Types)
            {
                field[i] = values[i];
            }
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            int[2] ints;
            Tuple!(int, int) t = ints;
        }

        /**
         * Constructor taking a compatible `Tuple`. Two `Tuple`s are compatible
         * $(B iff) they are both of the same length, and, for each type `T` on the
         * left-hand side, the corresponding type `U` on the right-hand side can
         * implicitly convert to `T`.
         *
         * Params:
         *     another = A compatible `Tuple` to build from. Its type must be
         *               compatible with the target `Tuple`'s type.
         */
        this(U)(U another)
        if (areBuildCompatibleTuples!(typeof(this), U))
        {
            field[] = another.field[];
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            alias IntVec = Tuple!(int, int, int);
            alias DubVec = Tuple!(double, double, double);

            IntVec iv = tuple(1, 1, 1);

            //Ok, int can implicitly convert to double
            DubVec dv = iv;
            //Error: double cannot implicitly convert to int
            //IntVec iv2 = dv;
        }

        /**
         * Comparison for equality. Two `Tuple`s are considered equal
         * $(B iff) they fulfill the following criteria:
         *
         * $(UL
         *   $(LI Each `Tuple` is the same length.)
         *   $(LI For each type `T` on the left-hand side and each type
         *        `U` on the right-hand side, values of type `T` can be
         *        compared with values of type `U`.)
         *   $(LI For each value `v1` on the left-hand side and each value
         *        `v2` on the right-hand side, the expression `v1 == v2` is
         *        true.))
         *
         * Params:
         *     rhs = The `Tuple` to compare against. It must meeting the criteria
         *           for comparison between `Tuple`s.
         *
         * Returns:
         *     true if both `Tuple`s are equal, otherwise false.
         */
        bool opEquals(R)(R rhs)
        if (areCompatibleTuples!(typeof(this), R, "=="))
        {
            return field[] == rhs.field[];
        }

        /// ditto
        bool opEquals(R)(R rhs) const
        if (areCompatibleTuples!(typeof(this), R, "=="))
        {
            return field[] == rhs.field[];
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            Tuple!(int, string) t1 = tuple(1, "test");
            Tuple!(double, string) t2 =  tuple(1.0, "test");
            //Ok, int can be compared with double and
            //both have a value of 1
            assert(t1 == t2);
        }

        /**
         * Comparison for ordering.
         *
         * Params:
         *     rhs = The `Tuple` to compare against. It must meet the criteria
         *           for comparison between `Tuple`s.
         *
         * Returns:
         * For any values `v1` on the right-hand side and `v2` on the
         * left-hand side:
         *
         * $(UL
         *   $(LI A negative integer if the expression `v1 < v2` is true.)
         *   $(LI A positive integer if the expression `v1 > v2` is true.)
         *   $(LI 0 if the expression `v1 == v2` is true.))
         */
        int opCmp(R)(R rhs)
        if (areCompatibleTuples!(typeof(this), R, "<"))
        {
            foreach (i, Unused; Types)
            {
                if (field[i] != rhs.field[i])
                {
                    return field[i] < rhs.field[i] ? -1 : 1;
                }
            }
            return 0;
        }

        /// ditto
        int opCmp(R)(R rhs) const
        if (areCompatibleTuples!(typeof(this), R, "<"))
        {
            foreach (i, Unused; Types)
            {
                if (field[i] != rhs.field[i])
                {
                    return field[i] < rhs.field[i] ? -1 : 1;
                }
            }
            return 0;
        }

        /**
            The first `v1` for which `v1 > v2` is true determines
            the result. This could lead to unexpected behaviour.
         */
        static if (Specs.length == 0) @safe unittest
        {
            auto tup1 = tuple(1, 1, 1);
            auto tup2 = tuple(1, 100, 100);
            assert(tup1 < tup2);

            //Only the first result matters for comparison
            tup1[0] = 2;
            assert(tup1 > tup2);
        }

        /**
         * Assignment from another `Tuple`.
         *
         * Params:
         *     rhs = The source `Tuple` to assign from. Each element of the
         *           source `Tuple` must be implicitly assignable to each
         *           respective element of the target `Tuple`.
         */
        void opAssign(R)(auto ref R rhs)
        if (areCompatibleTuples!(typeof(this), R, "="))
        {
            import std.algorithm.mutation : swap;

            static if (is(R : Tuple!Types) && !__traits(isRef, rhs))
            {
                if (__ctfe)
                {
                    // Cannot use swap at compile time
                    field[] = rhs.field[];
                }
                else
                {
                    // Use swap-and-destroy to optimize rvalue assignment
                    swap!(Tuple!Types)(this, rhs);
                }
            }
            else
            {
                // Do not swap; opAssign should be called on the fields.
                field[] = rhs.field[];
            }
        }

        /**
         * Renames the elements of a $(LREF Tuple).
         *
         * `rename` uses the passed `names` and returns a new
         * $(LREF Tuple) using these names, with the content
         * unchanged.
         * If fewer names are passed than there are members
         * of the $(LREF Tuple) then those trailing members are unchanged.
         * An empty string will remove the name for that member.
         * It is an compile-time error to pass more names than
         * there are members of the $(LREF Tuple).
         */
        ref rename(names...)() return
        if (names.length == 0 || allSatisfy!(isSomeString, typeof(names)))
        {
            import std.algorithm.comparison : equal;
            // to circumvent bug 16418
            static if (names.length == 0 || equal([names], [fieldNames]))
                return this;
            else
            {
                enum nT = Types.length;
                enum nN = names.length;
                static assert(nN <= nT, "Cannot have more names than tuple members");
                alias allNames = AliasSeq!(names, fieldNames[nN .. $]);

                template GetItem(size_t idx)
                {
                    import std.array : empty;
                    static if (idx < nT)
                        alias GetItem = Alias!(Types[idx]);
                    else static if (allNames[idx - nT].empty)
                        alias GetItem = AliasSeq!();
                    else
                        alias GetItem = Alias!(allNames[idx - nT]);
                }

                import std.range : roundRobin, iota;
                alias NewTupleT = Tuple!(staticMap!(GetItem, aliasSeqOf!(
                        roundRobin(iota(nT), iota(nT, 2*nT)))));
                return *(() @trusted => cast(NewTupleT*)&this)();
            }
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            auto t0 = tuple(4, "hello");

            auto t0Named = t0.rename!("val", "tag");
            assert(t0Named.val == 4);
            assert(t0Named.tag == "hello");

            Tuple!(float, "dat", size_t[2], "pos") t1;
            t1.pos = [2, 1];
            auto t1Named = t1.rename!"height";
            t1Named.height = 3.4f;
            assert(t1Named.height == 3.4f);
            assert(t1Named.pos == [2, 1]);
            t1Named.rename!"altitude".altitude = 5;
            assert(t1Named.height == 5);

            Tuple!(int, "a", int, int, "c") t2;
            t2 = tuple(3,4,5);
            auto t2Named = t2.rename!("", "b");
            // "a" no longer has a name
            static assert(!hasMember!(typeof(t2Named), "a"));
            assert(t2Named[0] == 3);
            assert(t2Named.b == 4);
            assert(t2Named.c == 5);

            // not allowed to specify more names than the tuple has members
            static assert(!__traits(compiles, t2.rename!("a","b","c","d")));

            // use it in a range pipeline
            import std.range : iota, zip;
            import std.algorithm.iteration : map, sum;
            auto res = zip(iota(1, 4), iota(10, 13))
                .map!(t => t.rename!("a", "b"))
                .map!(t => t.a * t.b)
                .sum;
            assert(res == 68);
        }

        /**
         * Overload of $(LREF _rename) that takes an associative array
         * `translate` as a template parameter, where the keys are
         * either the names or indices of the members to be changed
         * and the new names are the corresponding values.
         * Every key in `translate` must be the name of a member of the
         * $(LREF tuple).
         * The same rules for empty strings apply as for the variadic
         * template overload of $(LREF _rename).
        */
        ref rename(alias translate)()
        if (is(typeof(translate) : V[K], V, K) && isSomeString!V &&
                (isSomeString!K || is(K : size_t)))
        {
            import std.range : ElementType;
            static if (isSomeString!(ElementType!(typeof(translate.keys))))
            {
                {
                    import std.conv : to;
                    import std.algorithm.iteration : filter;
                    import std.algorithm.searching : canFind;
                    enum notFound = translate.keys
                        .filter!(k => fieldNames.canFind(k) == -1);
                    static assert(notFound.empty, "Cannot find members "
                        ~ notFound.to!string ~ " in type "
                        ~ typeof(this).stringof);
                }
                return this.rename!(aliasSeqOf!(
                    {
                        import std.array : empty;
                        auto names = [fieldNames];
                        foreach (ref n; names)
                            if (!n.empty)
                                if (auto p = n in translate)
                                    n = *p;
                        return names;
                    }()));
            }
            else
            {
                {
                    import std.algorithm.iteration : filter;
                    import std.conv : to;
                    enum invalid = translate.keys.
                        filter!(k => k < 0 || k >= this.length);
                    static assert(invalid.empty, "Indices " ~ invalid.to!string
                        ~ " are out of bounds for tuple with length "
                        ~ this.length.to!string);
                }
                return this.rename!(aliasSeqOf!(
                    {
                        auto names = [fieldNames];
                        foreach (k, v; translate)
                            names[k] = v;
                        return names;
                    }()));
            }
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            //replacing names by their current name

            Tuple!(float, "dat", size_t[2], "pos") t1;
            t1.pos = [2, 1];
            auto t1Named = t1.rename!(["dat": "height"]);
            t1Named.height = 3.4;
            assert(t1Named.pos == [2, 1]);
            t1Named.rename!(["height": "altitude"]).altitude = 5;
            assert(t1Named.height == 5);

            Tuple!(int, "a", int, "b") t2;
            t2 = tuple(3, 4);
            auto t2Named = t2.rename!(["a": "b", "b": "c"]);
            assert(t2Named.b == 3);
            assert(t2Named.c == 4);
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            //replace names by their position

            Tuple!(float, "dat", size_t[2], "pos") t1;
            t1.pos = [2, 1];
            auto t1Named = t1.rename!([0: "height"]);
            t1Named.height = 3.4;
            assert(t1Named.pos == [2, 1]);
            t1Named.rename!([0: "altitude"]).altitude = 5;
            assert(t1Named.height == 5);

            Tuple!(int, "a", int, "b", int, "c") t2;
            t2 = tuple(3, 4, 5);
            auto t2Named = t2.rename!([0: "c", 2: "a"]);
            assert(t2Named.a == 5);
            assert(t2Named.b == 4);
            assert(t2Named.c == 3);
        }

        static if (Specs.length == 0) @safe unittest
        {
            //check that empty translations work fine
            enum string[string] a0 = null;
            enum string[int] a1 = null;
            Tuple!(float, "a", float, "b") t0;

            auto t1 = t0.rename!a0;

            t1.a = 3;
            t1.b = 4;
            auto t2 = t0.rename!a1;
            t2.a = 3;
            t2.b = 4;
            auto t3 = t0.rename;
            t3.a = 3;
            t3.b = 4;
        }

        /**
         * Takes a slice by-reference of this `Tuple`.
         *
         * Params:
         *     from = A `size_t` designating the starting position of the slice.
         *     to = A `size_t` designating the ending position (exclusive) of the slice.
         *
         * Returns:
         *     A new `Tuple` that is a slice from `[from, to$(RPAREN)` of the original.
         *     It has the same types and values as the range `[from, to$(RPAREN)` in
         *     the original.
         */
        @property
        ref inout(Tuple!(sliceSpecs!(from, to))) slice(size_t from, size_t to)() inout @trusted
        if (from <= to && to <= Types.length)
        {
            static assert(
                (typeof(this).alignof % typeof(return).alignof == 0) &&
                (expand[from].offsetof % typeof(return).alignof == 0),
                "Slicing by reference is impossible because of an alignment mistmatch. (See Phobos issue #15645.)");

            return *cast(typeof(return)*) &(field[from]);
        }

        ///
        static if (Specs.length == 0) @safe unittest
        {
            Tuple!(int, string, float, double) a;
            a[1] = "abc";
            a[2] = 4.5;
            auto s = a.slice!(1, 3);
            static assert(is(typeof(s) == Tuple!(string, float)));
            assert(s[0] == "abc" && s[1] == 4.5);

            // Phobos issue #15645
            Tuple!(int, short, bool, double) b;
            static assert(!__traits(compiles, b.slice!(2, 4)));
        }

        /**
            Creates a hash of this `Tuple`.

            Returns:
                A `size_t` representing the hash of this `Tuple`.
         */
        size_t toHash() const nothrow @trusted
        {
            size_t h = 0;
            foreach (i, T; Types)
                h += typeid(T).getHash(cast(const void*)&field[i]);
            return h;
        }

        ///
        template toString()
        {
            /**
             * Converts to string.
             *
             * Returns:
             *     The string representation of this `Tuple`.
             */
            string toString()() const
            {
                import std.array : appender;
                auto app = appender!string();
                this.toString((const(char)[] chunk) => app ~= chunk);
                return app.data;
            }

            import std.format : FormatSpec;

            /**
             * Formats `Tuple` with either `%s`, `%(inner%)` or `%(inner%|sep%)`.
             *
             * $(TABLE2 Formats supported by Tuple,
             * $(THEAD Format, Description)
             * $(TROW $(P `%s`), $(P Format like `Tuple!(types)(elements formatted with %s each)`.))
             * $(TROW $(P `%(inner%)`), $(P The format `inner` is applied the expanded `Tuple`, so
             *      it may contain as many formats as the `Tuple` has fields.))
             * $(TROW $(P `%(inner%|sep%)`), $(P The format `inner` is one format, that is applied
             *      on all fields of the `Tuple`. The inner format must be compatible to all
             *      of them.)))
             * ---
             *  Tuple!(int, double)[3] tupList = [ tuple(1, 1.0), tuple(2, 4.0), tuple(3, 9.0) ];
             *
             *  // Default format
             *  assert(format("%s", tuple("a", 1)) == `Tuple!(string, int)("a", 1)`);
             *
             *  // One Format for each individual component
             *  assert(format("%(%#x v %.4f w %#x%)", tuple(1, 1.0, 10))         == `0x1 v 1.0000 w 0xa`);
             *  assert(format(  "%#x v %.4f w %#x"  , tuple(1, 1.0, 10).expand)  == `0x1 v 1.0000 w 0xa`);
             *
             *  // One Format for all components
             *  assert(format("%(>%s<%| & %)", tuple("abc", 1, 2.3, [4, 5])) == `>abc< & >1< & >2.3< & >[4, 5]<`);
             *
             *  // Array of Tuples
             *  assert(format("%(%(f(%d) = %.1f%);  %)", tupList) == `f(1) = 1.0;  f(2) = 4.0;  f(3) = 9.0`);
             *
             *
             *  // Error: %( %) missing.
             *  assertThrown!FormatException(
             *      format("%d, %f", tuple(1, 2.0)) == `1, 2.0`
             *  );
             *
             *  // Error: %( %| %) missing.
             *  assertThrown!FormatException(
             *      format("%d", tuple(1, 2)) == `1, 2`
             *  );
             *
             *  // Error: %d inadequate for double.
             *  assertThrown!FormatException(
             *      format("%(%d%|, %)", tuple(1, 2.0)) == `1, 2.0`
             *  );
             * ---
             */
            void toString(DG)(scope DG sink) const
            {
                toString(sink, FormatSpec!char());
            }

            /// ditto
            void toString(DG, Char)(scope DG sink, FormatSpec!Char fmt) const
            {
                import std.format : formatElement, formattedWrite, FormatException;
                if (fmt.nested)
                {
                    if (fmt.sep)
                    {
                        foreach (i, Type; Types)
                        {
                            static if (i > 0)
                            {
                                sink(fmt.sep);
                            }
                            // TODO: Change this once formattedWrite() works for shared objects.
                            static if (is(Type == class) && is(Type == shared))
                            {
                                sink(Type.stringof);
                            }
                            else
                            {
                                formattedWrite(sink, fmt.nested, this.field[i]);
                            }
                        }
                    }
                    else
                    {
                        formattedWrite(sink, fmt.nested, staticMap!(sharedToString, this.expand));
                    }
                }
                else if (fmt.spec == 's')
                {
                    enum header = Unqual!(typeof(this)).stringof ~ "(",
                         footer = ")",
                         separator = ", ";
                    sink(header);
                    foreach (i, Type; Types)
                    {
                        static if (i > 0)
                        {
                            sink(separator);
                        }
                        // TODO: Change this once formatElement() works for shared objects.
                        static if (is(Type == class) && is(Type == shared))
                        {
                            sink(Type.stringof);
                        }
                        else
                        {
                            FormatSpec!Char f;
                            formatElement(sink, field[i], f);
                        }
                    }
                    sink(footer);
                }
                else
                {
                    throw new FormatException(
                        "Expected '%s' or '%(...%)' or '%(...%|...%)' format specifier for type '" ~
                            Unqual!(typeof(this)).stringof ~ "', not '%" ~ fmt.spec ~ "'.");
                }
            }
        }
    }
}

///
@safe unittest
{
    Tuple!(int, int) point;
    // assign coordinates
    point[0] = 5;
    point[1] = 6;
    // read coordinates
    auto x = point[0];
    auto y = point[1];
}

/**
    `Tuple` members can be named. It is legal to mix named and unnamed
    members. The method above is still applicable to all fields.
 */
@safe unittest
{
    alias Entry = Tuple!(int, "index", string, "value");
    Entry e;
    e.index = 4;
    e.value = "Hello";
    assert(e[1] == "Hello");
    assert(e[0] == 4);
}

/**
    A `Tuple` with named fields is a distinct type from a `Tuple` with unnamed
    fields, i.e. each naming imparts a separate type for the `Tuple`. Two
    `Tuple`s differing in naming only are still distinct, even though they
    might have the same structure.
 */
@safe unittest
{
    Tuple!(int, "x", int, "y") point1;
    Tuple!(int, int) point2;
    assert(!is(typeof(point1) == typeof(point2)));
}

/**
    Creates a copy of a $(LREF Tuple) with its fields in _reverse order.

    Params:
        t = The `Tuple` to copy.

    Returns:
        A new `Tuple`.
 */
auto reverse(T)(T t)
    if (isTuple!T)
{
    import std.meta : Reverse;
    // @@@BUG@@@ Cannot be an internal function due to forward reference issues.

    // @@@BUG@@@ 9929 Need 'this' when calling template with expanded tuple
    // return tuple(Reverse!(t.expand));

    ReverseTupleType!T result;
    auto tup = t.expand;
    result.expand = Reverse!tup;
    return result;
}

///
@safe unittest
{
    auto tup = tuple(1, "2");
    assert(tup.reverse == tuple("2", 1));
}

/* Get a Tuple type with the reverse specification of Tuple T. */
private template ReverseTupleType(T)
    if (isTuple!T)
{
    static if (is(T : Tuple!A, A...))
        alias ReverseTupleType = Tuple!(ReverseTupleSpecs!A);
}

/* Reverse the Specs of a Tuple. */
private template ReverseTupleSpecs(T...)
{
    static if (T.length > 1)
    {
        static if (is(typeof(T[$-1]) : string))
        {
            alias ReverseTupleSpecs = AliasSeq!(T[$-2], T[$-1], ReverseTupleSpecs!(T[0 .. $-2]));
        }
        else
        {
            alias ReverseTupleSpecs = AliasSeq!(T[$-1], ReverseTupleSpecs!(T[0 .. $-1]));
        }
    }
    else
    {
        alias ReverseTupleSpecs = T;
    }
}

// ensure that internal Tuple unittests are compiled
@safe unittest
{
    Tuple!() t;
}

@safe unittest
{
    import std.conv;
    {
        Tuple!(int, "a", int, "b") nosh;
        static assert(nosh.length == 2);
        nosh.a = 5;
        nosh.b = 6;
        assert(nosh.a == 5);
        assert(nosh.b == 6);
    }
    {
        Tuple!(short, double) b;
        static assert(b.length == 2);
        b[1] = 5;
        auto a = Tuple!(int, real)(b);
        assert(a[0] == 0 && a[1] == 5);
        a = Tuple!(int, real)(1, 2);
        assert(a[0] == 1 && a[1] == 2);
        auto c = Tuple!(int, "a", double, "b")(a);
        assert(c[0] == 1 && c[1] == 2);
    }
    {
        Tuple!(int, real) nosh;
        nosh[0] = 5;
        nosh[1] = 0;
        assert(nosh[0] == 5 && nosh[1] == 0);
        assert(nosh.to!string == "Tuple!(int, real)(5, 0)", nosh.to!string);
        Tuple!(int, int) yessh;
        nosh = yessh;
    }
    {
        class A {}
        Tuple!(int, shared A) nosh;
        nosh[0] = 5;
        assert(nosh[0] == 5 && nosh[1] is null);
        assert(nosh.to!string == "Tuple!(int, shared(A))(5, shared(A))");
    }
    {
        Tuple!(int, string) t;
        t[0] = 10;
        t[1] = "str";
        assert(t[0] == 10 && t[1] == "str");
        assert(t.to!string == `Tuple!(int, string)(10, "str")`, t.to!string);
    }
    {
        Tuple!(int, "a", double, "b") x;
        static assert(x.a.offsetof == x[0].offsetof);
        static assert(x.b.offsetof == x[1].offsetof);
        x.b = 4.5;
        x.a = 5;
        assert(x[0] == 5 && x[1] == 4.5);
        assert(x.a == 5 && x.b == 4.5);
    }
    // indexing
    {
        Tuple!(int, real) t;
        static assert(is(typeof(t[0]) == int));
        static assert(is(typeof(t[1]) == real));
        int* p0 = &t[0];
        real* p1 = &t[1];
        t[0] = 10;
        t[1] = -200.0L;
        assert(*p0 == t[0]);
        assert(*p1 == t[1]);
    }
    // slicing
    {
        Tuple!(int, "x", real, "y", double, "z", string) t;
        t[0] = 10;
        t[1] = 11;
        t[2] = 12;
        t[3] = "abc";
        auto a = t.slice!(0, 3);
        assert(a.length == 3);
        assert(a.x == t.x);
        assert(a.y == t.y);
        assert(a.z == t.z);
        auto b = t.slice!(2, 4);
        assert(b.length == 2);
        assert(b.z == t.z);
        assert(b[1] == t[3]);
    }
    // nesting
    {
        Tuple!(Tuple!(int, real), Tuple!(string, "s")) t;
        static assert(is(typeof(t[0]) == Tuple!(int, real)));
        static assert(is(typeof(t[1]) == Tuple!(string, "s")));
        static assert(is(typeof(t[0][0]) == int));
        static assert(is(typeof(t[0][1]) == real));
        static assert(is(typeof(t[1].s) == string));
        t[0] = tuple(10, 20.0L);
        t[1].s = "abc";
        assert(t[0][0] == 10);
        assert(t[0][1] == 20.0L);
        assert(t[1].s == "abc");
    }
    // non-POD
    {
        static struct S
        {
            int count;
            this(this) { ++count; }
            ~this() { --count; }
            void opAssign(S rhs) { count = rhs.count; }
        }
        Tuple!(S, S) ss;
        Tuple!(S, S) ssCopy = ss;
        assert(ssCopy[0].count == 1);
        assert(ssCopy[1].count == 1);
        ssCopy[1] = ssCopy[0];
        assert(ssCopy[1].count == 2);
    }
    // bug 2800
    {
        static struct R
        {
            Tuple!(int, int) _front;
            @property ref Tuple!(int, int) front() return { return _front;  }
            @property bool empty() { return _front[0] >= 10; }
            void popFront() { ++_front[0]; }
        }
        foreach (a; R())
        {
            static assert(is(typeof(a) == Tuple!(int, int)));
            assert(0 <= a[0] && a[0] < 10);
            assert(a[1] == 0);
        }
    }
    // Construction with compatible elements
    {
        auto t1 = Tuple!(int, double)(1, 1);

        // 8702
        auto t8702a = tuple(tuple(1));
        auto t8702b = Tuple!(Tuple!(int))(Tuple!(int)(1));
    }
    // Construction with compatible tuple
    {
        Tuple!(int, int) x;
        x[0] = 10;
        x[1] = 20;
        Tuple!(int, "a", double, "b") y = x;
        assert(y.a == 10);
        assert(y.b == 20);
        // incompatible
        static assert(!__traits(compiles, Tuple!(int, int)(y)));
    }
    // 6275
    {
        const int x = 1;
        auto t1 = tuple(x);
        alias T = Tuple!(const(int));
        auto t2 = T(1);
    }
    // 9431
    {
        alias T = Tuple!(int[1][]);
        auto t = T([[10]]);
    }
    // 7666
    {
        auto tup = tuple(1, "2");
        assert(tup.reverse == tuple("2", 1));
    }
    {
        Tuple!(int, "x", string, "y") tup = tuple(1, "2");
        auto rev = tup.reverse;
        assert(rev == tuple("2", 1));
        assert(rev.x == 1 && rev.y == "2");
    }
    {
        Tuple!(wchar, dchar, int, "x", string, "y", char, byte, float) tup;
        tup = tuple('a', 'b', 3, "4", 'c', cast(byte) 0x0D, 0.00);
        auto rev = tup.reverse;
        assert(rev == tuple(0.00, cast(byte) 0x0D, 'c', "4", 3, 'b', 'a'));
        assert(rev.x == 3 && rev.y == "4");
    }
}
@safe unittest
{
    // opEquals
    {
        struct Equ1 { bool opEquals(Equ1) { return true; } }
        auto  tm1 = tuple(Equ1.init);
        const tc1 = tuple(Equ1.init);
        static assert( is(typeof(tm1 == tm1)));
        static assert(!is(typeof(tm1 == tc1)));
        static assert(!is(typeof(tc1 == tm1)));
        static assert(!is(typeof(tc1 == tc1)));

        struct Equ2 { bool opEquals(const Equ2) const { return true; } }
        auto  tm2 = tuple(Equ2.init);
        const tc2 = tuple(Equ2.init);
        static assert( is(typeof(tm2 == tm2)));
        static assert( is(typeof(tm2 == tc2)));
        static assert( is(typeof(tc2 == tm2)));
        static assert( is(typeof(tc2 == tc2)));

        struct Equ3 { bool opEquals(T)(T) { return true; } }
        auto  tm3 = tuple(Equ3.init);           // bugzilla 8686
        const tc3 = tuple(Equ3.init);
        static assert( is(typeof(tm3 == tm3)));
        static assert( is(typeof(tm3 == tc3)));
        static assert(!is(typeof(tc3 == tm3)));
        static assert(!is(typeof(tc3 == tc3)));

        struct Equ4 { bool opEquals(T)(T) const { return true; } }
        auto  tm4 = tuple(Equ4.init);
        const tc4 = tuple(Equ4.init);
        static assert( is(typeof(tm4 == tm4)));
        static assert( is(typeof(tm4 == tc4)));
        static assert( is(typeof(tc4 == tm4)));
        static assert( is(typeof(tc4 == tc4)));
    }
    // opCmp
    {
        struct Cmp1 { int opCmp(Cmp1) { return 0; } }
        auto  tm1 = tuple(Cmp1.init);
        const tc1 = tuple(Cmp1.init);
        static assert( is(typeof(tm1 < tm1)));
        static assert(!is(typeof(tm1 < tc1)));
        static assert(!is(typeof(tc1 < tm1)));
        static assert(!is(typeof(tc1 < tc1)));

        struct Cmp2 { int opCmp(const Cmp2) const { return 0; } }
        auto  tm2 = tuple(Cmp2.init);
        const tc2 = tuple(Cmp2.init);
        static assert( is(typeof(tm2 < tm2)));
        static assert( is(typeof(tm2 < tc2)));
        static assert( is(typeof(tc2 < tm2)));
        static assert( is(typeof(tc2 < tc2)));

        struct Cmp3 { int opCmp(T)(T) { return 0; } }
        auto  tm3 = tuple(Cmp3.init);
        const tc3 = tuple(Cmp3.init);
        static assert( is(typeof(tm3 < tm3)));
        static assert( is(typeof(tm3 < tc3)));
        static assert(!is(typeof(tc3 < tm3)));
        static assert(!is(typeof(tc3 < tc3)));

        struct Cmp4 { int opCmp(T)(T) const { return 0; } }
        auto  tm4 = tuple(Cmp4.init);
        const tc4 = tuple(Cmp4.init);
        static assert( is(typeof(tm4 < tm4)));
        static assert( is(typeof(tm4 < tc4)));
        static assert( is(typeof(tc4 < tm4)));
        static assert( is(typeof(tc4 < tc4)));
    }
    // Bugzilla 14890
    static void test14890(inout int[] dummy)
    {
        alias V = Tuple!(int, int);

                    V mv;
              const V cv;
          immutable V iv;
              inout V wv;   // OK <- NG
        inout const V wcv;  // OK <- NG

        foreach (v1; AliasSeq!(mv, cv, iv, wv, wcv))
        foreach (v2; AliasSeq!(mv, cv, iv, wv, wcv))
        {
            assert(!(v1 < v2));
        }
    }
    {
        int[2] ints = [ 1, 2 ];
        Tuple!(int, int) t = ints;
        assert(t[0] == 1 && t[1] == 2);
        Tuple!(long, uint) t2 = ints;
        assert(t2[0] == 1 && t2[1] == 2);
    }
}
@safe unittest
{
    auto t1 = Tuple!(int, "x", string, "y")(1, "a");
    assert(t1.x == 1);
    assert(t1.y == "a");
    void foo(Tuple!(int, string) t2) {}
    foo(t1);

    Tuple!(int, int)[] arr;
    arr ~= tuple(10, 20); // OK
    arr ~= Tuple!(int, "x", int, "y")(10, 20); // NG -> OK

    static assert(is(typeof(Tuple!(int, "x", string, "y").tupleof) ==
                     typeof(Tuple!(int,      string     ).tupleof)));
}
@safe unittest
{
    // Bugzilla 10686
    immutable Tuple!(int) t1;
    auto r1 = t1[0]; // OK
    immutable Tuple!(int, "x") t2;
    auto r2 = t2[0]; // error
}
@safe unittest
{
    import std.exception : assertCTFEable;

    // Bugzilla 10218
    assertCTFEable!(
    {
        auto t = tuple(1);
        t = tuple(2);   // assignment
    });
}
@safe unittest
{
    class Foo{}
    Tuple!(immutable(Foo)[]) a;
}

@safe unittest
{
    //Test non-assignable
    static struct S
    {
        int* p;
    }
    alias IS = immutable S;
    static assert(!isAssignable!IS);

    auto s = IS.init;

    alias TIS = Tuple!IS;
    TIS a = tuple(s);
    TIS b = a;

    alias TISIS = Tuple!(IS, IS);
    TISIS d = tuple(s, s);
    IS[2] ss;
    TISIS e = TISIS(ss);
}

// Bugzilla #9819
@safe unittest
{
    alias T = Tuple!(int, "x", double, "foo");
    static assert(T.fieldNames[0] == "x");
    static assert(T.fieldNames[1] == "foo");

    alias Fields = Tuple!(int, "id", string, float);
    static assert(Fields.fieldNames == AliasSeq!("id", "", ""));
}

// Bugzilla 13837
@safe unittest
{
    // New behaviour, named arguments.
    static assert(is(
        typeof(tuple!("x")(1)) == Tuple!(int, "x")));
    static assert(is(
        typeof(tuple!("x")(1.0)) == Tuple!(double, "x")));
    static assert(is(
        typeof(tuple!("x")("foo")) == Tuple!(string, "x")));
    static assert(is(
        typeof(tuple!("x", "y")(1, 2.0)) == Tuple!(int, "x", double, "y")));

    auto a = tuple!("a", "b", "c")("1", 2, 3.0f);
    static assert(is(typeof(a.a) == string));
    static assert(is(typeof(a.b) == int));
    static assert(is(typeof(a.c) == float));

    // Old behaviour, but with explicit type parameters.
    static assert(is(
        typeof(tuple!(int, double)(1, 2.0)) == Tuple!(int, double)));
    static assert(is(
        typeof(tuple!(const int)(1)) == Tuple!(const int)));
    static assert(is(
        typeof(tuple()) == Tuple!()));

    // Nonsensical behaviour
    static assert(!__traits(compiles, tuple!(1)(2)));
    static assert(!__traits(compiles, tuple!("x")(1, 2)));
    static assert(!__traits(compiles, tuple!("x", "y")(1)));
    static assert(!__traits(compiles, tuple!("x")()));
    static assert(!__traits(compiles, tuple!("x", int)(2)));
}

@safe unittest
{
    class C {}
    Tuple!(Rebindable!(const C)) a;
    Tuple!(const C) b;
    a = b;
}

@nogc @safe unittest
{
    alias T = Tuple!(string, "s");
    T x;
    x = T.init;
}

@safe unittest
{
    import std.format : format, FormatException;
    import std.exception : assertThrown;

    // enum tupStr = tuple(1, 1.0).toString; // toString is *impure*.
    //static assert(tupStr == `Tuple!(int, double)(1, 1)`);

    Tuple!(int, double)[3] tupList = [ tuple(1, 1.0), tuple(2, 4.0), tuple(3, 9.0) ];

    // Default format
    assert(format("%s", tuple("a", 1)) == `Tuple!(string, int)("a", 1)`);

    // One Format for each individual component
    assert(format("%(%#x v %.4f w %#x%)", tuple(1, 1.0, 10))         == `0x1 v 1.0000 w 0xa`);
    assert(format(  "%#x v %.4f w %#x"  , tuple(1, 1.0, 10).expand)  == `0x1 v 1.0000 w 0xa`);

    // One Format for all components
    assert(format("%(>%s<%| & %)", tuple("abc", 1, 2.3, [4, 5])) == `>abc< & >1< & >2.3< & >[4, 5]<`);

    // Array of Tuples
    assert(format("%(%(f(%d) = %.1f%);  %)", tupList) == `f(1) = 1.0;  f(2) = 4.0;  f(3) = 9.0`);


    // Error: %( %) missing.
    assertThrown!FormatException(
        format("%d, %f", tuple(1, 2.0)) == `1, 2.0`
    );

    // Error: %( %| %) missing.
    assertThrown!FormatException(
        format("%d", tuple(1, 2)) == `1, 2`
    );

    // Error: %d inadequate for double
    assertThrown!FormatException(
        format("%(%d%|, %)", tuple(1, 2.0)) == `1, 2.0`
    );
}

/**
    Constructs a $(LREF Tuple) object instantiated and initialized according to
    the given arguments.

    Params:
        Names = An optional list of strings naming each successive field of the `Tuple`.
                Each name matches up with the corresponding field given by `Args`.
                A name does not have to be provided for every field, but as
                the names must proceed in order, it is not possible to skip
                one field and name the next after it.
*/
template tuple(Names...)
{
    /**
    Params:
        args = Values to initialize the `Tuple` with. The `Tuple`'s type will
               be inferred from the types of the values given.

    Returns:
        A new `Tuple` with its type inferred from the arguments given.
     */
    auto tuple(Args...)(Args args)
    {
        static if (Names.length == 0)
        {
            // No specified names, just infer types from Args...
            return Tuple!Args(args);
        }
        else static if (!is(typeof(Names[0]) : string))
        {
            // Names[0] isn't a string, must be explicit types.
            return Tuple!Names(args);
        }
        else
        {
            // Names[0] is a string, so must be specifying names.
            static assert(Names.length == Args.length,
                "Insufficient number of names given.");

            // Interleave(a, b).and(c, d) == (a, c, b, d)
            // This is to get the interleaving of types and names for Tuple
            // e.g. Tuple!(int, "x", string, "y")
            template Interleave(A...)
            {
                template and(B...) if (B.length == 1)
                {
                    alias and = AliasSeq!(A[0], B[0]);
                }

                template and(B...) if (B.length != 1)
                {
                    alias and = AliasSeq!(A[0], B[0],
                        Interleave!(A[1..$]).and!(B[1..$]));
                }
            }
            return Tuple!(Interleave!(Args).and!(Names))(args);
        }
    }
}

///
@safe unittest
{
    auto value = tuple(5, 6.7, "hello");
    assert(value[0] == 5);
    assert(value[1] == 6.7);
    assert(value[2] == "hello");

    // Field names can be provided.
    auto entry = tuple!("index", "value")(4, "Hello");
    assert(entry.index == 4);
    assert(entry.value == "Hello");
}

/**
    Returns $(D true) if and only if $(D T) is an instance of $(D std.typecons.Tuple).

    Params:
        T = The type to check.

    Returns:
        true if `T` is a `Tuple` type, false otherwise.
 */
enum isTuple(T) = __traits(compiles,
                           {
                               void f(Specs...)(Tuple!Specs tup) {}
                               f(T.init);
                           } );

///
@safe unittest
{
    static assert(isTuple!(Tuple!()));
    static assert(isTuple!(Tuple!(int)));
    static assert(isTuple!(Tuple!(int, real, string)));
    static assert(isTuple!(Tuple!(int, "x", real, "y")));
    static assert(isTuple!(Tuple!(int, Tuple!(real), string)));
}

@safe unittest
{
    static assert(isTuple!(const Tuple!(int)));
    static assert(isTuple!(immutable Tuple!(int)));

    static assert(!isTuple!(int));
    static assert(!isTuple!(const int));

    struct S {}
    static assert(!isTuple!(S));
}

// used by both Rebindable and UnqualRef
private mixin template RebindableCommon(T, U, alias This)
    if (is(T == class) || is(T == interface) || isAssociativeArray!T)
{
    private union
    {
        T original;
        U stripped;
    }

    @trusted pure nothrow @nogc
    {
        void opAssign(T another)
        {
            stripped = cast(U) another;
        }

        void opAssign(typeof(this) another)
        {
            stripped = another.stripped;
        }

        static if (is(T == const U) && is(T == const shared U))
        {
            // safely assign immutable to const / const shared
            void opAssign(This!(immutable U) another)
            {
                stripped = another.stripped;
            }
        }

        this(T initializer)
        {
            opAssign(initializer);
        }

        @property inout(T) get() inout
        {
            return original;
        }
    }

    alias get this;
}

/**
$(D Rebindable!(T)) is a simple, efficient wrapper that behaves just
like an object of type $(D T), except that you can reassign it to
refer to another object. For completeness, $(D Rebindable!(T)) aliases
itself away to $(D T) if $(D T) is a non-const object type.

You may want to use $(D Rebindable) when you want to have mutable
storage referring to $(D const) objects, for example an array of
references that must be sorted in place. $(D Rebindable) does not
break the soundness of D's type system and does not incur any of the
risks usually associated with $(D cast).

Params:
    T = An object, interface, array slice type, or associative array type.
 */
template Rebindable(T)
    if (is(T == class) || is(T == interface) || isDynamicArray!T || isAssociativeArray!T)
{
    static if (is(T == const U, U) || is(T == immutable U, U))
    {
        static if (isDynamicArray!T)
        {
            import std.range.primitives : ElementEncodingType;
            alias Rebindable = const(ElementEncodingType!T)[];
        }
        else
        {
            struct Rebindable
            {
                mixin RebindableCommon!(T, U, Rebindable);
            }
        }
    }
    else
    {
        alias Rebindable = T;
    }
}

///Regular $(D const) object references cannot be reassigned.
@system unittest
{
    class Widget { int x; int y() const { return x; } }
    const a = new Widget;
    // Fine
    a.y();
    // error! can't modify const a
    // a.x = 5;
    // error! can't modify const a
    // a = new Widget;
}

/**
    However, $(D Rebindable!(Widget)) does allow reassignment,
    while otherwise behaving exactly like a $(D const Widget).
 */
@system unittest
{
    class Widget { int x; int y() const { return x; } }
    auto a = Rebindable!(const Widget)(new Widget);
    // Fine
    a.y();
    // error! can't modify const a
    // a.x = 5;
    // Fine
    a = new Widget;
}

@safe unittest // issue 16054
{
    Rebindable!(immutable Object) r;
    static assert(__traits(compiles, r.get()));
    static assert(!__traits(compiles, &r.get()));
}

/**
Convenience function for creating a $(D Rebindable) using automatic type
inference.

Params:
    obj = A reference to an object, interface, associative array, or an array slice
          to initialize the `Rebindable` with.

Returns:
    A newly constructed `Rebindable` initialized with the given reference.
*/
Rebindable!T rebindable(T)(T obj)
    if (is(T == class) || is(T == interface) || isDynamicArray!T || isAssociativeArray!T)
{
    typeof(return) ret;
    ret = obj;
    return ret;
}

/**
This function simply returns the $(D Rebindable) object passed in.  It's useful
in generic programming cases when a given object may be either a regular
$(D class) or a $(D Rebindable).

Params:
    obj = An instance of Rebindable!T.

Returns:
    `obj` without any modification.
*/
Rebindable!T rebindable(T)(Rebindable!T obj)
{
    return obj;
}

@system unittest
{
    interface CI { int foo() const; }
    class C : CI {
      int foo() const { return 42; }
      @property int bar() const { return 23; }
    }
    Rebindable!(C) obj0;
    static assert(is(typeof(obj0) == C));

    Rebindable!(const(C)) obj1;
    static assert(is(typeof(obj1.get) == const(C)), typeof(obj1.get).stringof);
    static assert(is(typeof(obj1.stripped) == C));
    obj1 = new C;
    assert(obj1.get !is null);
    obj1 = new const(C);
    assert(obj1.get !is null);

    Rebindable!(immutable(C)) obj2;
    static assert(is(typeof(obj2.get) == immutable(C)));
    static assert(is(typeof(obj2.stripped) == C));
    obj2 = new immutable(C);
    assert(obj1.get !is null);

    // test opDot
    assert(obj2.foo() == 42);
    assert(obj2.bar == 23);

    interface I { final int foo() const { return 42; } }
    Rebindable!(I) obj3;
    static assert(is(typeof(obj3) == I));

    Rebindable!(const I) obj4;
    static assert(is(typeof(obj4.get) == const I));
    static assert(is(typeof(obj4.stripped) == I));
    static assert(is(typeof(obj4.foo()) == int));
    obj4 = new class I {};

    Rebindable!(immutable C) obj5i;
    Rebindable!(const C) obj5c;
    obj5c = obj5c;
    obj5c = obj5i;
    obj5i = obj5i;
    static assert(!__traits(compiles, obj5i = obj5c));

    // Test the convenience functions.
    auto obj5convenience = rebindable(obj5i);
    assert(obj5convenience is obj5i);

    auto obj6 = rebindable(new immutable(C));
    static assert(is(typeof(obj6) == Rebindable!(immutable C)));
    assert(obj6.foo() == 42);

    auto obj7 = rebindable(new C);
    CI interface1 = obj7;
    auto interfaceRebind1 = rebindable(interface1);
    assert(interfaceRebind1.foo() == 42);

    const interface2 = interface1;
    auto interfaceRebind2 = rebindable(interface2);
    assert(interfaceRebind2.foo() == 42);

    auto arr = [1,2,3,4,5];
    const arrConst = arr;
    assert(rebindable(arr) == arr);
    assert(rebindable(arrConst) == arr);

    // Issue 7654
    immutable(char[]) s7654;
    Rebindable!(typeof(s7654)) r7654 = s7654;

    foreach (T; AliasSeq!(char, wchar, char, int))
    {
        static assert(is(Rebindable!(immutable(T[])) == immutable(T)[]));
        static assert(is(Rebindable!(const(T[])) == const(T)[]));
        static assert(is(Rebindable!(T[]) == T[]));
    }

    // Issue 12046
    static assert(!__traits(compiles, Rebindable!(int[1])));
    static assert(!__traits(compiles, Rebindable!(const int[1])));

    // Pull request 3341
    Rebindable!(immutable int[int]) pr3341 = [123:345];
    assert(pr3341[123] == 345);
    immutable int[int] pr3341_aa = [321:543];
    pr3341 = pr3341_aa;
    assert(pr3341[321] == 543);
    assert(rebindable(pr3341_aa)[321] == 543);
}

/**
    Similar to $(D Rebindable!(T)) but strips all qualifiers from the reference as
    opposed to just constness / immutability. Primary intended use case is with
    shared (having thread-local reference to shared class data)

    Params:
        T = A class or interface type.
 */
template UnqualRef(T)
    if (is(T == class) || is(T == interface))
{
    static if (is(T == const U, U)
        || is(T == immutable U, U)
        || is(T == shared U, U)
        || is(T == const shared U, U))
    {
        struct UnqualRef
        {
            mixin RebindableCommon!(T, U, UnqualRef);
        }
    }
    else
    {
        alias UnqualRef = T;
    }
}

///
@system unittest
{
    class Data {}

    static shared(Data) a;
    static UnqualRef!(shared Data) b;

    import core.thread;

    auto thread = new core.thread.Thread({
        a = new shared Data();
        b = new shared Data();
    });

    thread.start();
    thread.join();

    assert(a !is null);
    assert(b is null);
}

@safe unittest
{
    class C { }
    alias T = UnqualRef!(const shared C);
    static assert(is(typeof(T.stripped) == C));
}



/**
  Order the provided members to minimize size while preserving alignment.
  Alignment is not always optimal for 80-bit reals, nor for structs declared
  as align(1).

  Params:
      E = A list of the types to be aligned, representing fields
          of an aggregate such as a `struct` or `class`.

      names = The names of the fields that are to be aligned.

  Returns:
      A string to be mixed in to an aggregate, such as a `struct` or `class`.
*/
string alignForSize(E...)(const char[][] names...)
{
    // Sort all of the members by .alignof.
    // BUG: Alignment is not always optimal for align(1) structs
    // or 80-bit reals or 64-bit primitives on x86.
    // TRICK: Use the fact that .alignof is always a power of 2,
    // and maximum 16 on extant systems. Thus, we can perform
    // a very limited radix sort.
    // Contains the members with .alignof = 64,32,16,8,4,2,1

    assert(E.length == names.length,
        "alignForSize: There should be as many member names as the types");

    string[7] declaration = ["", "", "", "", "", "", ""];

    foreach (i, T; E)
    {
        auto a = T.alignof;
        auto k = a >= 64? 0 : a >= 32? 1 : a >= 16? 2 : a >= 8? 3 : a >= 4? 4 : a >= 2? 5 : 6;
        declaration[k] ~= T.stringof ~ " " ~ names[i] ~ ";\n";
    }

    auto s = "";
    foreach (decl; declaration)
        s ~= decl;
    return s;
}

///
@safe unittest
{
    struct Banner {
        mixin(alignForSize!(byte[6], double)(["name", "height"]));
    }
}

@safe unittest
{
    enum x = alignForSize!(int[], char[3], short, double[5])("x", "y","z", "w");
    struct Foo { int x; }
    enum y = alignForSize!(ubyte, Foo, cdouble)("x", "y", "z");

    enum passNormalX = x == "double[5] w;\nint[] x;\nshort z;\nchar[3] y;\n";
    enum passNormalY = y == "cdouble z;\nFoo y;\nubyte x;\n";

    enum passAbnormalX = x == "int[] x;\ndouble[5] w;\nshort z;\nchar[3] y;\n";
    enum passAbnormalY = y == "Foo y;\ncdouble z;\nubyte x;\n";
    // ^ blame http://d.puremagic.com/issues/show_bug.cgi?id=231

    static assert(passNormalX || passAbnormalX && double.alignof <= (int[]).alignof);
    static assert(passNormalY || passAbnormalY && double.alignof <= int.alignof);
}

// Issue 12914
@safe unittest
{
    immutable string[] fieldNames = ["x", "y"];
    struct S
    {
        mixin(alignForSize!(byte, int)(fieldNames));
    }
}

/**
Defines a value paired with a distinctive "null" state that denotes
the absence of a value. If default constructed, a $(D
Nullable!T) object starts in the null state. Assigning it renders it
non-null. Calling $(D nullify) can nullify it again.

Practically $(D Nullable!T) stores a $(D T) and a $(D bool).
 */
struct Nullable(T)
{
    private T _value;
    private bool _isNull = true;

/**
Constructor initializing $(D this) with $(D value).

Params:
    value = The value to initialize this `Nullable` with.
 */
    this(inout T value) inout
    {
        _value = value;
        _isNull = false;
    }

    /**
      If they are both null, then they are equal. If one is null and the other
      is not, then they are not equal. If they are both non-null, then they are
      equal if their values are equal.
      */
    bool opEquals()(auto ref const(typeof(this)) rhs) const
    {
        if (_isNull)
            return rhs._isNull;
        if (rhs._isNull)
            return false;
        return _value == rhs._value;
    }

    /// Ditto
    bool opEquals(U)(auto ref const(U) rhs) const
    if (is(typeof(this.get == rhs)))
    {
        return _isNull ? false : rhs == _value;
    }

    ///
    @safe unittest
    {
        Nullable!int empty;
        Nullable!int a = 42;
        Nullable!int b = 42;
        Nullable!int c = 27;

        assert(empty == empty);
        assert(empty == Nullable!int.init);
        assert(empty != a);
        assert(empty != b);
        assert(empty != c);

        assert(a == b);
        assert(a != c);

        assert(empty != 42);
        assert(a == 42);
        assert(c != 42);
    }

    @safe unittest
    {
        // Test constness
        immutable Nullable!int a = 42;
        Nullable!int b = 42;
        immutable Nullable!int c = 29;
        Nullable!int d = 29;
        immutable e = 42;
        int f = 29;
        assert(a == a);
        assert(a == b);
        assert(a != c);
        assert(a != d);
        assert(a == e);
        assert(a != f);

        // Test rvalue
        assert(a == const Nullable!int(42));
        assert(a != Nullable!int(29));
    }

    // Issue 17482
    @system unittest
    {
        import std.variant : Variant;
        Nullable!Variant a = Variant(12);
        assert(a == 12);
        Nullable!Variant e;
        assert(e != 12);
    }

    template toString()
    {
        import std.format : FormatSpec, formatValue;
        // Needs to be a template because of DMD @@BUG@@ 13737.
        void toString()(scope void delegate(const(char)[]) sink, FormatSpec!char fmt)
        {
            if (isNull)
            {
                sink.formatValue("Nullable.null", fmt);
            }
            else
            {
                sink.formatValue(_value, fmt);
            }
        }

        // Issue 14940
        void toString()(scope void delegate(const(char)[]) @safe sink, FormatSpec!char fmt)
        {
            if (isNull)
            {
                sink.formatValue("Nullable.null", fmt);
            }
            else
            {
                sink.formatValue(_value, fmt);
            }
        }
    }

/**
Check if `this` is in the null state.

Returns:
    true $(B iff) `this` is in the null state, otherwise false.
 */
    @property bool isNull() const @safe pure nothrow
    {
        return _isNull;
    }

///
@system unittest
{
    Nullable!int ni;
    assert(ni.isNull);

    ni = 0;
    assert(!ni.isNull);
}

// Issue 14940
@safe unittest
{
    import std.array : appender;
    import std.format : formattedWrite;

    auto app = appender!string();
    Nullable!int a = 1;
    formattedWrite(app, "%s", a);
    assert(app.data == "1");
}

/**
Forces $(D this) to the null state.
 */
    void nullify()()
    {
        .destroy(_value);
        _isNull = true;
    }

///
@safe unittest
{
    Nullable!int ni = 0;
    assert(!ni.isNull);

    ni.nullify();
    assert(ni.isNull);
}

/**
Assigns $(D value) to the internally-held state. If the assignment
succeeds, $(D this) becomes non-null.

Params:
    value = A value of type `T` to assign to this `Nullable`.
 */
    void opAssign()(T value)
    {
        _value = value;
        _isNull = false;
    }

/**
    If this `Nullable` wraps a type that already has a null value
    (such as a pointer), then assigning the null value to this
    `Nullable` is no different than assigning any other value of
    type `T`, and the resulting code will look very strange. It
    is strongly recommended that this be avoided by instead using
    the version of `Nullable` that takes an additional `nullValue`
    template argument.
 */
@safe unittest
{
    //Passes
    Nullable!(int*) npi;
    assert(npi.isNull);

    //Passes?!
    npi = null;
    assert(!npi.isNull);
}

/**
Gets the value. $(D this) must not be in the null state.
This function is also called for the implicit conversion to $(D T).

Returns:
    The value held internally by this `Nullable`.
 */
    @property ref inout(T) get() inout @safe pure nothrow
    {
        enum message = "Called `get' on null Nullable!" ~ T.stringof ~ ".";
        assert(!isNull, message);
        return _value;
    }

///
@system unittest
{
    import core.exception : AssertError;
    import std.exception : assertThrown, assertNotThrown;

    Nullable!int ni;
    int i = 42;
    //`get` is implicitly called. Will throw
    //an AssertError in non-release mode
    assertThrown!AssertError(i = ni);
    assert(i == 42);

    ni = 5;
    assertNotThrown!AssertError(i = ni);
    assert(i == 5);
}

/**
Implicitly converts to $(D T).
$(D this) must not be in the null state.
 */
    alias get this;
}

/// ditto
auto nullable(T)(T t)
{
    return Nullable!T(t);
}

///
@safe unittest
{
    struct CustomerRecord
    {
        string name;
        string address;
        int customerNum;
    }

    Nullable!CustomerRecord getByName(string name)
    {
        //A bunch of hairy stuff

        return Nullable!CustomerRecord.init;
    }

    auto queryResult = getByName("Doe, John");
    if (!queryResult.isNull)
    {
        //Process Mr. Doe's customer record
        auto address = queryResult.address;
        auto customerNum = queryResult.customerNum;

        //Do some things with this customer's info
    }
    else
    {
        //Add the customer to the database
    }
}

///
@system unittest
{
    import std.exception : assertThrown;

    auto a = 42.nullable;
    assert(!a.isNull);
    assert(a.get == 42);

    a.nullify();
    assert(a.isNull);
    assertThrown!Throwable(a.get);
}

@system unittest
{
    import std.exception : assertThrown;

    Nullable!int a;
    assert(a.isNull);
    assertThrown!Throwable(a.get);
    a = 5;
    assert(!a.isNull);
    assert(a == 5);
    assert(a != 3);
    assert(a.get != 3);
    a.nullify();
    assert(a.isNull);
    a = 3;
    assert(a == 3);
    a *= 6;
    assert(a == 18);
    a = a;
    assert(a == 18);
    a.nullify();
    assertThrown!Throwable(a += 2);
}
@safe unittest
{
    auto k = Nullable!int(74);
    assert(k == 74);
    k.nullify();
    assert(k.isNull);
}
@safe unittest
{
    static int f(in Nullable!int x) {
        return x.isNull ? 42 : x.get;
    }
    Nullable!int a;
    assert(f(a) == 42);
    a = 8;
    assert(f(a) == 8);
    a.nullify();
    assert(f(a) == 42);
}
@system unittest
{
    import std.exception : assertThrown;

    static struct S { int x; }
    Nullable!S s;
    assert(s.isNull);
    s = S(6);
    assert(s == S(6));
    assert(s != S(0));
    assert(s.get != S(0));
    s.x = 9190;
    assert(s.x == 9190);
    s.nullify();
    assertThrown!Throwable(s.x = 9441);
}
@safe unittest
{
    // Ensure Nullable can be used in pure/nothrow/@safe environment.
    function() @safe pure nothrow
    {
        Nullable!int n;
        assert(n.isNull);
        n = 4;
        assert(!n.isNull);
        assert(n == 4);
        n.nullify();
        assert(n.isNull);
    }();
}
@system unittest
{
    // Ensure Nullable can be used when the value is not pure/nothrow/@safe
    static struct S
    {
        int x;
        this(this) @system {}
    }

    Nullable!S s;
    assert(s.isNull);
    s = S(5);
    assert(!s.isNull);
    assert(s.x == 5);
    s.nullify();
    assert(s.isNull);
}
@safe unittest
{
    // Bugzilla 9404
    alias N = Nullable!int;

    void foo(N a)
    {
        N b;
        b = a; // `N b = a;` works fine
    }
    N n;
    foo(n);
}
@safe unittest
{
    //Check nullable immutable is constructable
    {
        auto a1 = Nullable!(immutable int)();
        auto a2 = Nullable!(immutable int)(1);
        auto i = a2.get;
    }
    //Check immutable nullable is constructable
    {
        auto a1 = immutable (Nullable!int)();
        auto a2 = immutable (Nullable!int)(1);
        auto i = a2.get;
    }
}
@safe unittest
{
    alias NInt   = Nullable!int;

    //Construct tests
    {
        //from other Nullable null
        NInt a1;
        NInt b1 = a1;
        assert(b1.isNull);

        //from other Nullable non-null
        NInt a2 = NInt(1);
        NInt b2 = a2;
        assert(b2 == 1);

        //Construct from similar nullable
        auto a3 = immutable(NInt)();
        NInt b3 = a3;
        assert(b3.isNull);
    }

    //Assign tests
    {
        //from other Nullable null
        NInt a1;
        NInt b1;
        b1 = a1;
        assert(b1.isNull);

        //from other Nullable non-null
        NInt a2 = NInt(1);
        NInt b2;
        b2 = a2;
        assert(b2 == 1);

        //Construct from similar nullable
        auto a3 = immutable(NInt)();
        NInt b3 = a3;
        b3 = a3;
        assert(b3.isNull);
    }
}
@safe unittest
{
    //Check nullable is nicelly embedable in a struct
    static struct S1
    {
        Nullable!int ni;
    }
    static struct S2 //inspired from 9404
    {
        Nullable!int ni;
        this(S2 other)
        {
            ni = other.ni;
        }
        void opAssign(S2 other)
        {
            ni = other.ni;
        }
    }
    foreach (S; AliasSeq!(S1, S2))
    {
        S a;
        S b = a;
        S c;
        c = a;
    }
}
@system unittest
{
    // Bugzilla 10268
    import std.json;
    JSONValue value = null;
    auto na = Nullable!JSONValue(value);

    struct S1 { int val; }
    struct S2 { int* val; }
    struct S3 { immutable int* val; }

    {
        auto sm = S1(1);
        immutable si = immutable S1(1);
        auto x1 =           Nullable!S1(sm);
        auto x2 = immutable Nullable!S1(sm);
        auto x3 =           Nullable!S1(si);
        auto x4 = immutable Nullable!S1(si);
        assert(x1.val == 1);
        assert(x2.val == 1);
        assert(x3.val == 1);
        assert(x4.val == 1);
    }

    auto nm = 10;
    immutable ni = 10;

    {
        auto sm = S2(&nm);
        immutable si = immutable S2(&ni);
        auto x1 =           Nullable!S2(sm);
        static assert(!__traits(compiles, { auto x2 = immutable Nullable!S2(sm); }));
        static assert(!__traits(compiles, { auto x3 =           Nullable!S2(si); }));
        auto x4 = immutable Nullable!S2(si);
        assert(*x1.val == 10);
        assert(*x4.val == 10);
    }

    {
        auto sm = S3(&ni);
        immutable si = immutable S3(&ni);
        auto x1 =           Nullable!S3(sm);
        auto x2 = immutable Nullable!S3(sm);
        auto x3 =           Nullable!S3(si);
        auto x4 = immutable Nullable!S3(si);
        assert(*x1.val == 10);
        assert(*x2.val == 10);
        assert(*x3.val == 10);
        assert(*x4.val == 10);
    }
}
@safe unittest
{
    // Bugzila 10357
    import std.datetime;
    Nullable!SysTime time = SysTime(0);
}
@system unittest
{
    import std.conv : to;
    import std.array;

    // Bugzilla 10915
    Appender!string buffer;

    Nullable!int ni;
    assert(ni.to!string() == "Nullable.null");

    struct Test { string s; }
    alias NullableTest = Nullable!Test;

    NullableTest nt = Test("test");
    assert(nt.to!string() == `Test("test")`);

    NullableTest ntn = Test("null");
    assert(ntn.to!string() == `Test("null")`);

    class TestToString
    {
        double d;

        this (double d)
        {
            this.d = d;
        }

        override string toString()
        {
            return d.to!string();
        }
    }
    Nullable!TestToString ntts = new TestToString(2.5);
    assert(ntts.to!string() == "2.5");
}

/**
Just like $(D Nullable!T), except that the null state is defined as a
particular value. For example, $(D Nullable!(uint, uint.max)) is an
$(D uint) that sets aside the value $(D uint.max) to denote a null
state. $(D Nullable!(T, nullValue)) is more storage-efficient than $(D
Nullable!T) because it does not need to store an extra $(D bool).

Params:
    T = The wrapped type for which Nullable provides a null value.

    nullValue = The null value which denotes the null state of this
                `Nullable`. Must be of type `T`.
 */
struct Nullable(T, T nullValue)
{
    private T _value = nullValue;

/**
Constructor initializing $(D this) with $(D value).

Params:
    value = The value to initialize this `Nullable` with.
 */
    this(T value)
    {
        _value = value;
    }

    template toString()
    {
        import std.format : FormatSpec, formatValue;
        // Needs to be a template because of DMD @@BUG@@ 13737.
        void toString()(scope void delegate(const(char)[]) sink, FormatSpec!char fmt)
        {
            if (isNull)
            {
                sink.formatValue("Nullable.null", fmt);
            }
            else
            {
                sink.formatValue(_value, fmt);
            }
        }
    }

/**
Check if `this` is in the null state.

Returns:
    true $(B iff) `this` is in the null state, otherwise false.
 */
    @property bool isNull() const
    {
        //Need to use 'is' if T is a nullable type and
        //nullValue is null, or it's a compiler error
        static if (is(CommonType!(T, typeof(null)) == T) && nullValue is null)
        {
            return _value is nullValue;
        }
        //Need to use 'is' if T is a float type
        //because NaN != NaN
        else static if (isFloatingPoint!T)
        {
            return _value is nullValue;
        }
        else
        {
            return _value == nullValue;
        }
    }

///
@system unittest
{
    Nullable!(int, -1) ni;
    //Initialized to "null" state
    assert(ni.isNull);

    ni = 0;
    assert(!ni.isNull);
}

// https://issues.dlang.org/show_bug.cgi?id=11135
// disable test until https://issues.dlang.org/show_bug.cgi?id=15316 gets fixed
version (none) @system unittest
{
    foreach (T; AliasSeq!(float, double, real))
    {
        Nullable!(T, T.init) nf;
        //Initialized to "null" state
        assert(nf.isNull);
        assert(nf is typeof(nf).init);

        nf = 0;
        assert(!nf.isNull);

        nf.nullify();
        assert(nf.isNull);
    }
}

/**
Forces $(D this) to the null state.
 */
    void nullify()()
    {
        _value = nullValue;
    }

///
@system unittest
{
    Nullable!(int, -1) ni = 0;
    assert(!ni.isNull);

    ni = -1;
    assert(ni.isNull);
}

/**
Assigns $(D value) to the internally-held state. If the assignment
succeeds, $(D this) becomes non-null. No null checks are made. Note
that the assignment may leave $(D this) in the null state.

Params:
    value = A value of type `T` to assign to this `Nullable`.
            If it is `nullvalue`, then the internal state of
            this `Nullable` will be set to null.
 */
    void opAssign()(T value)
    {
        _value = value;
    }

/**
    If this `Nullable` wraps a type that already has a null value
    (such as a pointer), and that null value is not given for
    `nullValue`, then assigning the null value to this `Nullable`
    is no different than assigning any other value of type `T`,
    and the resulting code will look very strange. It is strongly
    recommended that this be avoided by using `T`'s "built in"
    null value for `nullValue`.
 */
@system unittest
{
    //Passes
    enum nullVal = cast(int*) 0xCAFEBABE;
    Nullable!(int*, nullVal) npi;
    assert(npi.isNull);

    //Passes?!
    npi = null;
    assert(!npi.isNull);
}

/**
Gets the value. $(D this) must not be in the null state.
This function is also called for the implicit conversion to $(D T).

Returns:
    The value held internally by this `Nullable`.
 */
    @property ref inout(T) get() inout
    {
        //@@@6169@@@: We avoid any call that might evaluate nullValue's %s,
        //Because it might messup get's purity and safety inference.
        enum message = "Called `get' on null Nullable!(" ~ T.stringof ~ ",nullValue).";
        assert(!isNull, message);
        return _value;
    }

///
@system unittest
{
    import std.exception : assertThrown, assertNotThrown;

    Nullable!(int, -1) ni;
    //`get` is implicitly called. Will throw
    //an error in non-release mode
    assertThrown!Throwable(ni == 0);

    ni = 0;
    assertNotThrown!Throwable(ni == 0);
}

/**
Implicitly converts to $(D T).
$(D this) must not be in the null state.
 */
    alias get this;
}

/// ditto
auto nullable(alias nullValue, T)(T t)
    if (is (typeof(nullValue) == T))
{
    return Nullable!(T, nullValue)(t);
}

///
@safe unittest
{
    Nullable!(size_t, size_t.max) indexOf(string[] haystack, string needle)
    {
        //Find the needle, returning -1 if not found

        return Nullable!(size_t, size_t.max).init;
    }

    void sendLunchInvite(string name)
    {
    }

    //It's safer than C...
    auto coworkers = ["Jane", "Jim", "Marry", "Fred"];
    auto pos = indexOf(coworkers, "Bob");
    if (!pos.isNull)
    {
        //Send Bob an invitation to lunch
        sendLunchInvite(coworkers[pos]);
    }
    else
    {
        //Bob not found; report the error
    }

    //And there's no overhead
    static assert(Nullable!(size_t, size_t.max).sizeof == size_t.sizeof);
}

///
@system unittest
{
    import std.exception : assertThrown;

    Nullable!(int, int.min) a;
    assert(a.isNull);
    assertThrown!Throwable(a.get);
    a = 5;
    assert(!a.isNull);
    assert(a == 5);
    static assert(a.sizeof == int.sizeof);
}

///
@safe unittest
{
    auto a = nullable!(int.min)(8);
    assert(a == 8);
    a.nullify();
    assert(a.isNull);
}

@safe unittest
{
    static int f(in Nullable!(int, int.min) x) {
        return x.isNull ? 42 : x.get;
    }
    Nullable!(int, int.min) a;
    assert(f(a) == 42);
    a = 8;
    assert(f(a) == 8);
    a.nullify();
    assert(f(a) == 42);
}
@safe unittest
{
    // Ensure Nullable can be used in pure/nothrow/@safe environment.
    function() @safe pure nothrow
    {
        Nullable!(int, int.min) n;
        assert(n.isNull);
        n = 4;
        assert(!n.isNull);
        assert(n == 4);
        n.nullify();
        assert(n.isNull);
    }();
}
@system unittest
{
    // Ensure Nullable can be used when the value is not pure/nothrow/@system
    static struct S
    {
        int x;
        bool opEquals(const S s) const @system { return s.x == x; }
    }

    Nullable!(S, S(711)) s;
    assert(s.isNull);
    s = S(5);
    assert(!s.isNull);
    assert(s.x == 5);
    s.nullify();
    assert(s.isNull);
}
@safe unittest
{
    //Check nullable is nicelly embedable in a struct
    static struct S1
    {
        Nullable!(int, 0) ni;
    }
    static struct S2 //inspired from 9404
    {
        Nullable!(int, 0) ni;
        this(S2 other)
        {
            ni = other.ni;
        }
        void opAssign(S2 other)
        {
            ni = other.ni;
        }
    }
    foreach (S; AliasSeq!(S1, S2))
    {
        S a;
        S b = a;
        S c;
        c = a;
    }
}
@system unittest
{
    import std.conv : to;

    // Bugzilla 10915
    Nullable!(int, 1) ni = 1;
    assert(ni.to!string() == "Nullable.null");

    struct Test { string s; }
    alias NullableTest = Nullable!(Test, Test("null"));

    NullableTest nt = Test("test");
    assert(nt.to!string() == `Test("test")`);

    NullableTest ntn = Test("null");
    assert(ntn.to!string() == "Nullable.null");

    class TestToString
    {
        double d;

        this(double d)
        {
            this.d = d;
        }

        override string toString()
        {
            return d.to!string();
        }
    }
    alias NullableTestToString = Nullable!(TestToString, null);

    NullableTestToString ntts = new TestToString(2.5);
    assert(ntts.to!string() == "2.5");
}


/**
Just like $(D Nullable!T), except that the object refers to a value
sitting elsewhere in memory. This makes assignments overwrite the
initially assigned value. Internally $(D NullableRef!T) only stores a
pointer to $(D T) (i.e., $(D Nullable!T.sizeof == (T*).sizeof)).
 */
struct NullableRef(T)
{
    private T* _value;

/**
Constructor binding $(D this) to $(D value).

Params:
    value = The value to bind to.
 */
    this(T* value) @safe pure nothrow
    {
        _value = value;
    }

    template toString()
    {
        import std.format : FormatSpec, formatValue;
        // Needs to be a template because of DMD @@BUG@@ 13737.
        void toString()(scope void delegate(const(char)[]) sink, FormatSpec!char fmt)
        {
            if (isNull)
            {
                sink.formatValue("Nullable.null", fmt);
            }
            else
            {
                sink.formatValue(*_value, fmt);
            }
        }
    }

/**
Binds the internal state to $(D value).

Params:
    value = A pointer to a value of type `T` to bind this `NullableRef` to.
 */
    void bind(T* value) @safe pure nothrow
    {
        _value = value;
    }

    ///
    @safe unittest
    {
        NullableRef!int nr = new int(42);
        assert(nr == 42);

        int* n = new int(1);
        nr.bind(n);
        assert(nr == 1);
    }

/**
Returns $(D true) if and only if $(D this) is in the null state.

Returns:
    true if `this` is in the null state, otherwise false.
 */
    @property bool isNull() const @safe pure nothrow
    {
        return _value is null;
    }

    ///
    @safe unittest
    {
        NullableRef!int nr;
        assert(nr.isNull);

        int* n = new int(42);
        nr.bind(n);
        assert(!nr.isNull && nr == 42);
    }

/**
Forces $(D this) to the null state.
 */
    void nullify() @safe pure nothrow
    {
        _value = null;
    }

    ///
    @safe unittest
    {
        NullableRef!int nr = new int(42);
        assert(!nr.isNull);

        nr.nullify();
        assert(nr.isNull);
    }

/**
Assigns $(D value) to the internally-held state.

Params:
    value = A value of type `T` to assign to this `NullableRef`.
            If the internal state of this `NullableRef` has not
            been initialized, an error will be thrown in
            non-release mode.
 */
    void opAssign()(T value)
        if (isAssignable!T) //@@@9416@@@
    {
        enum message = "Called `opAssign' on null NullableRef!" ~ T.stringof ~ ".";
        assert(!isNull, message);
        *_value = value;
    }

    ///
    @system unittest
    {
        import std.exception : assertThrown, assertNotThrown;

        NullableRef!int nr;
        assert(nr.isNull);
        assertThrown!Throwable(nr = 42);

        nr.bind(new int(0));
        assert(!nr.isNull);
        assertNotThrown!Throwable(nr = 42);
        assert(nr == 42);
    }

/**
Gets the value. $(D this) must not be in the null state.
This function is also called for the implicit conversion to $(D T).
 */
    @property ref inout(T) get() inout @safe pure nothrow
    {
        enum message = "Called `get' on null NullableRef!" ~ T.stringof ~ ".";
        assert(!isNull, message);
        return *_value;
    }

    ///
    @system unittest
    {
        import std.exception : assertThrown, assertNotThrown;

        NullableRef!int nr;
        //`get` is implicitly called. Will throw
        //an error in non-release mode
        assertThrown!Throwable(nr == 0);

        nr.bind(new int(0));
        assertNotThrown!Throwable(nr == 0);
    }

/**
Implicitly converts to $(D T).
$(D this) must not be in the null state.
 */
    alias get this;
}

/// ditto
auto nullableRef(T)(T* t)
{
    return NullableRef!T(t);
}

///
@system unittest
{
    import std.exception : assertThrown;

    int x = 5, y = 7;
    auto a = nullableRef(&x);
    assert(!a.isNull);
    assert(a == 5);
    assert(x == 5);
    a = 42;
    assert(x == 42);
    assert(!a.isNull);
    assert(a == 42);
    a.nullify();
    assert(x == 42);
    assert(a.isNull);
    assertThrown!Throwable(a.get);
    assertThrown!Throwable(a = 71);
    a.bind(&y);
    assert(a == 7);
    y = 135;
    assert(a == 135);
}
@system unittest
{
    static int f(in NullableRef!int x) {
        return x.isNull ? 42 : x.get;
    }
    int x = 5;
    auto a = nullableRef(&x);
    assert(f(a) == 5);
    a.nullify();
    assert(f(a) == 42);
}
@safe unittest
{
    // Ensure NullableRef can be used in pure/nothrow/@safe environment.
    function() @safe pure nothrow
    {
        auto storage = new int;
        *storage = 19902;
        NullableRef!int n;
        assert(n.isNull);
        n.bind(storage);
        assert(!n.isNull);
        assert(n == 19902);
        n = 2294;
        assert(n == 2294);
        assert(*storage == 2294);
        n.nullify();
        assert(n.isNull);
    }();
}
@system unittest
{
    // Ensure NullableRef can be used when the value is not pure/nothrow/@safe
    static struct S
    {
        int x;
        this(this) @system {}
        bool opEquals(const S s) const @system { return s.x == x; }
    }

    auto storage = S(5);

    NullableRef!S s;
    assert(s.isNull);
    s.bind(&storage);
    assert(!s.isNull);
    assert(s.x == 5);
    s.nullify();
    assert(s.isNull);
}
@safe unittest
{
    //Check nullable is nicelly embedable in a struct
    static struct S1
    {
        NullableRef!int ni;
    }
    static struct S2 //inspired from 9404
    {
        NullableRef!int ni;
        this(S2 other)
        {
            ni = other.ni;
        }
        void opAssign(S2 other)
        {
            ni = other.ni;
        }
    }
    foreach (S; AliasSeq!(S1, S2))
    {
        S a;
        S b = a;
        S c;
        c = a;
    }
}
@system unittest
{
    import std.conv : to;

    // Bugzilla 10915
    NullableRef!int nri;
    assert(nri.to!string() == "Nullable.null");

    struct Test
    {
        string s;
    }
    NullableRef!Test nt = new Test("test");
    assert(nt.to!string() == `Test("test")`);

    class TestToString
    {
        double d;

        this(double d)
        {
            this.d = d;
        }

        override string toString()
        {
            return d.to!string();
        }
    }
    TestToString tts = new TestToString(2.5);
    NullableRef!TestToString ntts = &tts;
    assert(ntts.to!string() == "2.5");
}


/**
$(D BlackHole!Base) is a subclass of $(D Base) which automatically implements
all abstract member functions in $(D Base) as do-nothing functions.  Each
auto-implemented function just returns the default value of the return type
without doing anything.

The name came from
$(HTTP search.cpan.org/~sburke/Class-_BlackHole-0.04/lib/Class/_BlackHole.pm, Class::_BlackHole)
Perl module by Sean M. Burke.

Params:
    Base = A non-final class for `BlackHole` to inherit from.

See_Also:
  $(LREF AutoImplement), $(LREF generateEmptyFunction)
 */
alias BlackHole(Base) = AutoImplement!(Base, generateEmptyFunction, isAbstractFunction);

///
@system unittest
{
    import std.math : isNaN;

    static abstract class C
    {
        int m_value;
        this(int v) { m_value = v; }
        int value() @property { return m_value; }

        abstract real realValue() @property;
        abstract void doSomething();
    }

    auto c = new BlackHole!C(42);
    assert(c.value == 42);

    // Returns real.init which is NaN
    assert(c.realValue.isNaN);
    // Abstract functions are implemented as do-nothing
    c.doSomething();
}

@system unittest
{
    import std.math : isNaN;

    // return default
    {
        interface I_1 { real test(); }
        auto o = new BlackHole!I_1;
        assert(o.test().isNaN()); // NaN
    }
    // doc example
    {
        static class C
        {
            int m_value;
            this(int v) { m_value = v; }
            int value() @property { return m_value; }

            abstract real realValue() @property;
            abstract void doSomething();
        }

        auto c = new BlackHole!C(42);
        assert(c.value == 42);

        assert(c.realValue.isNaN); // NaN
        c.doSomething();
    }

    // Bugzilla 12058
    interface Foo
    {
        inout(Object) foo() inout;
    }
    BlackHole!Foo o;
}


/**
$(D WhiteHole!Base) is a subclass of $(D Base) which automatically implements
all abstract member functions as functions that always fail. These functions
simply throw an $(D Error) and never return. `Whitehole` is useful for
trapping the use of class member functions that haven't been implemented.

The name came from
$(HTTP search.cpan.org/~mschwern/Class-_WhiteHole-0.04/lib/Class/_WhiteHole.pm, Class::_WhiteHole)
Perl module by Michael G Schwern.

Params:
    Base = A non-final class for `WhiteHole` to inherit from.

See_Also:
  $(LREF AutoImplement), $(LREF generateAssertTrap)
 */
alias WhiteHole(Base) = AutoImplement!(Base, generateAssertTrap, isAbstractFunction);

///
@system unittest
{
    import std.exception : assertThrown;

    static class C
    {
        abstract void notYetImplemented();
    }

    auto c = new WhiteHole!C;
    assertThrown!NotImplementedError(c.notYetImplemented()); // throws an Error
}

// / ditto
class NotImplementedError : Error
{
    this(string method)
    {
        super(method ~ " is not implemented");
    }
}

@system unittest
{
    import std.exception : assertThrown;
    // nothrow
    {
        interface I_1
        {
            void foo();
            void bar() nothrow;
        }
        auto o = new WhiteHole!I_1;
        assertThrown!NotImplementedError(o.foo());
        assertThrown!NotImplementedError(o.bar());
    }
    // doc example
    {
        static class C
        {
            abstract void notYetImplemented();
        }

        auto c = new WhiteHole!C;
        try
        {
            c.notYetImplemented();
            assert(0);
        }
        catch (Error e) {}
    }
}


/**
$(D AutoImplement) automatically implements (by default) all abstract member
functions in the class or interface $(D Base) in specified way.

The second version of $(D AutoImplement) automatically implements
$(D Interface), while deriving from $(D BaseClass).

Params:
  how  = template which specifies _how functions will be implemented/overridden.

         Two arguments are passed to $(D how): the type $(D Base) and an alias
         to an implemented function.  Then $(D how) must return an implemented
         function body as a string.

         The generated function body can use these keywords:
         $(UL
            $(LI $(D a0), $(D a1), &hellip;: arguments passed to the function;)
            $(LI $(D args): a tuple of the arguments;)
            $(LI $(D self): an alias to the function itself;)
            $(LI $(D parent): an alias to the overridden function (if any).)
         )

        You may want to use templated property functions (instead of Implicit
        Template Properties) to generate complex functions:
--------------------
// Prints log messages for each call to overridden functions.
string generateLogger(C, alias fun)() @property
{
    import std.traits;
    enum qname = C.stringof ~ "." ~ __traits(identifier, fun);
    string stmt;

    stmt ~= q{ struct Importer { import std.stdio; } };
    stmt ~= `Importer.writeln("Log: ` ~ qname ~ `(", args, ")");`;
    static if (!__traits(isAbstractFunction, fun))
    {
        static if (is(ReturnType!fun == void))
            stmt ~= q{ parent(args); };
        else
            stmt ~= q{
                auto r = parent(args);
                Importer.writeln("--> ", r);
                return r;
            };
    }
    return stmt;
}
--------------------

  what = template which determines _what functions should be
         implemented/overridden.

         An argument is passed to $(D what): an alias to a non-final member
         function in $(D Base).  Then $(D what) must return a boolean value.
         Return $(D true) to indicate that the passed function should be
         implemented/overridden.

--------------------
// Sees if fun returns something.
enum bool hasValue(alias fun) = !is(ReturnType!(fun) == void);
--------------------


Note:

Generated code is inserted in the scope of $(D std.typecons) module.  Thus,
any useful functions outside $(D std.typecons) cannot be used in the generated
code.  To workaround this problem, you may $(D import) necessary things in a
local struct, as done in the $(D generateLogger()) template in the above
example.


BUGS:

$(UL
 $(LI Variadic arguments to constructors are not forwarded to super.)
 $(LI Deep interface inheritance causes compile error with messages like
      "Error: function std.typecons._AutoImplement!(Foo)._AutoImplement.bar
      does not override any function".  [$(BUGZILLA 2525), $(BUGZILLA 3525)] )
 $(LI The $(D parent) keyword is actually a delegate to the super class'
      corresponding member function.  [$(BUGZILLA 2540)] )
 $(LI Using alias template parameter in $(D how) and/or $(D what) may cause
     strange compile error.  Use template tuple parameter instead to workaround
     this problem.  [$(BUGZILLA 4217)] )
)
 */
class AutoImplement(Base, alias how, alias what = isAbstractFunction) : Base
    if (!is(how == class))
{
    private alias autoImplement_helper_ =
        AutoImplement_Helper!("autoImplement_helper_", "Base", Base, typeof(this), how, what);
    mixin(autoImplement_helper_.code);
}

/// ditto
class AutoImplement(
    Interface, BaseClass, alias how,
    alias what = isAbstractFunction) : BaseClass, Interface
    if (is(Interface == interface) && is(BaseClass == class))
{
    private alias autoImplement_helper_ = AutoImplement_Helper!(
            "autoImplement_helper_", "Interface", Interface, typeof(this), how, what);
    mixin(autoImplement_helper_.code);
}

/*
 * Code-generating stuffs are encupsulated in this helper template so that
 * namespace pollution, which can cause name confliction with Base's public
 * members, should be minimized.
 */
private template AutoImplement_Helper(string myName, string baseName,
        Base, Self, alias generateMethodBody, alias cherrypickMethod)
{
private static:
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    // Internal stuffs
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

    // Returns function overload sets in the class C, filtered with pred.
    template enumerateOverloads(C, alias pred)
    {
        template Impl(names...)
        {
            import std.meta : Filter;
            static if (names.length > 0)
            {
                alias methods = Filter!(pred, MemberFunctionsTuple!(C, names[0]));
                alias next = Impl!(names[1 .. $]);

                static if (methods.length > 0)
                    alias Impl = AliasSeq!(OverloadSet!(names[0], methods), next);
                else
                    alias Impl = next;
            }
            else
                alias Impl = AliasSeq!();
        }

        alias enumerateOverloads = Impl!(__traits(allMembers, C));
    }

    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    // Target functions
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

    // Add a non-final check to the cherrypickMethod.
    enum bool canonicalPicker(fun.../+[BUG 4217]+/) =
        !__traits(isFinalFunction, fun[0]) && cherrypickMethod!(fun);

    /*
     * A tuple of overload sets, each item of which consists of functions to be
     * implemented by the generated code.
     */
    alias targetOverloadSets = enumerateOverloads!(Base, canonicalPicker);

    /*
     * Super class of this AutoImplement instance
     */
    alias Super = BaseTypeTuple!(Self)[0];
    static assert(is(Super == class));
    static assert(is(Base == interface) || is(Super == Base));

    /*
     * A tuple of the super class' constructors.  Used for forwarding
     * constructor calls.
     */
    static if (__traits(hasMember, Super, "__ctor"))
        alias ctorOverloadSet = OverloadSet!("__ctor", __traits(getOverloads, Super, "__ctor"));
    else
        alias ctorOverloadSet = OverloadSet!("__ctor"); // empty


    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    // Type information
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

    /*
     * The generated code will be mixed into AutoImplement, which will be
     * instantiated in this module's scope.  Thus, any user-defined types are
     * out of scope and cannot be used directly (i.e. by their names).
     *
     * We will use FuncInfo instances for accessing return types and parameter
     * types of the implemented functions.  The instances will be populated to
     * the AutoImplement's scope in a certain way; see the populate() below.
     */

    // Returns the preferred identifier for the FuncInfo instance for the i-th
    // overloaded function with the name.
    template INTERNAL_FUNCINFO_ID(string name, size_t i)
    {
        import std.format : format;

        enum string INTERNAL_FUNCINFO_ID = format("F_%s_%s", name, i);
    }

    /*
     * Insert FuncInfo instances about all the target functions here.  This
     * enables the generated code to access type information via, for example,
     * "autoImplement_helper_.F_foo_1".
     */
    template populate(overloads...)
    {
        static if (overloads.length > 0)
        {
            mixin populate!(overloads[0].name, overloads[0].contents);
            mixin populate!(overloads[1 .. $]);
        }
    }
    template populate(string name, methods...)
    {
        static if (methods.length > 0)
        {
            mixin populate!(name, methods[0 .. $ - 1]);
            //
            alias target = methods[$ - 1];
            enum ith = methods.length - 1;
            mixin("alias " ~ INTERNAL_FUNCINFO_ID!(name, ith) ~ " = FuncInfo!target;");
        }
    }

    public mixin populate!(targetOverloadSets);
    public mixin populate!(  ctorOverloadSet );


    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    // Code-generating policies
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

    /* Common policy configurations for generating constructors and methods. */
    template CommonGeneratingPolicy()
    {
        // base class identifier which generated code should use
        enum string BASE_CLASS_ID = baseName;

        // FuncInfo instance identifier which generated code should use
        template FUNCINFO_ID(string name, size_t i)
        {
            enum string FUNCINFO_ID =
                myName ~ "." ~ INTERNAL_FUNCINFO_ID!(name, i);
        }
    }

    /* Policy configurations for generating constructors. */
    template ConstructorGeneratingPolicy()
    {
        mixin CommonGeneratingPolicy;

        /* Generates constructor body.  Just forward to the base class' one. */
        string generateFunctionBody(ctor.../+[BUG 4217]+/)() @property
        {
            enum varstyle = variadicFunctionStyle!(typeof(&ctor[0]));

            static if (varstyle & (Variadic.c | Variadic.d))
            {
                // the argptr-forwarding problem
                //pragma(msg, "Warning: AutoImplement!(", Base, ") ",
                //        "ignored variadic arguments to the constructor ",
                //        FunctionTypeOf!(typeof(&ctor[0])) );
            }
            return "super(args);";
        }
    }

    /* Policy configurations for genearting target methods. */
    template MethodGeneratingPolicy()
    {
        mixin CommonGeneratingPolicy;

        /* Geneartes method body. */
        string generateFunctionBody(func.../+[BUG 4217]+/)() @property
        {
            return generateMethodBody!(Base, func); // given
        }
    }


    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    // Generated code
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

    alias ConstructorGenerator = MemberFunctionGenerator!(ConstructorGeneratingPolicy!());
    alias MethodGenerator      = MemberFunctionGenerator!(MethodGeneratingPolicy!());

    public enum string code =
        ConstructorGenerator.generateCode!(  ctorOverloadSet ) ~ "\n" ~
             MethodGenerator.generateCode!(targetOverloadSets);

    debug (SHOW_GENERATED_CODE)
    {
        pragma(msg, "-------------------- < ", Base, " >");
        pragma(msg, code);
        pragma(msg, "--------------------");
    }
}

//debug = SHOW_GENERATED_CODE;
@system unittest
{
    import core.vararg;
    // no function to implement
    {
        interface I_1 {}
        auto o = new BlackHole!I_1;
    }
    // parameters
    {
        interface I_3 { void test(int, in int, out int, ref int, lazy int); }
        auto o = new BlackHole!I_3;
    }
    // use of user-defined type
    {
        struct S {}
        interface I_4 { S test(); }
        auto o = new BlackHole!I_4;
    }
    // overloads
    {
        interface I_5
        {
            void test(string);
            real test(real);
            int  test();
        }
        auto o = new BlackHole!I_5;
    }
    // constructor forwarding
    {
        static class C_6
        {
            this(int n) { assert(n == 42); }
            this(string s) { assert(s == "Deeee"); }
            this(...) {}
        }
        auto o1 = new BlackHole!C_6(42);
        auto o2 = new BlackHole!C_6("Deeee");
        auto o3 = new BlackHole!C_6(1, 2, 3, 4);
    }
    // attributes
    {
        interface I_7
        {
            ref int test_ref();
            int test_pure() pure;
            int test_nothrow() nothrow;
            int test_property() @property;
            int test_safe() @safe;
            int test_trusted() @trusted;
            int test_system() @system;
            int test_pure_nothrow() pure nothrow;
        }
        auto o = new BlackHole!I_7;
    }
    // storage classes
    {
        interface I_8
        {
            void test_const() const;
            void test_immutable() immutable;
            void test_shared() shared;
            void test_shared_const() shared const;
        }
        auto o = new BlackHole!I_8;
    }
    // use baseclass
    {
        static class C_9
        {
            private string foo_;

            this(string s) {
                foo_ = s;
            }

            protected string boilerplate() @property
            {
                return "Boilerplate stuff.";
            }

            public string foo() @property
            {
                return foo_;
            }
        }

        interface I_10
        {
            string testMethod(size_t);
        }

        static string generateTestMethod(C, alias fun)() @property
        {
            return "return this.boilerplate[0 .. a0];";
        }

        auto o = new AutoImplement!(I_10, C_9, generateTestMethod)("Testing");
        assert(o.testMethod(11) == "Boilerplate");
        assert(o.foo == "Testing");
    }
    /+ // deep inheritance
    {
    // XXX [BUG 2525,3525]
    // NOTE: [r494] func.c(504-571) FuncDeclaration::semantic()
        interface I { void foo(); }
        interface J : I {}
        interface K : J {}
        static abstract class C_9 : K {}
        auto o = new BlackHole!C_9;
    }+/
}

// Issue 17177 - AutoImplement fails on function overload sets with "cannot infer type from overloaded function symbol"
@system unittest
{
    static class Issue17177
    {
        private string n_;

        public {
            Issue17177 overloaded(string n)
            {
                this.n_ = n;

                return this;
            }

            string overloaded()
            {
                return this.n_;
            }
        }
    }

    static string how(C, alias fun)()
    {
        static if (!is(ReturnType!fun == void))
        {
            return q{
                return parent(args);
            };
        }
        else
        {
            return q{
                parent(args);
            };
        }
    }

    alias Implementation = AutoImplement!(Issue17177, how, templateNot!isFinalFunction);
}

version (unittest)
{
    // Issue 10647
    // Add prefix "issue10647_" as a workaround for issue 1238
    private string issue10647_generateDoNothing(C, alias fun)() @property
    {
        string stmt;

        static if (is(ReturnType!fun == void))
            stmt ~= "";
        else
        {
            string returnType = ReturnType!fun.stringof;
            stmt ~= "return "~returnType~".init;";
        }
        return stmt;
    }

    private template issue10647_isAlwaysTrue(alias fun)
    {
        enum issue10647_isAlwaysTrue = true;
    }

    // Do nothing template
    private template issue10647_DoNothing(Base)
    {
        alias issue10647_DoNothing = AutoImplement!(Base, issue10647_generateDoNothing, issue10647_isAlwaysTrue);
    }

    // A class to be overridden
    private class issue10647_Foo{
        void bar(int a) { }
    }
}
@system unittest
{
    auto foo = new issue10647_DoNothing!issue10647_Foo();
    foo.bar(13);
}

/*
Used by MemberFunctionGenerator.
 */
package template OverloadSet(string nam, T...)
{
    enum string name = nam;
    alias contents = T;
}

/*
Used by MemberFunctionGenerator.
 */
package template FuncInfo(alias func, /+[BUG 4217 ?]+/ T = typeof(&func))
{
    alias RT = ReturnType!T;
    alias PT = Parameters!T;
}
package template FuncInfo(Func)
{
    alias RT = ReturnType!Func;
    alias PT = Parameters!Func;
}

/*
General-purpose member function generator.
--------------------
template GeneratingPolicy()
{
    // [optional] the name of the class where functions are derived
    enum string BASE_CLASS_ID;

    // [optional] define this if you have only function types
    enum bool WITHOUT_SYMBOL;

    // [optional] Returns preferred identifier for i-th parameter.
    template PARAMETER_VARIABLE_ID(size_t i);

    // Returns the identifier of the FuncInfo instance for the i-th overload
    // of the specified name.  The identifier must be accessible in the scope
    // where generated code is mixed.
    template FUNCINFO_ID(string name, size_t i);

    // Returns implemented function body as a string.  When WITHOUT_SYMBOL is
    // defined, the latter is used.
    template generateFunctionBody(alias func);
    template generateFunctionBody(string name, FuncType);
}
--------------------
 */
package template MemberFunctionGenerator(alias Policy)
{
private static:
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    // Internal stuffs
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    import std.format;

    enum CONSTRUCTOR_NAME = "__ctor";

    // true if functions are derived from a base class
    enum WITH_BASE_CLASS = __traits(hasMember, Policy, "BASE_CLASS_ID");

    // true if functions are specified as types, not symbols
    enum WITHOUT_SYMBOL = __traits(hasMember, Policy, "WITHOUT_SYMBOL");

    // preferred identifier for i-th parameter variable
    static if (__traits(hasMember, Policy, "PARAMETER_VARIABLE_ID"))
    {
        alias PARAMETER_VARIABLE_ID = Policy.PARAMETER_VARIABLE_ID;
    }
    else
    {
        enum string PARAMETER_VARIABLE_ID(size_t i) = format("a%s", i);
            // default: a0, a1, ...
    }

    // Returns a tuple consisting of 0,1,2,...,n-1.  For static foreach.
    template CountUp(size_t n)
    {
        static if (n > 0)
            alias CountUp = AliasSeq!(CountUp!(n - 1), n - 1);
        else
            alias CountUp = AliasSeq!();
    }


    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://
    // Code generator
    //:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::://

    /*
     * Runs through all the target overload sets and generates D code which
     * implements all the functions in the overload sets.
     */
    public string generateCode(overloads...)() @property
    {
        string code = "";

        // run through all the overload sets
        foreach (i_; CountUp!(0 + overloads.length)) // workaround
        {
            enum i = 0 + i_; // workaround
            alias oset = overloads[i];

            code ~= generateCodeForOverloadSet!(oset);

            static if (WITH_BASE_CLASS && oset.name != CONSTRUCTOR_NAME)
            {
                // The generated function declarations may hide existing ones
                // in the base class (cf. HiddenFuncError), so we put an alias
                // declaration here to reveal possible hidden functions.
                code ~= format("alias %s = %s.%s;\n",
                            oset.name,
                            Policy.BASE_CLASS_ID, // [BUG 2540] super.
                            oset.name);
            }
        }
        return code;
    }

    // handle each overload set
    private string generateCodeForOverloadSet(alias oset)() @property
    {
        string code = "";

        foreach (i_; CountUp!(0 + oset.contents.length)) // workaround
        {
            enum i = 0 + i_; // workaround
            code ~= generateFunction!(
                    Policy.FUNCINFO_ID!(oset.name, i), oset.name,
                    oset.contents[i]) ~ "\n";
        }
        return code;
    }

    /*
     * Returns D code which implements the function func.  This function
     * actually generates only the declarator part; the function body part is
     * generated by the functionGenerator() policy.
     */
    public string generateFunction(
            string myFuncInfo, string name, func... )() @property
    {
        import std.format : format;

        enum isCtor = (name == CONSTRUCTOR_NAME);

        string code; // the result

        auto paramsRes = generateParameters!(myFuncInfo, func)();
        code ~= paramsRes.imports;

        /*** Function Declarator ***/
        {
            alias Func = FunctionTypeOf!(func);
            alias FA = FunctionAttribute;
            enum atts     = functionAttributes!(func);
            enum realName = isCtor ? "this" : name;

            // FIXME?? Make it so that these aren't CTFE funcs any more, since
            // Format is deprecated, and format works at compile time?
            /* Made them CTFE funcs just for the sake of Format!(...) */

            // return type with optional "ref"
            static string make_returnType()
            {
                string rtype = "";

                if (!isCtor)
                {
                    if (atts & FA.ref_) rtype ~= "ref ";
                    rtype ~= myFuncInfo ~ ".RT";
                }
                return rtype;
            }
            enum returnType = make_returnType();

            // function attributes attached after declaration
            static string make_postAtts()
            {
                string poatts = "";
                if (atts & FA.pure_   ) poatts ~= " pure";
                if (atts & FA.nothrow_) poatts ~= " nothrow";
                if (atts & FA.property) poatts ~= " @property";
                if (atts & FA.safe    ) poatts ~= " @safe";
                if (atts & FA.trusted ) poatts ~= " @trusted";
                return poatts;
            }
            enum postAtts = make_postAtts();

            // function storage class
            static string make_storageClass()
            {
                string postc = "";
                if (is(Func ==    shared)) postc ~= " shared";
                if (is(Func ==     const)) postc ~= " const";
                if (is(Func ==     inout)) postc ~= " inout";
                if (is(Func == immutable)) postc ~= " immutable";
                return postc;
            }
            enum storageClass = make_storageClass();

            //
            if (__traits(isVirtualMethod, func))
                code ~= "override ";
            code ~= format("extern(%s) %s %s(%s) %s %s\n",
                    functionLinkage!(func),
                    returnType,
                    realName,
                    paramsRes.params,
                    postAtts, storageClass );
        }

        /*** Function Body ***/
        code ~= "{\n";
        {
            enum nparams = Parameters!(func).length;

            /* Declare keywords: args, self and parent. */
            string preamble;

            preamble ~= "alias args = AliasSeq!(" ~ enumerateParameters!(nparams) ~ ");\n";
            if (!isCtor)
            {
                preamble ~= "alias self = " ~ name ~ ";\n";
                if (WITH_BASE_CLASS && !__traits(isAbstractFunction, func))
                    preamble ~= "alias parent = AliasSeq!(__traits(getMember, super, \"" ~ name ~ "\"))[0];";
            }

            // Function body
            static if (WITHOUT_SYMBOL)
                enum fbody = Policy.generateFunctionBody!(name, func);
            else
                enum fbody = Policy.generateFunctionBody!(func);

            code ~= preamble;
            code ~= fbody;
        }
        code ~= "}";

        return code;
    }

    /*
     * Returns D code which declares function parameters,
     * and optionally any imports (e.g. core.vararg)
     * "ref int a0, real a1, ..."
     */
    static struct GenParams { string imports, params; }
    private GenParams generateParameters(string myFuncInfo, func...)()
    {
        alias STC = ParameterStorageClass;
        alias stcs = ParameterStorageClassTuple!(func);
        enum nparams = stcs.length;

        string imports = ""; // any imports required
        string params = ""; // parameters

        foreach (i, stc; stcs)
        {
            if (i > 0) params ~= ", ";

            // Parameter storage classes.
            if (stc & STC.scope_) params ~= "scope ";
            if (stc & STC.out_  ) params ~= "out ";
            if (stc & STC.ref_  ) params ~= "ref ";
            if (stc & STC.lazy_ ) params ~= "lazy ";

            // Take parameter type from the FuncInfo.
            params ~= format("%s.PT[%s]", myFuncInfo, i);

            // Declare a parameter variable.
            params ~= " " ~ PARAMETER_VARIABLE_ID!(i);
        }

        // Add some ellipsis part if needed.
        auto style = variadicFunctionStyle!(func);
        final switch (style)
        {
            case Variadic.no:
                break;

            case Variadic.c, Variadic.d:
                imports ~= "import core.vararg;\n";
                // (...) or (a, b, ...)
                params ~= (nparams == 0) ? "..." : ", ...";
                break;

            case Variadic.typesafe:
                params ~= " ...";
                break;
        }

        return typeof(return)(imports, params);
    }

    // Returns D code which enumerates n parameter variables using comma as the
    // separator.  "a0, a1, a2, a3"
    private string enumerateParameters(size_t n)() @property
    {
        string params = "";

        foreach (i_; CountUp!(n))
        {
            enum i = 0 + i_; // workaround
            if (i > 0) params ~= ", ";
            params ~= PARAMETER_VARIABLE_ID!(i);
        }
        return params;
    }
}


/**
Predefined how-policies for $(D AutoImplement).  These templates are also used by
$(D BlackHole) and $(D WhiteHole), respectively.
 */
template generateEmptyFunction(C, func.../+[BUG 4217]+/)
{
    static if (is(ReturnType!(func) == void))
        enum string generateEmptyFunction = q{
        };
    else static if (functionAttributes!(func) & FunctionAttribute.ref_)
        enum string generateEmptyFunction = q{
            static typeof(return) dummy;
            return dummy;
        };
    else
        enum string generateEmptyFunction = q{
            return typeof(return).init;
        };
}

/// ditto
template generateAssertTrap(C, func...)
{
    enum string generateAssertTrap =
        `throw new NotImplementedError("` ~ C.stringof ~ "."
                ~ __traits(identifier, func) ~ `");`;
}

private
{
    pragma(mangle, "_d_toObject")
    extern(C) pure nothrow Object typecons_d_toObject(void* p);
}

/*
 * Avoids opCast operator overloading.
 */
private template dynamicCast(T)
if (is(T == class) || is(T == interface))
{
    @trusted
    T dynamicCast(S)(inout S source)
    if (is(S == class) || is(S == interface))
    {
        static if (is(Unqual!S : Unqual!T))
        {
            import std.traits : QualifierOf;
            alias Qual = QualifierOf!S; // SharedOf or MutableOf
            alias TmpT = Qual!(Unqual!T);
            inout(TmpT) tmp = source;   // bypass opCast by implicit conversion
            return *cast(T*)(&tmp);     // + variable pointer cast + dereference
        }
        else
        {
            return cast(T) typecons_d_toObject(*cast(void**)(&source));
        }
    }
}

@system unittest
{
    class C { @disable opCast(T)() {} }
    auto c = new C;
    static assert(!__traits(compiles, cast(Object) c));
    auto o = dynamicCast!Object(c);
    assert(c is o);

    interface I { @disable opCast(T)() {} Object instance(); }
    interface J { @disable opCast(T)() {} Object instance(); }
    class D : I, J { Object instance() { return this; } }
    I i = new D();
    static assert(!__traits(compiles, cast(J) i));
    J j = dynamicCast!J(i);
    assert(i.instance() is j.instance());
}

/**
 * Supports structural based typesafe conversion.
 *
 * If $(D Source) has structural conformance with the $(D interface) $(D Targets),
 * wrap creates internal wrapper class which inherits $(D Targets) and
 * wrap $(D src) object, then return it.
 */
template wrap(Targets...)
if (Targets.length >= 1 && allSatisfy!(isMutable, Targets))
{
    import std.meta : staticMap;

    // strict upcast
    auto wrap(Source)(inout Source src) @trusted pure nothrow
    if (Targets.length == 1 && is(Source : Targets[0]))
    {
        alias T = Select!(is(Source == shared), shared Targets[0], Targets[0]);
        return dynamicCast!(inout T)(src);
    }
    // structural upcast
    template wrap(Source)
    if (!allSatisfy!(Bind!(isImplicitlyConvertible, Source), Targets))
    {
        auto wrap(inout Source src)
        {
            static assert(hasRequireMethods!(),
                          "Source "~Source.stringof~
                          " does not have structural conformance to "~
                          Targets.stringof);

            alias T = Select!(is(Source == shared), shared Impl, Impl);
            return new inout T(src);
        }

        template FuncInfo(string s, F)
        {
            enum name = s;
            alias type = F;
        }

        // Concat all Targets function members into one tuple
        template Concat(size_t i = 0)
        {
            static if (i >= Targets.length)
                alias Concat = AliasSeq!();
            else
            {
                alias Concat = AliasSeq!(GetOverloadedMethods!(Targets[i]), Concat!(i + 1));
            }
        }
        // Remove duplicated functions based on the identifier name and function type covariance
        template Uniq(members...)
        {
            static if (members.length == 0)
                alias Uniq = AliasSeq!();
            else
            {
                alias func = members[0];
                enum  name = __traits(identifier, func);
                alias type = FunctionTypeOf!func;
                template check(size_t i, mem...)
                {
                    static if (i >= mem.length)
                        enum ptrdiff_t check = -1;
                    else
                    {
                        enum ptrdiff_t check =
                            __traits(identifier, func) == __traits(identifier, mem[i]) &&
                            !is(DerivedFunctionType!(type, FunctionTypeOf!(mem[i])) == void)
                          ? i : check!(i + 1, mem);
                    }
                }
                enum ptrdiff_t x = 1 + check!(0, members[1 .. $]);
                static if (x >= 1)
                {
                    alias typex = DerivedFunctionType!(type, FunctionTypeOf!(members[x]));
                    alias remain = Uniq!(members[1 .. x], members[x + 1 .. $]);

                    static if (remain.length >= 1 && remain[0].name == name &&
                               !is(DerivedFunctionType!(typex, remain[0].type) == void))
                    {
                        alias F = DerivedFunctionType!(typex, remain[0].type);
                        alias Uniq = AliasSeq!(FuncInfo!(name, F), remain[1 .. $]);
                    }
                    else
                        alias Uniq = AliasSeq!(FuncInfo!(name, typex), remain);
                }
                else
                {
                    alias Uniq = AliasSeq!(FuncInfo!(name, type), Uniq!(members[1 .. $]));
                }
            }
        }
        alias TargetMembers = Uniq!(Concat!());             // list of FuncInfo
        alias SourceMembers = GetOverloadedMethods!Source;  // list of function symbols

        // Check whether all of SourceMembers satisfy covariance target in TargetMembers
        template hasRequireMethods(size_t i = 0)
        {
            static if (i >= TargetMembers.length)
                enum hasRequireMethods = true;
            else
            {
                enum hasRequireMethods =
                    findCovariantFunction!(TargetMembers[i], Source, SourceMembers) != -1 &&
                    hasRequireMethods!(i + 1);
            }
        }

        // Internal wrapper class
        final class Impl : Structural, Targets
        {
        private:
            Source _wrap_source;

            this(       inout Source s)        inout @safe pure nothrow { _wrap_source = s; }
            this(shared inout Source s) shared inout @safe pure nothrow { _wrap_source = s; }

            // BUG: making private should work with NVI.
            protected final inout(Object) _wrap_getSource() inout @trusted
            {
                return dynamicCast!(inout Object)(_wrap_source);
            }

            import std.conv : to;
            import std.functional : forward;
            template generateFun(size_t i)
            {
                enum name = TargetMembers[i].name;
                enum fa = functionAttributes!(TargetMembers[i].type);
                static @property stc()
                {
                    string r;
                    if (fa & FunctionAttribute.property)    r ~= "@property ";
                    if (fa & FunctionAttribute.ref_)        r ~= "ref ";
                    if (fa & FunctionAttribute.pure_)       r ~= "pure ";
                    if (fa & FunctionAttribute.nothrow_)    r ~= "nothrow ";
                    if (fa & FunctionAttribute.trusted)     r ~= "@trusted ";
                    if (fa & FunctionAttribute.safe)        r ~= "@safe ";
                    return r;
                }
                static @property mod()
                {
                    alias type = AliasSeq!(TargetMembers[i].type)[0];
                    string r;
                    static if (is(type == immutable))       r ~= " immutable";
                    else
                    {
                        static if (is(type == shared))      r ~= " shared";
                        static if (is(type == const))       r ~= " const";
                        else static if (is(type == inout))  r ~= " inout";
                        //else  --> mutable
                    }
                    return r;
                }
                enum n = to!string(i);
                static if (fa & FunctionAttribute.property)
                {
                    static if (Parameters!(TargetMembers[i].type).length == 0)
                        enum fbody = "_wrap_source."~name;
                    else
                        enum fbody = "_wrap_source."~name~" = forward!args";
                }
                else
                {
                        enum fbody = "_wrap_source."~name~"(forward!args)";
                }
                enum generateFun =
                    "override "~stc~"ReturnType!(TargetMembers["~n~"].type) "
                    ~ name~"(Parameters!(TargetMembers["~n~"].type) args) "~mod~
                    "{ return "~fbody~"; }";
            }

        public:
            mixin mixinAll!(
                staticMap!(generateFun, staticIota!(0, TargetMembers.length)));
        }
    }
}
/// ditto
template wrap(Targets...)
if (Targets.length >= 1 && !allSatisfy!(isMutable, Targets))
{
    import std.meta : staticMap;

    alias wrap = .wrap!(staticMap!(Unqual, Targets));
}

// Internal class to support dynamic cross-casting
private interface Structural
{
    inout(Object) _wrap_getSource() inout @safe pure nothrow;
}

/**
 * Extract object which wrapped by $(D wrap).
 */
template unwrap(Target)
if (isMutable!Target)
{
    // strict downcast
    auto unwrap(Source)(inout Source src) @trusted pure nothrow
    if (is(Target : Source))
    {
        alias T = Select!(is(Source == shared), shared Target, Target);
        return dynamicCast!(inout T)(src);
    }
    // structural downcast
    auto unwrap(Source)(inout Source src) @trusted pure nothrow
    if (!is(Target : Source))
    {
        alias T = Select!(is(Source == shared), shared Target, Target);
        Object o = dynamicCast!(Object)(src);   // remove qualifier
        do
        {
            if (auto a = dynamicCast!(Structural)(o))
            {
                if (auto d = dynamicCast!(inout T)(o = a._wrap_getSource()))
                    return d;
            }
            else if (auto d = dynamicCast!(inout T)(o))
                return d;
            else
                break;
        } while (o);
        return null;
    }
}
/// ditto
template unwrap(Target)
if (!isMutable!Target)
{
    alias unwrap = .unwrap!(Unqual!Target);
}

///
@system unittest
{
    interface Quack
    {
        int quack();
        @property int height();
    }
    interface Flyer
    {
        @property int height();
    }
    class Duck : Quack
    {
        int quack() { return 1; }
        @property int height() { return 10; }
    }
    class Human
    {
        int quack() { return 2; }
        @property int height() { return 20; }
    }

    Duck d1 = new Duck();
    Human h1 = new Human();

    interface Refleshable
    {
        int reflesh();
    }
    // does not have structural conformance
    static assert(!__traits(compiles, d1.wrap!Refleshable));
    static assert(!__traits(compiles, h1.wrap!Refleshable));

    // strict upcast
    Quack qd = d1.wrap!Quack;
    assert(qd is d1);
    assert(qd.quack() == 1);    // calls Duck.quack
    // strict downcast
    Duck d2 = qd.unwrap!Duck;
    assert(d2 is d1);

    // structural upcast
    Quack qh = h1.wrap!Quack;
    assert(qh.quack() == 2);    // calls Human.quack
    // structural downcast
    Human h2 = qh.unwrap!Human;
    assert(h2 is h1);

    // structural upcast (two steps)
    Quack qx = h1.wrap!Quack;   // Human -> Quack
    Flyer fx = qx.wrap!Flyer;   // Quack -> Flyer
    assert(fx.height == 20);    // calls Human.height
    // strucural downcast (two steps)
    Quack qy = fx.unwrap!Quack; // Flyer -> Quack
    Human hy = qy.unwrap!Human; // Quack -> Human
    assert(hy is h1);
    // strucural downcast (one step)
    Human hz = fx.unwrap!Human; // Flyer -> Human
    assert(hz is h1);
}
///
@system unittest
{
    import std.traits : FunctionAttribute, functionAttributes;
    interface A { int run(); }
    interface B { int stop(); @property int status(); }
    class X
    {
        int run() { return 1; }
        int stop() { return 2; }
        @property int status() { return 3; }
    }

    auto x = new X();
    auto ab = x.wrap!(A, B);
    A a = ab;
    B b = ab;
    assert(a.run() == 1);
    assert(b.stop() == 2);
    assert(b.status == 3);
    static assert(functionAttributes!(typeof(ab).status) & FunctionAttribute.property);
}
@system unittest
{
    class A
    {
        int draw()              { return 1; }
        int draw(int v)         { return v; }

        int draw() const        { return 2; }
        int draw() shared       { return 3; }
        int draw() shared const { return 4; }
        int draw() immutable    { return 5; }
    }
    interface Drawable
    {
        int draw();
        int draw() const;
        int draw() shared;
        int draw() shared const;
        int draw() immutable;
    }
    interface Drawable2
    {
        int draw(int v);
    }

    auto ma = new A();
    auto sa = new shared A();
    auto ia = new immutable A();
    {
                     Drawable  md = ma.wrap!Drawable;
               const Drawable  cd = ma.wrap!Drawable;
              shared Drawable  sd = sa.wrap!Drawable;
        shared const Drawable scd = sa.wrap!Drawable;
           immutable Drawable  id = ia.wrap!Drawable;
        assert( md.draw() == 1);
        assert( cd.draw() == 2);
        assert( sd.draw() == 3);
        assert(scd.draw() == 4);
        assert( id.draw() == 5);
    }
    {
        Drawable2 d = ma.wrap!Drawable2;
        static assert(!__traits(compiles, d.draw()));
        assert(d.draw(10) == 10);
    }
}
@system unittest
{
    // Bugzilla 10377
    import std.range, std.algorithm;

    interface MyInputRange(T)
    {
        @property T front();
        void popFront();
        @property bool empty();
    }

    //auto o = iota(0,10,1).inputRangeObject();
    //pragma(msg, __traits(allMembers, typeof(o)));
    auto r = iota(0,10,1).inputRangeObject().wrap!(MyInputRange!int)();
    assert(equal(r, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9]));
}
@system unittest
{
    // Bugzilla 10536
    interface Interface
    {
        int foo();
    }
    class Pluggable
    {
        int foo() { return 1; }
        @disable void opCast(T, this X)();  // !
    }

    Interface i = new Pluggable().wrap!Interface;
    assert(i.foo() == 1);
}
@system unittest
{
    // Enhancement 10538
    interface Interface
    {
        int foo();
        int bar(int);
    }
    class Pluggable
    {
        int opDispatch(string name, A...)(A args) { return 100; }
    }

    Interface i = wrap!Interface(new Pluggable());
    assert(i.foo() == 100);
    assert(i.bar(10) == 100);
}

// Make a tuple of non-static function symbols
package template GetOverloadedMethods(T)
{
    import std.meta : Filter;

    alias allMembers = AliasSeq!(__traits(allMembers, T));
    template follows(size_t i = 0)
    {
        static if (i >= allMembers.length)
        {
            alias follows = AliasSeq!();
        }
        else static if (!__traits(compiles, mixin("T."~allMembers[i])))
        {
            alias follows = follows!(i + 1);
        }
        else
        {
            enum name = allMembers[i];

            template isMethod(alias f)
            {
                static if (is(typeof(&f) F == F*) && is(F == function))
                    enum isMethod = !__traits(isStaticFunction, f);
                else
                    enum isMethod = false;
            }
            alias follows = AliasSeq!(
                std.meta.Filter!(isMethod, __traits(getOverloads, T, name)),
                follows!(i + 1));
        }
    }
    alias GetOverloadedMethods = follows!();
}
// find a function from Fs that has same identifier and covariant type with f
private template findCovariantFunction(alias finfo, Source, Fs...)
{
    template check(size_t i = 0)
    {
        static if (i >= Fs.length)
            enum ptrdiff_t check = -1;
        else
        {
            enum ptrdiff_t check =
                (finfo.name == __traits(identifier, Fs[i])) &&
                isCovariantWith!(FunctionTypeOf!(Fs[i]), finfo.type)
              ? i : check!(i + 1);
        }
    }
    enum x = check!();
    static if (x == -1 && is(typeof(Source.opDispatch)))
    {
        alias Params = Parameters!(finfo.type);
        enum ptrdiff_t findCovariantFunction =
            is(typeof((             Source).init.opDispatch!(finfo.name)(Params.init))) ||
            is(typeof((       const Source).init.opDispatch!(finfo.name)(Params.init))) ||
            is(typeof((   immutable Source).init.opDispatch!(finfo.name)(Params.init))) ||
            is(typeof((      shared Source).init.opDispatch!(finfo.name)(Params.init))) ||
            is(typeof((shared const Source).init.opDispatch!(finfo.name)(Params.init)))
          ? ptrdiff_t.max : -1;
    }
    else
        enum ptrdiff_t findCovariantFunction = x;
}

private enum TypeModifier
{
    mutable     = 0,    // type is mutable
    const_      = 1,    // type is const
    immutable_  = 2,    // type is immutable
    shared_     = 4,    // type is shared
    inout_      = 8,    // type is wild
}
private template TypeMod(T)
{
    static if (is(T == immutable))
    {
        enum mod1 = TypeModifier.immutable_;
        enum mod2 = 0;
    }
    else
    {
        enum mod1 = is(T == shared) ? TypeModifier.shared_ : 0;
        static if (is(T == const))
            enum mod2 = TypeModifier.const_;
        else static if (is(T == inout))
            enum mod2 = TypeModifier.inout_;
        else
            enum mod2 = TypeModifier.mutable;
    }
    enum TypeMod = cast(TypeModifier)(mod1 | mod2);
}

version (unittest)
{
    private template UnittestFuncInfo(alias f)
    {
        enum name = __traits(identifier, f);
        alias type = FunctionTypeOf!f;
    }
}
@system unittest
{
    class A
    {
        int draw() { return 1; }
        @property int value() { return 2; }
        final int run() { return 3; }
    }
    alias methods = GetOverloadedMethods!A;

    alias int F1();
    alias @property int F2();
    alias string F3();
    alias nothrow @trusted uint F4();
    alias int F5(Object);
    alias bool F6(Object);
    static assert(methods.length == 3 + 4);
    static assert(__traits(identifier, methods[0]) == "draw"     && is(typeof(&methods[0]) == F1*));
    static assert(__traits(identifier, methods[1]) == "value"    && is(typeof(&methods[1]) == F2*));
    static assert(__traits(identifier, methods[2]) == "run"      && is(typeof(&methods[2]) == F1*));

    int draw();
    @property int value();
    void opEquals();
    int nomatch();
    static assert(findCovariantFunction!(UnittestFuncInfo!draw,     A, methods) == 0);
    static assert(findCovariantFunction!(UnittestFuncInfo!value,    A, methods) == 1);
    static assert(findCovariantFunction!(UnittestFuncInfo!opEquals, A, methods) == -1);
    static assert(findCovariantFunction!(UnittestFuncInfo!nomatch,  A, methods) == -1);

    // considering opDispatch
    class B
    {
        void opDispatch(string name, A...)(A) {}
    }
    alias methodsB = GetOverloadedMethods!B;
    static assert(findCovariantFunction!(UnittestFuncInfo!draw,     B, methodsB) == ptrdiff_t.max);
    static assert(findCovariantFunction!(UnittestFuncInfo!value,    B, methodsB) == ptrdiff_t.max);
    static assert(findCovariantFunction!(UnittestFuncInfo!opEquals, B, methodsB) == ptrdiff_t.max);
    static assert(findCovariantFunction!(UnittestFuncInfo!nomatch,  B, methodsB) == ptrdiff_t.max);
}

package template DerivedFunctionType(T...)
{
    static if (!T.length)
    {
        alias DerivedFunctionType = void;
    }
    else static if (T.length == 1)
    {
        static if (is(T[0] == function))
        {
            alias DerivedFunctionType = T[0];
        }
        else
        {
            alias DerivedFunctionType = void;
        }
    }
    else static if (is(T[0] P0 == function) && is(T[1] P1 == function))
    {
        alias FA = FunctionAttribute;

        alias F0 = T[0], R0 = ReturnType!F0, PSTC0 = ParameterStorageClassTuple!F0;
        alias F1 = T[1], R1 = ReturnType!F1, PSTC1 = ParameterStorageClassTuple!F1;
        enum FA0 = functionAttributes!F0;
        enum FA1 = functionAttributes!F1;

        template CheckParams(size_t i = 0)
        {
            static if (i >= P0.length)
                enum CheckParams = true;
            else
            {
                enum CheckParams = (is(P0[i] == P1[i]) && PSTC0[i] == PSTC1[i]) &&
                                   CheckParams!(i + 1);
            }
        }
        static if (R0.sizeof == R1.sizeof && !is(CommonType!(R0, R1) == void) &&
                   P0.length == P1.length && CheckParams!() && TypeMod!F0 == TypeMod!F1 &&
                   variadicFunctionStyle!F0 == variadicFunctionStyle!F1 &&
                   functionLinkage!F0 == functionLinkage!F1 &&
                   ((FA0 ^ FA1) & (FA.ref_ | FA.property)) == 0)
        {
            alias R = Select!(is(R0 : R1), R0, R1);
            alias FX = FunctionTypeOf!(R function(P0));
            // @system is default
            alias FY = SetFunctionAttributes!(FX, functionLinkage!F0, (FA0 | FA1) & ~FA.system);
            alias DerivedFunctionType = DerivedFunctionType!(FY, T[2 .. $]);
        }
        else
            alias DerivedFunctionType = void;
    }
    else
        alias DerivedFunctionType = void;
}
@safe unittest
{
    // attribute covariance
    alias int F1();
    static assert(is(DerivedFunctionType!(F1, F1) == F1));
    alias int F2() pure nothrow;
    static assert(is(DerivedFunctionType!(F1, F2) == F2));
    alias int F3() @safe;
    alias int F23() @safe pure nothrow;
    static assert(is(DerivedFunctionType!(F2, F3) == F23));

    // return type covariance
    alias long F4();
    static assert(is(DerivedFunctionType!(F1, F4) == void));
    class C {}
    class D : C {}
    alias C F5();
    alias D F6();
    static assert(is(DerivedFunctionType!(F5, F6) == F6));
    alias typeof(null) F7();
    alias int[] F8();
    alias int* F9();
    static assert(is(DerivedFunctionType!(F5, F7) == F7));
    static assert(is(DerivedFunctionType!(F7, F8) == void));
    static assert(is(DerivedFunctionType!(F7, F9) == F7));

    // variadic type equality
    alias int F10(int);
    alias int F11(int...);
    alias int F12(int, ...);
    static assert(is(DerivedFunctionType!(F10, F11) == void));
    static assert(is(DerivedFunctionType!(F10, F12) == void));
    static assert(is(DerivedFunctionType!(F11, F12) == void));

    // linkage equality
    alias extern(C) int F13(int);
    alias extern(D) int F14(int);
    alias extern(Windows) int F15(int);
    static assert(is(DerivedFunctionType!(F13, F14) == void));
    static assert(is(DerivedFunctionType!(F13, F15) == void));
    static assert(is(DerivedFunctionType!(F14, F15) == void));

    // ref & @property equality
    alias int F16(int);
    alias ref int F17(int);
    alias @property int F18(int);
    static assert(is(DerivedFunctionType!(F16, F17) == void));
    static assert(is(DerivedFunctionType!(F16, F18) == void));
    static assert(is(DerivedFunctionType!(F17, F18) == void));
}

package template staticIota(int beg, int end)
{
    static if (beg + 1 >= end)
    {
        static if (beg >= end)
        {
            alias staticIota = AliasSeq!();
        }
        else
        {
            alias staticIota = AliasSeq!(+beg);
        }
    }
    else
    {
        enum mid = beg + (end - beg) / 2;
        alias staticIota = AliasSeq!(staticIota!(beg, mid), staticIota!(mid, end));
    }
}

package template mixinAll(mixins...)
{
    static if (mixins.length == 1)
    {
        static if (is(typeof(mixins[0]) == string))
        {
            mixin(mixins[0]);
        }
        else
        {
            alias it = mixins[0];
            mixin it;
        }
    }
    else static if (mixins.length >= 2)
    {
        mixin mixinAll!(mixins[ 0 .. $/2]);
        mixin mixinAll!(mixins[$/2 .. $ ]);
    }
}

package template Bind(alias Template, args1...)
{
    alias Bind(args2...) = Template!(args1, args2);
}


/**
Options regarding auto-initialization of a $(D RefCounted) object (see
the definition of $(D RefCounted) below).
 */
enum RefCountedAutoInitialize
{
    /// Do not auto-initialize the object
    no,
    /// Auto-initialize the object
    yes,
}

/**
Defines a reference-counted object containing a $(D T) value as
payload.

An instance of $(D RefCounted) is a reference to a structure,
which is referred to as the $(I store), or $(I storage implementation
struct) in this documentation.  The store contains a reference count
and the $(D T) payload.  $(D RefCounted) uses $(D malloc) to allocate
the store.  As instances of $(D RefCounted) are copied or go out of
scope, they will automatically increment or decrement the reference
count.  When the reference count goes down to zero, $(D RefCounted)
will call $(D destroy) against the payload and call $(D free) to
deallocate the store.  If the $(D T) payload contains any references
to GC-allocated memory, then `RefCounted` will add it to the GC memory
that is scanned for pointers, and remove it from GC scanning before
$(D free) is called on the store.

One important consequence of $(D destroy) is that it will call the
destructor of the $(D T) payload.  GC-managed references are not
guaranteed to be valid during a destructor call, but other members of
$(D T), such as file handles or pointers to $(D malloc) memory, will
still be valid during the destructor call.  This allows the $(D T) to
deallocate or clean up any non-GC resources immediately after the
reference count has reached zero.

$(D RefCounted) is unsafe and should be used with care. No references
to the payload should be escaped outside the $(D RefCounted) object.

The $(D autoInit) option makes the object ensure the store is
automatically initialized. Leaving $(D autoInit ==
RefCountedAutoInitialize.yes) (the default option) is convenient but
has the cost of a test whenever the payload is accessed. If $(D
autoInit == RefCountedAutoInitialize.no), user code must call either
$(D refCountedStore.isInitialized) or $(D refCountedStore.ensureInitialized)
before attempting to access the payload. Not doing so results in null
pointer dereference.
 */
struct RefCounted(T, RefCountedAutoInitialize autoInit =
        RefCountedAutoInitialize.yes)
if (!is(T == class) && !(is(T == interface)))
{
    extern(C) private pure nothrow @nogc static // TODO remove pure when https://issues.dlang.org/show_bug.cgi?id=15862 has been fixed
    {
        pragma(mangle, "free") void pureFree( void *ptr );
        pragma(mangle, "gc_addRange") void pureGcAddRange( in void* p, size_t sz, const TypeInfo ti = null );
        pragma(mangle, "gc_removeRange") void pureGcRemoveRange( in void* p );
    }

    /// $(D RefCounted) storage implementation.
    struct RefCountedStore
    {
        import core.memory : pureMalloc;
        private struct Impl
        {
            T _payload;
            size_t _count;
        }

        private Impl* _store;

        private void initialize(A...)(auto ref A args)
        {
            import core.exception : onOutOfMemoryError;
            import std.conv : emplace;

            _store = cast(Impl*) pureMalloc(Impl.sizeof);
            if (_store is null)
                onOutOfMemoryError();
            static if (hasIndirections!T)
                pureGcAddRange(&_store._payload, T.sizeof);
            emplace(&_store._payload, args);
            _store._count = 1;
        }

        private void move(ref T source)
        {
            import core.exception : onOutOfMemoryError;
            import core.stdc.string : memcpy, memset;

            _store = cast(Impl*) pureMalloc(Impl.sizeof);
            if (_store is null)
                onOutOfMemoryError();
            static if (hasIndirections!T)
                pureGcAddRange(&_store._payload, T.sizeof);

            // Can't use std.algorithm.move(source, _store._payload)
            // here because it requires the target to be initialized.
            // Might be worth to add this as `moveEmplace`

            // Can avoid destructing result.
            static if (hasElaborateAssign!T || !isAssignable!T)
                memcpy(&_store._payload, &source, T.sizeof);
            else
                _store._payload = source;

            // If the source defines a destructor or a postblit hook, we must obliterate the
            // object in order to avoid double freeing and undue aliasing
            static if (hasElaborateDestructor!T || hasElaborateCopyConstructor!T)
            {
                // If T is nested struct, keep original context pointer
                static if (__traits(isNested, T))
                    enum sz = T.sizeof - (void*).sizeof;
                else
                    enum sz = T.sizeof;

                auto init = typeid(T).initializer();
                if (init.ptr is null) // null ptr means initialize to 0s
                    memset(&source, 0, sz);
                else
                    memcpy(&source, init.ptr, sz);
            }

            _store._count = 1;
        }

        /**
           Returns $(D true) if and only if the underlying store has been
           allocated and initialized.
        */
        @property nothrow @safe pure @nogc
        bool isInitialized() const
        {
            return _store !is null;
        }

        /**
           Returns underlying reference count if it is allocated and initialized
           (a positive integer), and $(D 0) otherwise.
        */
        @property nothrow @safe pure @nogc
        size_t refCount() const
        {
            return isInitialized ? _store._count : 0;
        }

        /**
           Makes sure the payload was properly initialized. Such a
           call is typically inserted before using the payload.
        */
        void ensureInitialized()
        {
            if (!isInitialized) initialize();
        }

    }
    RefCountedStore _refCounted;

    /// Returns storage implementation struct.
    @property nothrow @safe
    ref inout(RefCountedStore) refCountedStore() inout
    {
        return _refCounted;
    }

/**
Constructor that initializes the payload.

Postcondition: $(D refCountedStore.isInitialized)
 */
    this(A...)(auto ref A args) if (A.length > 0)
    {
        _refCounted.initialize(args);
    }

    /// Ditto
    this(T val)
    {
        _refCounted.move(val);
    }

/**
Constructor that tracks the reference count appropriately. If $(D
!refCountedStore.isInitialized), does nothing.
 */
    this(this) @safe pure nothrow @nogc
    {
        if (!_refCounted.isInitialized) return;
        ++_refCounted._store._count;
    }

/**
Destructor that tracks the reference count appropriately. If $(D
!refCountedStore.isInitialized), does nothing. When the reference count goes
down to zero, calls $(D destroy) agaist the payload and calls $(D free)
to deallocate the corresponding resource.
 */
    ~this()
    {
        if (!_refCounted.isInitialized) return;
        assert(_refCounted._store._count > 0);
        if (--_refCounted._store._count)
            return;
        // Done, deallocate
        .destroy(_refCounted._store._payload);
        static if (hasIndirections!T)
        {
            pureGcRemoveRange(&_refCounted._store._payload);
        }

        pureFree(_refCounted._store);
        _refCounted._store = null;
    }

/**
Assignment operators
 */
    void opAssign(typeof(this) rhs)
    {
        import std.algorithm.mutation : swap;

        swap(_refCounted._store, rhs._refCounted._store);
    }

/// Ditto
    void opAssign(T rhs)
    {
        import std.algorithm.mutation : move;

        static if (autoInit == RefCountedAutoInitialize.yes)
        {
            _refCounted.ensureInitialized();
        }
        else
        {
            assert(_refCounted.isInitialized);
        }
        move(rhs, _refCounted._store._payload);
    }

    //version to have a single properly ddoc'ed function (w/ correct sig)
    version (StdDdoc)
    {
        /**
        Returns a reference to the payload. If (autoInit ==
        RefCountedAutoInitialize.yes), calls $(D
        refCountedStore.ensureInitialized). Otherwise, just issues $(D
        assert(refCountedStore.isInitialized)). Used with $(D alias
        refCountedPayload this;), so callers can just use the $(D RefCounted)
        object as a $(D T).

        $(BLUE The first overload exists only if $(D autoInit == RefCountedAutoInitialize.yes).)
        So if $(D autoInit == RefCountedAutoInitialize.no)
        or called for a constant or immutable object, then
        $(D refCountedPayload) will also be qualified as safe and nothrow
        (but will still assert if not initialized).
         */
        @property
        ref T refCountedPayload() return;

        /// ditto
        @property nothrow @safe pure @nogc
        ref inout(T) refCountedPayload() inout return;
    }
    else
    {
        static if (autoInit == RefCountedAutoInitialize.yes)
        {
            //Can't use inout here because of potential mutation
            @property
            ref T refCountedPayload() return
            {
                _refCounted.ensureInitialized();
                return _refCounted._store._payload;
            }
        }

        @property nothrow @safe pure @nogc
        ref inout(T) refCountedPayload() inout return
        {
            assert(_refCounted.isInitialized, "Attempted to access an uninitialized payload.");
            return _refCounted._store._payload;
        }
    }

/**
Returns a reference to the payload. If (autoInit ==
RefCountedAutoInitialize.yes), calls $(D
refCountedStore.ensureInitialized). Otherwise, just issues $(D
assert(refCountedStore.isInitialized)).
 */
    alias refCountedPayload this;
}

///
pure @system nothrow @nogc unittest
{
    // A pair of an `int` and a `size_t` - the latter being the
    // reference count - will be dynamically allocated
    auto rc1 = RefCounted!int(5);
    assert(rc1 == 5);
    // No more allocation, add just one extra reference count
    auto rc2 = rc1;
    // Reference semantics
    rc2 = 42;
    assert(rc1 == 42);
    // the pair will be freed when rc1 and rc2 go out of scope
}

pure @system unittest
{
    RefCounted!int* p;
    {
        auto rc1 = RefCounted!int(5);
        p = &rc1;
        assert(rc1 == 5);
        assert(rc1._refCounted._store._count == 1);
        auto rc2 = rc1;
        assert(rc1._refCounted._store._count == 2);
        // Reference semantics
        rc2 = 42;
        assert(rc1 == 42);
        rc2 = rc2;
        assert(rc2._refCounted._store._count == 2);
        rc1 = rc2;
        assert(rc1._refCounted._store._count == 2);
    }
    assert(p._refCounted._store == null);

    // RefCounted as a member
    struct A
    {
        RefCounted!int x;
        this(int y)
        {
            x._refCounted.initialize(y);
        }
        A copy()
        {
            auto another = this;
            return another;
        }
    }
    auto a = A(4);
    auto b = a.copy();
    assert(a.x._refCounted._store._count == 2, "BUG 4356 still unfixed");
}

pure @system nothrow @nogc unittest
{
    import std.algorithm.mutation : swap;

    RefCounted!int p1, p2;
    swap(p1, p2);
}

// 6606
@safe pure nothrow @nogc unittest
{
    union U {
       size_t i;
       void* p;
    }

    struct S {
       U u;
    }

    alias SRC = RefCounted!S;
}

// 6436
@system pure unittest
{
    struct S { this(ref int val) { assert(val == 3); ++val; } }

    int val = 3;
    auto s = RefCounted!S(val);
    assert(val == 4);
}

// gc_addRange coverage
@system pure unittest
{
    struct S { int* p; }

    auto s = RefCounted!S(null);
}

@system pure nothrow @nogc unittest
{
    RefCounted!int a;
    a = 5; //This should not assert
    assert(a == 5);

    RefCounted!int b;
    b = a; //This should not assert either
    assert(b == 5);

    RefCounted!(int*) c;
}

/**
 * Initializes a `RefCounted` with `val`. The template parameter
 * `T` of `RefCounted` is inferred from `val`.
 * This function can be used to move non-copyable values to the heap.
 * It also disables the `autoInit` option of `RefCounted`.
 *
 * Params:
 *   val = The value to be reference counted
 * Returns:
 *   An initialized $(D RefCounted) containing $(D val).
 * See_Also:
 *   $(HTTP en.cppreference.com/w/cpp/memory/shared_ptr/make_shared, C++'s make_shared)
 */
RefCounted!(T, RefCountedAutoInitialize.no) refCounted(T)(T val)
{
    typeof(return) res;
    res._refCounted.move(val);
    return res;
}

///
@system unittest
{
    static struct File
    {
        string name;
        @disable this(this); // not copyable
        ~this() { name = null; }
    }

    auto file = File("name");
    assert(file.name == "name");
    // file cannot be copied and has unique ownership
    static assert(!__traits(compiles, {auto file2 = file;}));

    // make the file refcounted to share ownership
    import std.algorithm.mutation : move;
    auto rcFile = refCounted(move(file));
    assert(rcFile.name == "name");
    assert(file.name == null);
    auto rcFile2 = rcFile;
    assert(rcFile.refCountedStore.refCount == 2);
    // file gets properly closed when last reference is dropped
}

/**
    Creates a proxy for the value `a` that will forward all operations
    while disabling implicit conversions. The aliased item `a` must be
    an $(B lvalue). This is useful for creating a new type from the
    "base" type (though this is $(B not) a subtype-supertype
    relationship; the new type is not related to the old type in any way,
    by design).

    The new type supports all operations that the underlying type does,
    including all operators such as `+`, `--`, `<`, `[]`, etc.

    Params:
        a = The value to act as a proxy for all operations. It must
            be an lvalue.
 */
mixin template Proxy(alias a)
{
    private alias ValueType = typeof({ return a; }());

    /* Determine if 'T.a' can referenced via a const(T).
     * Use T* as the parameter because 'scope' inference needs a fully
     * analyzed T, which doesn't work when accessibleFrom() is used in a
     * 'static if' in the definition of Proxy or T.
     */
    private enum bool accessibleFrom(T) =
        is(typeof((T* self){ cast(void) mixin("(*self)."~__traits(identifier, a)); }));

    static if (is(typeof(this) == class))
    {
        override bool opEquals(Object o)
        {
            if (auto b = cast(typeof(this))o)
            {
                return a == mixin("b."~__traits(identifier, a));
            }
            return false;
        }

        bool opEquals(T)(T b)
            if (is(ValueType : T) || is(typeof(a.opEquals(b))) || is(typeof(b.opEquals(a))))
        {
            static if (is(typeof(a.opEquals(b))))
                return a.opEquals(b);
            else static if (is(typeof(b.opEquals(a))))
                return b.opEquals(a);
            else
                return a == b;
        }

        override int opCmp(Object o)
        {
            if (auto b = cast(typeof(this))o)
            {
                return a < mixin("b."~__traits(identifier, a)) ? -1
                     : a > mixin("b."~__traits(identifier, a)) ? +1 : 0;
            }
            static if (is(ValueType == class))
                return a.opCmp(o);
            else
                throw new Exception("Attempt to compare a "~typeid(this).toString~" and a "~typeid(o).toString);
        }

        int opCmp(T)(auto ref const T b)
            if (is(ValueType : T) || is(typeof(a.opCmp(b))) || is(typeof(b.opCmp(a))))
        {
            static if (is(typeof(a.opCmp(b))))
                return a.opCmp(b);
            else static if (is(typeof(b.opCmp(a))))
                return -b.opCmp(b);
            else
                return a < b ? -1 : a > b ? +1 : 0;
        }

        static if (accessibleFrom!(const typeof(this)))
        {
            override hash_t toHash() const nothrow @trusted
            {
                static if (is(typeof(&a) == ValueType*))
                    alias v = a;
                else
                    auto v = a;     // if a is (property) function
                return typeid(ValueType).getHash(cast(const void*)&v);
            }
        }
    }
    else
    {
        auto ref opEquals(this X, B)(auto ref B b)
        {
            static if (is(immutable B == immutable typeof(this)))
            {
                return a == mixin("b."~__traits(identifier, a));
            }
            else
                return a == b;
        }

        auto ref opCmp(this X, B)(auto ref B b)
          if (!is(typeof(a.opCmp(b))) || !is(typeof(b.opCmp(a))))
        {
            static if (is(typeof(a.opCmp(b))))
                return a.opCmp(b);
            else static if (is(typeof(b.opCmp(a))))
                return -b.opCmp(a);
            else static if (isFloatingPoint!ValueType || isFloatingPoint!B)
                return a < b ? -1 : a > b ? +1 : a == b ? 0 : float.nan;
            else
                return a < b ? -1 : (a > b);
        }

        static if (accessibleFrom!(const typeof(this)))
        {
            hash_t toHash() const nothrow @trusted
            {
                static if (is(typeof(&a) == ValueType*))
                    alias v = a;
                else
                    auto v = a;     // if a is (property) function
                return typeid(ValueType).getHash(cast(const void*)&v);
            }
        }
    }

    auto ref opCall(this X, Args...)(auto ref Args args) { return a(args); }

    auto ref opCast(T, this X)() { return cast(T) a; }

    auto ref opIndex(this X, D...)(auto ref D i)               { return a[i]; }
    auto ref opSlice(this X      )()                           { return a[]; }
    auto ref opSlice(this X, B, E)(auto ref B b, auto ref E e) { return a[b .. e]; }

    auto ref opUnary     (string op, this X      )()                           { return mixin(op~"a"); }
    auto ref opIndexUnary(string op, this X, D...)(auto ref D i)               { return mixin(op~"a[i]"); }
    auto ref opSliceUnary(string op, this X      )()                           { return mixin(op~"a[]"); }
    auto ref opSliceUnary(string op, this X, B, E)(auto ref B b, auto ref E e) { return mixin(op~"a[b .. e]"); }

    auto ref opBinary(string op, this X, B)(auto ref B b)
    if (op == "in" && is(typeof(a in b)) || op != "in")
    {
        return mixin("a "~op~" b");
    }
    auto ref opBinaryRight(string op, this X, B)(auto ref B b) { return mixin("b "~op~" a"); }

    static if (!is(typeof(this) == class))
    {
        import std.traits;
        static if (isAssignable!ValueType)
        {
            auto ref opAssign(this X)(auto ref typeof(this) v)
            {
                a = mixin("v."~__traits(identifier, a));
                return this;
            }
        }
        else
        {
            @disable void opAssign(this X)(auto ref typeof(this) v);
        }
    }

    auto ref opAssign     (this X, V      )(auto ref V v) if (!is(V == typeof(this))) { return a       = v; }
    auto ref opIndexAssign(this X, V, D...)(auto ref V v, auto ref D i)               { return a[i]    = v; }
    auto ref opSliceAssign(this X, V      )(auto ref V v)                             { return a[]     = v; }
    auto ref opSliceAssign(this X, V, B, E)(auto ref V v, auto ref B b, auto ref E e) { return a[b .. e] = v; }

    auto ref opOpAssign     (string op, this X, V      )(auto ref V v)
    {
        return mixin("a "      ~op~"= v");
    }
    auto ref opIndexOpAssign(string op, this X, V, D...)(auto ref V v, auto ref D i)
    {
        return mixin("a[i] "   ~op~"= v");
    }
    auto ref opSliceOpAssign(string op, this X, V      )(auto ref V v)
    {
        return mixin("a[] "    ~op~"= v");
    }
    auto ref opSliceOpAssign(string op, this X, V, B, E)(auto ref V v, auto ref B b, auto ref E e)
    {
        return mixin("a[b .. e] "~op~"= v");
    }

    template opDispatch(string name)
    {
        static if (is(typeof(__traits(getMember, a, name)) == function))
        {
            // non template function
            auto ref opDispatch(this X, Args...)(auto ref Args args) { return mixin("a."~name~"(args)"); }
        }
        else static if (is(typeof({ enum x = mixin("a."~name); })))
        {
            // built-in type field, manifest constant, and static non-mutable field
            enum opDispatch = mixin("a."~name);
        }
        else static if (is(typeof(mixin("a."~name))) || __traits(getOverloads, a, name).length != 0)
        {
            // field or property function
            @property auto ref opDispatch(this X)()                { return mixin("a."~name);        }
            @property auto ref opDispatch(this X, V)(auto ref V v) { return mixin("a."~name~" = v"); }
        }
        else
        {
            // member template
            template opDispatch(T...)
            {
                enum targs = T.length ? "!T" : "";
                auto ref opDispatch(this X, Args...)(auto ref Args args){ return mixin("a."~name~targs~"(args)"); }
            }
        }
    }

    import std.traits : isArray;

    static if (isArray!ValueType)
    {
        auto opDollar() const { return a.length; }
    }
    else static if (is(typeof(a.opDollar!0)))
    {
        auto ref opDollar(size_t pos)() { return a.opDollar!pos(); }
    }
    else static if (is(typeof(a.opDollar) == function))
    {
        auto ref opDollar() { return a.opDollar(); }
    }
    else static if (is(typeof(a.opDollar)))
    {
        alias opDollar = a.opDollar;
    }
}

///
@safe unittest
{
    struct MyInt
    {
        private int value;
        mixin Proxy!value;

        this(int n){ value = n; }
    }

    MyInt n = 10;

    // Enable operations that original type has.
    ++n;
    assert(n == 11);
    assert(n * 2 == 22);

    void func(int n) { }

    // Disable implicit conversions to original type.
    //int x = n;
    //func(n);
}

///The proxied value must be an $(B lvalue).
@safe unittest
{
    struct NewIntType
    {
        //Won't work; the literal '1'
        //is an rvalue, not an lvalue
        //mixin Proxy!1;

        //Okay, n is an lvalue
        int n;
        mixin Proxy!n;

        this(int n) { this.n = n; }
    }

    NewIntType nit = 0;
    nit++;
    assert(nit == 1);


    struct NewObjectType
    {
        Object obj;
        //Ok, obj is an lvalue
        mixin Proxy!obj;

        this (Object o) { obj = o; }
    }

    NewObjectType not = new Object();
    assert(__traits(compiles, not.toHash()));
}

/**
    There is one exception to the fact that the new type is not related to the
    old type. $(DDSUBLINK spec/function,pseudo-member, Pseudo-member)
    functions are usable with the new type; they will be forwarded on to the
    proxied value.
 */
@safe unittest
{
    import std.math;

    float f = 1.0;
    assert(!f.isInfinity);

    struct NewFloat
    {
        float _;
        mixin Proxy!_;

        this(float f) { _ = f; }
    }

    NewFloat nf = 1.0f;
    assert(!nf.isInfinity);
}

@safe unittest
{
    static struct MyInt
    {
        private int value;
        mixin Proxy!value;
        this(int n) inout { value = n; }

        enum str = "str";
        static immutable arr = [1,2,3];
    }

    foreach (T; AliasSeq!(MyInt, const MyInt, immutable MyInt))
    {
        T m = 10;
        static assert(!__traits(compiles, { int x = m; }));
        static assert(!__traits(compiles, { void func(int n){} func(m); }));
        assert(m == 10);
        assert(m != 20);
        assert(m < 20);
        assert(+m == 10);
        assert(-m == -10);
        assert(cast(double) m == 10.0);
        assert(m + 10 == 20);
        assert(m - 5 == 5);
        assert(m * 20 == 200);
        assert(m / 2 == 5);
        assert(10 + m == 20);
        assert(15 - m == 5);
        assert(20 * m == 200);
        assert(50 / m == 5);
        static if (is(T == MyInt))  // mutable
        {
            assert(++m == 11);
            assert(m++ == 11); assert(m == 12);
            assert(--m == 11);
            assert(m-- == 11); assert(m == 10);
            m = m;
            m = 20; assert(m == 20);
        }
        static assert(T.max == int.max);
        static assert(T.min == int.min);
        static assert(T.init == int.init);
        static assert(T.str == "str");
        static assert(T.arr == [1,2,3]);
    }
}
@system unittest
{
    static struct MyArray
    {
        private int[] value;
        mixin Proxy!value;
        this(int[] arr) { value = arr; }
        this(immutable int[] arr) immutable { value = arr; }
    }

    foreach (T; AliasSeq!(MyArray, const MyArray, immutable MyArray))
    {
      static if (is(T == immutable) && !is(typeof({ T a = [1,2,3,4]; })))
        T a = [1,2,3,4].idup;   // workaround until qualified ctor is properly supported
      else
        T a = [1,2,3,4];
        assert(a == [1,2,3,4]);
        assert(a != [5,6,7,8]);
        assert(+a[0]    == 1);
        version (LittleEndian)
            assert(cast(ulong[]) a == [0x0000_0002_0000_0001, 0x0000_0004_0000_0003]);
        else
            assert(cast(ulong[]) a == [0x0000_0001_0000_0002, 0x0000_0003_0000_0004]);
        assert(a ~ [10,11] == [1,2,3,4,10,11]);
        assert(a[0]    == 1);
        assert(a[]     == [1,2,3,4]);
        assert(a[2 .. 4] == [3,4]);
        static if (is(T == MyArray))    // mutable
        {
            a = a;
            a = [5,6,7,8];  assert(a == [5,6,7,8]);
            a[0]     = 0;   assert(a == [0,6,7,8]);
            a[]      = 1;   assert(a == [1,1,1,1]);
            a[0 .. 3]  = 2;   assert(a == [2,2,2,1]);
            a[0]    += 2;   assert(a == [4,2,2,1]);
            a[]     *= 2;   assert(a == [8,4,4,2]);
            a[0 .. 2] /= 2;   assert(a == [4,2,4,2]);
        }
    }
}
@system unittest
{
    class Foo
    {
        int field;

        @property int val1() const { return field; }
        @property void val1(int n) { field = n; }

        @property ref int val2() { return field; }

        int func(int x, int y) const { return x; }
        void func1(ref int a) { a = 9; }

        T ifti1(T)(T t) { return t; }
        void ifti2(Args...)(Args args) { }
        void ifti3(T, Args...)(Args args) { }

        T opCast(T)(){ return T.init; }

        T tempfunc(T)() { return T.init; }
    }
    class Hoge
    {
        Foo foo;
        mixin Proxy!foo;
        this(Foo f) { foo = f; }
    }

    auto h = new Hoge(new Foo());
    int n;

    static assert(!__traits(compiles, { Foo f = h; }));

    // field
    h.field = 1;            // lhs of assign
    n = h.field;            // rhs of assign
    assert(h.field == 1);   // lhs of BinExp
    assert(1 == h.field);   // rhs of BinExp
    assert(n == 1);

    // getter/setter property function
    h.val1 = 4;
    n = h.val1;
    assert(h.val1 == 4);
    assert(4 == h.val1);
    assert(n == 4);

    // ref getter property function
    h.val2 = 8;
    n = h.val2;
    assert(h.val2 == 8);
    assert(8 == h.val2);
    assert(n == 8);

    // member function
    assert(h.func(2,4) == 2);
    h.func1(n);
    assert(n == 9);

    // IFTI
    assert(h.ifti1(4) == 4);
    h.ifti2(4);
    h.ifti3!int(4, 3);

    // bug5896 test
    assert(h.opCast!int() == 0);
    assert(cast(int) h == 0);
    const ih = new const Hoge(new Foo());
    static assert(!__traits(compiles, ih.opCast!int()));
    static assert(!__traits(compiles, cast(int) ih));

    // template member function
    assert(h.tempfunc!int() == 0);
}

@system unittest // about Proxy inside a class
{
    class MyClass
    {
        int payload;
        mixin Proxy!payload;
        this(int i){ payload = i; }
        string opCall(string msg){ return msg; }
        int pow(int i){ return payload ^^ i; }
    }

    class MyClass2
    {
        MyClass payload;
        mixin Proxy!payload;
        this(int i){ payload = new MyClass(i); }
    }

    class MyClass3
    {
        int payload;
        mixin Proxy!payload;
        this(int i){ payload = i; }
    }

    // opEquals
    Object a = new MyClass(5);
    Object b = new MyClass(5);
    Object c = new MyClass2(5);
    Object d = new MyClass3(5);
    assert(a == b);
    assert((cast(MyClass) a) == 5);
    assert(5 == (cast(MyClass) b));
    assert(5 == cast(MyClass2) c);
    assert(a != d);

    assert(c != a);
    // oops! above line is unexpected, isn't it?
    // the reason is below.
    // MyClass2.opEquals knows MyClass but,
    // MyClass.opEquals doesn't know MyClass2.
    // so, c.opEquals(a) is true, but a.opEquals(c) is false.
    // furthermore, opEquals(T) couldn't be invoked.
    assert((cast(MyClass2) c) != (cast(MyClass) a));

    // opCmp
    Object e = new MyClass2(7);
    assert(a < cast(MyClass2) e); // OK. and
    assert(e > a); // OK, but...
    // assert(a < e); // RUNTIME ERROR!
    // assert((cast(MyClass) a) < e); // RUNTIME ERROR!
    assert(3 < cast(MyClass) a);
    assert((cast(MyClass2) e) < 11);

    // opCall
    assert((cast(MyClass2) e)("hello") == "hello");

    // opCast
    assert((cast(MyClass)(cast(MyClass2) c)) == a);
    assert((cast(int)(cast(MyClass2) c)) == 5);

    // opIndex
    class MyClass4
    {
        string payload;
        mixin Proxy!payload;
        this(string s){ payload = s; }
    }
    class MyClass5
    {
        MyClass4 payload;
        mixin Proxy!payload;
        this(string s){ payload = new MyClass4(s); }
    }
    auto f = new MyClass4("hello");
    assert(f[1] == 'e');
    auto g = new MyClass5("hello");
    assert(f[1] == 'e');

    // opSlice
    assert(f[2 .. 4] == "ll");

    // opUnary
    assert(-(cast(MyClass2) c) == -5);

    // opBinary
    assert((cast(MyClass) a) + (cast(MyClass2) c) == 10);
    assert(5 + cast(MyClass) a == 10);

    // opAssign
    (cast(MyClass2) c) = 11;
    assert((cast(MyClass2) c) == 11);
    (cast(MyClass2) c) = new MyClass(13);
    assert((cast(MyClass2) c) == 13);

    // opOpAssign
    assert((cast(MyClass2) c) += 4);
    assert((cast(MyClass2) c) == 17);

    // opDispatch
    assert((cast(MyClass2) c).pow(2) == 289);

    // opDollar
    assert(f[2..$-1] == "ll");

    // toHash
    int[Object] hash;
    hash[a] = 19;
    hash[c] = 21;
    assert(hash[b] == 19);
    assert(hash[c] == 21);
}

@safe unittest
{
    struct MyInt
    {
        int payload;

        mixin Proxy!payload;
    }

    MyInt v;
    v = v;

    struct Foo
    {
        @disable void opAssign(typeof(this));
    }
    struct MyFoo
    {
        Foo payload;

        mixin Proxy!payload;
    }
    MyFoo f;
    static assert(!__traits(compiles, f = f));

    struct MyFoo2
    {
        Foo payload;

        mixin Proxy!payload;

        // override default Proxy behavior
        void opAssign(typeof(this) rhs){}
    }
    MyFoo2 f2;
    f2 = f2;
}
@safe unittest
{
    // bug8613
    static struct Name
    {
        mixin Proxy!val;
        private string val;
        this(string s) { val = s; }
    }

    bool[Name] names;
    names[Name("a")] = true;
    bool* b = Name("a") in names;
}

@system unittest
{
    // bug14213, using function for the payload
    static struct S
    {
        int foo() { return 12; }
        mixin Proxy!foo;
    }
    static class C
    {
        int foo() { return 12; }
        mixin Proxy!foo;
    }
    S s;
    assert(s + 1 == 13);
    C c = new C();
    assert(s * 2 == 24);
}

// Check all floating point comparisons for both Proxy and Typedef,
// also against int and a Typedef!int, to be as regression-proof
// as possible. bug 15561
@safe unittest
{
    static struct MyFloatImpl
    {
        float value;
        mixin Proxy!value;
    }
    static void allFail(T0, T1)(T0 a, T1 b)
    {
        assert(!(a == b));
        assert(!(a<b));
        assert(!(a <= b));
        assert(!(a>b));
        assert(!(a >= b));
    }
    foreach (T1; AliasSeq!(MyFloatImpl, Typedef!float, Typedef!double,
        float, real, Typedef!int, int))
    {
        foreach (T2; AliasSeq!(MyFloatImpl, Typedef!float))
        {
            T1 a;
            T2 b;

            static if (isFloatingPoint!T1 || isFloatingPoint!(TypedefType!T1))
                allFail(a, b);
            a = 3;
            allFail(a, b);

            b = 4;
            assert(a != b);
            assert(a<b);
            assert(a <= b);
            assert(!(a>b));
            assert(!(a >= b));

            a = 4;
            assert(a == b);
            assert(!(a<b));
            assert(a <= b);
            assert(!(a>b));
            assert(a >= b);
        }
    }
}

/**
$(B Typedef) allows the creation of a unique type which is
based on an existing type. Unlike the $(D alias) feature,
$(B Typedef) ensures the two types are not considered as equals.

Example:
----
alias MyInt = Typedef!int;
static void takeInt(int) { }
static void takeMyInt(MyInt) { }

int i;
takeInt(i);    // ok
takeMyInt(i);  // fails

MyInt myInt;
takeInt(myInt);    // fails
takeMyInt(myInt);  // ok
----

Params:

init = Optional initial value for the new type. For example:

----
alias MyInt = Typedef!(int, 10);
MyInt myInt;
assert(myInt == 10);  // default-initialized to 10
----

cookie = Optional, used to create multiple unique types which are
based on the same origin type $(D T). For example:

----
alias TypeInt1 = Typedef!int;
alias TypeInt2 = Typedef!int;

// The two Typedefs are the same type.
static assert(is(TypeInt1 == TypeInt2));

alias MoneyEuros = Typedef!(float, float.init, "euros");
alias MoneyDollars = Typedef!(float, float.init, "dollars");

// The two Typedefs are _not_ the same type.
static assert(!is(MoneyEuros == MoneyDollars));
----

Note: If a library routine cannot handle the Typedef type,
you can use the $(D TypedefType) template to extract the
type which the Typedef wraps.
 */
struct Typedef(T, T init = T.init, string cookie=null)
{
    private T Typedef_payload = init;

    this(T init)
    {
        Typedef_payload = init;
    }

    this(Typedef tdef)
    {
        this(tdef.Typedef_payload);
    }

    // We need to add special overload for cast(Typedef!X) exp,
    // thus we can't simply inherit Proxy!Typedef_payload
    T2 opCast(T2 : Typedef!(T, Unused), this X, T, Unused...)()
    {
        return T2(cast(T) Typedef_payload);
    }

    auto ref opCast(T2, this X)()
    {
        return cast(T2) Typedef_payload;
    }

    mixin Proxy!Typedef_payload;

    pure nothrow @nogc @safe @property
    {
        alias TD = typeof(this);
        static if (isIntegral!T)
        {
            static TD min() {return TD(T.min);}
            static TD max() {return TD(T.max);}
        }
        else static if (isFloatingPoint!T)
        {
            static TD infinity() {return TD(T.infinity);}
            static TD nan() {return TD(T.nan);}
            static TD dig() {return TD(T.dig);}
            static TD epsilon() {return TD(T.epsilon);}
            static TD mant_dig() {return TD(T.mant_dig);}
            static TD max_10_exp() {return TD(T.max_10_exp);}
            static TD max_exp()  {return TD(T.max_exp);}
            static TD min_10_exp() {return TD(T.min_10_exp);}
            static TD min_exp() {return TD(T.min_exp);}
            static TD max() {return TD(T.max);}
            static TD min_normal() {return TD(T.min_normal);}
            TD re() {return TD(Typedef_payload.re);}
            TD im() {return TD(Typedef_payload.im);}
        }
    }
}

/**
Get the underlying type which a $(D Typedef) wraps.
If $(D T) is not a $(D Typedef) it will alias itself to $(D T).
*/
template TypedefType(T)
{
    static if (is(T : Typedef!Arg, Arg))
        alias TypedefType = Arg;
    else
        alias TypedefType = T;
}

///
@safe unittest
{
    import std.typecons : Typedef, TypedefType;
    import std.conv : to;

    alias MyInt = Typedef!int;
    static assert(is(TypedefType!MyInt == int));

    /// Instantiating with a non-Typedef will return that type
    static assert(is(TypedefType!int == int));

    string num = "5";

    // extract the needed type
    MyInt myInt = MyInt( num.to!(TypedefType!MyInt) );
    assert(myInt == 5);

    // cast to the underlying type to get the value that's being wrapped
    int x = cast(TypedefType!MyInt) myInt;

    alias MyIntInit = Typedef!(int, 42);
    static assert(is(TypedefType!MyIntInit == int));
    static assert(MyIntInit() == 42);
}

@safe unittest
{
    Typedef!int x = 10;
    static assert(!__traits(compiles, { int y = x; }));
    static assert(!__traits(compiles, { long z = x; }));

    Typedef!int y = 10;
    assert(x == y);

    static assert(Typedef!int.init == int.init);

    Typedef!(float, 1.0) z; // specifies the init
    assert(z == 1.0);

    static assert(typeof(z).init == 1.0);

    alias Dollar = Typedef!(int, 0, "dollar");
    alias Yen    = Typedef!(int, 0, "yen");
    static assert(!is(Dollar == Yen));

    Typedef!(int[3]) sa;
    static assert(sa.length == 3);
    static assert(typeof(sa).length == 3);

    Typedef!(int[3]) dollar1;
    assert(dollar1[0..$] is dollar1[0 .. 3]);

    Typedef!(int[]) dollar2;
    dollar2.length = 3;
    assert(dollar2[0..$] is dollar2[0 .. 3]);

    static struct Dollar1
    {
        static struct DollarToken {}
        enum opDollar = DollarToken.init;
        auto opSlice(size_t, DollarToken) { return 1; }
        auto opSlice(size_t, size_t) { return 2; }
    }

    Typedef!Dollar1 drange1;
    assert(drange1[0..$] == 1);
    assert(drange1[0 .. 1] == 2);

    static struct Dollar2
    {
        size_t opDollar(size_t pos)() { return pos == 0 ? 1 : 100; }
        size_t opIndex(size_t i, size_t j) { return i + j; }
    }

    Typedef!Dollar2 drange2;
    assert(drange2[$, $] == 101);

    static struct Dollar3
    {
        size_t opDollar() { return 123; }
        size_t opIndex(size_t i) { return i; }
    }

    Typedef!Dollar3 drange3;
    assert(drange3[$] == 123);
}

@safe @nogc pure nothrow unittest // Bugzilla 11703
{
    alias I = Typedef!int;
    static assert(is(typeof(I.min) == I));
    static assert(is(typeof(I.max) == I));

    alias F = Typedef!double;
    static assert(is(typeof(F.infinity) == F));
    static assert(is(typeof(F.epsilon) == F));

    F f;
    assert(!is(typeof(F.re).stringof == double));
    assert(!is(typeof(F.im).stringof == double));
}

@safe unittest
{
    // bug8655
    import std.typecons;
    import std.bitmanip;
    static import core.stdc.config;

    alias c_ulong = Typedef!(core.stdc.config.c_ulong);

    static struct Foo
    {
        mixin(bitfields!(
            c_ulong, "NameOffset", 31,
            c_ulong, "NameIsString", 1
        ));
    }
}

@safe unittest // Issue 12596
{
    import std.typecons;
    alias TD = Typedef!int;
    TD x = TD(1);
    TD y = TD(x);
    assert(x == y);
}

@safe unittest // about toHash
{
    import std.typecons;
    {
        alias TD = Typedef!int;
        int[TD] td;
        td[TD(1)] = 1;
        assert(td[TD(1)] == 1);
    }

    {
        alias TD = Typedef!(int[]);
        int[TD] td;
        td[TD([1,2,3,4])] = 2;
        assert(td[TD([1,2,3,4])] == 2);
    }

    {
        alias TD = Typedef!(int[][]);
        int[TD] td;
        td[TD([[1,0,0,0], [0,1,0,0], [0,0,1,0], [0,0,0,1]])] = 3;
        assert(td[TD([[1,0,0,0], [0,1,0,0], [0,0,1,0], [0,0,0,1]])] == 3);
    }

    {
        struct MyStruct{ int x; }
        alias TD = Typedef!MyStruct;
        int[TD] td;
        td[TD(MyStruct(10))] = 4;
        assert(TD(MyStruct(20)) !in td);
        assert(td[TD(MyStruct(10))] == 4);
    }

    {
        static struct MyStruct2
        {
            int x;
            size_t toHash() const nothrow @safe { return x; }
            bool opEquals(ref const MyStruct2 r) const { return r.x == x; }
        }

        alias TD = Typedef!MyStruct2;
        int[TD] td;
        td[TD(MyStruct2(50))] = 5;
        assert(td[TD(MyStruct2(50))] == 5);
    }

    {
        class MyClass{}
        alias TD = Typedef!MyClass;
        int[TD] td;
        auto c = new MyClass;
        td[TD(c)] = 6;
        assert(TD(new MyClass) !in td);
        assert(td[TD(c)] == 6);
    }
}

@system unittest
{
    alias String = Typedef!(char[]);
    alias CString = Typedef!(const(char)[]);
    CString cs = "fubar";
    String s = cast(String) cs;
    assert(cs == s);
    char[] s2 = cast(char[]) cs;
    const(char)[] cs2 = cast(const(char)[])s;
    assert(s2 == cs2);
}

/**
Allocates a $(D class) object right inside the current scope,
therefore avoiding the overhead of $(D new). This facility is unsafe;
it is the responsibility of the user to not escape a reference to the
object outside the scope.

The class destructor will be called when the result of `scoped()` is
itself destroyed.

Scoped class instances can be embedded in a parent `class` or `struct`,
just like a child struct instance. Scoped member variables must have
type `typeof(scoped!Class(args))`, and be initialized with a call to
scoped. See below for an example.

Note:
It's illegal to move a class instance even if you are sure there
are no pointers to it. As such, it is illegal to move a scoped object.
 */
template scoped(T)
    if (is(T == class))
{
    // _d_newclass now use default GC alignment (looks like (void*).sizeof * 2 for
    // small objects). We will just use the maximum of filed alignments.
    alias alignment = classInstanceAlignment!T;
    alias aligned = _alignUp!alignment;

    static struct Scoped
    {
        // Addition of `alignment` is required as `Scoped_store` can be misaligned in memory.
        private void[aligned(__traits(classInstanceSize, T) + size_t.sizeof) + alignment] Scoped_store = void;

        @property inout(T) Scoped_payload() inout
        {
            void* alignedStore = cast(void*) aligned(cast(uintptr_t) Scoped_store.ptr);
            // As `Scoped` can be unaligned moved in memory class instance should be moved accordingly.
            immutable size_t d = alignedStore - Scoped_store.ptr;
            size_t* currD = cast(size_t*) &Scoped_store[$ - size_t.sizeof];
            if (d != *currD)
            {
                import core.stdc.string : memmove;
                memmove(alignedStore, Scoped_store.ptr + *currD, __traits(classInstanceSize, T));
                *currD = d;
            }
            return cast(inout(T)) alignedStore;
        }
        alias Scoped_payload this;

        @disable this();
        @disable this(this);

        ~this()
        {
            // `destroy` will also write .init but we have no functions in druntime
            // for deterministic finalization and memory releasing for now.
            .destroy(Scoped_payload);
        }
    }

    /** Returns the _scoped object.
    Params: args = Arguments to pass to $(D T)'s constructor.
    */
    @system auto scoped(Args...)(auto ref Args args)
    {
        import std.conv : emplace;

        Scoped result = void;
        void* alignedStore = cast(void*) aligned(cast(uintptr_t) result.Scoped_store.ptr);
        immutable size_t d = alignedStore - result.Scoped_store.ptr;
        *cast(size_t*) &result.Scoped_store[$ - size_t.sizeof] = d;
        emplace!(Unqual!T)(result.Scoped_store[d .. $ - size_t.sizeof], args);
        return result;
    }
}

///
@system unittest
{
    class A
    {
        int x;
        this()     {x = 0;}
        this(int i){x = i;}
        ~this()    {}
    }

    // Standard usage, constructing A on the stack
    auto a1 = scoped!A();
    a1.x = 42;

    // Result of `scoped` call implicitly converts to a class reference
    A aRef = a1;
    assert(aRef.x == 42);

    // Scoped destruction
    {
        auto a2 = scoped!A(1);
        assert(a2.x == 1);
        aRef = a2;
        // a2 is destroyed here, calling A's destructor
    }
    // aRef is now an invalid reference

    // Here the temporary scoped A is immediately destroyed.
    // This means the reference is then invalid.
    version (Bug)
    {
        // Wrong, should use `auto`
        A invalid = scoped!A();
    }

    // Restrictions
    version (Bug)
    {
        import std.algorithm.mutation : move;
        auto invalid = a1.move; // illegal, scoped objects can't be moved
    }
    static assert(!is(typeof({
        auto e1 = a1; // illegal, scoped objects can't be copied
        assert([a1][0].x == 42); // ditto
    })));
    static assert(!is(typeof({
        alias ScopedObject = typeof(a1);
        auto e2 = ScopedObject();  // illegal, must be built via scoped!A
        auto e3 = ScopedObject(1); // ditto
    })));

    // Use with alias
    alias makeScopedA = scoped!A;
    auto a3 = makeScopedA();
    auto a4 = makeScopedA(1);

    // Use as member variable
    struct B
    {
        typeof(scoped!A()) a; // note the trailing parentheses

        this(int i)
        {
            // construct member
            a = scoped!A(i);
        }
    }

    // Stack-allocate
    auto b1 = B(5);
    aRef = b1.a;
    assert(aRef.x == 5);
    destroy(b1); // calls A's destructor for b1.a
    // aRef is now an invalid reference

    // Heap-allocate
    auto b2 = new B(6);
    assert(b2.a.x == 6);
    destroy(*b2); // calls A's destructor for b2.a
}

private uintptr_t _alignUp(uintptr_t alignment)(uintptr_t n)
    if (alignment > 0 && !((alignment - 1) & alignment))
{
    enum badEnd = alignment - 1; // 0b11, 0b111, ...
    return (n + badEnd) & ~badEnd;
}

@system unittest // Issue 6580 testcase
{
    enum alignment = (void*).alignof;

    static class C0 { }
    static class C1 { byte b; }
    static class C2 { byte[2] b; }
    static class C3 { byte[3] b; }
    static class C7 { byte[7] b; }
    static assert(scoped!C0().sizeof % alignment == 0);
    static assert(scoped!C1().sizeof % alignment == 0);
    static assert(scoped!C2().sizeof % alignment == 0);
    static assert(scoped!C3().sizeof % alignment == 0);
    static assert(scoped!C7().sizeof % alignment == 0);

    enum longAlignment = long.alignof;
    static class C1long
    {
        long long_; byte byte_ = 4;
        this() { }
        this(long _long, ref int i) { long_ = _long; ++i; }
    }
    static class C2long { byte[2] byte_ = [5, 6]; long long_ = 7; }
    static assert(scoped!C1long().sizeof % longAlignment == 0);
    static assert(scoped!C2long().sizeof % longAlignment == 0);

    void alignmentTest()
    {
        int var = 5;
        auto c1long = scoped!C1long(3, var);
        assert(var == 6);
        auto c2long = scoped!C2long();
        assert(cast(uint)&c1long.long_ % longAlignment == 0);
        assert(cast(uint)&c2long.long_ % longAlignment == 0);
        assert(c1long.long_ == 3 && c1long.byte_ == 4);
        assert(c2long.byte_ == [5, 6] && c2long.long_ == 7);
    }

    alignmentTest();

    version (DigitalMars)
    {
        void test(size_t size)
        {
            import core.stdc.stdlib;
            alloca(size);
            alignmentTest();
        }
        foreach (i; 0 .. 10)
            test(i);
    }
    else
    {
        void test(size_t size)()
        {
            byte[size] arr;
            alignmentTest();
        }
        foreach (i; AliasSeq!(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10))
            test!i();
    }
}

@system unittest // Original Issue 6580 testcase
{
    class C { int i; byte b; }

    auto sa = [scoped!C(), scoped!C()];
    assert(cast(uint)&sa[0].i % int.alignof == 0);
    assert(cast(uint)&sa[1].i % int.alignof == 0); // fails
}

@system unittest
{
    class A { int x = 1; }
    auto a1 = scoped!A();
    assert(a1.x == 1);
    auto a2 = scoped!A();
    a1.x = 42;
    a2.x = 53;
    assert(a1.x == 42);
}

@system unittest
{
    class A { int x = 1; this() { x = 2; } }
    auto a1 = scoped!A();
    assert(a1.x == 2);
    auto a2 = scoped!A();
    a1.x = 42;
    a2.x = 53;
    assert(a1.x == 42);
}

@system unittest
{
    class A { int x = 1; this(int y) { x = y; } ~this() {} }
    auto a1 = scoped!A(5);
    assert(a1.x == 5);
    auto a2 = scoped!A(42);
    a1.x = 42;
    a2.x = 53;
    assert(a1.x == 42);
}

@system unittest
{
    class A { static bool dead; ~this() { dead = true; } }
    class B : A { static bool dead; ~this() { dead = true; } }
    {
        auto b = scoped!B();
    }
    assert(B.dead, "asdasd");
    assert(A.dead, "asdasd");
}

@system unittest // Issue 8039 testcase
{
    static int dels;
    static struct S { ~this(){ ++dels; } }

    static class A { S s; }
    dels = 0; { scoped!A(); }
    assert(dels == 1);

    static class B { S[2] s; }
    dels = 0; { scoped!B(); }
    assert(dels == 2);

    static struct S2 { S[3] s; }
    static class C { S2[2] s; }
    dels = 0; { scoped!C(); }
    assert(dels == 6);

    static class D: A { S2[2] s; }
    dels = 0; { scoped!D(); }
    assert(dels == 1+6);
}

@system unittest
{
    // bug4500
    class A
    {
        this() { a = this; }
        this(int i) { a = this; }
        A a;
        bool check() { return this is a; }
    }

    auto a1 = scoped!A();
    assert(a1.check());

    auto a2 = scoped!A(1);
    assert(a2.check());

    a1.a = a1;
    assert(a1.check());
}

@system unittest
{
    static class A
    {
        static int sdtor;

        this() { ++sdtor; assert(sdtor == 1); }
        ~this() { assert(sdtor == 1); --sdtor; }
    }

    interface Bob {}

    static class ABob : A, Bob
    {
        this() { ++sdtor; assert(sdtor == 2); }
        ~this() { assert(sdtor == 2); --sdtor; }
    }

    A.sdtor = 0;
    scope(exit) assert(A.sdtor == 0);
    auto abob = scoped!ABob();
}

@safe unittest
{
    static class A { this(int) {} }
    static assert(!__traits(compiles, scoped!A()));
}

@system unittest
{
    static class A { @property inout(int) foo() inout { return 1; } }

    auto a1 = scoped!A();
    assert(a1.foo == 1);
    static assert(is(typeof(a1.foo) == int));

    auto a2 = scoped!(const(A))();
    assert(a2.foo == 1);
    static assert(is(typeof(a2.foo) == const(int)));

    auto a3 = scoped!(immutable(A))();
    assert(a3.foo == 1);
    static assert(is(typeof(a3.foo) == immutable(int)));

    const c1 = scoped!A();
    assert(c1.foo == 1);
    static assert(is(typeof(c1.foo) == const(int)));

    const c2 = scoped!(const(A))();
    assert(c2.foo == 1);
    static assert(is(typeof(c2.foo) == const(int)));

    const c3 = scoped!(immutable(A))();
    assert(c3.foo == 1);
    static assert(is(typeof(c3.foo) == immutable(int)));
}

@system unittest
{
    class C { this(ref int val) { assert(val == 3); ++val; } }

    int val = 3;
    auto s = scoped!C(val);
    assert(val == 4);
}

@system unittest
{
    class C
    {
        this(){}
        this(int){}
        this(int, int){}
    }
    alias makeScopedC = scoped!C;

    auto a = makeScopedC();
    auto b = makeScopedC(1);
    auto c = makeScopedC(1, 1);

    static assert(is(typeof(a) == typeof(b)));
    static assert(is(typeof(b) == typeof(c)));
}

/**
Defines a simple, self-documenting yes/no flag. This makes it easy for
APIs to define functions accepting flags without resorting to $(D
bool), which is opaque in calls, and without needing to define an
enumerated type separately. Using $(D Flag!"Name") instead of $(D
bool) makes the flag's meaning visible in calls. Each yes/no flag has
its own type, which makes confusions and mix-ups impossible.

Example:

Code calling $(D getLine) (usually far away from its definition) can't be
understood without looking at the documentation, even by users familiar with
the API:
----
string getLine(bool keepTerminator)
{
    ...
    if (keepTerminator) ...
    ...
}
...
auto line = getLine(false);
----

Assuming the reverse meaning (i.e. "ignoreTerminator") and inserting the wrong
code compiles and runs with erroneous results.

After replacing the boolean parameter with an instantiation of $(D Flag), code
calling $(D getLine) can be easily read and understood even by people not
fluent with the API:

----
string getLine(Flag!"keepTerminator" keepTerminator)
{
    ...
    if (keepTerminator) ...
    ...
}
...
auto line = getLine(Yes.keepTerminator);
----

The structs $(D Yes) and $(D No) are provided as shorthand for
$(D Flag!"Name".yes) and $(D Flag!"Name".no) and are preferred for brevity and
readability. These convenience structs mean it is usually unnecessary and
counterproductive to create an alias of a $(D Flag) as a way of avoiding typing
out the full type while specifying the affirmative or negative options.

Passing categorical data by means of unstructured $(D bool)
parameters is classified under "simple-data coupling" by Steve
McConnell in the $(LUCKY Code Complete) book, along with three other
kinds of coupling. The author argues citing several studies that
coupling has a negative effect on code quality. $(D Flag) offers a
simple structuring method for passing yes/no flags to APIs.
 */
template Flag(string name) {
    ///
    enum Flag : bool
    {
        /**
         When creating a value of type $(D Flag!"Name"), use $(D
         Flag!"Name".no) for the negative option. When using a value
         of type $(D Flag!"Name"), compare it against $(D
         Flag!"Name".no) or just $(D false) or $(D 0).  */
        no = false,

        /** When creating a value of type $(D Flag!"Name"), use $(D
         Flag!"Name".yes) for the affirmative option. When using a
         value of type $(D Flag!"Name"), compare it against $(D
         Flag!"Name".yes).
        */
        yes = true
    }
}

/**
Convenience names that allow using e.g. $(D Yes.encryption) instead of
$(D Flag!"encryption".yes) and $(D No.encryption) instead of $(D
Flag!"encryption".no).
*/
struct Yes
{
    template opDispatch(string name)
    {
        enum opDispatch = Flag!name.yes;
    }
}
//template yes(string name) { enum Flag!name yes = Flag!name.yes; }

/// Ditto
struct No
{
    template opDispatch(string name)
    {
        enum opDispatch = Flag!name.no;
    }
}

///
@safe unittest
{
    Flag!"abc" flag1;
    assert(flag1 == Flag!"abc".no);
    assert(flag1 == No.abc);
    assert(!flag1);
    if (flag1) assert(false);
    flag1 = Yes.abc;
    assert(flag1);
    if (!flag1) assert(false);
    if (flag1) {} else assert(false);
    assert(flag1 == Yes.abc);
}

/**
Detect whether an enum is of integral type and has only "flag" values
(i.e. values with a bit count of exactly 1).
Additionally, a zero value is allowed for compatibility with enums including
a "None" value.
*/
template isBitFlagEnum(E)
{
    static if (is(E Base == enum) && isIntegral!Base)
    {
        enum isBitFlagEnum = (E.min >= 0) &&
        {
            foreach (immutable flag; EnumMembers!E)
            {
                Base value = flag;
                value &= value - 1;
                if (value != 0) return false;
            }
            return true;
        }();
    }
    else
    {
        enum isBitFlagEnum = false;
    }
}

///
@safe pure nothrow unittest
{
    enum A
    {
        None,
        A = 1 << 0,
        B = 1 << 1,
        C = 1 << 2,
        D = 1 << 3,
    }

    static assert(isBitFlagEnum!A);

    enum B
    {
        A,
        B,
        C,
        D // D == 3
    }

    static assert(!isBitFlagEnum!B);

    enum C: double
    {
        A = 1 << 0,
        B = 1 << 1
    }

    static assert(!isBitFlagEnum!C);
}

/**
A typesafe structure for storing combinations of enum values.

This template defines a simple struct to represent bitwise OR combinations of
enum values. It can be used if all the enum values are integral constants with
a bit count of at most 1, or if the $(D unsafe) parameter is explicitly set to
Yes.
This is much safer than using the enum itself to store
the OR combination, which can produce surprising effects like this:
----
enum E
{
    A = 1 << 0,
    B = 1 << 1
}
E e = E.A | E.B;
// will throw SwitchError
final switch (e)
{
    case E.A:
        return;
    case E.B:
        return;
}
----
*/
struct BitFlags(E, Flag!"unsafe" unsafe = No.unsafe) if (unsafe || isBitFlagEnum!(E))
{
@safe @nogc pure nothrow:
private:
    enum isBaseEnumType(T) = is(E == T);
    alias Base = OriginalType!E;
    Base mValue;
    static struct Negation
    {
    @safe @nogc pure nothrow:
    private:
        Base mValue;

        // Prevent non-copy construction outside the module.
        @disable this();
        this(Base value)
        {
            mValue = value;
        }
    }

public:
    this(E flag)
    {
        this = flag;
    }

    this(T...)(T flags)
        if (allSatisfy!(isBaseEnumType, T))
    {
        this = flags;
    }

    bool opCast(B: bool)() const
    {
        return mValue != 0;
    }

    Base opCast(B)() const
        if (isImplicitlyConvertible!(Base, B))
    {
        return mValue;
    }

    Negation opUnary(string op)() const
        if (op == "~")
    {
        return Negation(~mValue);
    }

    auto ref opAssign(T...)(T flags)
        if (allSatisfy!(isBaseEnumType, T))
    {
        mValue = 0;
        foreach (E flag; flags)
        {
            mValue |= flag;
        }
        return this;
    }

    auto ref opAssign(E flag)
    {
        mValue = flag;
        return this;
    }

    auto ref opOpAssign(string op: "|")(BitFlags flags)
    {
        mValue |= flags.mValue;
        return this;
    }

    auto ref opOpAssign(string op: "&")(BitFlags  flags)
    {
        mValue &= flags.mValue;
        return this;
    }

    auto ref opOpAssign(string op: "|")(E flag)
    {
        mValue |= flag;
        return this;
    }

    auto ref opOpAssign(string op: "&")(E flag)
    {
        mValue &= flag;
        return this;
    }

    auto ref opOpAssign(string op: "&")(Negation negatedFlags)
    {
        mValue &= negatedFlags.mValue;
        return this;
    }

    auto opBinary(string op)(BitFlags flags) const
        if (op == "|" || op == "&")
    {
        BitFlags result = this;
        result.opOpAssign!op(flags);
        return result;
    }

    auto opBinary(string op)(E flag) const
        if (op == "|" || op == "&")
    {
        BitFlags result = this;
        result.opOpAssign!op(flag);
        return result;
    }

    auto opBinary(string op: "&")(Negation negatedFlags) const
    {
        BitFlags result = this;
        result.opOpAssign!op(negatedFlags);
        return result;
    }

    auto opBinaryRight(string op)(E flag) const
        if (op == "|" || op == "&")
    {
        return opBinary!op(flag);
    }
}

/// BitFlags can be manipulated with the usual operators
@safe @nogc pure nothrow unittest
{
    import std.traits : EnumMembers;

    // You can use such an enum with BitFlags straight away
    enum Enum
    {
        None,
        A = 1 << 0,
        B = 1 << 1,
        C = 1 << 2
    }
    BitFlags!Enum flags1;
    assert(!(flags1 & (Enum.A | Enum.B | Enum.C)));

    // You need to specify the `unsafe` parameter for enum with custom values
    enum UnsafeEnum
    {
        A,
        B,
        C,
        D = B|C
    }
    static assert(!__traits(compiles, { BitFlags!UnsafeEnum flags2; }));
    BitFlags!(UnsafeEnum, Yes.unsafe) flags3;

    immutable BitFlags!Enum flags_empty;
    // A default constructed BitFlags has no value set
    assert(!(flags_empty & Enum.A) && !(flags_empty & Enum.B) && !(flags_empty & Enum.C));

    // Value can be set with the | operator
    immutable BitFlags!Enum flags_A = flags_empty | Enum.A;

    // And tested with the & operator
    assert(flags_A & Enum.A);

    // Which commutes
    assert(Enum.A & flags_A);

    // BitFlags can be variadically initialized
    immutable BitFlags!Enum flags_AB = BitFlags!Enum(Enum.A, Enum.B);
    assert((flags_AB & Enum.A) && (flags_AB & Enum.B) && !(flags_AB & Enum.C));

    // Use the ~ operator for subtracting flags
    immutable BitFlags!Enum flags_B = flags_AB & ~BitFlags!Enum(Enum.A);
    assert(!(flags_B & Enum.A) && (flags_B & Enum.B) && !(flags_B & Enum.C));

    // You can use the EnumMembers template to set all flags
    immutable BitFlags!Enum flags_all = EnumMembers!Enum;

    // use & between BitFlags for intersection
    immutable BitFlags!Enum flags_BC = BitFlags!Enum(Enum.B, Enum.C);
    assert(flags_B == (flags_BC & flags_AB));

    // All the binary operators work in their assignment version
    BitFlags!Enum temp = flags_empty;
    temp |= flags_AB;
    assert(temp == (flags_empty | flags_AB));
    temp = flags_empty;
    temp |= Enum.B;
    assert(temp == (flags_empty | Enum.B));
    temp = flags_empty;
    temp &= flags_AB;
    assert(temp == (flags_empty & flags_AB));
    temp = flags_empty;
    temp &= Enum.A;
    assert(temp == (flags_empty & Enum.A));

    // BitFlags with no value set evaluate to false
    assert(!flags_empty);

    // BitFlags with at least one value set evaluate to true
    assert(flags_A);

    // This can be useful to check intersection between BitFlags
    assert(flags_A & flags_AB);
    assert(flags_AB & Enum.A);

    // Finally, you can of course get you raw value out of flags
    auto value = cast(int) flags_A;
    assert(value == Enum.A);
}

// ReplaceType
/**
Replaces all occurrences of `From` into `To`, in one or more types `T`. For
example, $(D ReplaceType!(int, uint, Tuple!(int, float)[string])) yields
$(D Tuple!(uint, float)[string]). The types in which replacement is performed
may be arbitrarily complex, including qualifiers, built-in type constructors
(pointers, arrays, associative arrays, functions, and delegates), and template
instantiations; replacement proceeds transitively through the type definition.
However, member types in `struct`s or `class`es are not replaced because there
are no ways to express the types resulting after replacement.

This is an advanced type manipulation necessary e.g. for replacing the
placeholder type `This` in $(REF Algebraic, std,variant).

Returns: `ReplaceType` aliases itself to the type(s) that result after
replacement.
*/
template ReplaceType(From, To, T...)
{
    static if (T.length == 1)
    {
        static if (is(T[0] == From))
            alias ReplaceType = To;
        else static if (is(T[0] == const(U), U))
            alias ReplaceType = const(ReplaceType!(From, To, U));
        else static if (is(T[0] == immutable(U), U))
            alias ReplaceType = immutable(ReplaceType!(From, To, U));
        else static if (is(T[0] == shared(U), U))
            alias ReplaceType = shared(ReplaceType!(From, To, U));
        else static if (is(T[0] == U*, U))
        {
            static if (is(U == function))
                alias ReplaceType = replaceTypeInFunctionType!(From, To, T[0]);
            else
                alias ReplaceType = ReplaceType!(From, To, U)*;
        }
        else static if (is(T[0] == delegate))
        {
            alias ReplaceType = replaceTypeInFunctionType!(From, To, T[0]);
        }
        else static if (is(T[0] == function))
        {
            static assert(0, "Function types not supported," ~
                " use a function pointer type instead of " ~ T[0].stringof);
        }
        else static if (is(T[0] : U!V, alias U, V...))
        {
            template replaceTemplateArgs(T...)
            {
                static if (is(typeof(T[0])))    // template argument is value or symbol
                    enum replaceTemplateArgs = T[0];
                else
                    alias replaceTemplateArgs = ReplaceType!(From, To, T[0]);
            }
            alias ReplaceType = U!(staticMap!(replaceTemplateArgs, V));
        }
        else static if (is(T[0] == struct))
            // don't match with alias this struct below (Issue 15168)
            alias ReplaceType = T[0];
        else static if (is(T[0] == U[], U))
            alias ReplaceType = ReplaceType!(From, To, U)[];
        else static if (is(T[0] == U[n], U, size_t n))
            alias ReplaceType = ReplaceType!(From, To, U)[n];
        else static if (is(T[0] == U[V], U, V))
            alias ReplaceType =
                ReplaceType!(From, To, U)[ReplaceType!(From, To, V)];
        else
            alias ReplaceType = T[0];
    }
    else static if (T.length > 1)
    {
        alias ReplaceType = AliasSeq!(ReplaceType!(From, To, T[0]),
            ReplaceType!(From, To, T[1 .. $]));
    }
    else
    {
        alias ReplaceType = AliasSeq!();
    }
}

///
@safe unittest
{
    static assert(
        is(ReplaceType!(int, string, int[]) == string[]) &&
        is(ReplaceType!(int, string, int[int]) == string[string]) &&
        is(ReplaceType!(int, string, const(int)[]) == const(string)[]) &&
        is(ReplaceType!(int, string, Tuple!(int[], float))
            == Tuple!(string[], float))
    );
}

private template replaceTypeInFunctionType(From, To, fun)
{
    alias RX = ReplaceType!(From, To, ReturnType!fun);
    alias PX = AliasSeq!(ReplaceType!(From, To, Parameters!fun));
    // Wrapping with AliasSeq is neccesary because ReplaceType doesn't return
    // tuple if Parameters!fun.length == 1

    string gen()
    {
        enum  linkage = functionLinkage!fun;
        alias attributes = functionAttributes!fun;
        enum  variadicStyle = variadicFunctionStyle!fun;
        alias storageClasses = ParameterStorageClassTuple!fun;

        string result;

        result ~= "extern(" ~ linkage ~ ") ";
        static if (attributes & FunctionAttribute.ref_)
        {
            result ~= "ref ";
        }

        result ~= "RX";
        static if (is(fun == delegate))
            result ~= " delegate";
        else
            result ~= " function";

        result ~= "(";
        foreach (i, _; PX)
        {
            if (i)
                result ~= ", ";
            if (storageClasses[i] & ParameterStorageClass.scope_)
                result ~= "scope ";
            if (storageClasses[i] & ParameterStorageClass.out_)
                result ~= "out ";
            if (storageClasses[i] & ParameterStorageClass.ref_)
                result ~= "ref ";
            if (storageClasses[i] & ParameterStorageClass.lazy_)
                result ~= "lazy ";
            if (storageClasses[i] & ParameterStorageClass.return_)
                result ~= "return ";

            result ~= "PX[" ~ i.stringof ~ "]";
        }
        static if (variadicStyle == Variadic.typesafe)
            result ~= " ...";
        else static if (variadicStyle != Variadic.no)
            result ~= ", ...";
        result ~= ")";

        static if (attributes & FunctionAttribute.pure_)
            result ~= " pure";
        static if (attributes & FunctionAttribute.nothrow_)
            result ~= " nothrow";
        static if (attributes & FunctionAttribute.property)
            result ~= " @property";
        static if (attributes & FunctionAttribute.trusted)
            result ~= " @trusted";
        static if (attributes & FunctionAttribute.safe)
            result ~= " @safe";
        static if (attributes & FunctionAttribute.nogc)
            result ~= " @nogc";
        static if (attributes & FunctionAttribute.system)
            result ~= " @system";
        static if (attributes & FunctionAttribute.const_)
            result ~= " const";
        static if (attributes & FunctionAttribute.immutable_)
            result ~= " immutable";
        static if (attributes & FunctionAttribute.inout_)
            result ~= " inout";
        static if (attributes & FunctionAttribute.shared_)
            result ~= " shared";
        static if (attributes & FunctionAttribute.return_)
            result ~= " return";

        return result;
    }
    //pragma(msg, "gen ==> ", gen());

    mixin("alias replaceTypeInFunctionType = " ~ gen() ~ ";");
}

@safe unittest
{
    template Test(Ts...)
    {
        static if (Ts.length)
        {
            //pragma(msg, "Testing: ReplaceType!("~Ts[0].stringof~", "
            //    ~Ts[1].stringof~", "~Ts[2].stringof~")");
            static assert(is(ReplaceType!(Ts[0], Ts[1], Ts[2]) == Ts[3]),
                "ReplaceType!("~Ts[0].stringof~", "~Ts[1].stringof~", "
                    ~Ts[2].stringof~") == "
                    ~ReplaceType!(Ts[0], Ts[1], Ts[2]).stringof);
            alias Test = Test!(Ts[4 .. $]);
        }
        else alias Test = void;
    }

    //import core.stdc.stdio;
    alias RefFun1 = ref int function(float, long);
    alias RefFun2 = ref float function(float, long);
    extern(C) int printf(const char*, ...) nothrow @nogc @system;
    extern(C) float floatPrintf(const char*, ...) nothrow @nogc @system;
    int func(float);

    int x;
    struct S1 { void foo() { x = 1; } }
    struct S2 { void bar() { x = 2; } }

    alias Pass = Test!(
        int, float, typeof(&func), float delegate(float),
        int, float, typeof(&printf), typeof(&floatPrintf),
        int, float, int function(out long, ...),
            float function(out long, ...),
        int, float, int function(ref float, long),
            float function(ref float, long),
        int, float, int function(ref int, long),
            float function(ref float, long),
        int, float, int function(out int, long),
            float function(out float, long),
        int, float, int function(lazy int, long),
            float function(lazy float, long),
        int, float, int function(out long, ref const int),
            float function(out long, ref const float),
        int, int, int, int,
        int, float, int, float,
        int, float, const int, const float,
        int, float, immutable int, immutable float,
        int, float, shared int, shared float,
        int, float, int*, float*,
        int, float, const(int)*, const(float)*,
        int, float, const(int*), const(float*),
        const(int)*, float, const(int*), const(float),
        int*, float, const(int)*, const(int)*,
        int, float, int[], float[],
        int, float, int[42], float[42],
        int, float, const(int)[42], const(float)[42],
        int, float, const(int[42]), const(float[42]),
        int, float, int[int], float[float],
        int, float, int[double], float[double],
        int, float, double[int], double[float],
        int, float, int function(float, long), float function(float, long),
        int, float, int function(float), float function(float),
        int, float, int function(float, int), float function(float, float),
        int, float, int delegate(float, long), float delegate(float, long),
        int, float, int delegate(float), float delegate(float),
        int, float, int delegate(float, int), float delegate(float, float),
        int, float, Unique!int, Unique!float,
        int, float, Tuple!(float, int), Tuple!(float, float),
        int, float, RefFun1, RefFun2,
        S1, S2,
            S1[1][][S1]* function(),
            S2[1][][S2]* function(),
        int, string,
               int[3] function(   int[] arr,    int[2] ...) pure @trusted,
            string[3] function(string[] arr, string[2] ...) pure @trusted,
    );

    // Bugzilla 15168
    static struct T1 { string s; alias s this; }
    static struct T2 { char[10] s; alias s this; }
    static struct T3 { string[string] s; alias s this; }
    alias Pass2 = Test!(
        ubyte, ubyte, T1, T1,
        ubyte, ubyte, T2, T2,
        ubyte, ubyte, T3, T3,
    );
}

@safe unittest // Bugzilla 17116
{
    alias ConstDg = void delegate(float) const;
    alias B = void delegate(int) const;
    alias A = ReplaceType!(float, int, ConstDg);
    static assert(is(B == A));
}

/**
Ternary type with three truth values:

$(UL
    $(LI `Ternary.yes` for `true`)
    $(LI `Ternary.no` for `false`)
    $(LI `Ternary.unknown` as an unknown state)
)

Also known as trinary, trivalent, or trilean.

See_Also:
    $(HTTP en.wikipedia.org/wiki/Three-valued_logic,
        Three Valued Logic on Wikipedia)
*/
struct Ternary
{
    @safe @nogc nothrow pure:

    private ubyte value = 6;
    private static Ternary make(ubyte b)
    {
        Ternary r = void;
        r.value = b;
        return r;
    }

    /**
        The possible states of the `Ternary`
    */
    enum no = make(0);
    /// ditto
    enum yes = make(2);
    /// ditto
    enum unknown = make(6);

    /**
     Construct and assign from a `bool`, receiving `no` for `false` and `yes`
     for `true`.
    */
    this(bool b) { value = b << 1; }

    /// ditto
    void opAssign(bool b) { value = b << 1; }

    /**
    Construct a ternary value from another ternary value
    */
    this(const Ternary b) { value = b.value; }

    /**
    $(TABLE Truth table for logical operations,
      $(TR $(TH `a`) $(TH `b`) $(TH `$(TILDE)a`) $(TH `a | b`) $(TH `a & b`) $(TH `a ^ b`))
      $(TR $(TD `no`) $(TD `no`) $(TD `yes`) $(TD `no`) $(TD `no`) $(TD `no`))
      $(TR $(TD `no`) $(TD `yes`) $(TD) $(TD `yes`) $(TD `no`) $(TD `yes`))
      $(TR $(TD `no`) $(TD `unknown`) $(TD) $(TD `unknown`) $(TD `no`) $(TD `unknown`))
      $(TR $(TD `yes`) $(TD `no`) $(TD `no`) $(TD `yes`) $(TD `no`) $(TD `yes`))
      $(TR $(TD `yes`) $(TD `yes`) $(TD) $(TD `yes`) $(TD `yes`) $(TD `no`))
      $(TR $(TD `yes`) $(TD `unknown`) $(TD) $(TD `yes`) $(TD `unknown`) $(TD `unknown`))
      $(TR $(TD `unknown`) $(TD `no`) $(TD `unknown`) $(TD `unknown`) $(TD `no`) $(TD `unknown`))
      $(TR $(TD `unknown`) $(TD `yes`) $(TD) $(TD `yes`) $(TD `unknown`) $(TD `unknown`))
      $(TR $(TD `unknown`) $(TD `unknown`) $(TD) $(TD `unknown`) $(TD `unknown`) $(TD `unknown`))
    )
    */
    Ternary opUnary(string s)() if (s == "~")
    {
        return make((386 >> value) & 6);
    }

    /// ditto
    Ternary opBinary(string s)(Ternary rhs) if (s == "|")
    {
        return make((25_512 >> (value + rhs.value)) & 6);
    }

    /// ditto
    Ternary opBinary(string s)(Ternary rhs) if (s == "&")
    {
        return make((26_144 >> (value + rhs.value)) & 6);
    }

    /// ditto
    Ternary opBinary(string s)(Ternary rhs) if (s == "^")
    {
        return make((26_504 >> (value + rhs.value)) & 6);
    }
}

///
@safe @nogc nothrow pure
unittest
{
    Ternary a;
    assert(a == Ternary.unknown);

    assert(~Ternary.yes == Ternary.no);
    assert(~Ternary.no == Ternary.yes);
    assert(~Ternary.unknown == Ternary.unknown);
}

@safe @nogc nothrow pure
unittest
{
    alias f = Ternary.no, t = Ternary.yes, u = Ternary.unknown;
    Ternary[27] truthTableAnd =
    [
        t, t, t,
        t, u, u,
        t, f, f,
        u, t, u,
        u, u, u,
        u, f, f,
        f, t, f,
        f, u, f,
        f, f, f,
    ];

    Ternary[27] truthTableOr =
    [
        t, t, t,
        t, u, t,
        t, f, t,
        u, t, t,
        u, u, u,
        u, f, u,
        f, t, t,
        f, u, u,
        f, f, f,
    ];

    Ternary[27] truthTableXor =
    [
        t, t, f,
        t, u, u,
        t, f, t,
        u, t, u,
        u, u, u,
        u, f, u,
        f, t, t,
        f, u, u,
        f, f, f,
    ];

    for (auto i = 0; i != truthTableAnd.length; i += 3)
    {
        assert((truthTableAnd[i] & truthTableAnd[i + 1])
            == truthTableAnd[i + 2]);
        assert((truthTableOr[i] | truthTableOr[i + 1])
            == truthTableOr[i + 2]);
        assert((truthTableXor[i] ^ truthTableXor[i + 1])
            == truthTableXor[i + 2]);
    }

    Ternary a;
    assert(a == Ternary.unknown);
    static assert(!is(typeof({ if (a) {} })));
    assert(!is(typeof({ auto b = Ternary(3); })));
    a = true;
    assert(a == Ternary.yes);
    a = false;
    assert(a == Ternary.no);
    a = Ternary.unknown;
    assert(a == Ternary.unknown);
    Ternary b;
    b = a;
    assert(b == a);
    assert(~Ternary.yes == Ternary.no);
    assert(~Ternary.no == Ternary.yes);
    assert(~Ternary.unknown == Ternary.unknown);
}
