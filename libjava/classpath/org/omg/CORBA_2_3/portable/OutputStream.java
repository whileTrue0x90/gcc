/* OutputStream.java --
   Copyright (C) 2005, 2006 Free Software Foundation, Inc.

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


package org.omg.CORBA_2_3.portable;

import gnu.CORBA.CDR.Vio;

import org.omg.CORBA.portable.BoxedValueHelper;
import org.omg.CORBA.portable.CustomValue;
import org.omg.CORBA.portable.StreamableValue;
import org.omg.CORBA.portable.ValueBase;

import java.io.Serializable;

/**
 * This class defines a new CDR input stream methods, added since
 * CORBA 2.3.
 *
 * This class is abstract; no direct instances can be instantiated.
 * Also, up till v 1.4 inclusive there are no methods that would
 * return it directly.
 *
 * However since 1.3 all methods, declared as returning an
 * org.omg.CORBA.portable.InputStream actually return the instance of this
 * derived class and the new methods are accessible after the casting
 * operation.
 *
 * @author Audrius Meskauskas (AudriusA@Bioinformatics.org)
 */
public abstract class OutputStream
  extends org.omg.CORBA.portable.OutputStream
{
  /**
   * Writes an abstract interface to the stream. An abstract interface can be
   * eithe CORBA object or value type and is written as a union with the boolean
   * discriminator (false for objects, true for value types).
   * 
   * The object from value is separated by fact that all values implement the
   * {@link ValueBase} interface. Also, the passed parameter is treated as value
   * it it does not implement CORBA Object.
   * 
   * @param an_interface an abstract interface to write.
   */
  public void write_abstract_interface(java.lang.Object an_interface)
  {
    boolean isObject = !(an_interface instanceof ValueBase)
      && an_interface instanceof org.omg.CORBA.Object;

    write_boolean(isObject);

    if (isObject)
      write_Object((org.omg.CORBA.Object) an_interface);
    else
      write_value((Serializable) an_interface);

  }

  /**
   * Writes a value type into the output stream.
   * 
   * The value type must implement either {@link CustomValue} (for user-defined
   * writing method) or {@link StreamableValue} (for standard writing using code,
   * generated by IDL compiler).
   * 
   * The written record will have a repository id, matching the class of the
   * passed object. The codebase will not be written.
   * 
   * @param value a value type object to write.
   */
  public void write_value(Serializable value)
  {
    Vio.write(this, value);
  }

  /**
   * Write value to the stream using the boxed value helper.
   *
   * The value type must implement either {@link CustomValue}
   * (for user-defined writing method) or {@link StreamableValue}
   * (for standard writing using code, generated by IDL compiler).
   *
   * @param value a value to write.
   * @param helper a helper, responsible for the writing operation.
   */
  public void write_value(Serializable value, BoxedValueHelper helper)
  {
    Vio.write(this, value, helper);
  }

  /**
   * Writes a value type into the output stream, stating it is an
   * instance of the given class. The written record
   * will have a repository id, matching the passed class.
   * The codebase will not be written. It the object
   * being written is an instance of the different class, this results
   * writing two Id inheritance hierarchy.
   *
   * The value type must implement either {@link CustomValue}
   * (for user-defined writing method) or {@link StreamableValue}
   * (for standard writing using code, generated by IDL compiler).
   *
   * @param value a value type object to write.
   */
  @SuppressWarnings("unchecked") // Needed for API compatibility
  public void write_value(Serializable value, Class clz)
  {
    Vio.write(this, value, clz);
  }

  /**
   * Writes a value type into the output stream, stating it has the given
   * repository id.
   * 
   * The value type must implement either {@link CustomValue} (for user-defined
   * writing method) or {@link StreamableValue} (for standard writing using code,
   * generated by IDL compiler).
   * 
   * @param repository_id a repository id of the value type.
   * 
   * @param value a value type object to write.
   */
  public void write_value(Serializable value, String repository_id)
  {
    Vio.write(this, value, repository_id);
  }
}