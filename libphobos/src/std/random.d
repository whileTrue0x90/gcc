// Written in the D programming language.

/**
Facilities for random number generation.

$(RED Disclaimer:) The _random number generators and API provided in this
module are not designed to be cryptographically secure, and are therefore
unsuitable for cryptographic or security-related purposes such as generating
authentication tokens or network sequence numbers. For such needs, please use a
reputable cryptographic library instead.

The new-style generator objects hold their own state so they are
immune of threading issues. The generators feature a number of
well-known and well-documented methods of generating random
numbers. An overall fast and reliable means to generate random numbers
is the $(D_PARAM Mt19937) generator, which derives its name from
"$(LINK2 https://en.wikipedia.org/wiki/Mersenne_Twister, Mersenne Twister)
with a period of 2 to the power of
19937". In memory-constrained situations,
$(LINK2 https://en.wikipedia.org/wiki/Linear_congruential_generator,
linear congruential generators) such as $(D MinstdRand0) and $(D MinstdRand) might be
useful. The standard library provides an alias $(D_PARAM Random) for
whichever generator it considers the most fit for the target
environment.

In addition to random number generators, this module features
distributions, which skew a generator's output statistical
distribution in various ways. So far the uniform distribution for
integers and real numbers have been implemented.

Source:    $(PHOBOSSRC std/_random.d)

Macros:

Copyright: Copyright Andrei Alexandrescu 2008 - 2009, Joseph Rushton Wakeling 2012.
License:   $(HTTP www.boost.org/LICENSE_1_0.txt, Boost License 1.0).
Authors:   $(HTTP erdani.org, Andrei Alexandrescu)
           Masahiro Nakagawa (Xorshift random generator)
           $(HTTP braingam.es, Joseph Rushton Wakeling) (Algorithm D for random sampling)
           Ilya Yaroshenko (Mersenne Twister implementation, adapted from $(HTTPS github.com/libmir/mir-_random, mir-_random))
Credits:   The entire random number library architecture is derived from the
           excellent $(HTTP open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2461.pdf, C++0X)
           random number facility proposed by Jens Maurer and contributed to by
           researchers at the Fermi laboratory (excluding Xorshift).
*/
/*
         Copyright Andrei Alexandrescu 2008 - 2009.
Distributed under the Boost Software License, Version 1.0.
   (See accompanying file LICENSE_1_0.txt or copy at
         http://www.boost.org/LICENSE_1_0.txt)
*/
module std.random;


import std.range.primitives;
import std.traits;

///
@safe unittest
{
    // seed a random generator with a constant
    auto rnd = Random(42);

    // Generate a uniformly-distributed integer in the range [0, 14]
    // If no random generator is passed, the global `rndGen` would be used
    auto i = uniform(0, 15, rnd);
    assert(i == 12);

    // Generate a uniformly-distributed real in the range [0, 100)
    auto r = uniform(0.0L, 100.0L, rnd);
    assert(r == 79.65429843861011285);

    // Generate a 32-bit random number
    auto u = uniform!uint(rnd);
    assert(u == 4083286876);
}

version (unittest)
{
    static import std.meta;
    package alias PseudoRngTypes = std.meta.AliasSeq!(MinstdRand0, MinstdRand, Mt19937, Xorshift32, Xorshift64,
                                                      Xorshift96, Xorshift128, Xorshift160, Xorshift192);
}

// Segments of the code in this file Copyright (c) 1997 by Rick Booth
// From "Inner Loops" by Rick Booth, Addison-Wesley

// Work derived from:

/*
   A C-program for MT19937, with initialization improved 2002/1/26.
   Coded by Takuji Nishimura and Makoto Matsumoto.

   Before using, initialize the state by using init_genrand(seed)
   or init_by_array(init_key, key_length).

   Copyright (C) 1997 - 2002, Makoto Matsumoto and Takuji Nishimura,
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

     1. Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.

     2. Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

     3. The names of its contributors may not be used to endorse or promote
        products derived from this software without specific prior written
        permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


   Any feedback is very welcome.
   http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html
   email: m-mat @ math.sci.hiroshima-u.ac.jp (remove space)
*/

/**
 * Test if Rng is a random-number generator. The overload
 * taking a ElementType also makes sure that the Rng generates
 * values of that type.
 *
 * A random-number generator has at least the following features:
 * $(UL
 *   $(LI it's an InputRange)
 *   $(LI it has a 'bool isUniformRandom' field readable in CTFE)
 * )
 */
template isUniformRNG(Rng, ElementType)
{
    enum bool isUniformRNG = isInputRange!Rng &&
        is(typeof(Rng.front) == ElementType) &&
        is(typeof(
        {
            static assert(Rng.isUniformRandom); //tag
        }));
}

/**
 * ditto
 */
template isUniformRNG(Rng)
{
    enum bool isUniformRNG = isInputRange!Rng &&
        is(typeof(
        {
            static assert(Rng.isUniformRandom); //tag
        }));
}

/**
 * Test if Rng is seedable. The overload
 * taking a SeedType also makes sure that the Rng can be seeded with SeedType.
 *
 * A seedable random-number generator has the following additional features:
 * $(UL
 *   $(LI it has a 'seed(ElementType)' function)
 * )
 */
template isSeedable(Rng, SeedType)
{
    enum bool isSeedable = isUniformRNG!(Rng) &&
        is(typeof(
        {
            Rng r = void;              // can define a Rng object
            r.seed(SeedType.init);     // can seed a Rng
        }));
}

///ditto
template isSeedable(Rng)
{
    enum bool isSeedable = isUniformRNG!Rng &&
        is(typeof(
        {
            Rng r = void;                     // can define a Rng object
            r.seed(typeof(r.front).init);     // can seed a Rng
        }));
}

@safe pure nothrow unittest
{
    struct NoRng
    {
        @property uint front() {return 0;}
        @property bool empty() {return false;}
        void popFront() {}
    }
    static assert(!isUniformRNG!(NoRng, uint));
    static assert(!isUniformRNG!(NoRng));
    static assert(!isSeedable!(NoRng, uint));
    static assert(!isSeedable!(NoRng));

    struct NoRng2
    {
        @property uint front() {return 0;}
        @property bool empty() {return false;}
        void popFront() {}

        enum isUniformRandom = false;
    }
    static assert(!isUniformRNG!(NoRng2, uint));
    static assert(!isUniformRNG!(NoRng2));
    static assert(!isSeedable!(NoRng2, uint));
    static assert(!isSeedable!(NoRng2));

    struct NoRng3
    {
        @property bool empty() {return false;}
        void popFront() {}

        enum isUniformRandom = true;
    }
    static assert(!isUniformRNG!(NoRng3, uint));
    static assert(!isUniformRNG!(NoRng3));
    static assert(!isSeedable!(NoRng3, uint));
    static assert(!isSeedable!(NoRng3));

    struct validRng
    {
        @property uint front() {return 0;}
        @property bool empty() {return false;}
        void popFront() {}

        enum isUniformRandom = true;
    }
    static assert(isUniformRNG!(validRng, uint));
    static assert(isUniformRNG!(validRng));
    static assert(!isSeedable!(validRng, uint));
    static assert(!isSeedable!(validRng));

    struct seedRng
    {
        @property uint front() {return 0;}
        @property bool empty() {return false;}
        void popFront() {}
        void seed(uint val){}
        enum isUniformRandom = true;
    }
    static assert(isUniformRNG!(seedRng, uint));
    static assert(isUniformRNG!(seedRng));
    static assert(isSeedable!(seedRng, uint));
    static assert(isSeedable!(seedRng));
}

/**
Linear Congruential generator.
 */
