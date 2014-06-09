/* StringBuilder.java -- Unsynchronized growable strings
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2008
   Free Software Foundation, Inc.

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

package java.lang;

import java.io.Serializable;

/**
 * <code>StringBuilder</code> represents a changeable <code>String</code>.
 * It provides the operations required to modify the
 * <code>StringBuilder</code>, including insert, replace, delete, append,
 * and reverse. It like <code>StringBuffer</code>, but is not
 * synchronized.  It is ideal for use when it is known that the
 * object will only be used from a single thread.
 *
 * <p><code>StringBuilder</code>s are variable-length in nature, so even if
 * you initialize them to a certain size, they can still grow larger than
 * that. <em>Capacity</em> indicates the number of characters the
 * <code>StringBuilder</code> can have in it before it has to grow (growing
 * the char array is an expensive operation involving <code>new</code>).
 *
 * <p>Incidentally, compilers often implement the String operator "+"
 * by using a <code>StringBuilder</code> operation:<br>
 * <code>a + b</code><br>
 * is the same as<br>
 * <code>new StringBuilder().append(a).append(b).toString()</code>.
 *
 * <p>Classpath's StringBuilder is capable of sharing memory with Strings for
 * efficiency.  This will help when a StringBuilder is converted to a String
 * and the StringBuilder is not changed after that (quite common when
 * performing string concatenation).
 *
 * @author Paul Fisher
 * @author John Keiser
 * @author Tom Tromey
 * @author Eric Blake (ebb9@email.byu.edu)
 * @see String
 * @see StringBuffer
 *
 * @since 1.5
 */
