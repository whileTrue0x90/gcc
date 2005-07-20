/* Finishings.java --
   Copyright (C) 2004 Free Software Foundation, Inc.

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

package javax.print.attribute.standard;

import javax.print.attribute.DocAttribute;
import javax.print.attribute.EnumSyntax;
import javax.print.attribute.PrintJobAttribute;
import javax.print.attribute.PrintRequestAttribute;


/**
 * @author Michael Koch (konqueror@gmx.de)
 */
public class Finishings extends EnumSyntax
  implements DocAttribute, PrintJobAttribute, PrintRequestAttribute
{
  private static final long serialVersionUID = -627840419548391754L;

  public static final Finishings NONE = new Finishings(0);
  public static final Finishings STAPLE = new Finishings(1);
  public static final Finishings COVER = new Finishings(2);
  public static final Finishings BIND = new Finishings(3);
  public static final Finishings SADDLE_STITCH = new Finishings(4);
  public static final Finishings EDGE_STITCH = new Finishings(5);
  public static final Finishings STAPLE_TOP_LEFT = new Finishings(6);
  public static final Finishings STAPLE_BOTTOM_LEFT = new Finishings(7);
  public static final Finishings STAPLE_TOP_RIGHT = new Finishings(8);
  public static final Finishings STAPLE_BOTTOM_RIGHT = new Finishings(9);
  public static final Finishings EDGE_STITCH_LEFT = new Finishings(10);
  public static final Finishings EDGE_STITCH_TOP = new Finishings(11);
  public static final Finishings EDGE_STITCH_RIGHT = new Finishings(12);
  public static final Finishings EDGE_STITCH_BOTTOM = new Finishings(13);
  public static final Finishings STAPLE_DUAL_LEFT = new Finishings(14);
  public static final Finishings STAPLE_DUAL_TOP = new Finishings(15);
  public static final Finishings STAPLE_DUAL_RIGHT = new Finishings(16);
  public static final Finishings STAPLE_DUAL_BOTTOM = new Finishings(17);

  /**
   * Constructs a <code>Finishings</code> object.
   * 
   * @param value the value
   */
  protected Finishings(int value)
  {
    super(value);
  }

  /**
   * Returns category of this class.
   *
   * @return the class <code>Finishings</code> itself
   */
  public Class getCategory()
  {
    return Finishings.class;
  }

  /**
   * Returns the name of this attribute.
   *
   * @return the name
   */
  public String getName()
  {
    return "finishings";
  }
}
