// socket.go -- Socket handling.

// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// Low-level socket interface.
// Only for implementing net package.
// DO NOT USE DIRECTLY.

package syscall

import "unsafe"

const SizeofSockaddrInet4 = 16
const SizeofSockaddrInet6 = 28
const SizeofSockaddrUnix = 110

type RawSockaddrAny struct {
	Addr RawSockaddr;
	Pad [12]int8;
}

const SizeofSockaddrAny = 0x1c;

// For testing: clients can set this flag to force
// creation of IPv6 sockets to return EAFNOSUPPORT.
var SocketDisableIPv6 bool

type Sockaddr interface {
	sockaddr() (ptr *RawSockaddrAny, len Socklen_t, errno int);	// lowercase; only we can define Sockaddrs
}

type SockaddrInet4 struct {
	Port int;
	Addr [4]byte;
	raw RawSockaddrInet4;
}

type SockaddrInet6 struct {
	Port int;
	Addr [16]byte;
	raw RawSockaddrInet6;
}

type SockaddrUnix struct {
	Name string;
	raw RawSockaddrUnix;
}

type Linger struct {
	Onoff int32;
	Linger int32;
}

func libc_accept(fd int, sa *RawSockaddrAny, len *Socklen_t) int __asm__ ("accept");
func libc_bind(fd int, sa *RawSockaddrAny, len Socklen_t) int __asm__ ("bind");
func libc_connect(fd int, sa *RawSockaddrAny, len Socklen_t) int __asm__ ("connect");
func libc_socket(domain, typ, protocol int) int __asm__ ("socket");
func libc_setsockopt(fd, level, optname int, optval *byte, optlen Socklen_t) int __asm__ ("setsockopt");
func libc_listen(fd, backlog int) int __asm__ ("listen");
func libc_getsockopt(fd, level, optname int, optval *byte, optlen *Socklen_t) int __asm__ ("getsockopt");
func libc_getsockname(fd int, sa *RawSockaddrAny, len *Socklen_t) int __asm__ ("getsockname");
func libc_getpeername(fd int, sa *RawSockaddrAny, len *Socklen_t) int __asm__ ("getpeername");
func libc_recv(fd int, buf *byte, len Size_t, flags int) Ssize_t __asm__ ("recv");
func libc_recvfrom(fd int, buf *byte, len Size_t, flags int,
	from *RawSockaddrAny, fromlen *Socklen_t) Ssize_t __asm__("recvfrom");
func libc_send(fd int, buf *byte, len Size_t, flags int) Ssize_t __asm__("send");
func libc_sendto(fd int, buf *byte, len Size_t, flags int,
	to *RawSockaddrAny, tolen Socklen_t) Ssize_t __asm__("sendto");
func libc_shutdown(fd int, how int) int __asm__ ("shutdown");

func Accept(fd int) (nfd int, sa Sockaddr, errno int) {
	var rsa RawSockaddrAny;
	var len Socklen_t = SizeofSockaddrAny;
	nfd = libc_accept(fd, &rsa, &len);
	if nfd < 0 {
		errno = GetErrno();
		return;
	}
	sa, errno = anyToSockaddr(&rsa);
	if errno != 0 {
		Close(nfd);
		nfd = 0;
	}
	return;
}

func Bind(fd int, sa Sockaddr) (errno int) {
	ptr, n, err := sa.sockaddr();
	if err != 0 {
		return err;
	}
	if libc_bind(fd, ptr, n) < 0 {
		errno = GetErrno();
	}
	return;
}

func Connect(fd int, sa Sockaddr) (errno int) {
	ptr, n, err := sa.sockaddr();
	if err != 0 {
		return err;
	}
	if libc_connect(fd, ptr, n) < 0 {
		errno = GetErrno();
	}
	return;
}

func Socket(domain, typ, proto int) (fd, errno int) {
  if domain == AF_INET6 && SocketDisableIPv6 {
    return -1, EAFNOSUPPORT
  }
  fd = libc_socket(int(domain), int(typ), int(proto));
  if fd < 0 {
    errno = GetErrno();
  }
  return;
}

func Listen(fd int, n int) (errno int) {
  r := libc_listen(int(fd), int(n));
  if r < 0 { errno = GetErrno() }
  return;
}

func setsockopt(fd, level, opt int, valueptr uintptr, length Socklen_t) (errno int) {
  r := libc_setsockopt(fd, level, opt, (*byte)(unsafe.Pointer(valueptr)),
		       length);
  if r < 0 { errno = GetErrno() }
  return;
}

func SetsockoptInt(fd, level, opt int, value int) (errno int) {
	var n = int32(value);
	return setsockopt(fd, level, opt, uintptr(unsafe.Pointer(&n)), 4);
}

func SetsockoptTimeval(fd, level, opt int, tv *Timeval) (errno int) {
	return setsockopt(fd, level, opt, uintptr(unsafe.Pointer(tv)), Socklen_t(unsafe.Sizeof(*tv)));
}

func SetsockoptLinger(fd, level, opt int, l *Linger) (errno int) {
	return setsockopt(fd, level, opt, uintptr(unsafe.Pointer(l)), Socklen_t(unsafe.Sizeof(*l)));
}

func SetsockoptString(fd, level, opt int, s string) (errno int) {
	return setsockopt(fd, level, opt, uintptr(unsafe.Pointer(&[]byte(s)[0])), Socklen_t(len(s)))
}

// BindToDevice binds the socket associated with fd to device.
func BindToDevice(fd int, device string) (errno int) {
	return SetsockoptString(fd, SOL_SOCKET, SO_BINDTODEVICE, device)
}

func Getsockname(fd int) (sa Sockaddr, errno int) {
	var rsa RawSockaddrAny;
	var len Socklen_t = SizeofSockaddrAny;
	if libc_getsockname(fd, &rsa, &len) != 0 {
		errno = GetErrno();
		return;
	}
	return anyToSockaddr(&rsa);
}

func Getpeername(fd int) (sa Sockaddr, errno int) {
	var rsa RawSockaddrAny;
	var len Socklen_t = SizeofSockaddrAny;
	if libc_getpeername(fd, &rsa, &len) != 0 {
		errno = GetErrno();
		return;
	}
	return anyToSockaddr(&rsa);
}

func Recvfrom(fd int, p []byte, flags int) (n int, from Sockaddr, errno int) {
	var rsa RawSockaddrAny;
	var slen Socklen_t = SizeofSockaddrAny;
	var _p0 *byte;
	if len(p) > 0 { _p0 = &p[0]; }
	r := libc_recvfrom(fd, _p0, Size_t(len(p)), flags, &rsa, &slen);
	n = int(r);
	if r == -1 {
		errno = GetErrno();
	} else {
		from, errno = anyToSockaddr(&rsa);
	}
	return;
}

func Sendto(fd int, p []byte, flags int, to Sockaddr) (errno int) {
	ptr, n, err := to.sockaddr();
	if err != 0 {
		return err;
	}
	var _p0 *byte;
	if len(p) > 0 { _p0 = &p[0]; }
	r := libc_sendto(fd, _p0, Size_t(len(p)), flags, ptr, n);
	if r == -1 { errno = GetErrno(); }
	return;
}

func Shutdown(fd int, how int) (errno int) {
	r := libc_shutdown(fd, how);
	if r < 0 { errno = GetErrno() }
	return;
}

// FIXME: No getsockopt.
