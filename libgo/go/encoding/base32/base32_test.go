// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package base32

import (
	"bytes"
	"io"
	"io/ioutil"
	"testing"
)

type testpair struct {
	decoded, encoded string
}

var pairs = []testpair{
	// RFC 4648 examples
	{"", ""},
	{"f", "MY======"},
	{"fo", "MZXQ===="},
	{"foo", "MZXW6==="},
	{"foob", "MZXW6YQ="},
	{"fooba", "MZXW6YTB"},
	{"foobar", "MZXW6YTBOI======"},

	// Wikipedia examples, converted to base32
	{"sure.", "ON2XEZJO"},
	{"sure", "ON2XEZI="},
	{"sur", "ON2XE==="},
	{"su", "ON2Q===="},
	{"leasure.", "NRSWC43VOJSS4==="},
	{"easure.", "MVQXG5LSMUXA===="},
	{"asure.", "MFZXK4TFFY======"},
	{"sure.", "ON2XEZJO"},
}

var bigtest = testpair{
	"Twas brillig, and the slithy toves",
	"KR3WC4ZAMJZGS3DMNFTSYIDBNZSCA5DIMUQHG3DJORUHSIDUN53GK4Y=",
}

func testEqual(t *testing.T, msg string, args ...interface{}) bool {
	if args[len(args)-2] != args[len(args)-1] {
		t.Errorf(msg, args...)
		return false
	}
	return true
}

func TestEncode(t *testing.T) {
	for _, p := range pairs {
		got := StdEncoding.EncodeToString([]byte(p.decoded))
		testEqual(t, "Encode(%q) = %q, want %q", p.decoded, got, p.encoded)
	}
}

func TestEncoder(t *testing.T) {
	for _, p := range pairs {
		bb := &bytes.Buffer{}
		encoder := NewEncoder(StdEncoding, bb)
		encoder.Write([]byte(p.decoded))
		encoder.Close()
		testEqual(t, "Encode(%q) = %q, want %q", p.decoded, bb.String(), p.encoded)
	}
}

func TestEncoderBuffering(t *testing.T) {
	input := []byte(bigtest.decoded)
	for bs := 1; bs <= 12; bs++ {
		bb := &bytes.Buffer{}
		encoder := NewEncoder(StdEncoding, bb)
		for pos := 0; pos < len(input); pos += bs {
			end := pos + bs
			if end > len(input) {
				end = len(input)
			}
			n, err := encoder.Write(input[pos:end])
			testEqual(t, "Write(%q) gave error %v, want %v", input[pos:end], err, error(nil))
			testEqual(t, "Write(%q) gave length %v, want %v", input[pos:end], n, end-pos)
		}
		err := encoder.Close()
		testEqual(t, "Close gave error %v, want %v", err, error(nil))
		testEqual(t, "Encoding/%d of %q = %q, want %q", bs, bigtest.decoded, bb.String(), bigtest.encoded)
	}
}

func TestDecode(t *testing.T) {
	for _, p := range pairs {
		dbuf := make([]byte, StdEncoding.DecodedLen(len(p.encoded)))
		count, end, err := StdEncoding.decode(dbuf, []byte(p.encoded))
		testEqual(t, "Decode(%q) = error %v, want %v", p.encoded, err, error(nil))
		testEqual(t, "Decode(%q) = length %v, want %v", p.encoded, count, len(p.decoded))
		if len(p.encoded) > 0 {
			testEqual(t, "Decode(%q) = end %v, want %v", p.encoded, end, (p.encoded[len(p.encoded)-1] == '='))
		}
		testEqual(t, "Decode(%q) = %q, want %q", p.encoded,
			string(dbuf[0:count]),
			p.decoded)

		dbuf, err = StdEncoding.DecodeString(p.encoded)
		testEqual(t, "DecodeString(%q) = error %v, want %v", p.encoded, err, error(nil))
		testEqual(t, "DecodeString(%q) = %q, want %q", p.encoded, string(dbuf), p.decoded)
	}
}

