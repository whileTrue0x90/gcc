/* gtklabelpeer.c -- Native implementation of GtkLabelPeer
   Copyright (C) 1998, 1999 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */


#include "gtkpeer.h"
#include "gnu_java_awt_peer_gtk_GtkLabelPeer.h"

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkLabelPeer_create
  (JNIEnv *env, jobject obj, jstring text, jfloat xalign)
{
  GtkWidget *label;
  GtkWidget *ebox;
  GtkContainer *ebox_container;
  const char *str;

  str = (*env)->GetStringUTFChars (env, text, 0);

  gdk_threads_enter ();

  ebox = gtk_event_box_new ();
  ebox_container = GTK_CONTAINER (ebox);
  label = gtk_label_new (str);
  gtk_misc_set_alignment (GTK_MISC (label), xalign, 0.5);
  gtk_container_add (ebox_container, label);
  gtk_widget_show (label);

  gdk_threads_leave ();

  (*env)->ReleaseStringUTFChars (env, text, str);

  NSA_SET_PTR (env, obj, ebox);
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkLabelPeer_setText
  (JNIEnv *env, jobject obj, jstring text)
{
  const char *str;
  void *ptr;
  GtkWidget *label;

  ptr = NSA_GET_PTR (env, obj);

  str = (*env)->GetStringUTFChars (env, text, 0);

  gdk_threads_enter ();

  label = gtk_bin_get_child (GTK_BIN(ptr));

  gtk_label_set_label (GTK_LABEL (label), str);

  gdk_threads_leave ();

  (*env)->ReleaseStringUTFChars (env, text, str);
}

JNIEXPORT void JNICALL
Java_gnu_java_awt_peer_gtk_GtkLabelPeer_nativeSetAlignment
  (JNIEnv *env, jobject obj, jfloat xalign)
{
  void *ptr;
  GtkWidget *label;

  ptr = NSA_GET_PTR (env, obj);

  gdk_threads_enter ();

  label = gtk_bin_get_child (GTK_BIN(ptr));

  gtk_misc_set_alignment (GTK_MISC (label), xalign, 0.5);

  gdk_threads_leave ();
}
