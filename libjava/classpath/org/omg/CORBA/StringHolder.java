/* StringHolder.java --
   Copyright (C) 2005 Free Software Foundation, Inc.

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


package org.omg.CORBA;

import gnu.CORBA.stringTypeCode;

import org.omg.CORBA.portable.InputStream;
import org.omg.CORBA.portable.OutputStream;
import org.omg.CORBA.portable.Streamable;

/**
 * A holder for CORBA <code>string</code> that is mapped into
 * java <code>String</code>.
 *
 * The holders have several application areas. The end user usually
 * sees them implementing CORBA methods where the primitive type
 * is passed by reference. While CORBA (or, for example, C) supports
 * this, the java does not and a wrapper class is required.
 *
 * @author Audrius Meskauskas (AudriusA@Bioinformatics.org)
 */
public final class StringHolder
  implements Streamable
{
  /**
   * The default type code for this holder.
   */
  private static final stringTypeCode t_string =
    new stringTypeCode(TCKind.tk_string);

  /**
   * The <code>String</code> (CORBA <code>string</code>) value,
   * held by this StringHolder.
   */
  public String value;

  /**
   * Constructs an instance of StringHolder,
   * initializing {@link #value} to <code>null</code>.
   */
  public StringHolder()
  {
  }

  /**
   * Constructs an instance of StringHolder,
   * initializing {@link #value} to the given <code>String</code>.
   *
   * @param initial_value a value that will be assigned to the
   * {@link #value} field.
   */
  public StringHolder(String initial_value)
  {
    value = initial_value;
  }

  /**
   * Fill in the {@link #value } field by reading the required data
   * from the given stream. For <code>string</code>, the functionality
   * is delegated to
   * {@link org.omg.CORBA.portable.InputStream#read_string}.
   *
   * @param input the input stream to read from.
   */
  public void _read(InputStream input)
  {
    value = input.read_string();
  }

  /**
   * Returns the TypeCode, corresponding the CORBA type that is stored
   * using this holder. The {@link TypeCode#length()} method of the
   * returned typecode always returns 0.
   */
  public TypeCode _type()
  {
    return t_string;
  }

  /**
   * Write the {@link #value } field to the given stream.
   * For <code>string</code>, the functionality
   * is delegated to
   * {@link org.omg.CORBA.portable.OutputStream#write_string(String) }.
   *
   * @param output the output stream to write into.
   */
  public void _write(OutputStream output)
  {
    output.write_string(value);
  }
}