struct LinearCongruentialEngine(UIntType, UIntType a, UIntType c, UIntType m)
if (isUnsigned!UIntType)
{
    ///Mark this as a Rng
    enum bool isUniformRandom = true;
    /// Does this generator have a fixed range? ($(D_PARAM true)).
    enum bool hasFixedRange = true;
    /// Lowest generated value ($(D 1) if $(D c == 0), $(D 0) otherwise).
    enum UIntType min = ( c == 0 ? 1 : 0 );
    /// Highest generated value ($(D modulus - 1)).
    enum UIntType max = m - 1;
/**
The parameters of this distribution. The random number is $(D_PARAM x
= (x * multipler + increment) % modulus).
 */
    enum UIntType multiplier = a;
    ///ditto
    enum UIntType increment = c;
    ///ditto
    enum UIntType modulus = m;

    static assert(isIntegral!(UIntType));
    static assert(m == 0 || a < m);
    static assert(m == 0 || c < m);
    static assert(m == 0 ||
            (cast(ulong) a * (m-1) + c) % m == (c < a ? c - a + m : c - a));

    // Check for maximum range
    private static ulong gcd(ulong a, ulong b) @safe pure nothrow @nogc
    {
        while (b)
        {
            auto t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    private static ulong primeFactorsOnly(ulong n) @safe pure nothrow @nogc
    {
        ulong result = 1;
        ulong iter = 2;
        for (; n >= iter * iter; iter += 2 - (iter == 2))
        {
            if (n % iter) continue;
            result *= iter;
            do
            {
                n /= iter;
            } while (n % iter == 0);
        }
        return result * n;
    }

    @safe pure nothrow unittest
    {
        static assert(primeFactorsOnly(100) == 10);
        //writeln(primeFactorsOnly(11));
        static assert(primeFactorsOnly(11) == 11);
        static assert(primeFactorsOnly(7 * 7 * 7 * 11 * 15 * 11) == 7 * 11 * 15);
        static assert(primeFactorsOnly(129 * 2) == 129 * 2);
        // enum x = primeFactorsOnly(7 * 7 * 7 * 11 * 15);
        // static assert(x == 7 * 11 * 15);
    }

    private static bool properLinearCongruentialParameters(ulong m,
            ulong a, ulong c) @safe pure nothrow @nogc
    {
        if (m == 0)
        {
            static if (is(UIntType == uint))
            {
                // Assume m is uint.max + 1
                m = (1uL << 32);
            }
            else
            {
                return false;
            }
        }
        // Bounds checking
        if (a == 0 || a >= m || c >= m) return false;
        // c and m are relatively prime
        if (c > 0 && gcd(c, m) != 1) return false;
        // a - 1 is divisible by all prime factors of m
        if ((a - 1) % primeFactorsOnly(m)) return false;
        // if a - 1 is multiple of 4, then m is a  multiple of 4 too.
        if ((a - 1) % 4 == 0 && m % 4) return false;
        // Passed all tests
        return true;
    }

    // check here
    static assert(c == 0 || properLinearCongruentialParameters(m, a, c),
            "Incorrect instantiation of LinearCongruentialEngine");

/**
Constructs a $(D_PARAM LinearCongruentialEngine) generator seeded with
$(D x0).
 */
    this(UIntType x0) @safe pure
    {
        seed(x0);
    }

/**
   (Re)seeds the generator.
*/
    void seed(UIntType x0 = 1) @safe pure
    {
        static if (c == 0)
        {
            import std.exception : enforce;
            enforce(x0, "Invalid (zero) seed for "
                    ~ LinearCongruentialEngine.stringof);
        }
        _x = modulus ? (x0 % modulus) : x0;
        popFront();
    }

/**
   Advances the random sequence.
*/
    void popFront() @safe pure nothrow @nogc
    {
        static if (m)
        {
            static if (is(UIntType == uint) && m == uint.max)
            {
                immutable ulong
                    x = (cast(ulong) a * _x + c),
                    v = x >> 32,
                    w = x & uint.max;
                immutable y = cast(uint)(v + w);
                _x = (y < v || y == uint.max) ? (y + 1) : y;
            }
            else static if (is(UIntType == uint) && m == int.max)
            {
                immutable ulong
                    x = (cast(ulong) a * _x + c),
                    v = x >> 31,
                    w = x & int.max;
                immutable uint y = cast(uint)(v + w);
                _x = (y >= int.max) ? (y - int.max) : y;
            }
            else
            {
                _x = cast(UIntType) ((cast(ulong) a * _x + c) % m);
            }
        }
        else
        {
            _x = a * _x + c;
        }
    }

/**
   Returns the current number in the random sequence.
*/
    @property UIntType front() const @safe pure nothrow @nogc
    {
        return _x;
    }

///
    @property typeof(this) save() @safe pure nothrow @nogc
    {
        return this;
    }

/**
Always $(D false) (random generators are infinite ranges).
 */
    enum bool empty = false;

/**
   Compares against $(D_PARAM rhs) for equality.
 */
    bool opEquals(ref const LinearCongruentialEngine rhs) const @safe pure nothrow @nogc
    {
        return _x == rhs._x;
    }

    private UIntType _x = m ? (a + c) % m : (a + c);
}

/**
Define $(D_PARAM LinearCongruentialEngine) generators with well-chosen
parameters. $(D MinstdRand0) implements Park and Miller's "minimal
standard" $(HTTP
wikipedia.org/wiki/Park%E2%80%93Miller_random_number_generator,
generator) that uses 16807 for the multiplier. $(D MinstdRand)
implements a variant that has slightly better spectral behavior by
using the multiplier 48271. Both generators are rather simplistic.
 */
alias MinstdRand0 = LinearCongruentialEngine!(uint, 16_807, 0, 2_147_483_647);
/// ditto
alias MinstdRand = LinearCongruentialEngine!(uint, 48_271, 0, 2_147_483_647);

///
@safe unittest
{
    // seed with a constant
    auto rnd0 = MinstdRand0(1);
    auto n = rnd0.front; // same for each run
    // Seed with an unpredictable value
    rnd0.seed(unpredictableSeed);
    n = rnd0.front; // different across runs
}

@safe unittest
{
    import std.range;
    static assert(isForwardRange!MinstdRand);
    static assert(isUniformRNG!MinstdRand);
    static assert(isUniformRNG!MinstdRand0);
    static assert(isUniformRNG!(MinstdRand, uint));
    static assert(isUniformRNG!(MinstdRand0, uint));
    static assert(isSeedable!MinstdRand);
    static assert(isSeedable!MinstdRand0);
    static assert(isSeedable!(MinstdRand, uint));
    static assert(isSeedable!(MinstdRand0, uint));

    // The correct numbers are taken from The Database of Integer Sequences
    // http://www.research.att.com/~njas/sequences/eisBTfry00128.txt
    auto checking0 = [
        16807UL,282475249,1622650073,984943658,1144108930,470211272,
        101027544,1457850878,1458777923,2007237709,823564440,1115438165,
        1784484492,74243042,114807987,1137522503,1441282327,16531729,
        823378840,143542612 ];
    //auto rnd0 = MinstdRand0(1);
    MinstdRand0 rnd0;

    foreach (e; checking0)
    {
        assert(rnd0.front == e);
        rnd0.popFront();
    }
    // Test the 10000th invocation
    // Correct value taken from:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2461.pdf
    rnd0.seed();
    popFrontN(rnd0, 9999);
    assert(rnd0.front == 1043618065);

    // Test MinstdRand
    auto checking = [48271UL,182605794,1291394886,1914720637,2078669041,
                     407355683];
    //auto rnd = MinstdRand(1);
    MinstdRand rnd;
    foreach (e; checking)
    {
        assert(rnd.front == e);
        rnd.popFront();
    }

    // Test the 10000th invocation
    // Correct value taken from:
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2461.pdf
    rnd.seed();
    popFrontN(rnd, 9999);
    assert(rnd.front == 399268537);

    // Check .save works
    foreach (Type; std.meta.AliasSeq!(MinstdRand0, MinstdRand))
    {
        auto rnd1 = Type(unpredictableSeed);
        auto rnd2 = rnd1.save;
        assert(rnd1 == rnd2);
        // Enable next test when RNGs are reference types
        version (none) { assert(rnd1 !is rnd2); }
        assert(rnd1.take(100).array() == rnd2.take(100).array());
    }
}

/**
The $(LINK2 https://en.wikipedia.org/wiki/Mersenne_Twister, Mersenne Twister) generator.
 */
struct MersenneTwisterEngine(UIntType, size_t w, size_t n, size_t m, size_t r,
                             UIntType a, size_t u, UIntType d, size_t s,
                             UIntType b, size_t t,
                             UIntType c, size_t l, UIntType f)
if (isUnsigned!UIntType)
{
    static assert(0 < w && w <= UIntType.sizeof * 8);
    static assert(1 <= m && m <= n);
    static assert(0 <= r && 0 <= u && 0 <= s && 0 <= t && 0 <= l);
    static assert(r <= w && u <= w && s <= w && t <= w && l <= w);
    static assert(0 <= a && 0 <= b && 0 <= c);
    static assert(n <= sizediff_t.max);

    ///Mark this as a Rng
    enum bool isUniformRandom = true;

/**
Parameters for the generator.
*/
    enum size_t   wordSize   = w;
    enum size_t   stateSize  = n; /// ditto
    enum size_t   shiftSize  = m; /// ditto
    enum size_t   maskBits   = r; /// ditto
    enum UIntType xorMask    = a; /// ditto
    enum size_t   temperingU = u; /// ditto
    enum UIntType temperingD = d; /// ditto
    enum size_t   temperingS = s; /// ditto
    enum UIntType temperingB = b; /// ditto
    enum size_t   temperingT = t; /// ditto
    enum UIntType temperingC = c; /// ditto
    enum size_t   temperingL = l; /// ditto
    enum UIntType initializationMultiplier = f; /// ditto

    /// Smallest generated value (0).
    enum UIntType min = 0;
    /// Largest generated value.
    enum UIntType max = UIntType.max >> (UIntType.sizeof * 8u - w);
    // note, `max` also serves as a bitmask for the lowest `w` bits
    static assert(a <= max && b <= max && c <= max && f <= max);

    /// The default seed value.
    enum UIntType defaultSeed = 5489u;

    // Bitmasks used in the 'twist' part of the algorithm
    private enum UIntType lowerMask = (cast(UIntType) 1u << r) - 1,
                          upperMask = (~lowerMask) & this.max;

    /*
       Collection of all state variables
       used by the generator
    */
    private struct State
    {
        /*
           State array of the generator.  This
           is iterated through backwards (from
           last element to first), providing a
           few extra compiler optimizations by
           comparison to the forward iteration
           used in most implementations.
        */
        UIntType[n] data;

        /*
           Cached copy of most recently updated
           element of `data` state array, ready
           to be tempered to generate next
           `front` value
        */
        UIntType z;

        /*
           Most recently generated random variate
        */
        UIntType front;

        /*
           Index of the entry in the `data`
           state array that will be twisted
           in the next `popFront()` call
        */
        size_t index;
    }

    /*
       State variables used by the generator;
       initialized to values equivalent to
       explicitly seeding the generator with
       `defaultSeed`
    */
    private State state = defaultState();
    /* NOTE: the above is a workaround to ensure
       backwards compatibility with the original
       implementation, which permitted implicit
       construction.  With `@disable this();`
       it would not be necessary. */

/**
   Constructs a MersenneTwisterEngine object.
*/
    this(UIntType value) @safe pure nothrow @nogc
    {
        seed(value);
    }

    /**
       Generates the default initial state for a Mersenne
       Twister; equivalent to the internal state obtained
       by calling `seed(defaultSeed)`
    */
    private static State defaultState() @safe pure nothrow @nogc
    {
        if (!__ctfe) assert(false);
        State mtState;
        seedImpl(defaultSeed, mtState);
        return mtState;
    }

/**
   Seeds a MersenneTwisterEngine object.
   Note:
   This seed function gives 2^w starting points (the lowest w bits of
   the value provided will be used). To allow the RNG to be started
   in any one of its internal states use the seed overload taking an
   InputRange.
*/
    void seed()(UIntType value = defaultSeed) @safe pure nothrow @nogc
    {
        this.seedImpl(value, this.state);
    }

    /**
       Implementation of the seeding mechanism, which
       can be used with an arbitrary `State` instance
    */
    private static void seedImpl(UIntType value, ref State mtState)
    {
        mtState.data[$ - 1] = value;
        static if (this.max != UIntType.max)
        {
            mtState.data[$ - 1] &= this.max;
        }

        foreach_reverse (size_t i, ref e; mtState.data[0 .. $ - 1])
        {
            e = f * (mtState.data[i + 1] ^ (mtState.data[i + 1] >> (w - 2))) + cast(UIntType)(n - (i + 1));
            static if (this.max != UIntType.max)
            {
                e &= this.max;
            }
        }

        mtState.index = n - 1;

        /* double popFront() to guarantee both `mtState.z`
           and `mtState.front` are derived from the newly
           set values in `mtState.data` */
        MersenneTwisterEngine.popFrontImpl(mtState);
        MersenneTwisterEngine.popFrontImpl(mtState);
    }

/**
   Seeds a MersenneTwisterEngine object using an InputRange.

   Throws:
   $(D Exception) if the InputRange didn't provide enough elements to seed the generator.
   The number of elements required is the 'n' template parameter of the MersenneTwisterEngine struct.
 */
    void seed(T)(T range) if (isInputRange!T && is(Unqual!(ElementType!T) == UIntType))
    {
        this.seedImpl(range, this.state);
    }

    /**
       Implementation of the range-based seeding mechanism,
       which can be used with an arbitrary `State` instance
    */
    private static void seedImpl(T)(T range, ref State mtState)
        if (isInputRange!T && is(Unqual!(ElementType!T) == UIntType))
    {
        size_t j;
        for (j = 0; j < n && !range.empty; ++j, range.popFront())
        {
            sizediff_t idx = n - j - 1;
            mtState.data[idx] = range.front;
        }

        mtState.index = n - 1;

        if (range.empty && j < n)
        {
            import core.internal.string : UnsignedStringBuf, unsignedToTempString;

            UnsignedStringBuf buf = void;
            string s = "MersenneTwisterEngine.seed: Input range didn't provide enough elements: Need ";
            s ~= unsignedToTempString(n, buf, 10) ~ " elements.";
            throw new Exception(s);
        }

        /* double popFront() to guarantee both `mtState.z`
           and `mtState.front` are derived from the newly
           set values in `mtState.data` */
        MersenneTwisterEngine.popFrontImpl(mtState);
        MersenneTwisterEngine.popFrontImpl(mtState);
    }

/**
   Advances the generator.
*/
    void popFront() @safe pure nothrow @nogc
    {
        this.popFrontImpl(this.state);
    }

    /*
       Internal implementation of `popFront()`, which
       can be used with an arbitrary `State` instance
    */
    private static void popFrontImpl(ref State mtState)
    {
        /* This function blends two nominally independent
           processes: (i) calculation of the next random
           variate `mtState.front` from the cached previous
           `data` entry `z`, and (ii) updating the value
           of `data[index]` and `mtState.z` and advancing
           the `index` value to the next in sequence.

           By interweaving the steps involved in these
           procedures, rather than performing each of
           them separately in sequence, the variables
           are kept 'hot' in CPU registers, allowing
           for significantly faster performance. */
        sizediff_t index = mtState.index;
        sizediff_t next = index - 1;
        if (next < 0)
            next = n - 1;
        auto z = mtState.z;
        sizediff_t conj = index - m;
        if (conj < 0)
            conj = index - m + n;

        static if (d == UIntType.max)
        {
            z ^= (z >> u);
        }
        else
        {
            z ^= (z >> u) & d;
        }

        auto q = mtState.data[index] & upperMask;
        auto p = mtState.data[next] & lowerMask;
        z ^= (z << s) & b;
        auto y = q | p;
        auto x = y >> 1;
        z ^= (z << t) & c;
        if (y & 1)
            x ^= a;
        auto e = mtState.data[conj] ^ x;
        z ^= (z >> l);
        mtState.z = mtState.data[index] = e;
        mtState.index = next;

        /* technically we should take the lowest `w`
           bits here, but if the tempering bitmasks
           `b` and `c` are set correctly, this should
           be unnecessary */
        mtState.front = z;
    }

/**
   Returns the current random value.
 */
    @property UIntType front() @safe const pure nothrow @nogc
    {
        return this.state.front;
    }

///
    @property typeof(this) save() @safe pure nothrow @nogc
    {
        return this;
    }

/**
Always $(D false).
 */
    enum bool empty = false;
}

/**
A $(D MersenneTwisterEngine) instantiated with the parameters of the
original engine $(HTTP math.sci.hiroshima-u.ac.jp/~m-mat/MT/emt.html,
MT19937), generating uniformly-distributed 32-bit numbers with a
period of 2 to the power of 19937. Recommended for random number
generation unless memory is severely restricted, in which case a $(D
LinearCongruentialEngine) would be the generator of choice.
 */
alias Mt19937 = MersenneTwisterEngine!(uint, 32, 624, 397, 31,
                                       0x9908b0df, 11, 0xffffffff, 7,
                                       0x9d2c5680, 15,
                                       0xefc60000, 18, 1_812_433_253);

///
@safe unittest
{
    // seed with a constant
    Mt19937 gen;
    auto n = gen.front; // same for each run
    // Seed with an unpredictable value
    gen.seed(unpredictableSeed);
    n = gen.front; // different across runs
}

@safe nothrow unittest
{
    import std.algorithm;
    import std.range;
    static assert(isUniformRNG!Mt19937);
    static assert(isUniformRNG!(Mt19937, uint));
    static assert(isSeedable!Mt19937);
    static assert(isSeedable!(Mt19937, uint));
    static assert(isSeedable!(Mt19937, typeof(map!((a) => unpredictableSeed)(repeat(0)))));
    Mt19937 gen;
    assert(gen.front == 3499211612);
    popFrontN(gen, 9999);
    assert(gen.front == 4123659995);
    try { gen.seed(iota(624u)); } catch (Exception) { assert(false); }
    assert(gen.front == 3708921088u);
    popFrontN(gen, 9999);
    assert(gen.front == 165737292u);
}

/**
A $(D MersenneTwisterEngine) instantiated with the parameters of the
original engine $(HTTP en.wikipedia.org/wiki/Mersenne_Twister,
MT19937-64), generating uniformly-distributed 64-bit numbers with a
period of 2 to the power of 19937.
*/
alias Mt19937_64 = MersenneTwisterEngine!(ulong, 64, 312, 156, 31,
                                          0xb5026f5aa96619e9, 29, 0x5555555555555555, 17,
                                          0x71d67fffeda60000, 37,
                                          0xfff7eee000000000, 43, 6_364_136_223_846_793_005);

///
@safe unittest
{
    // Seed with a constant
    auto gen = Mt19937_64(12345);
    auto n = gen.front; // same for each run
    // Seed with an unpredictable value
    gen.seed(unpredictableSeed);
    n = gen.front; // different across runs
}

@safe nothrow unittest
{
    import std.algorithm;
    import std.range;
    static assert(isUniformRNG!Mt19937_64);
    static assert(isUniformRNG!(Mt19937_64, ulong));
    static assert(isSeedable!Mt19937_64);
    static assert(isSeedable!(Mt19937_64, ulong));
    // FIXME: this test demonstrates viably that Mt19937_64
    // is seedable with an infinite range of `ulong` values
    // but it's a poor example of how to actually seed the
    // generator, since it can't cover the full range of
    // possible seed values.  Ideally we need a 64-bit
    // unpredictable seed to complement the 32-bit one!
    static assert(isSeedable!(Mt19937_64, typeof(map!((a) => (cast(ulong) unpredictableSeed))(repeat(0)))));
    Mt19937_64 gen;
    assert(gen.front == 14514284786278117030uL);
    popFrontN(gen, 9999);
    assert(gen.front == 9981545732273789042uL);
    try { gen.seed(iota(312uL)); } catch (Exception) { assert(false); }
    assert(gen.front == 14660652410669508483uL);
    popFrontN(gen, 9999);
    assert(gen.front == 15956361063660440239uL);
}

@safe unittest
{
    import std.algorithm;
    import std.exception;
    import std.range;

    Mt19937 gen;

    assertThrown(gen.seed(map!((a) => unpredictableSeed)(repeat(0, 623))));

    gen.seed(map!((a) => unpredictableSeed)(repeat(0, 624)));
    //infinite Range
    gen.seed(map!((a) => unpredictableSeed)(repeat(0)));
}

@safe pure nothrow unittest
{
    uint a, b;
    {
        Mt19937 gen;
        a = gen.front;
    }
    {
        Mt19937 gen;
        gen.popFront();
        //popFrontN(gen, 1);  // skip 1 element
        b = gen.front;
    }
    assert(a != b);
}

@safe unittest
{
    import std.range;
    // Check .save works
    foreach (Type; std.meta.AliasSeq!(Mt19937, Mt19937_64))
    {
        auto gen1 = Type(unpredictableSeed);
        auto gen2 = gen1.save;
        assert(gen1 == gen2);  // Danger, Will Robinson -- no opEquals for MT
        // Enable next test when RNGs are reference types
        version (none) { assert(gen1 !is gen2); }
        assert(gen1.take(100).array() == gen2.take(100).array());
    }
}

@safe pure nothrow unittest //11690
{
    alias MT(UIntType, uint w) = MersenneTwisterEngine!(UIntType, w, 624, 397, 31,
                                                        0x9908b0df, 11, 0xffffffff, 7,
                                                        0x9d2c5680, 15,
                                                        0xefc60000, 18, 1812433253);

    ulong[] expectedFirstValue = [3499211612uL, 3499211612uL,
                                  171143175841277uL, 1145028863177033374uL];

    ulong[] expected10kValue = [4123659995uL, 4123659995uL,
                                51991688252792uL, 3031481165133029945uL];

    foreach (i, R; std.meta.AliasSeq!(MT!(uint, 32), MT!(ulong, 32), MT!(ulong, 48), MT!(ulong, 64)))
    {
        auto a = R();
        a.seed(a.defaultSeed); // checks that some alternative paths in `seed` are utilized
        assert(a.front == expectedFirstValue[i]);
        a.popFrontN(9999);
        assert(a.front == expected10kValue[i]);
    }
}


/**
 * Xorshift generator using 32bit algorithm.
 *
 * Implemented according to $(HTTP www.jstatsoft.org/v08/i14/paper, Xorshift RNGs).
 * Supporting bits are below, $(D bits) means second parameter of XorshiftEngine.
 *
 * $(BOOKTABLE ,
 *  $(TR $(TH bits) $(TH period))
 *  $(TR $(TD 32)   $(TD 2^32 - 1))
 *  $(TR $(TD 64)   $(TD 2^64 - 1))
 *  $(TR $(TD 96)   $(TD 2^96 - 1))
 *  $(TR $(TD 128)  $(TD 2^128 - 1))
 *  $(TR $(TD 160)  $(TD 2^160 - 1))
 *  $(TR $(TD 192)  $(TD 2^192 - 2^32))
 * )
 */
struct XorshiftEngine(UIntType, UIntType bits, UIntType a, UIntType b, UIntType c)
if (isUnsigned!UIntType)
{
    static assert(bits == 32 || bits == 64 || bits == 96 || bits == 128 || bits == 160 || bits == 192,
                  "Xorshift supports only 32, 64, 96, 128, 160 and 192 bit versions. "
                  ~ to!string(bits) ~ " is not supported.");

  public:
    ///Mark this as a Rng
    enum bool isUniformRandom = true;
    /// Always $(D false) (random generators are infinite ranges).
    enum empty = false;
    /// Smallest generated value.
    enum UIntType min = 0;
    /// Largest generated value.
    enum UIntType max = UIntType.max;


  private:
    enum size = bits / 32;

    static if (bits == 32)
        UIntType[size] seeds_ = [2_463_534_242];
    else static if (bits == 64)
        UIntType[size] seeds_ = [123_456_789, 362_436_069];
    else static if (bits == 96)
        UIntType[size] seeds_ = [123_456_789, 362_436_069, 521_288_629];
    else static if (bits == 128)
        UIntType[size] seeds_ = [123_456_789, 362_436_069, 521_288_629, 88_675_123];
    else static if (bits == 160)
        UIntType[size] seeds_ = [123_456_789, 362_436_069, 521_288_629, 88_675_123, 5_783_321];
    else static if (bits == 192)
    {
        UIntType[size] seeds_ = [123_456_789, 362_436_069, 521_288_629, 88_675_123, 5_783_321, 6_615_241];
        UIntType       value_;
    }
    else
    {
        static assert(false, "Phobos Error: Xorshift has no instantiation rule for "
                             ~ to!string(bits) ~ " bits.");
    }


  public:
    /**
     * Constructs a $(D XorshiftEngine) generator seeded with $(D_PARAM x0).
     */
    this(UIntType x0) @safe pure nothrow @nogc
    {
        seed(x0);
    }


    /**
     * (Re)seeds the generator.
     */
    void seed(UIntType x0) @safe pure nothrow @nogc
    {
        // Initialization routine from MersenneTwisterEngine.
        foreach (i, e; seeds_)
            seeds_[i] = x0 = cast(UIntType)(1_812_433_253U * (x0 ^ (x0 >> 30)) + i + 1);

        // All seeds must not be 0.
        sanitizeSeeds(seeds_);

        popFront();
    }


    /**
     * Returns the current number in the random sequence.
     */
    @property
    UIntType front() const @safe pure nothrow @nogc
    {
        static if (bits == 192)
            return value_;
        else
            return seeds_[size - 1];
    }


    /**
     * Advances the random sequence.
     */
    void popFront() @safe pure nothrow @nogc
    {
        UIntType temp;

        static if (bits == 32)
        {
            temp      = seeds_[0] ^ (seeds_[0] << a);
            temp      = temp ^ (temp >> b);
            seeds_[0] = temp ^ (temp << c);
        }
        else static if (bits == 64)
        {
            temp      = seeds_[0] ^ (seeds_[0] << a);
            seeds_[0] = seeds_[1];
            seeds_[1] = seeds_[1] ^ (seeds_[1] >> c) ^ temp ^ (temp >> b);
        }
        else static if (bits == 96)
        {
            temp      = seeds_[0] ^ (seeds_[0] << a);
            seeds_[0] = seeds_[1];
            seeds_[1] = seeds_[2];
            seeds_[2] = seeds_[2] ^ (seeds_[2] >> c) ^ temp ^ (temp >> b);
        }
        else static if (bits == 128)
        {
            temp      = seeds_[0] ^ (seeds_[0] << a);
            seeds_[0] = seeds_[1];
            seeds_[1] = seeds_[2];
            seeds_[2] = seeds_[3];
            seeds_[3] = seeds_[3] ^ (seeds_[3] >> c) ^ temp ^ (temp >> b);
        }
        else static if (bits == 160)
        {
            temp      = seeds_[0] ^ (seeds_[0] << a);
            seeds_[0] = seeds_[1];
            seeds_[1] = seeds_[2];
            seeds_[2] = seeds_[3];
            seeds_[3] = seeds_[4];
            seeds_[4] = seeds_[4] ^ (seeds_[4] >> c) ^ temp ^ (temp >> b);
        }
        else static if (bits == 192)
        {
            temp      = seeds_[0] ^ (seeds_[0] >> a);
            seeds_[0] = seeds_[1];
            seeds_[1] = seeds_[2];
            seeds_[2] = seeds_[3];
            seeds_[3] = seeds_[4];
            seeds_[4] = seeds_[4] ^ (seeds_[4] << c) ^ temp ^ (temp << b);
            value_    = seeds_[4] + (seeds_[5] += 362_437);
        }
        else
        {
            static assert(false, "Phobos Error: Xorshift has no popFront() update for "
                                 ~ to!string(bits) ~ " bits.");
        }
    }


    /**
     * Captures a range state.
     */
    @property
    typeof(this) save() @safe pure nothrow @nogc
    {
        return this;
    }


    /**
     * Compares against $(D_PARAM rhs) for equality.
     */
    bool opEquals(ref const XorshiftEngine rhs) const @safe pure nothrow @nogc
    {
        return seeds_ == rhs.seeds_;
    }


  private:
    static void sanitizeSeeds(ref UIntType[size] seeds) @safe pure nothrow @nogc
    {
        for (uint i; i < seeds.length; i++)
        {
            if (seeds[i] == 0)
                seeds[i] = i + 1;
        }
    }


    @safe pure nothrow unittest
    {
        static if (size  ==  4)  // Other bits too
        {
            UIntType[size] seeds = [1, 0, 0, 4];

            sanitizeSeeds(seeds);

            assert(seeds == [1, 2, 3, 4]);
        }
    }
}


/**
 * Define $(D XorshiftEngine) generators with well-chosen parameters. See each bits examples of "Xorshift RNGs".
 * $(D Xorshift) is a Xorshift128's alias because 128bits implementation is mostly used.
 */
alias Xorshift32  = XorshiftEngine!(uint, 32,  13, 17, 15) ;
alias Xorshift64  = XorshiftEngine!(uint, 64,  10, 13, 10); /// ditto
alias Xorshift96  = XorshiftEngine!(uint, 96,  10, 5,  26); /// ditto
alias Xorshift128 = XorshiftEngine!(uint, 128, 11, 8,  19); /// ditto
alias Xorshift160 = XorshiftEngine!(uint, 160, 2,  1,  4);  /// ditto
alias Xorshift192 = XorshiftEngine!(uint, 192, 2,  1,  4);  /// ditto
alias Xorshift    = Xorshift128;                            /// ditto

///
@safe unittest
{
    // Seed with a constant
    auto rnd = Xorshift(1);
    auto num = rnd.front;  // same for each run

    // Seed with an unpredictable value
    rnd.seed(unpredictableSeed);
    num = rnd.front; // different across rnd
}

@safe unittest
{
    import std.range;
    static assert(isForwardRange!Xorshift);
    static assert(isUniformRNG!Xorshift);
    static assert(isUniformRNG!(Xorshift, uint));
    static assert(isSeedable!Xorshift);
    static assert(isSeedable!(Xorshift, uint));

    // Result from reference implementation.
    auto checking = [
        [2463534242UL, 901999875, 3371835698, 2675058524, 1053936272, 3811264849,
        472493137, 3856898176, 2131710969, 2312157505],
        [362436069UL, 2113136921, 19051112, 3010520417, 951284840, 1213972223,
        3173832558, 2611145638, 2515869689, 2245824891],
        [521288629UL, 1950277231, 185954712, 1582725458, 3580567609, 2303633688,
        2394948066, 4108622809, 1116800180, 3357585673],
        [88675123UL, 3701687786, 458299110, 2500872618, 3633119408, 516391518,
        2377269574, 2599949379, 717229868, 137866584],
        [5783321UL, 393427209, 1947109840, 565829276, 1006220149, 971147905,
        1436324242, 2800460115, 1484058076, 3823330032],
        [0UL, 246875399, 3690007200, 1264581005, 3906711041, 1866187943, 2481925219,
        2464530826, 1604040631, 3653403911]
    ];

    alias XorshiftTypes = std.meta.AliasSeq!(Xorshift32, Xorshift64, Xorshift96, Xorshift128, Xorshift160, Xorshift192);

    foreach (I, Type; XorshiftTypes)
    {
        Type rnd;

        foreach (e; checking[I])
        {
            assert(rnd.front == e);
            rnd.popFront();
        }
    }

    // Check .save works
    foreach (Type; XorshiftTypes)
    {
        auto rnd1 = Type(unpredictableSeed);
        auto rnd2 = rnd1.save;
        assert(rnd1 == rnd2);
        // Enable next test when RNGs are reference types
        version (none) { assert(rnd1 !is rnd2); }
        assert(rnd1.take(100).array() == rnd2.take(100).array());
    }
}


/* A complete list of all pseudo-random number generators implemented in
 * std.random.  This can be used to confirm that a given function or
 * object is compatible with all the pseudo-random number generators
 * available.  It is enabled only in unittest mode.
 */
@safe unittest
{
    foreach (Rng; PseudoRngTypes)
    {
        static assert(isUniformRNG!Rng);
        auto rng = Rng(unpredictableSeed);
    }
}


/**
A "good" seed for initializing random number engines. Initializing
with $(D_PARAM unpredictableSeed) makes engines generate different
random number sequences every run.

Returns:
A single unsigned integer seed value, different on each successive call
*/
@property uint unpredictableSeed() @trusted
{
    import core.thread : Thread, getpid, MonoTime;
    static bool seeded;
    static MinstdRand0 rand;
    if (!seeded)
    {
        uint threadID = cast(uint) cast(void*) Thread.getThis();
        rand.seed((getpid() + threadID) ^ cast(uint) MonoTime.currTime.ticks);
        seeded = true;
    }
    rand.popFront();
    return cast(uint) (MonoTime.currTime.ticks ^ rand.front);
}

///
@safe unittest
{
    auto rnd = Random(unpredictableSeed);
    auto n = rnd.front;
    static assert(is(typeof(n) == uint));
}

/**
The "default", "favorite", "suggested" random number generator type on
the current platform. It is an alias for one of the previously-defined
generators. You may want to use it if (1) you need to generate some
nice random numbers, and (2) you don't care for the minutiae of the
method being used.
 */

alias Random = Mt19937;

@safe unittest
{
    static assert(isUniformRNG!Random);
    static assert(isUniformRNG!(Random, uint));
    static assert(isSeedable!Random);
    static assert(isSeedable!(Random, uint));
}

/**
Global random number generator used by various functions in this
module whenever no generator is specified. It is allocated per-thread
and initialized to an unpredictable value for each thread.

Returns:
A singleton instance of the default random number generator
 */
@property ref Random rndGen() @safe
{
    import std.algorithm.iteration : map;
    import std.range : repeat;

    static Random result;
    static bool initialized;
    if (!initialized)
    {
        static if (isSeedable!(Random, typeof(map!((a) => unpredictableSeed)(repeat(0)))))
            result.seed(map!((a) => unpredictableSeed)(repeat(0)));
        else
            result = Random(unpredictableSeed);
        initialized = true;
    }
    return result;
}

/**
Generates a number between $(D a) and $(D b). The $(D boundaries)
parameter controls the shape of the interval (open vs. closed on
either side). Valid values for $(D boundaries) are $(D "[]"), $(D
"$(LPAREN)]"), $(D "[$(RPAREN)"), and $(D "()"). The default interval
is closed to the left and open to the right. The version that does not
take $(D urng) uses the default generator $(D rndGen).

Params:
    a = lower bound of the _uniform distribution
    b = upper bound of the _uniform distribution
    urng = (optional) random number generator to use;
           if not specified, defaults to $(D rndGen)

Returns:
    A single random variate drawn from the _uniform distribution
    between $(D a) and $(D b), whose type is the common type of
    these parameters
 */
auto uniform(string boundaries = "[)", T1, T2)
(T1 a, T2 b)
if (!is(CommonType!(T1, T2) == void))
{
    return uniform!(boundaries, T1, T2, Random)(a, b, rndGen);
}

///
@safe unittest
{
    auto gen = Random(unpredictableSeed);
    // Generate an integer in [0, 1023]
    auto a = uniform(0, 1024, gen);
    // Generate a float in [0, 1)
    auto b = uniform(0.0f, 1.0f, gen);
}

@safe unittest
{
    MinstdRand0 gen;
    foreach (i; 0 .. 20)
    {
        auto x = uniform(0.0, 15.0, gen);
        assert(0 <= x && x < 15);
    }
    foreach (i; 0 .. 20)
    {
        auto x = uniform!"[]"('a', 'z', gen);
        assert('a' <= x && x <= 'z');
    }

    foreach (i; 0 .. 20)
    {
        auto x = uniform('a', 'z', gen);
        assert('a' <= x && x < 'z');
    }

    foreach (i; 0 .. 20)
    {
        immutable ubyte a = 0;
            immutable ubyte b = 15;
        auto x = uniform(a, b, gen);
            assert(a <= x && x < b);
    }
}

// Implementation of uniform for floating-point types
/// ditto
auto uniform(string boundaries = "[)",
        T1, T2, UniformRandomNumberGenerator)
(T1 a, T2 b, ref UniformRandomNumberGenerator urng)
if (isFloatingPoint!(CommonType!(T1, T2)) && isUniformRNG!UniformRandomNumberGenerator)
{
    import std.conv : text;
    import std.exception : enforce;
    alias NumberType = Unqual!(CommonType!(T1, T2));
    static if (boundaries[0] == '(')
    {
        import std.math : nextafter;
        NumberType _a = nextafter(cast(NumberType) a, NumberType.infinity);
    }
    else
    {
        NumberType _a = a;
    }
    static if (boundaries[1] == ')')
    {
        import std.math : nextafter;
        NumberType _b = nextafter(cast(NumberType) b, -NumberType.infinity);
    }
    else
    {
        NumberType _b = b;
    }
    enforce(_a <= _b,
            text("std.random.uniform(): invalid bounding interval ",
                    boundaries[0], a, ", ", b, boundaries[1]));
    NumberType result =
        _a + (_b - _a) * cast(NumberType) (urng.front - urng.min)
        / (urng.max - urng.min);
    urng.popFront();
    return result;
}

// Implementation of uniform for integral types
/+ Description of algorithm and suggestion of correctness:

The modulus operator maps an integer to a small, finite space. For instance, `x
% 3` will map whatever x is into the range [0 .. 3). 0 maps to 0, 1 maps to 1, 2
maps to 2, 3 maps to 0, and so on infinitely. As long as the integer is
uniformly chosen from the infinite space of all non-negative integers then `x %
3` will uniformly fall into that range.

(Non-negative is important in this case because some definitions of modulus,
namely the one used in computers generally, map negative numbers differently to
(-3 .. 0]. `uniform` does not use negative number modulus, thus we can safely
ignore that fact.)

The issue with computers is that integers have a finite space they must fit in,
and our uniformly chosen random number is picked in that finite space. So, that
method is not sufficient. You can look at it as the integer space being divided
into "buckets" and every bucket after the first bucket maps directly into that
first bucket. `[0, 1, 2]`, `[3, 4, 5]`, ... When integers are finite, then the
last bucket has the chance to be "incomplete": `[uint.max - 3, uint.max - 2,
uint.max - 1]`, `[uint.max]` ... (the last bucket only has 1!). The issue here
is that _every_ bucket maps _completely_ to the first bucket except for that
last one. The last one doesn't have corresponding mappings to 1 or 2, in this
case, which makes it unfair.

So, the answer is to simply "reroll" if you're in that last bucket, since it's
the only unfair one. Eventually you'll roll into a fair bucket. Simply, instead
of the meaning of the last bucket being "maps to `[0]`", it changes to "maps to
`[0, 1, 2]`", which is precisely what we want.

To generalize, `upperDist` represents the size of our buckets (and, thus, the
exclusive upper bound for our desired uniform number). `rnum` is a uniformly
random number picked from the space of integers that a computer can hold (we'll
say `UpperType` represents that type).

We'll first try to do the mapping into the first bucket by doing `offset = rnum
% upperDist`. We can figure out the position of the front of the bucket we're in
by `bucketFront = rnum - offset`.

If we start at `UpperType.max` and walk backwards `upperDist - 1` spaces, then
the space we land on is the last acceptable position where a full bucket can
fit:

```
   bucketFront     UpperType.max
      v                 v
[..., 0, 1, 2, ..., upperDist - 1]
      ^~~ upperDist - 1 ~~^
```

If the bucket starts any later, then it must have lost at least one number and
at least that number won't be represented fairly.

```
                bucketFront     UpperType.max
                     v                v
[..., upperDist - 1, 0, 1, 2, ..., upperDist - 2]
          ^~~~~~~~ upperDist - 1 ~~~~~~~^
```

Hence, our condition to reroll is
`bucketFront > (UpperType.max - (upperDist - 1))`
+/
auto uniform(string boundaries = "[)", T1, T2, RandomGen)
(T1 a, T2 b, ref RandomGen rng)
if ((isIntegral!(CommonType!(T1, T2)) || isSomeChar!(CommonType!(T1, T2))) &&
     isUniformRNG!RandomGen)
{
    import std.conv : text, unsigned;
    import std.exception : enforce;
    alias ResultType = Unqual!(CommonType!(T1, T2));
    static if (boundaries[0] == '(')
    {
        enforce(a < ResultType.max,
                text("std.random.uniform(): invalid left bound ", a));
        ResultType lower = cast(ResultType) (a + 1);
    }
    else
    {
        ResultType lower = a;
    }

    static if (boundaries[1] == ']')
    {
        enforce(lower <= b,
                text("std.random.uniform(): invalid bounding interval ",
                        boundaries[0], a, ", ", b, boundaries[1]));
        /* Cannot use this next optimization with dchar, as dchar
         * only partially uses its full bit range
         */
        static if (!is(ResultType == dchar))
        {
            if (b == ResultType.max && lower == ResultType.min)
            {
                // Special case - all bits are occupied
                return std.random.uniform!ResultType(rng);
            }
        }
        auto upperDist = unsigned(b - lower) + 1u;
    }
    else
    {
        enforce(lower < b,
                text("std.random.uniform(): invalid bounding interval ",
                        boundaries[0], a, ", ", b, boundaries[1]));
        auto upperDist = unsigned(b - lower);
    }

    assert(upperDist != 0);

    alias UpperType = typeof(upperDist);
    static assert(UpperType.min == 0);

    UpperType offset, rnum, bucketFront;
    do
    {
        rnum = uniform!UpperType(rng);
        offset = rnum % upperDist;
        bucketFront = rnum - offset;
    } // while we're in an unfair bucket...
    while (bucketFront > (UpperType.max - (upperDist - 1)));

    return cast(ResultType)(lower + offset);
}

@safe unittest
{
    import std.conv : to;
    auto gen = Mt19937(unpredictableSeed);
    static assert(isForwardRange!(typeof(gen)));

    auto a = uniform(0, 1024, gen);
    assert(0 <= a && a <= 1024);
    auto b = uniform(0.0f, 1.0f, gen);
    assert(0 <= b && b < 1, to!string(b));
    auto c = uniform(0.0, 1.0);
    assert(0 <= c && c < 1);

    foreach (T; std.meta.AliasSeq!(char, wchar, dchar, byte, ubyte, short, ushort,
                          int, uint, long, ulong, float, double, real))
    {
        T lo = 0, hi = 100;

        // Try tests with each of the possible bounds
        {
            T init = uniform(lo, hi);
            size_t i = 50;
            while (--i && uniform(lo, hi) == init) {}
            assert(i > 0);
        }
        {
            T init = uniform!"[)"(lo, hi);
            size_t i = 50;
            while (--i && uniform(lo, hi) == init) {}
            assert(i > 0);
        }
        {
            T init = uniform!"(]"(lo, hi);
            size_t i = 50;
            while (--i && uniform(lo, hi) == init) {}
            assert(i > 0);
        }
        {
            T init = uniform!"()"(lo, hi);
            size_t i = 50;
            while (--i && uniform(lo, hi) == init) {}
            assert(i > 0);
        }
        {
            T init = uniform!"[]"(lo, hi);
            size_t i = 50;
            while (--i && uniform(lo, hi) == init) {}
            assert(i > 0);
        }

        /* Test case with closed boundaries covering whole range
         * of integral type
         */
        static if (isIntegral!T || isSomeChar!T)
        {
            foreach (immutable _; 0 .. 100)
            {
                auto u = uniform!"[]"(T.min, T.max);
                static assert(is(typeof(u) == T));
                assert(T.min <= u, "Lower bound violation for uniform!\"[]\" with " ~ T.stringof);
                assert(u <= T.max, "Upper bound violation for uniform!\"[]\" with " ~ T.stringof);
            }
        }
    }

    auto reproRng = Xorshift(239842);

    foreach (T; std.meta.AliasSeq!(char, wchar, dchar, byte, ubyte, short,
                          ushort, int, uint, long, ulong))
    {
        T lo = T.min + 10, hi = T.max - 10;
        T init = uniform(lo, hi, reproRng);
        size_t i = 50;
        while (--i && uniform(lo, hi, reproRng) == init) {}
        assert(i > 0);
    }

    {
        bool sawLB = false, sawUB = false;
        foreach (i; 0 .. 50)
        {
            auto x = uniform!"[]"('a', 'd', reproRng);
            if (x == 'a') sawLB = true;
            if (x == 'd') sawUB = true;
            assert('a' <= x && x <= 'd');
        }
        assert(sawLB && sawUB);
    }

    {
        bool sawLB = false, sawUB = false;
        foreach (i; 0 .. 50)
        {
            auto x = uniform('a', 'd', reproRng);
            if (x == 'a') sawLB = true;
            if (x == 'c') sawUB = true;
            assert('a' <= x && x < 'd');
        }
        assert(sawLB && sawUB);
    }

    {
        bool sawLB = false, sawUB = false;
        foreach (i; 0 .. 50)
        {
            immutable int lo = -2, hi = 2;
            auto x = uniform!"()"(lo, hi, reproRng);
            if (x == (lo+1)) sawLB = true;
            if (x == (hi-1)) sawUB = true;
            assert(lo < x && x < hi);
        }
        assert(sawLB && sawUB);
    }

    {
        bool sawLB = false, sawUB = false;
        foreach (i; 0 .. 50)
        {
            immutable ubyte lo = 0, hi = 5;
            auto x = uniform(lo, hi, reproRng);
            if (x == lo) sawLB = true;
            if (x == (hi-1)) sawUB = true;
            assert(lo <= x && x < hi);
        }
        assert(sawLB && sawUB);
    }

    {
        foreach (i; 0 .. 30)
        {
            assert(i == uniform(i, i+1, reproRng));
        }
    }
}

/**
Generates a uniformly-distributed number in the range $(D [T.min,
T.max]) for any integral or character type $(D T). If no random
number generator is passed, uses the default $(D rndGen).

Params:
    urng = (optional) random number generator to use;
           if not specified, defaults to $(D rndGen)

Returns:
    Random variate drawn from the _uniform distribution across all
    possible values of the integral or character type $(D T).
 */
auto uniform(T, UniformRandomNumberGenerator)
(ref UniformRandomNumberGenerator urng)
if (!is(T == enum) && (isIntegral!T || isSomeChar!T) && isUniformRNG!UniformRandomNumberGenerator)
{
    /* dchar does not use its full bit range, so we must
     * revert to the uniform with specified bounds
     */
    static if (is(T == dchar))
    {
        return uniform!"[]"(T.min, T.max);
    }
    else
    {
        auto r = urng.front;
        urng.popFront();
        static if (T.sizeof <= r.sizeof)
        {
            return cast(T) r;
        }
        else
        {
            static assert(T.sizeof == 8 && r.sizeof == 4);
            T r1 = urng.front | (cast(T) r << 32);
            urng.popFront();
            return r1;
        }
    }
}

/// Ditto
auto uniform(T)()
if (!is(T == enum) && (isIntegral!T || isSomeChar!T))
{
    return uniform!T(rndGen);
}

@safe unittest
{
    foreach (T; std.meta.AliasSeq!(char, wchar, dchar, byte, ubyte, short, ushort,
                          int, uint, long, ulong))
    {
        T init = uniform!T();
        size_t i = 50;
        while (--i && uniform!T() == init) {}
        assert(i > 0);

        foreach (immutable _; 0 .. 100)
        {
            auto u = uniform!T();
            static assert(is(typeof(u) == T));
            assert(T.min <= u, "Lower bound violation for uniform!" ~ T.stringof);
            assert(u <= T.max, "Upper bound violation for uniform!" ~ T.stringof);
        }
    }
}

/**
Returns a uniformly selected member of enum $(D E). If no random number
generator is passed, uses the default $(D rndGen).

Params:
    urng = (optional) random number generator to use;
           if not specified, defaults to $(D rndGen)

Returns:
    Random variate drawn with equal probability from any
    of the possible values of the enum $(D E).
 */
auto uniform(E, UniformRandomNumberGenerator)
(ref UniformRandomNumberGenerator urng)
if (is(E == enum) && isUniformRNG!UniformRandomNumberGenerator)
{
    static immutable E[EnumMembers!E.length] members = [EnumMembers!E];
    return members[std.random.uniform(0, members.length, urng)];
}

/// Ditto
auto uniform(E)()
if (is(E == enum))
{
    return uniform!E(rndGen);
}

///
@safe unittest
{
    enum Fruit { apple, mango, pear }
    auto randFruit = uniform!Fruit();
}

@safe unittest
{
    enum Fruit { Apple = 12, Mango = 29, Pear = 72 }
    foreach (_; 0 .. 100)
    {
        foreach (f; [uniform!Fruit(), rndGen.uniform!Fruit()])
        {
            assert(f == Fruit.Apple || f == Fruit.Mango || f == Fruit.Pear);
        }
    }
}

/**
 * Generates a uniformly-distributed floating point number of type
 * $(D T) in the range [0, 1$(RPAREN).  If no random number generator is
 * specified, the default RNG $(D rndGen) will be used as the source
 * of randomness.
 *
 * $(D uniform01) offers a faster generation of random variates than
 * the equivalent $(D uniform!"[$(RPAREN)"(0.0, 1.0)) and so may be preferred
 * for some applications.
 *
 * Params:
 *     rng = (optional) random number generator to use;
 *           if not specified, defaults to $(D rndGen)
 *
 * Returns:
 *     Floating-point random variate of type $(D T) drawn from the _uniform
 *     distribution across the half-open interval [0, 1$(RPAREN).
 *
 */
T uniform01(T = double)()
if (isFloatingPoint!T)
{
    return uniform01!T(rndGen);
}

/// ditto
T uniform01(T = double, UniformRNG)(ref UniformRNG rng)
if (isFloatingPoint!T && isUniformRNG!UniformRNG)
out (result)
{
    assert(0 <= result);
    assert(result < 1);
}
body
{
    alias R = typeof(rng.front);
    static if (isIntegral!R)
    {
        enum T factor = 1 / (T(1) + rng.max - rng.min);
    }
    else static if (isFloatingPoint!R)
    {
        enum T factor = 1 / (rng.max - rng.min);
    }
    else
    {
        static assert(false);
    }

    while (true)
    {
        immutable T u = (rng.front - rng.min) * factor;
        rng.popFront();

        import core.stdc.limits : CHAR_BIT;  // CHAR_BIT is always 8
        static if (isIntegral!R && T.mant_dig >= (CHAR_BIT * R.sizeof))
        {
            /* If RNG variates are integral and T has enough precision to hold
             * R without loss, we're guaranteed by the definition of factor
             * that precisely u < 1.
             */
            return u;
        }
        else
        {
            /* Otherwise we have to check whether u is beyond the assumed range
             * because of the loss of precision, or for another reason, a
             * floating-point RNG can return a variate that is exactly equal to
             * its maximum.
             */
            if (u < 1)
            {
                return u;
            }
        }
    }

    // Shouldn't ever get here.
    assert(false);
}

@safe unittest
{
    import std.meta;
    foreach (UniformRNG; PseudoRngTypes)
    {

        foreach (T; std.meta.AliasSeq!(float, double, real))
        (){ // avoid slow optimizations for large functions @@@BUG@@@ 2396
            UniformRNG rng = UniformRNG(unpredictableSeed);

            auto a = uniform01();
            assert(is(typeof(a) == double));
            assert(0 <= a && a < 1);

            auto b = uniform01(rng);
            assert(is(typeof(a) == double));
            assert(0 <= b && b < 1);

            auto c = uniform01!T();
            assert(is(typeof(c) == T));
            assert(0 <= c && c < 1);

            auto d = uniform01!T(rng);
            assert(is(typeof(d) == T));
            assert(0 <= d && d < 1);

            T init = uniform01!T(rng);
            size_t i = 50;
            while (--i && uniform01!T(rng) == init) {}
            assert(i > 0);
            assert(i < 50);
        }();
    }
}

/**
Generates a uniform probability distribution of size $(D n), i.e., an
array of size $(D n) of positive numbers of type $(D F) that sum to
$(D 1). If $(D useThis) is provided, it is used as storage.
 */
F[] uniformDistribution(F = double)(size_t n, F[] useThis = null)
if (isFloatingPoint!F)
{
    import std.numeric : normalize;
    useThis.length = n;
    foreach (ref e; useThis)
    {
        e = uniform(0.0, 1);
    }
    normalize(useThis);
    return useThis;
}

@safe unittest
{
    import std.algorithm;
    import std.math;
    static assert(is(CommonType!(double, int) == double));
    auto a = uniformDistribution(5);
    assert(a.length == 5);
    assert(approxEqual(reduce!"a + b"(a), 1));
    a = uniformDistribution(10, a);
    assert(a.length == 10);
    assert(approxEqual(reduce!"a + b"(a), 1));
}

/**
Returns a random, uniformly chosen, element `e` from the supplied
$(D Range range). If no random number generator is passed, the default
`rndGen` is used.

Params:
    range = a random access range that has the `length` property defined
    urng = (optional) random number generator to use;
           if not specified, defaults to `rndGen`

Returns:
    A single random element drawn from the `range`. If it can, it will
    return a `ref` to the $(D range element), otherwise it will return
    a copy.
 */
auto ref choice(Range, RandomGen = Random)(auto ref Range range,
                                           ref RandomGen urng = rndGen)
if (isRandomAccessRange!Range && hasLength!Range && isUniformRNG!RandomGen)
{
    assert(range.length > 0,
           __PRETTY_FUNCTION__ ~ ": invalid Range supplied. Range cannot be empty");

    return range[uniform(size_t(0), $, urng)];
}

///
@safe unittest
{
    import std.algorithm.searching : canFind;

    auto array = [1, 2, 3, 4, 5];
    auto elem = choice(array);

    assert(canFind(array, elem),
           "Choice did not return a valid element from the given Range");

    auto urng = Random(unpredictableSeed);
    elem = choice(array, urng);

    assert(canFind(array, elem),
           "Choice did not return a valid element from the given Range");
}

@safe unittest
{
    import std.algorithm.searching : canFind;

    class MyTestClass
    {
        int x;

        this(int x)
        {
            this.x = x;
        }
    }

    MyTestClass[] testClass;
    foreach (i; 0 .. 5)
    {
        testClass ~= new MyTestClass(i);
    }

    auto elem = choice(testClass);

    assert(canFind!((ref MyTestClass a, ref MyTestClass b) => a.x == b.x)(testClass, elem),
           "Choice did not return a valid element from the given Range");
}

@system unittest
{
    import std.algorithm.iteration : map;
    import std.algorithm.searching : canFind;

    auto array = [1, 2, 3, 4, 5];
    auto elemAddr = &choice(array);

    assert(array.map!((ref e) => &e).canFind(elemAddr),
           "Choice did not return a ref to an element from the given Range");
    assert(array.canFind(*(cast(int *)(elemAddr))),
           "Choice did not return a valid element from the given Range");
}

/**
Shuffles elements of $(D r) using $(D gen) as a shuffler. $(D r) must be
a random-access range with length.  If no RNG is specified, $(D rndGen)
will be used.

Params:
    r = random-access range whose elements are to be shuffled
    gen = (optional) random number generator to use; if not
          specified, defaults to $(D rndGen)
 */

void randomShuffle(Range, RandomGen)(Range r, ref RandomGen gen)
if (isRandomAccessRange!Range && isUniformRNG!RandomGen)
{
    return partialShuffle!(Range, RandomGen)(r, r.length, gen);
}

/// ditto
void randomShuffle(Range)(Range r)
if (isRandomAccessRange!Range)
{
    return randomShuffle(r, rndGen);
}

@safe unittest
{
    import std.algorithm.sorting : sort;
    foreach (RandomGen; PseudoRngTypes)
    {
        // Also tests partialShuffle indirectly.
        auto a = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9];
        auto b = a.dup;
        auto gen = RandomGen(unpredictableSeed);
        randomShuffle(a, gen);
        sort(a);
        assert(a == b);
        randomShuffle(a);
        sort(a);
        assert(a == b);
    }
}

/**
Partially shuffles the elements of $(D r) such that upon returning $(D r[0 .. n])
is a random subset of $(D r) and is randomly ordered.  $(D r[n .. r.length])
will contain the elements not in $(D r[0 .. n]).  These will be in an undefined
order, but will not be random in the sense that their order after
$(D partialShuffle) returns will not be independent of their order before
$(D partialShuffle) was called.

$(D r) must be a random-access range with length.  $(D n) must be less than
or equal to $(D r.length).  If no RNG is specified, $(D rndGen) will be used.

Params:
    r = random-access range whose elements are to be shuffled
    n = number of elements of $(D r) to shuffle (counting from the beginning);
        must be less than $(D r.length)
    gen = (optional) random number generator to use; if not
          specified, defaults to $(D rndGen)
*/
void partialShuffle(Range, RandomGen)(Range r, in size_t n, ref RandomGen gen)
if (isRandomAccessRange!Range && isUniformRNG!RandomGen)
{
    import std.algorithm.mutation : swapAt;
    import std.exception : enforce;
    enforce(n <= r.length, "n must be <= r.length for partialShuffle.");
    foreach (i; 0 .. n)
    {
        r.swapAt(i, uniform(i, r.length, gen));
    }
}

/// ditto
void partialShuffle(Range)(Range r, in size_t n)
if (isRandomAccessRange!Range)
{
    return partialShuffle(r, n, rndGen);
}

@safe unittest
{
    import std.algorithm;
    foreach (RandomGen; PseudoRngTypes)
    {
        auto a = [0, 1, 1, 2, 3];
        auto b = a.dup;

        // Pick a fixed seed so that the outcome of the statistical
        // test below is deterministic.
        auto gen = RandomGen(12345);

        // NUM times, pick LEN elements from the array at random.
        immutable int LEN = 2;
        immutable int NUM = 750;
        int[][] chk;
        foreach (step; 0 .. NUM)
        {
            partialShuffle(a, LEN, gen);
            chk ~= a[0 .. LEN].dup;
        }

        // Check that each possible a[0 .. LEN] was produced at least once.
        // For a perfectly random RandomGen, the probability that each
        // particular combination failed to appear would be at most
        // 0.95 ^^ NUM which is approximately 1,962e-17.
        // As long as hardware failure (e.g. bit flip) probability
        // is higher, we are fine with this unittest.
        sort(chk);
        assert(equal(uniq(chk), [       [0,1], [0,2], [0,3],
                                 [1,0], [1,1], [1,2], [1,3],
                                 [2,0], [2,1],        [2,3],
                                 [3,0], [3,1], [3,2],      ]));

        // Check that all the elements are still there.
        sort(a);
        assert(equal(a, b));
    }
}

/**
Rolls a dice with relative probabilities stored in $(D
proportions). Returns the index in $(D proportions) that was chosen.

Params:
    rnd = (optional) random number generator to use; if not
          specified, defaults to $(D rndGen)
    proportions = forward range or list of individual values
                  whose elements correspond to the probabilities
                  with which to choose the corresponding index
                  value

Returns:
    Random variate drawn from the index values
    [0, ... $(D proportions.length) - 1], with the probability
    of getting an individual index value $(D i) being proportional to
    $(D proportions[i]).
*/
size_t dice(Rng, Num)(ref Rng rnd, Num[] proportions...)
if (isNumeric!Num && isForwardRange!Rng)
{
    return diceImpl(rnd, proportions);
}

/// Ditto
size_t dice(R, Range)(ref R rnd, Range proportions)
if (isForwardRange!Range && isNumeric!(ElementType!Range) && !isArray!Range)
{
    return diceImpl(rnd, proportions);
}

/// Ditto
size_t dice(Range)(Range proportions)
if (isForwardRange!Range && isNumeric!(ElementType!Range) && !isArray!Range)
{
    return diceImpl(rndGen, proportions);
}

/// Ditto
size_t dice(Num)(Num[] proportions...)
if (isNumeric!Num)
{
    return diceImpl(rndGen, proportions);
}

///
@safe unittest
{
    auto x = dice(0.5, 0.5);   // x is 0 or 1 in equal proportions
    auto y = dice(50, 50);     // y is 0 or 1 in equal proportions
    auto z = dice(70, 20, 10); // z is 0 70% of the time, 1 20% of the time,
                               // and 2 10% of the time
}

private size_t diceImpl(Rng, Range)(ref Rng rng, scope Range proportions)
if (isForwardRange!Range && isNumeric!(ElementType!Range) && isForwardRange!Rng)
in
{
    import std.algorithm.searching : all;
    assert(proportions.save.all!"a >= 0");
}
body
{
    import std.algorithm.iteration : reduce;
    import std.exception : enforce;
    double sum = reduce!"a + b"(0.0, proportions.save);
    enforce(sum > 0, "Proportions in a dice cannot sum to zero");
    immutable point = uniform(0.0, sum, rng);
    assert(point < sum);
    auto mass = 0.0;

    size_t i = 0;
    foreach (e; proportions)
    {
        mass += e;
        if (point < mass) return i;
        i++;
    }
    // this point should not be reached
    assert(false);
}

@safe unittest
{
    auto rnd = Random(unpredictableSeed);
    auto i = dice(rnd, 0.0, 100.0);
    assert(i == 1);
    i = dice(rnd, 100.0, 0.0);
    assert(i == 0);

    i = dice(100U, 0U);
    assert(i == 0);
}

/**
Covers a given range $(D r) in a random manner, i.e. goes through each
element of $(D r) once and only once, just in a random order. $(D r)
must be a random-access range with length.

If no random number generator is passed to $(D randomCover), the
thread-global RNG rndGen will be used internally.

Params:
    r = random-access range to cover
    rng = (optional) random number generator to use;
          if not specified, defaults to $(D rndGen)

Returns:
    Range whose elements consist of the elements of $(D r),
    in random order.  Will be a forward range if both $(D r) and
    $(D rng) are forward ranges, an input range otherwise.

Example:
----
int[] a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8 ];
foreach (e; randomCover(a))
{
    writeln(e);
}
----

$(B WARNING:) If an alternative RNG is desired, it is essential for this
to be a $(I new) RNG seeded in an unpredictable manner. Passing it a RNG
used elsewhere in the program will result in unintended correlations,
due to the current implementation of RNGs as value types.

Example:
----
int[] a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8 ];
foreach (e; randomCover(a, Random(unpredictableSeed)))  // correct!
{
    writeln(e);
}

foreach (e; randomCover(a, rndGen))  // DANGEROUS!! rndGen gets copied by value
{
    writeln(e);
}

foreach (e; randomCover(a, rndGen))  // ... so this second random cover
{                                    // will output the same sequence as
    writeln(e);                      // the previous one.
}
----
 */
struct RandomCover(Range, UniformRNG = void)
if (isRandomAccessRange!Range && (isUniformRNG!UniformRNG || is(UniformRNG == void)))
{
    private Range _input;
    private bool[] _chosen;
    private size_t _current;
    private size_t _alreadyChosen = 0;
    private bool _isEmpty = false;

    static if (is(UniformRNG == void))
    {
        this(Range input)
        {
            _input = input;
            _chosen.length = _input.length;
            if (_input.empty)
            {
                _isEmpty = true;
            }
            else
            {
                _current = uniform(0, _chosen.length);
            }
        }
    }
    else
    {
        private UniformRNG _rng;

        this(Range input, ref UniformRNG rng)
        {
            _input = input;
            _rng = rng;
            _chosen.length = _input.length;
            if (_input.empty)
            {
                _isEmpty = true;
            }
            else
            {
                _current = uniform(0, _chosen.length, rng);
            }
        }

        this(Range input, UniformRNG rng)
        {
            this(input, rng);
        }
    }

    static if (hasLength!Range)
    {
        @property size_t length()
        {
            return _input.length - _alreadyChosen;
        }
    }

    @property auto ref front()
    {
        assert(!_isEmpty);
        return _input[_current];
    }

    void popFront()
    {
        assert(!_isEmpty);

        size_t k = _input.length - _alreadyChosen - 1;
        if (k == 0)
        {
            _isEmpty = true;
            ++_alreadyChosen;
            return;
        }

        size_t i;
        foreach (e; _input)
        {
            if (_chosen[i] || i == _current) { ++i; continue; }
            // Roll a dice with k faces
            static if (is(UniformRNG == void))
            {
                auto chooseMe = uniform(0, k) == 0;
            }
            else
            {
                auto chooseMe = uniform(0, k, _rng) == 0;
            }
            assert(k > 1 || chooseMe);
            if (chooseMe)
            {
                _chosen[_current] = true;
                _current = i;
                ++_alreadyChosen;
                return;
            }
            --k;
            ++i;
        }
    }

    static if (isForwardRange!UniformRNG)
    {
        @property typeof(this) save()
        {
            auto ret = this;
            ret._input = _input.save;
            ret._rng = _rng.save;
            return ret;
        }
    }

    @property bool empty() { return _isEmpty; }
}

/// Ditto
auto randomCover(Range, UniformRNG)(Range r, auto ref UniformRNG rng)
if (isRandomAccessRange!Range && isUniformRNG!UniformRNG)
{
    return RandomCover!(Range, UniformRNG)(r, rng);
}

/// Ditto
auto randomCover(Range)(Range r)
if (isRandomAccessRange!Range)
{
    return RandomCover!(Range, void)(r);
}

@safe unittest
{
    import std.algorithm;
    import std.conv;
    int[] a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8 ];
    int[] c;
    foreach (UniformRNG; std.meta.AliasSeq!(void, PseudoRngTypes))
    {
        static if (is(UniformRNG == void))
        {
            auto rc = randomCover(a);
            static assert(isInputRange!(typeof(rc)));
            static assert(!isForwardRange!(typeof(rc)));
        }
        else
        {
            auto rng = UniformRNG(unpredictableSeed);
            auto rc = randomCover(a, rng);
            static assert(isForwardRange!(typeof(rc)));
            // check for constructor passed a value-type RNG
            auto rc2 = RandomCover!(int[], UniformRNG)(a, UniformRNG(unpredictableSeed));
            static assert(isForwardRange!(typeof(rc2)));
            auto rcEmpty = randomCover(c, rng);
            assert(rcEmpty.length == 0);
        }

        int[] b = new int[9];
        uint i;
        foreach (e; rc)
        {
            //writeln(e);
            b[i++] = e;
        }
        sort(b);
        assert(a == b, text(b));
    }
}

@safe unittest
{
    // Bugzilla 12589
    int[] r = [];
    auto rc = randomCover(r);
    assert(rc.length == 0);
    assert(rc.empty);

    // Bugzilla 16724
    import std.range : iota;
    auto range = iota(10);
    auto randy = range.randomCover;

    for (int i=1; i <= range.length; i++)
    {
        randy.popFront;
        assert(randy.length == range.length - i);
    }
}

// RandomSample
/**
Selects a random subsample out of $(D r), containing exactly $(D n)
elements. The order of elements is the same as in the original
range. The total length of $(D r) must be known. If $(D total) is
passed in, the total number of sample is considered to be $(D
total). Otherwise, $(D RandomSample) uses $(D r.length).

Params:
    r = range to sample from
    n = number of elements to include in the sample;
        must be less than or equal to the total number
        of elements in $(D r) and/or the parameter
        $(D total) (if provided)
    total = (semi-optional) number of elements of $(D r)
            from which to select the sample (counting from
            the beginning); must be less than or equal to
            the total number of elements in $(D r) itself.
            May be omitted if $(D r) has the $(D .length)
            property and the sample is to be drawn from
            all elements of $(D r).
    rng = (optional) random number generator to use;
          if not specified, defaults to $(D rndGen)

Returns:
    Range whose elements consist of a randomly selected subset of
    the elements of $(D r), in the same order as these elements
    appear in $(D r) itself.  Will be a forward range if both $(D r)
    and $(D rng) are forward ranges, an input range otherwise.

$(D RandomSample) implements Jeffrey Scott Vitter's Algorithm D
(see Vitter $(HTTP dx.doi.org/10.1145/358105.893, 1984), $(HTTP
dx.doi.org/10.1145/23002.23003, 1987)), which selects a sample
of size $(D n) in O(n) steps and requiring O(n) random variates,
regardless of the size of the data being sampled.  The exception
to this is if traversing k elements on the input range is itself
an O(k) operation (e.g. when sampling lines from an input file),
in which case the sampling calculation will inevitably be of
O(total).

RandomSample will throw an exception if $(D total) is verifiably
less than the total number of elements available in the input,
or if $(D n > total).

If no random number generator is passed to $(D randomSample), the
thread-global RNG rndGen will be used internally.

Example:
----
int[] a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
// Print 5 random elements picked off from a
foreach (e; randomSample(a, 5))
{
    writeln(e);
}
----

$(B WARNING:) If an alternative RNG is desired, it is essential for this
to be a $(I new) RNG seeded in an unpredictable manner. Passing it a RNG
used elsewhere in the program will result in unintended correlations,
due to the current implementation of RNGs as value types.

Example:
----
int[] a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ];
foreach (e; randomSample(a, 5, Random(unpredictableSeed)))  // correct!
{
    writeln(e);
}

foreach (e; randomSample(a, 5, rndGen))  // DANGEROUS!! rndGen gets
{                                        // copied by value
    writeln(e);
}

foreach (e; randomSample(a, 5, rndGen))  // ... so this second random
{                                        // sample will select the same
    writeln(e);                          // values as the previous one.
}
----
*/
struct RandomSample(Range, UniformRNG = void)
if (isInputRange!Range && (isUniformRNG!UniformRNG || is(UniformRNG == void)))
{
    private size_t _available, _toSelect;
    private enum ushort _alphaInverse = 13; // Vitter's recommended value.
    private double _Vprime;
    private Range _input;
    private size_t _index;
    private enum Skip { None, A, D }
    private Skip _skip = Skip.None;

    // If we're using the default thread-local random number generator then
    // we shouldn't store a copy of it here.  UniformRNG == void is a sentinel
    // for this.  If we're using a user-specified generator then we have no
    // choice but to store a copy.
    static if (is(UniformRNG == void))
    {
        static if (hasLength!Range)
        {
            this(Range input, size_t howMany)
            {
                _input = input;
                initialize(howMany, input.length);
            }
        }

        this(Range input, size_t howMany, size_t total)
        {
            _input = input;
            initialize(howMany, total);
        }
    }
    else
    {
        UniformRNG _rng;

        static if (hasLength!Range)
        {
            this(Range input, size_t howMany, ref UniformRNG rng)
            {
                _rng = rng;
                _input = input;
                initialize(howMany, input.length);
            }

            this(Range input, size_t howMany, UniformRNG rng)
            {
                this(input, howMany, rng);
            }
        }

        this(Range input, size_t howMany, size_t total, ref UniformRNG rng)
        {
            _rng = rng;
            _input = input;
            initialize(howMany, total);
        }

        this(Range input, size_t howMany, size_t total, UniformRNG rng)
        {
            this(input, howMany, total, rng);
        }
    }

    private void initialize(size_t howMany, size_t total)
    {
        import std.conv : text;
        import std.exception : enforce;
        _available = total;
        _toSelect = howMany;
        enforce(_toSelect <= _available,
                text("RandomSample: cannot sample ", _toSelect,
                     " items when only ", _available, " are available"));
        static if (hasLength!Range)
        {
            enforce(_available <= _input.length,
                    text("RandomSample: specified ", _available,
                         " items as available when input contains only ",
                         _input.length));
        }
    }

    private void initializeFront()
    {
        assert(_skip == Skip.None);
        // We can save ourselves a random variate by checking right
        // at the beginning if we should use Algorithm A.
        if ((_alphaInverse * _toSelect) > _available)
        {
            _skip = Skip.A;
        }
        else
        {
            _skip = Skip.D;
            _Vprime = newVprime(_toSelect);
        }
        prime();
    }

/**
   Range primitives.
*/
    @property bool empty() const
    {
        return _toSelect == 0;
    }

/// Ditto
    @property auto ref front()
    {
        assert(!empty);
        // The first sample point must be determined here to avoid
        // having it always correspond to the first element of the
        // input.  The rest of the sample points are determined each
        // time we call popFront().
        if (_skip == Skip.None)
        {
            initializeFront();
        }
        return _input.front;
    }

/// Ditto
    void popFront()
    {
        // First we need to check if the sample has
        // been initialized in the first place.
        if (_skip == Skip.None)
        {
            initializeFront();
        }

        _input.popFront();
        --_available;
        --_toSelect;
        ++_index;
        prime();
    }

/// Ditto
    static if (isForwardRange!Range && isForwardRange!UniformRNG)
    {
        @property typeof(this) save()
        {
            auto ret = this;
            ret._input = _input.save;
            ret._rng = _rng.save;
            return ret;
        }
    }

/// Ditto
    @property size_t length()
    {
        return _toSelect;
    }

/**
Returns the index of the visited record.
 */
    @property size_t index()
    {
        if (_skip == Skip.None)
        {
            initializeFront();
        }
        return _index;
    }

    private size_t skip()
    {
        assert(_skip != Skip.None);

        // Step D1: if the number of points still to select is greater
        // than a certain proportion of the remaining data points, i.e.
        // if n >= alpha * N where alpha = 1/13, we carry out the
        // sampling with Algorithm A.
        if (_skip == Skip.A)
        {
            return skipA();
        }
        else if ((_alphaInverse * _toSelect) > _available)
        {
            // We shouldn't get here unless the current selected
            // algorithm is D.
            assert(_skip == Skip.D);
            _skip = Skip.A;
            return skipA();
        }
        else
        {
            assert(_skip == Skip.D);
            return skipD();
        }
    }

/*
Vitter's Algorithm A, used when the ratio of needed sample values
to remaining data values is sufficiently large.
*/
    private size_t skipA()
    {
        size_t s;
        double v, quot, top;

        if (_toSelect == 1)
        {
            static if (is(UniformRNG == void))
            {
                s = uniform(0, _available);
            }
            else
            {
                s = uniform(0, _available, _rng);
            }
        }
        else
        {
            v = 0;
            top = _available - _toSelect;
            quot = top / _available;

            static if (is(UniformRNG == void))
            {
                v = uniform!"()"(0.0, 1.0);
            }
            else
            {
                v = uniform!"()"(0.0, 1.0, _rng);
            }

            while (quot > v)
            {
                ++s;
                quot *= (top - s) / (_available - s);
            }
        }

        return s;
    }

/*
Randomly reset the value of _Vprime.
*/
    private double newVprime(size_t remaining)
    {
        static if (is(UniformRNG == void))
        {
            double r = uniform!"()"(0.0, 1.0);
        }
        else
        {
            double r = uniform!"()"(0.0, 1.0, _rng);
        }

        return r ^^ (1.0 / remaining);
    }

/*
Vitter's Algorithm D.  For an extensive description of the algorithm
and its rationale, see:

  * Vitter, J.S. (1984), "Faster methods for random sampling",
    Commun. ACM 27(7): 703--718

  * Vitter, J.S. (1987) "An efficient algorithm for sequential random
    sampling", ACM Trans. Math. Softw. 13(1): 58-67.

Variable names are chosen to match those in Vitter's paper.
*/
    private size_t skipD()
    {
        import std.math : isNaN, trunc;
        // Confirm that the check in Step D1 is valid and we
        // haven't been sent here by mistake
        assert((_alphaInverse * _toSelect) <= _available);

        // Now it's safe to use the standard Algorithm D mechanism.
        if (_toSelect > 1)
        {
            size_t s;
            size_t qu1 = 1 + _available - _toSelect;
            double x, y1;

            assert(!_Vprime.isNaN());

            while (true)
            {
                // Step D2: set values of x and u.
                while (1)
                {
                    x = _available * (1-_Vprime);
                    s = cast(size_t) trunc(x);
                    if (s < qu1)
                        break;
                    _Vprime = newVprime(_toSelect);
                }

                static if (is(UniformRNG == void))
                {
                    double u = uniform!"()"(0.0, 1.0);
                }
                else
                {
                    double u = uniform!"()"(0.0, 1.0, _rng);
                }

                y1 = (u * (cast(double) _available) / qu1) ^^ (1.0/(_toSelect - 1));

                _Vprime = y1 * ((-x/_available)+1.0) * ( qu1/( (cast(double) qu1) - s ) );

                // Step D3: if _Vprime <= 1.0 our work is done and we return S.
                // Otherwise ...
                if (_Vprime > 1.0)
                {
                    size_t top = _available - 1, limit;
                    double y2 = 1.0, bottom;

                    if (_toSelect > (s+1))
                    {
                        bottom = _available - _toSelect;
                        limit = _available - s;
                    }
                    else
                    {
                        bottom = _available - (s+1);
                        limit = qu1;
                    }

                    foreach (size_t t; limit .. _available)
                    {
                        y2 *= top/bottom;
                        top--;
                        bottom--;
                    }

                    // Step D4: decide whether or not to accept the current value of S.
                    if (_available/(_available-x) < y1 * (y2 ^^ (1.0/(_toSelect-1))))
                    {
                        // If it's not acceptable, we generate a new value of _Vprime
                        // and go back to the start of the for (;;) loop.
                        _Vprime = newVprime(_toSelect);
                    }
                    else
                    {
                        // If it's acceptable we generate a new value of _Vprime
                        // based on the remaining number of sample points needed,
                        // and return S.
                        _Vprime = newVprime(_toSelect-1);
                        return s;
                    }
                }
                else
                {
                    // Return if condition D3 satisfied.
                    return s;
                }
            }
        }
        else
        {
            // If only one sample point remains to be taken ...
            return cast(size_t) trunc(_available * _Vprime);
        }
    }

    private void prime()
    {
        if (empty)
        {
            return;
        }
        assert(_available && _available >= _toSelect);
        immutable size_t s = skip();
        assert(s + _toSelect <= _available);
        static if (hasLength!Range)
        {
            assert(s + _toSelect <= _input.length);
        }
        assert(!_input.empty);
        _input.popFrontExactly(s);
        _index += s;
        _available -= s;
        assert(_available > 0);
    }
}

/// Ditto
auto randomSample(Range)(Range r, size_t n, size_t total)
if (isInputRange!Range)
{
    return RandomSample!(Range, void)(r, n, total);
}

/// Ditto
auto randomSample(Range)(Range r, size_t n)
if (isInputRange!Range && hasLength!Range)
{
    return RandomSample!(Range, void)(r, n, r.length);
}

/// Ditto
auto randomSample(Range, UniformRNG)(Range r, size_t n, size_t total, auto ref UniformRNG rng)
if (isInputRange!Range && isUniformRNG!UniformRNG)
{
    return RandomSample!(Range, UniformRNG)(r, n, total, rng);
}

/// Ditto
auto randomSample(Range, UniformRNG)(Range r, size_t n, auto ref UniformRNG rng)
if (isInputRange!Range && hasLength!Range && isUniformRNG!UniformRNG)
{
    return RandomSample!(Range, UniformRNG)(r, n, r.length, rng);
}

@system unittest
{
    // @system because it takes the address of a local
    import std.conv : text;
    import std.exception;
    import std.range;
    // For test purposes, an infinite input range
    struct TestInputRange
    {
        private auto r = recurrence!"a[n-1] + 1"(0);
        bool empty() @property const pure nothrow { return r.empty; }
        auto front() @property pure nothrow { return r.front; }
        void popFront() pure nothrow { r.popFront(); }
    }
    static assert(isInputRange!TestInputRange);
    static assert(!isForwardRange!TestInputRange);

    int[] a = [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 ];

    foreach (UniformRNG; PseudoRngTypes)
    {
        auto rng = UniformRNG(1234);
        /* First test the most general case: randomSample of input range, with and
         * without a specified random number generator.
         */
        static assert(isInputRange!(typeof(randomSample(TestInputRange(), 5, 10))));
        static assert(isInputRange!(typeof(randomSample(TestInputRange(), 5, 10, rng))));
        static assert(!isForwardRange!(typeof(randomSample(TestInputRange(), 5, 10))));
        static assert(!isForwardRange!(typeof(randomSample(TestInputRange(), 5, 10, rng))));
        // test case with range initialized by direct call to struct
        {
            auto sample =
                RandomSample!(TestInputRange, UniformRNG)
                             (TestInputRange(), 5, 10, UniformRNG(unpredictableSeed));
            static assert(isInputRange!(typeof(sample)));
            static assert(!isForwardRange!(typeof(sample)));
        }

        /* Now test the case of an input range with length.  We ignore the cases
         * already covered by the previous tests.
         */
        static assert(isInputRange!(typeof(randomSample(TestInputRange().takeExactly(10), 5))));
        static assert(isInputRange!(typeof(randomSample(TestInputRange().takeExactly(10), 5, rng))));
        static assert(!isForwardRange!(typeof(randomSample(TestInputRange().takeExactly(10), 5))));
        static assert(!isForwardRange!(typeof(randomSample(TestInputRange().takeExactly(10), 5, rng))));
        // test case with range initialized by direct call to struct
        {
            auto sample =
                RandomSample!(typeof(TestInputRange().takeExactly(10)), UniformRNG)
                             (TestInputRange().takeExactly(10), 5, 10, UniformRNG(unpredictableSeed));
            static assert(isInputRange!(typeof(sample)));
            static assert(!isForwardRange!(typeof(sample)));
        }

        // Now test the case of providing a forward range as input.
        static assert(!isForwardRange!(typeof(randomSample(a, 5))));
        static if (isForwardRange!UniformRNG)
        {
            static assert(isForwardRange!(typeof(randomSample(a, 5, rng))));
            // ... and test with range initialized directly
            {
                auto sample =
                    RandomSample!(int[], UniformRNG)
                                 (a, 5, UniformRNG(unpredictableSeed));
                static assert(isForwardRange!(typeof(sample)));
            }
        }
        else
        {
            static assert(isInputRange!(typeof(randomSample(a, 5, rng))));
            static assert(!isForwardRange!(typeof(randomSample(a, 5, rng))));
            // ... and test with range initialized directly
            {
                auto sample =
                    RandomSample!(int[], UniformRNG)
                                 (a, 5, UniformRNG(unpredictableSeed));
                static assert(isInputRange!(typeof(sample)));
                static assert(!isForwardRange!(typeof(sample)));
            }
        }

        /* Check that randomSample will throw an error if we claim more
         * items are available than there actually are, or if we try to
         * sample more items than are available. */
        assert(collectExceptionMsg(
            randomSample(a, 5, 15)
        ) == "RandomSample: specified 15 items as available when input contains only 10");
        assert(collectExceptionMsg(
            randomSample(a, 15)
        ) == "RandomSample: cannot sample 15 items when only 10 are available");
        assert(collectExceptionMsg(
            randomSample(a, 9, 8)
        ) == "RandomSample: cannot sample 9 items when only 8 are available");
        assert(collectExceptionMsg(
            randomSample(TestInputRange(), 12, 11)
        ) == "RandomSample: cannot sample 12 items when only 11 are available");

        /* Check that sampling algorithm never accidentally overruns the end of
         * the input range.  If input is an InputRange without .length, this
         * relies on the user specifying the total number of available items
         * correctly.
         */
        {
            uint i = 0;
            foreach (e; randomSample(a, a.length))
            {
                assert(e == i);
                ++i;
            }
            assert(i == a.length);

            i = 0;
            foreach (e; randomSample(TestInputRange(), 17, 17))
            {
                assert(e == i);
                ++i;
            }
            assert(i == 17);
        }


        // Check length properties of random samples.
        assert(randomSample(a, 5).length == 5);
        assert(randomSample(a, 5, 10).length == 5);
        assert(randomSample(a, 5, rng).length == 5);
        assert(randomSample(a, 5, 10, rng).length == 5);
        assert(randomSample(TestInputRange(), 5, 10).length == 5);
        assert(randomSample(TestInputRange(), 5, 10, rng).length == 5);

        // ... and emptiness!
        assert(randomSample(a, 0).empty);
        assert(randomSample(a, 0, 5).empty);
        assert(randomSample(a, 0, rng).empty);
        assert(randomSample(a, 0, 5, rng).empty);
        assert(randomSample(TestInputRange(), 0, 10).empty);
        assert(randomSample(TestInputRange(), 0, 10, rng).empty);

        /* Test that the (lazy) evaluation of random samples works correctly.
         *
         * We cover 2 different cases: a sample where the ratio of sample points
         * to total points is greater than the threshold for using Algorithm, and
         * one where the ratio is small enough (< 1/13) for Algorithm D to be used.
         *
         * For each, we also cover the case with and without a specified RNG.
         */
        {
            // Small sample/source ratio, no specified RNG.
            uint i = 0;
            foreach (e; randomSample(randomCover(a), 5))
            {
                ++i;
            }
            assert(i == 5);

            // Small sample/source ratio, specified RNG.
            i = 0;
            foreach (e; randomSample(randomCover(a), 5, rng))
            {
                ++i;
            }
            assert(i == 5);

            // Large sample/source ratio, no specified RNG.
            i = 0;
            foreach (e; randomSample(TestInputRange(), 123, 123_456))
            {
                ++i;
            }
            assert(i == 123);

            // Large sample/source ratio, specified RNG.
            i = 0;
            foreach (e; randomSample(TestInputRange(), 123, 123_456, rng))
            {
                ++i;
            }
            assert(i == 123);

            /* Sample/source ratio large enough to start with Algorithm D,
             * small enough to switch to Algorithm A.
             */
            i = 0;
            foreach (e; randomSample(TestInputRange(), 10, 131))
            {
                ++i;
            }
            assert(i == 10);
        }

        // Test that the .index property works correctly
        {
            auto sample1 = randomSample(TestInputRange(), 654, 654_321);
            for (; !sample1.empty; sample1.popFront())
            {
                assert(sample1.front == sample1.index);
            }

            auto sample2 = randomSample(TestInputRange(), 654, 654_321, rng);
            for (; !sample2.empty; sample2.popFront())
            {
                assert(sample2.front == sample2.index);
            }

            /* Check that it also works if .index is called before .front.
             * See: http://d.puremagic.com/issues/show_bug.cgi?id=10322
             */
            auto sample3 = randomSample(TestInputRange(), 654, 654_321);
            for (; !sample3.empty; sample3.popFront())
            {
                assert(sample3.index == sample3.front);
            }

            auto sample4 = randomSample(TestInputRange(), 654, 654_321, rng);
            for (; !sample4.empty; sample4.popFront())
            {
                assert(sample4.index == sample4.front);
            }
        }

        /* Test behaviour if .popFront() is called before sample is read.
         * This is a rough-and-ready check that the statistical properties
         * are in the ballpark -- not a proper validation of statistical
         * quality!  This incidentally also checks for reference-type
         * initialization bugs, as the foreach () loop will operate on a
         * copy of the popFronted (and hence initialized) sample.
         */
        {
            size_t count0, count1, count99;
            foreach (_; 0 .. 100_000)
            {
                auto sample = randomSample(iota(100), 5, &rng);
                sample.popFront();
                foreach (s; sample)
                {
                    if (s == 0)
                    {
                        ++count0;
                    }
                    else if (s == 1)
                    {
                        ++count1;
                    }
                    else if (s == 99)
                    {
                        ++count99;
                    }
                }
            }
            /* Statistical assumptions here: this is a sequential sampling process
             * so (i) 0 can only be the first sample point, so _can't_ be in the
             * remainder of the sample after .popFront() is called. (ii) By similar
             * token, 1 can only be in the remainder if it's the 2nd point of the
             * whole sample, and hence if 0 was the first; probability of 0 being
             * first and 1 second is 5/100 * 4/99 (thank you, Algorithm S:-) and
             * so the mean count of 1 should be about 202.  Finally, 99 can only
             * be the _last_ sample point to be picked, so its probability of
             * inclusion should be independent of the .popFront() and it should
             * occur with frequency 5/100, hence its count should be about 5000.
             * Unfortunately we have to set quite a high tolerance because with
             * sample size small enough for unittests to run in reasonable time,
             * the variance can be quite high.
             */
            assert(count0 == 0);
            assert(count1 < 300, text("1: ", count1, " > 300."));
            assert(4_700 < count99, text("99: ", count99, " < 4700."));
            assert(count99 < 5_300, text("99: ", count99, " > 5300."));
        }

        /* Odd corner-cases: RandomSample has 2 constructors that are not called
         * by the randomSample() helper functions, but that can be used if the
         * constructor is called directly.  These cover the case of the user
         * specifying input but not input length.
         */
        {
            auto input1 = TestInputRange().takeExactly(456_789);
            static assert(hasLength!(typeof(input1)));
            auto sample1 = RandomSample!(typeof(input1), void)(input1, 789);
            static assert(isInputRange!(typeof(sample1)));
            static assert(!isForwardRange!(typeof(sample1)));
            assert(sample1.length == 789);
            assert(sample1._available == 456_789);
            uint i = 0;
            for (; !sample1.empty; sample1.popFront())
            {
                assert(sample1.front == sample1.index);
                ++i;
            }
            assert(i == 789);

            auto input2 = TestInputRange().takeExactly(456_789);
            static assert(hasLength!(typeof(input2)));
            auto sample2 = RandomSample!(typeof(input2), typeof(rng))(input2, 789, rng);
            static assert(isInputRange!(typeof(sample2)));
            static assert(!isForwardRange!(typeof(sample2)));
            assert(sample2.length == 789);
            assert(sample2._available == 456_789);
            i = 0;
            for (; !sample2.empty; sample2.popFront())
            {
                assert(sample2.front == sample2.index);
                ++i;
            }
            assert(i == 789);
        }

        /* Test that the save property works where input is a forward range,
         * and RandomSample is using a (forward range) random number generator
         * that is not rndGen.
         */
        static if (isForwardRange!UniformRNG)
        {
            auto sample1 = randomSample(a, 5, rng);
            auto sample2 = sample1.save;
            assert(sample1.array() == sample2.array());
        }

        // Bugzilla 8314
        {
            auto sample(RandomGen)(uint seed) { return randomSample(a, 1, RandomGen(seed)).front; }

            // Start from 1 because not all RNGs accept 0 as seed.
            immutable fst = sample!UniformRNG(1);
            uint n = 1;
            while (sample!UniformRNG(++n) == fst && n < n.max) {}
            assert(n < n.max);
        }
    }
}
