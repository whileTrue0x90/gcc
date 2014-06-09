/* SwingComponent.java -- An interface that defines a Swing component for peers
   Copyright (C)  2006  Free Software Foundation, Inc.

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
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

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

package gnu.java.awt.peer.swing;

import java.awt.event.FocusEvent;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;

import javax.swing.JComponent;

/**
 * Defines some additional methods that the Swing components must implement
 * in order to work with the Swing peers. This is usually achieved by
 * subclassing a Swing component and forwarding the method calls to some
 * protected JComponent method.
 *
 * @author Roman Kennke (kennke@aicas.com)
 */
public interface SwingComponent
{

  /**
   * Returns the actual swing compenent.
   *
   * @return the actual swing compenent
   */
  JComponent getJComponent();

  /**
   * Handles a mouse event. This is usually forwarded to
   * {@link Component#processMouseMotionEvent(MouseEvent)} of the swing
   * component.
   *
   * @param ev the mouse event
   */
  void handleMouseEvent(MouseEvent ev);

  /**
   * Handles a mouse motion event. This is usually forwarded to
   * {@link Component#processMouseEvent(MouseEvent)} of the swing
   * component.
   *
   * @param ev the mouse motion event
   */
  void handleMouseMotionEvent(MouseEvent ev);

  /**
   * Handles a key event. This is usually forwarded to
   * {@link Component#processKeyEvent(KeyEvent)} of the swing
   * component.
   *
   * @param ev the key event
   */
  void handleKeyEvent(KeyEvent ev);

  /**
   * Handles a focus event. This is usually forwarded to
   * {@link Component#processFocusEvent(FocusEvent)} of the swing
   * component.
   *
   * @param ev the focus event
   */
  void handleFocusEvent(FocusEvent ev);
}