func TestDecoder(t *testing.T) {
	for _, p := range pairs {
		decoder := NewDecoder(StdEncoding, bytes.NewBufferString(p.encoded))
		dbuf := make([]byte, StdEncoding.DecodedLen(len(p.encoded)))
		count, err := decoder.Read(dbuf)
		if err != nil && err != io.EOF {
			t.Fatal("Read failed", err)
		}
		testEqual(t, "Read from %q = length %v, want %v", p.encoded, count, len(p.decoded))
		testEqual(t, "Decoding of %q = %q, want %q", p.encoded, string(dbuf[0:count]), p.decoded)
		if err != io.EOF {
			count, err = decoder.Read(dbuf)
		}
		testEqual(t, "Read from %q = %v, want %v", p.encoded, err, io.EOF)
	}
}

func TestDecoderBuffering(t *testing.T) {
	for bs := 1; bs <= 12; bs++ {
		decoder := NewDecoder(StdEncoding, bytes.NewBufferString(bigtest.encoded))
		buf := make([]byte, len(bigtest.decoded)+12)
		var total int
		for total = 0; total < len(bigtest.decoded); {
			n, err := decoder.Read(buf[total : total+bs])
			testEqual(t, "Read from %q at pos %d = %d, %v, want _, %v", bigtest.encoded, total, n, err, error(nil))
			total += n
		}
		testEqual(t, "Decoding/%d of %q = %q, want %q", bs, bigtest.encoded, string(buf[0:total]), bigtest.decoded)
	}
}

func TestDecodeCorrupt(t *testing.T) {
	type corrupt struct {
		e string
		p int
	}
	examples := []corrupt{
		{"!!!!", 0},
		{"x===", 0},
		{"AA=A====", 2},
		{"AAA=AAAA", 3},
		{"MMMMMMMMM", 8},
		{"MMMMMM", 0},
	}

	for _, e := range examples {
		dbuf := make([]byte, StdEncoding.DecodedLen(len(e.e)))
		_, err := StdEncoding.Decode(dbuf, []byte(e.e))
		switch err := err.(type) {
		case CorruptInputError:
			testEqual(t, "Corruption in %q at offset %v, want %v", e.e, int(err), e.p)
		default:
			t.Error("Decoder failed to detect corruption in", e)
		}
	}
}

func TestBig(t *testing.T) {
	n := 3*1000 + 1
	raw := make([]byte, n)
	const alpha = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
	for i := 0; i < n; i++ {
		raw[i] = alpha[i%len(alpha)]
	}
	encoded := new(bytes.Buffer)
	w := NewEncoder(StdEncoding, encoded)
	nn, err := w.Write(raw)
	if nn != n || err != nil {
		t.Fatalf("Encoder.Write(raw) = %d, %v want %d, nil", nn, err, n)
	}
	err = w.Close()
	if err != nil {
		t.Fatalf("Encoder.Close() = %v want nil", err)
	}
	decoded, err := ioutil.ReadAll(NewDecoder(StdEncoding, encoded))
	if err != nil {
		t.Fatalf("ioutil.ReadAll(NewDecoder(...)): %v", err)
	}

	if !bytes.Equal(raw, decoded) {
		var i int
		for i = 0; i < len(decoded) && i < len(raw); i++ {
			if decoded[i] != raw[i] {
				break
			}
		}
		t.Errorf("Decode(Encode(%d-byte string)) failed at offset %d", n, i)
	}
}

func TestNewLineCharacters(t *testing.T) {
	// Each of these should decode to the string "sure", without errors.
	const expected = "sure"
	examples := []string{
		"ON2XEZI=",
		"ON2XEZI=\r",
		"ON2XEZI=\n",
		"ON2XEZI=\r\n",
		"ON2XEZ\r\nI=",
		"ON2X\rEZ\nI=",
		"ON2X\nEZ\rI=",
		"ON2XEZ\nI=",
		"ON2XEZI\n=",
	}
	for _, e := range examples {
		buf, err := StdEncoding.DecodeString(e)
		if err != nil {
			t.Errorf("Decode(%q) failed: %v", e, err)
			continue
		}
		if s := string(buf); s != expected {
			t.Errorf("Decode(%q) = %q, want %q", e, s, expected)
		}
	}
}
