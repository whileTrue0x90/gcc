/* java.lang.reflect.AccessibleObject
   Copyright (C) 2001 Free Software Foundation, Inc.

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

As a special exception, if you link this library with other files to
produce an executable, this library does not by itself cause the
resulting executable to be covered by the GNU General Public License.
This exception does not however invalidate any other reasons why the
executable file might be covered by the GNU General Public License. */


package java.lang.reflect;

/**
 * This class is the superclass of various reflection classes, and
 * allows sufficiently trusted code to bypass normal restrictions to
 * do necessary things like invoke private methods outside of the
 * class during Serialization.  If you don't have a good reason
 * to mess with this, don't try. Fortunately, there are adequate
 * security checks before you can set a reflection object as accessible.
 *
 * @author Tom Tromey <tromey@cygnus.com>
 * @author Eric Blake <ebb9@email.byu.edu>
 * @see Field
 * @see Constructor
 * @see Method
 * @see ReflectPermission
 * @since 1.2
 * @status updated to 1.4
 */
public class AccessibleObject
{
  /**
   * True if this object is marked accessible, which means the reflected
   * object bypasses normal security checks. <em>NOTE</em>Don't try messing
   * with this by reflection.  You'll mess yourself up.
   */
  // default visibility for use by inherited classes
  boolean flag = false;

  /**
   * Only the three reflection classes that extend this can create an
   * accessible object.  This is not serializable for security reasons.
   */
  protected AccessibleObject()
  {
  }

  /**
   * Return the accessibility status of this object.
   *
   * @return true if this object bypasses security checks
   */
  public boolean isAccessible()
  {
    return flag;
  }

  /**
   * Convenience method to set the flag on a number of objects with a single
   * security check. If a security manager exists, it is checked for
   * <code>ReflectPermission("suppressAccessChecks")</code>.<p>
   *
   * If <code>flag</code> is true, and the initial security check succeeds,
   * this can still fail if a forbidden object is encountered, leaving the
   * array half-modified. At the moment, the forbidden members are:<br>
   * <ul>
   *  <li>Any Constructor for java.lang.Class</li>
   *  <li>Any AccessibleObject for java.lang.reflect.AccessibleObject
   *      (this is not specified by Sun, but it closes a big security hole
   *      where you can use reflection to bypass the security checks that
   *      reflection is supposed to provide)</li>
   * </ul>
   * (Sun has not specified others, but good candidates might include
   * ClassLoader, String, and such. However, the more checks we do, the
   * slower this method gets).
   *
   * @param array the array of accessible objects
   * @param flag the desired state of accessibility, true to bypass security
   * @throws NullPointerException if array is null
   * @throws SecurityException if the request is denied
   * @see SecurityManager#checkPermission(java.security.Permission)
   * @see RuntimePermission
   */
  public static void setAccessible(AccessibleObject[] array, boolean flag)
  {
    checkPermission();
    for (int i = 0; i < array.length; i++)
      array[i].secureSetAccessible(flag);
  }

  /**
   * Sets the accessibility flag for this reflection object. If a security
   * manager exists, it is checked for
   * <code>ReflectPermission("suppressAccessChecks")</code>.<p>
   *
   * If <code>flag</code> is true, and the initial security check succeeds,
   * this will still fail for a forbidden object. At the moment, the
   * forbidden members are:<br>
   * <ul>
   *  <li>Any Constructor for java.lang.Class</li>
   *  <li>Any AccessibleObject for java.lang.reflect.AccessibleObject
   *      (this is not specified by Sun, but it closes a big security hole
   *      where you can use reflection to bypass the security checks that
   *      reflection is supposed to provide)</li>
   * </ul>
   * (Sun has not specified others, but good candidates might include
   * ClassLoader, String, and such. However, the more checks we do, the
   * slower this method gets).
   *
   * @param flag the desired state of accessibility, true to bypass security
   * @throws NullPointerException if array is null
   * @throws SecurityException if the request is denied
   * @see SecurityManager#checkPermission(java.security.Permission)
   * @see RuntimePermission
   */
  public void setAccessible(boolean flag)
  {
    checkPermission();
    secureSetAccessible(flag);
  }

  /**
   * Performs the specified security check, for
   * <code>ReflectPermission("suppressAccessChecks")</code>.
   *
   * @throws SecurityException if permission is denied
   */
  private static final void checkPermission()
  {
    SecurityManager sm = System.getSecurityManager();
    if (sm != null)
      sm.checkPermission(new ReflectPermission("suppressAccessChecks"));
  }

  /**
   * Performs the actual accessibility change, this must always be invoked
   * after calling checkPermission.
   *
   * @param flag the desired status
   * @throws SecurityException if flag is true and this is one of the
   *         forbidden members mentioned in {@link setAccessible(boolean)}.
   */
  private final void secureSetAccessible(boolean flag)
  {
    if (flag &&
        ((this instanceof Constructor
          && ((Constructor) this).getDeclaringClass() == Class.class)
         || ((Member) this).getDeclaringClass() == AccessibleObject.class))
      throw new SecurityException("Cannot make object accessible: " + this);
    this.flag = flag;
  }
}
