/* GtkWindowPeer.java -- Implements WindowPeer with GTK
   Copyright (C) 1998, 1999, 2002 Free Software Foundation, Inc.

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


package gnu.java.awt.peer.gtk;

import java.awt.Component;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.Window;
import java.awt.Frame;
import java.awt.event.WindowEvent;
import java.awt.peer.WindowPeer;

public class GtkWindowPeer extends GtkContainerPeer
  implements WindowPeer
{
  static protected final int GDK_WINDOW_TYPE_HINT_NORMAL = 0;
  static protected final int GDK_WINDOW_TYPE_HINT_DIALOG = 1;
  static protected final int GDK_WINDOW_TYPE_HINT_MENU = 2;
  static protected final int GDK_WINDOW_TYPE_HINT_TOOLBAR = 3;
  static protected final int GDK_WINDOW_TYPE_HINT_SPLASHSCREEN = 4;
  static protected final int GDK_WINDOW_TYPE_HINT_UTILITY = 5;
  static protected final int GDK_WINDOW_TYPE_HINT_DOCK = 6;
  static protected final int GDK_WINDOW_TYPE_HINT_DESKTOP = 7;

  private boolean hasBeenShown = false;
  private int oldState = Frame.NORMAL;

  // Unfortunately, X does not provide a clean way to calculate the
  // dimensions of a window's borders before it has been displayed.
  // So when creating the application's first window we guess the
  // border dimensions.  Then if need be for that window, we fix the
  // dimensions upon receipt of the first configure event.  Windows
  // created after the first one will use the latest inset values
  // received in postConfigureEvent.
  static Insets latestInsets = new Insets (20, 6, 6, 6);

  native void create (int type, boolean decorated,
		      int width, int height,
		      GtkWindowPeer parent);

  void create (int type, boolean decorated)
  {
    GtkWindowPeer parent_peer = null;
    Component parent = awtComponent.getParent();
    if (parent != null)
      parent_peer = (GtkWindowPeer) awtComponent.getParent().getPeer();

    create (type, decorated,
	    awtComponent.getWidth(),
	    awtComponent.getHeight(),
	    parent_peer);
  }

  void create ()
  {
    // Create a normal undecorated window.
    create (GDK_WINDOW_TYPE_HINT_NORMAL, false);
  }

  native void connectHooks ();

  public GtkWindowPeer (Window window)
  {
    super (window);
  }

  public void getArgs (Component component, GtkArgList args)
  {
    args.add ("visible", component.isVisible ());
    args.add ("sensitive", component.isEnabled ());
  }

  native public void toBack ();
  native public void toFront ();

  native void nativeSetBounds (int x, int y, int width, int height);

  public void setBounds (int x, int y, int width, int height)
  {
    nativeSetBounds (x, y,
		     width - insets.left - insets.right,
		     height - insets.top - insets.bottom);
  }

  public void setTitle (String title)
  {
    set ("title", title);
  }

  native void setSize (int width, int height);

  public void setResizable (boolean resizable)
  {
    // Call setSize; otherwise when resizable is changed from true to
    // false the window will shrink to the dimensions it had before it
    // was resizable.
    setSize (awtComponent.getWidth() - insets.left - insets.right,
    	     awtComponent.getHeight() - insets.top - insets.bottom);
    set ("allow_shrink", resizable);
    set ("allow_grow", resizable);
  }

  native void setBoundsCallback (Window window,
				 int x, int y,
				 int width, int height);

  protected void postConfigureEvent (int x, int y, int width, int height,
				     int top, int left, int bottom, int right)
  {
    // Configure events tell us the location and dimensions of the
    // window within the frame borders, and the dimensions of the
    // frame borders (top, left, bottom, right).

    // If our borders change we need to make sure that a new layout
    // will happen, since Sun forgets to handle this case.
    if (insets.top != top
	|| insets.left != left
	|| insets.bottom != bottom
	|| insets.right != right)
      {
	// When our insets change, we receive a configure event with
	// the new insets, the old window location and the old window
	// dimensions.  We update our Window object's location and
	// size using our old inset values.
	setBoundsCallback ((Window) awtComponent,
			   x - insets.left,
			   y - insets.top,
			   width + insets.left + insets.right,
			   height + insets.top + insets.bottom);

	// The peer's dimensions do not get updated automatically when
	// insets change so we need to do it manually.
	setSize (width + (insets.left - left) + (insets.right - right),
		 height + (insets.top - top) + (insets.bottom - bottom));

	insets.top = top;
	insets.left = left;
	insets.bottom = bottom;
	insets.right = right;

	synchronized (latestInsets)
	  {
	    latestInsets.top = top;
	    latestInsets.left = left;
	    latestInsets.bottom = bottom;
	    latestInsets.right = right;
	  }
      }
    else
      {
	int frame_x = x - insets.left;
	int frame_y = y - insets.top;
	int frame_width = width + insets.left + insets.right;
	int frame_height = height + insets.top + insets.bottom;

	if (frame_x != awtComponent.getX()
	    || frame_y != awtComponent.getY()
	    || frame_width != awtComponent.getWidth()
	    || frame_height != awtComponent.getHeight())
	  {
	    setBoundsCallback ((Window) awtComponent,
			       frame_x,
			       frame_y,
			       frame_width,
			       frame_height);
	  }
      }
    awtComponent.validate();
  }

  native void nativeSetVisible (boolean b);
  public void setVisible (boolean b)
  {
    // Prevent the window manager from automatically placing this
    // window when it is shown.
    if (b)
      setBounds (awtComponent.getX(),
		 awtComponent.getY(),
		 awtComponent.getWidth(),
		 awtComponent.getHeight());
    nativeSetVisible (b);
  }

  void postWindowEvent (int id, Window opposite, int newState)
  {
    if (id == WindowEvent.WINDOW_OPENED)
      {
	// Post a WINDOW_OPENED event the first time this window is shown.
	if (!hasBeenShown)
	  {
	    q.postEvent (new WindowEvent ((Window) awtComponent, id,
					  opposite));
	    hasBeenShown = true;
	  }
      }
    else if (id == WindowEvent.WINDOW_STATE_CHANGED)
      {
	if (oldState != newState)
	  {
	    q.postEvent (new WindowEvent ((Window) awtComponent, id, opposite,
					  oldState, newState));
	    oldState = newState;
	  }
      }
    else
      q.postEvent (new WindowEvent ((Window) awtComponent, id, opposite));
  }
}
