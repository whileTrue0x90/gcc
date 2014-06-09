/* NO_RESPONSE.java --
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


package org.omg.CORBA;

import java.io.Serializable;

/**
 * Means that the client attempts to retrieve the result that is not
 * yet available.
 *
 * @author Audrius Meskauskas (AudriusA@Bioinformatics.org)
 */
public final class NO_RESPONSE
  extends SystemException
  implements Serializable
{
  /**
   * Use serialVersionUID for interoperability.
   */
  private static final long serialVersionUID = 1971973765161647047L;

  /**
   * Creates a NO_RESPONSE with the default minor code of 0,
   * completion state COMPLETED_NO and the given explaining message.
   * @param message the explaining message.
   */
  public NO_RESPONSE(String message)
  {
    super(message, 0, CompletionStatus.COMPLETED_NO);
  }

  /**
   * Creates NO_RESPONSE with the default minor code of 0 and a
   * completion state COMPLETED_NO.
   */
  public NO_RESPONSE()
  {
    super("", 0, CompletionStatus.COMPLETED_NO);
  }

  /** Creates a NO_RESPONSE exception with the specified minor
   * code and completion status.
   * @param minor additional error code.
   * @param completed the method completion status.
   */
  public NO_RESPONSE(int minor, CompletionStatus completed)
  {
    super("", minor, completed);
  }

  /**
   * Created NO_RESPONSE exception, providing full information.
   * @param reason explaining message.
   * @param minor additional error code (the "minor").
   * @param completed the method completion status.
   */
  public NO_RESPONSE(String reason, int minor, CompletionStatus completed)
  {
    super(reason, minor, completed);
  }
}
