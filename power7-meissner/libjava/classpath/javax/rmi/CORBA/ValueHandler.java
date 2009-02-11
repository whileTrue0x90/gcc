/* ValueHandler.java --
   Copyright (C) 2002, 2004, 2005 Free Software Foundation, Inc.

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


package javax.rmi.CORBA;

import java.io.Serializable;

import org.omg.CORBA.CustomMarshal;
import org.omg.CORBA.portable.InputStream;
import org.omg.CORBA.portable.OutputStream;
import org.omg.CORBA.portable.Streamable;
import org.omg.SendingContext.RunTime;

/**
 * Serializes Java objects to and from CDR (GIOP) streams. The working instance
 * of the value handler is returned by {@link Util#createValueHandler} and can
 * be altered by setting the system property "javax.rmi.CORBA.ValueHandlerClass"
 * to the name of the alternative class that must implement ValueHandler.
 * 
 * @author Audrius Meskauskas (AudriusA@Bioinformatics.org)
 */
public interface ValueHandler
{
  /**
   * Get CORBA repository Id for the given java class.
   * 
   * The syntax of the repository ID is the initial ?RMI:?, followed by the Java
   * class name, followed by name, followed by a hash code string, followed
   * optionally by a serialization version UID string.
   * 
   * For Java identifiers that contain illegal OMG IDL identifier characters
   * such as ?$?, any such illegal characters are replaced by ?\U? followed by
   * the 4 hexadecimal characters (in upper case) representing the Unicode
   * value.
   * 
   * @param clz a class for that the repository Id is required.
   * 
   * @return the class repository id.
   */
  String getRMIRepositoryID(Class clz);

  /**
   * Returns the CodeBase for this ValueHandler.
   * 
   * @return the codebase.
   */
  RunTime getRunTimeCodeBase();

  /**
   * Indicates that the given class is responsible itself for writing its
   * content to the stream. Such classes implement either {@link Streamable}
   * (default marshalling, generated by IDL-to-java compiler) or
   * {@link CustomMarshal} (the user-programmed marshalling).
   * 
   * @param clz the class being checked.
   * @return true if the class supports custom or default marshalling, false
   * otherwise.
   */
  boolean isCustomMarshaled(Class clz);

  /**
   * Read value from the CORBA input stream in the case when the value is not
   * Streamable or CustomMarshall'ed. The fields of the class being written will
   * be accessed using reflection.
   * 
   * @param in a CORBA stream to read.
   * @param offset the current position in the input stream.
   * @param clz the type of value being read.
   * @param repositoryID the repository Id of the value being read.
   * @param sender the sending context that should provide data about the
   * message originator.
   * 
   * @return the object, extracted from the stream.
   */
  Serializable readValue(InputStream in, int offset, Class clz,
    String repositoryID, RunTime sender);

  /**
   * When the value provides the writeReplace method, the result of this method
   * is written. Otherwise, the value itself is written.
   * 
   * @param value the value that should be written to the stream.
   * 
   * @return the value that will be actually written to the stream.
   */
  Serializable writeReplace(Serializable value);

  /**
   * Write value to CORBA output stream using java senmatics.
   * 
   * @param out a stream to write into.
   * @param value a java object to write.
   */
  void writeValue(OutputStream out, Serializable value);
}