public final class StringBuilder
  extends AbstractStringBuffer
  implements Serializable, CharSequence, Appendable
{
  // Implementation note: if you change this class, you usually will
  // want to change StringBuffer as well.

  /**
   * For compatability with Sun's JDK
   */
  private static final long serialVersionUID = 4383685877147921099L;

  /**
   * Create a new StringBuilder with default capacity 16.
   */
  public StringBuilder()
  {
    super();
  }

  /**
   * Create an empty <code>StringBuilder</code> with the specified initial
   * capacity.
   *
   * @param capacity the initial capacity
   * @throws NegativeArraySizeException if capacity is negative
   */
  public StringBuilder(int capacity)
  {
    super(capacity);
  }

  /**
   * Create a new <code>StringBuilder</code> with the characters in the
   * specified <code>String</code>. Initial capacity will be the size of the
   * String plus 16.
   *
   * @param str the <code>String</code> to convert
   * @throws NullPointerException if str is null
   */
  public StringBuilder(String str)
  {
    super(str);
  }

  /**
   * Create a new <code>StringBuilder</code> with the characters in the
   * specified <code>CharSequence</code>. Initial capacity will be the
   * length of the sequence plus 16; if the sequence reports a length
   * less than or equal to 0, then the initial capacity will be 16.
   *
   * @param seq the initializing <code>CharSequence</code>
   * @throws NullPointerException if str is null
   */
  public StringBuilder(CharSequence seq)
  {
    super(seq);
  }

  /**
   * Get the length of the <code>String</code> this <code>StringBuilder</code>
   * would create. Not to be confused with the <em>capacity</em> of the
   * <code>StringBuilder</code>.
   *
   * @return the length of this <code>StringBuilder</code>
   * @see #capacity()
   * @see #setLength(int)
   */
  public int length()
  {
    return count;
  }

  /**
   * Get the total number of characters this <code>StringBuilder</code> can
   * support before it must be grown.  Not to be confused with <em>length</em>.
   *
   * @return the capacity of this <code>StringBuilder</code>
   * @see #length()
   * @see #ensureCapacity(int)
   */
  public int capacity()
  {
    return value.length;
  }

  /**
   * Append the <code>String</code> value of the argument to this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param obj the <code>Object</code> to convert and append
   * @return this <code>StringBuilder</code>
   * @see String#valueOf(Object)
   * @see #append(String)
   */
  public StringBuilder append(Object obj)
  {
    super.append(obj);
    return this;
  }

  /**
   * Append the <code>String</code> to this <code>StringBuilder</code>. If
   * str is null, the String "null" is appended.
   *
   * @param str the <code>String</code> to append
   * @return this <code>StringBuilder</code>
   */
  public StringBuilder append(String str)
  {
    super.append(str);
    return this;
  }

  /**
   * Append the <code>StringBuilder</code> value of the argument to this
   * <code>StringBuilder</code>. This behaves the same as
   * <code>append((Object) stringBuffer)</code>, except it is more efficient.
   *
   * @param stringBuffer the <code>StringBuilder</code> to convert and append
   * @return this <code>StringBuilder</code>
   * @see #append(Object)
   */
  public StringBuilder append(StringBuffer stringBuffer)
  {
    super.append(stringBuffer);
    return this;
  }

  /**
   * Append the <code>char</code> array to this <code>StringBuilder</code>.
   * This is similar (but more efficient) than
   * <code>append(new String(data))</code>, except in the case of null.
   *
   * @param data the <code>char[]</code> to append
   * @return this <code>StringBuilder</code>
   * @throws NullPointerException if <code>str</code> is <code>null</code>
   * @see #append(char[], int, int)
   */
  public StringBuilder append(char[] data)
  {
    super.append(data, 0, data.length);
    return this;
  }

  /**
   * Append part of the <code>char</code> array to this
   * <code>StringBuilder</code>. This is similar (but more efficient) than
   * <code>append(new String(data, offset, count))</code>, except in the case
   * of null.
   *
   * @param data the <code>char[]</code> to append
   * @param offset the start location in <code>str</code>
   * @param count the number of characters to get from <code>str</code>
   * @return this <code>StringBuilder</code>
   * @throws NullPointerException if <code>str</code> is <code>null</code>
   * @throws IndexOutOfBoundsException if offset or count is out of range
   *         (while unspecified, this is a StringIndexOutOfBoundsException)
   */
  public StringBuilder append(char[] data, int offset, int count)
  {
    super.append(data, offset, count);
    return this;
  }

  /**
   * Append the <code>String</code> value of the argument to this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param bool the <code>boolean</code> to convert and append
   * @return this <code>StringBuilder</code>
   * @see String#valueOf(boolean)
   */
  public StringBuilder append(boolean bool)
  {
    super.append(bool);
    return this;
  }

  /**
   * Append the <code>char</code> to this <code>StringBuilder</code>.
   *
   * @param ch the <code>char</code> to append
   * @return this <code>StringBuilder</code>
   */
  public StringBuilder append(char ch)
  {
    super.append(ch);
    return this;
  }

  /**
   * Append the characters in the <code>CharSequence</code> to this
   * buffer.
   *
   * @param seq the <code>CharSequence</code> providing the characters
   * @return this <code>StringBuilder</code>
   */
  public StringBuilder append(CharSequence seq)
  {
    super.append(seq, 0, seq.length());
    return this;
  }

  /**
   * Append some characters from the <code>CharSequence</code> to this
   * buffer.  If the argument is null, the four characters "null" are
   * appended.
   *
   * @param seq the <code>CharSequence</code> providing the characters
   * @param start the starting index
   * @param end one past the final index
   * @return this <code>StringBuilder</code>
   */
  public StringBuilder append(CharSequence seq, int start,
                              int end)
  {
    super.append(seq, start, end);
    return this;
  }

  /**
   * Append the <code>String</code> value of the argument to this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param inum the <code>int</code> to convert and append
   * @return this <code>StringBuilder</code>
   * @see String#valueOf(int)
   */
  // This is native in libgcj, for efficiency.
  public StringBuilder append(int inum)
  {
    super.append(inum);
    return this;
  }

  /**
   * Append the <code>String</code> value of the argument to this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param lnum the <code>long</code> to convert and append
   * @return this <code>StringBuilder</code>
   * @see String#valueOf(long)
   */
  public StringBuilder append(long lnum)
  {
    super.append(lnum);
    return this;
  }

  /**
   * Append the <code>String</code> value of the argument to this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param fnum the <code>float</code> to convert and append
   * @return this <code>StringBuilder</code>
   * @see String#valueOf(float)
   */
  public StringBuilder append(float fnum)
  {
    super.append(fnum);
    return this;
  }

  /**
   * Append the <code>String</code> value of the argument to this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param dnum the <code>double</code> to convert and append
   * @return this <code>StringBuilder</code>
   * @see String#valueOf(double)
   */
  public StringBuilder append(double dnum)
  {
    super.append(dnum);
    return this;
  }

  /**
   * Append the code point to this <code>StringBuilder</code>.
   * This is like #append(char), but will append two characters
   * if a supplementary code point is given.
   *
   * @param code the code point to append
   * @return this <code>StringBuilder</code>
   * @see Character#toChars(int, char[], int)
   * @since 1.5
   */
  public StringBuilder appendCodePoint(int code)
  {
    super.appendCodePoint(code);
    return this;
  }

  /**
   * Delete characters from this <code>StringBuilder</code>.
   * <code>delete(10, 12)</code> will delete 10 and 11, but not 12. It is
   * harmless for end to be larger than length().
   *
   * @param start the first character to delete
   * @param end the index after the last character to delete
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if start or end are out of bounds
   */
  public StringBuilder delete(int start, int end)
  {
    super.delete(start, end);
    return this;
  }

  /**
   * Delete a character from this <code>StringBuilder</code>.
   *
   * @param index the index of the character to delete
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if index is out of bounds
   */
  public StringBuilder deleteCharAt(int index)
  {
    super.deleteCharAt(index);
    return this;
  }

  /**
   * Replace characters between index <code>start</code> (inclusive) and
   * <code>end</code> (exclusive) with <code>str</code>. If <code>end</code>
   * is larger than the size of this StringBuilder, all characters after
   * <code>start</code> are replaced.
   *
   * @param start the beginning index of characters to delete (inclusive)
   * @param end the ending index of characters to delete (exclusive)
   * @param str the new <code>String</code> to insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if start or end are out of bounds
   * @throws NullPointerException if str is null
   */
  public StringBuilder replace(int start, int end, String str)
  {
    super.replace(start, end, str);
    return this;
  }

  /**
   * Creates a substring of this StringBuilder, starting at a specified index
   * and ending at the end of this StringBuilder.
   *
   * @param beginIndex index to start substring (base 0)
   * @return new String which is a substring of this StringBuilder
   * @throws StringIndexOutOfBoundsException if beginIndex is out of bounds
   * @see #substring(int, int)
   */
  public String substring(int beginIndex)
  {
    return substring(beginIndex, count);
  }

  /**
   * Creates a substring of this StringBuilder, starting at a specified index
   * and ending at one character before a specified index. This is implemented
   * the same as <code>substring(beginIndex, endIndex)</code>, to satisfy
   * the CharSequence interface.
   *
   * @param beginIndex index to start at (inclusive, base 0)
   * @param endIndex index to end at (exclusive)
   * @return new String which is a substring of this StringBuilder
   * @throws IndexOutOfBoundsException if beginIndex or endIndex is out of
   *         bounds
   * @see #substring(int, int)
   */
  public CharSequence subSequence(int beginIndex, int endIndex)
  {
    return substring(beginIndex, endIndex);
  }

  /**
   * Creates a substring of this StringBuilder, starting at a specified index
   * and ending at one character before a specified index.
   *
   * @param beginIndex index to start at (inclusive, base 0)
   * @param endIndex index to end at (exclusive)
   * @return new String which is a substring of this StringBuilder
   * @throws StringIndexOutOfBoundsException if beginIndex or endIndex is out
   *         of bounds
   */
  public String substring(int beginIndex, int endIndex)
  {
    int len = endIndex - beginIndex;
    if (beginIndex < 0 || endIndex > count || endIndex < beginIndex)
      throw new StringIndexOutOfBoundsException();
    if (len == 0)
      return "";
    return new String(value, beginIndex, len);
  }

  /**
   * Insert a subarray of the <code>char[]</code> argument into this
   * <code>StringBuilder</code>.
   *
   * @param offset the place to insert in this buffer
   * @param str the <code>char[]</code> to insert
   * @param str_offset the index in <code>str</code> to start inserting from
   * @param len the number of characters to insert
   * @return this <code>StringBuilder</code>
   * @throws NullPointerException if <code>str</code> is <code>null</code>
   * @throws StringIndexOutOfBoundsException if any index is out of bounds
   */
  public StringBuilder insert(int offset,
                              char[] str, int str_offset, int len)
  {
    super.insert(offset, str, str_offset, len);
    return this;
  }

  /**
   * Insert the <code>String</code> value of the argument into this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param offset the place to insert in this buffer
   * @param obj the <code>Object</code> to convert and insert
   * @return this <code>StringBuilder</code>
   * @exception StringIndexOutOfBoundsException if offset is out of bounds
   * @see String#valueOf(Object)
   */
  public StringBuilder insert(int offset, Object obj)
  {
    super.insert(offset, obj);
    return this;
  }

  /**
   * Insert the <code>String</code> argument into this
   * <code>StringBuilder</code>. If str is null, the String "null" is used
   * instead.
   *
   * @param offset the place to insert in this buffer
   * @param str the <code>String</code> to insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   */
  public StringBuilder insert(int offset, String str)
  {
    super.insert(offset, str);
    return this;
  }

  /**
   * Insert the <code>CharSequence</code> argument into this
   * <code>StringBuilder</code>.  If the sequence is null, the String
   * "null" is used instead.
   *
   * @param offset the place to insert in this buffer
   * @param sequence the <code>CharSequence</code> to insert
   * @return this <code>StringBuilder</code>
   * @throws IndexOutOfBoundsException if offset is out of bounds
   */
  public StringBuilder insert(int offset, CharSequence sequence)
  {
    super.insert(offset, sequence);
    return this;
  }

  /**
   * Insert a subsequence of the <code>CharSequence</code> argument into this
   * <code>StringBuilder</code>.  If the sequence is null, the String
   * "null" is used instead.
   *
   * @param offset the place to insert in this buffer
   * @param sequence the <code>CharSequence</code> to insert
   * @param start the starting index of the subsequence
   * @param end one past the ending index of the subsequence
   * @return this <code>StringBuilder</code>
   * @throws IndexOutOfBoundsException if offset, start,
   * or end are out of bounds
   */
  public StringBuilder insert(int offset, CharSequence sequence,
                              int start, int end)
  {
    super.insert(offset, sequence, start, end);
    return this;
  }

  /**
   * Insert the <code>char[]</code> argument into this
   * <code>StringBuilder</code>.
   *
   * @param offset the place to insert in this buffer
   * @param data the <code>char[]</code> to insert
   * @return this <code>StringBuilder</code>
   * @throws NullPointerException if <code>data</code> is <code>null</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   * @see #insert(int, char[], int, int)
   */
  public StringBuilder insert(int offset, char[] data)
  {
    super.insert(offset, data);
    return this;
  }

  /**
   * Insert the <code>String</code> value of the argument into this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param offset the place to insert in this buffer
   * @param bool the <code>boolean</code> to convert and insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   * @see String#valueOf(boolean)
   */
  public StringBuilder insert(int offset, boolean bool)
  {
    super.insert(offset, bool);
    return this;
  }

  /**
   * Insert the <code>char</code> argument into this <code>StringBuilder</code>.
   *
   * @param offset the place to insert in this buffer
   * @param ch the <code>char</code> to insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   */
  public StringBuilder insert(int offset, char ch)
  {
    super.insert(offset, ch);
    return this;
  }

  /**
   * Insert the <code>String</code> value of the argument into this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param offset the place to insert in this buffer
   * @param inum the <code>int</code> to convert and insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   * @see String#valueOf(int)
   */
  public StringBuilder insert(int offset, int inum)
  {
    super.insert(offset, inum);
    return this;
  }

  /**
   * Insert the <code>String</code> value of the argument into this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param offset the place to insert in this buffer
   * @param lnum the <code>long</code> to convert and insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   * @see String#valueOf(long)
   */
  public StringBuilder insert(int offset, long lnum)
  {
    super.insert(offset, lnum);
    return this;
  }

  /**
   * Insert the <code>String</code> value of the argument into this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param offset the place to insert in this buffer
   * @param fnum the <code>float</code> to convert and insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   * @see String#valueOf(float)
   */
  public StringBuilder insert(int offset, float fnum)
  {
    super.insert(offset, fnum);
    return this;
  }

  /**
   * Insert the <code>String</code> value of the argument into this
   * <code>StringBuilder</code>. Uses <code>String.valueOf()</code> to convert
   * to <code>String</code>.
   *
   * @param offset the place to insert in this buffer
   * @param dnum the <code>double</code> to convert and insert
   * @return this <code>StringBuilder</code>
   * @throws StringIndexOutOfBoundsException if offset is out of bounds
   * @see String#valueOf(double)
   */
  public StringBuilder insert(int offset, double dnum)
  {
    super.insert(offset, dnum);
    return this;
  }

  /**
   * Reverse the characters in this StringBuilder. The same sequence of
   * characters exists, but in the reverse index ordering.
   *
   * @return this <code>StringBuilder</code>
   */
  public StringBuilder reverse()
  {
    super.reverse();
    return this;
  }

  /**
   * Convert this <code>StringBuilder</code> to a <code>String</code>. The
   * String is composed of the characters currently in this StringBuilder. Note
   * that the result is a copy, and that future modifications to this buffer
   * do not affect the String.
   *
   * @return the characters in this StringBuilder
   */
  public String toString()
  {
    return new String(this);
  }

}
