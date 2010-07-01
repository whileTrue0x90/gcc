// Copyright 2009 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

package websocket

import (
	"bytes"
	"fmt"
	"http"
	"io"
	"log"
	"net"
	"once"
	"testing"
)

var serverAddr string

func echoServer(ws *Conn) { io.Copy(ws, ws) }

func startServer() {
	l, e := net.Listen("tcp", ":0") // any available address
	if e != nil {
		log.Exitf("net.Listen tcp :0 %v", e)
	}
	serverAddr = l.Addr().String()
	log.Stderr("Test WebSocket server listening on ", serverAddr)
	http.Handle("/echo", Handler(echoServer))
	http.Handle("/echoDraft75", Draft75Handler(echoServer))
	go http.Serve(l, nil)
}

func TestEcho(t *testing.T) {
	once.Do(startServer)

	// websocket.Dial()
	client, err := net.Dial("tcp", "", serverAddr)
	if err != nil {
		t.Fatal("dialing", err)
	}
	ws, err := newClient("/echo", "localhost", "http://localhost",
		"ws://localhost/echo", "", client, handshake)
	if err != nil {
		t.Errorf("WebSocket handshake error: %v", err)
		return
	}

	msg := []byte("hello, world\n")
	if _, err := ws.Write(msg); err != nil {
		t.Errorf("Write: %v", err)
	}
	var actual_msg = make([]byte, 512)
	n, err := ws.Read(actual_msg)
	if err != nil {
		t.Errorf("Read: %v", err)
	}
	actual_msg = actual_msg[0:n]
	if !bytes.Equal(msg, actual_msg) {
		t.Errorf("Echo: expected %q got %q", msg, actual_msg)
	}
	ws.Close()
}

func TestEchoDraft75(t *testing.T) {
	once.Do(startServer)

	// websocket.Dial()
	client, err := net.Dial("tcp", "", serverAddr)
	if err != nil {
		t.Fatal("dialing", err)
	}
	ws, err := newClient("/echoDraft75", "localhost", "http://localhost",
		"ws://localhost/echoDraft75", "", client, draft75handshake)
	if err != nil {
		t.Errorf("WebSocket handshake: %v", err)
		return
	}

	msg := []byte("hello, world\n")
	if _, err := ws.Write(msg); err != nil {
		t.Errorf("Write: error %v", err)
	}
	var actual_msg = make([]byte, 512)
	n, err := ws.Read(actual_msg)
	if err != nil {
		t.Errorf("Read: error %v", err)
	}
	actual_msg = actual_msg[0:n]
	if !bytes.Equal(msg, actual_msg) {
		t.Errorf("Echo: expected %q got %q", msg, actual_msg)
	}
	ws.Close()
}

func TestWithQuery(t *testing.T) {
	once.Do(startServer)

	client, err := net.Dial("tcp", "", serverAddr)
	if err != nil {
		t.Fatal("dialing", err)
	}

	ws, err := newClient("/echo?q=v", "localhost", "http://localhost",
		"ws://localhost/echo?q=v", "", client, handshake)
	if err != nil {
		t.Errorf("WebSocket handshake: %v", err)
		return
	}
	ws.Close()
}

func TestHTTP(t *testing.T) {
	once.Do(startServer)

	// If the client did not send a handshake that matches the protocol
	// specification, the server should abort the WebSocket connection.
	_, _, err := http.Get(fmt.Sprintf("http://%s/echo", serverAddr))
	if err == nil {
		t.Errorf("Get: unexpected success")
		return
	}
	urlerr, ok := err.(*http.URLError)
	if !ok {
		t.Errorf("Get: not URLError %#v", err)
		return
	}
	if urlerr.Error != io.ErrUnexpectedEOF {
		t.Errorf("Get: error %#v", err)
		return
	}
}

func TestHTTPDraft75(t *testing.T) {
	once.Do(startServer)

	r, _, err := http.Get(fmt.Sprintf("http://%s/echoDraft75", serverAddr))
	if err != nil {
		t.Errorf("Get: error %#v", err)
		return
	}
	if r.StatusCode != http.StatusBadRequest {
		t.Errorf("Get: got status %d", r.StatusCode)
	}
}
