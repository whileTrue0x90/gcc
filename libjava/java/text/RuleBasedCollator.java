/* RuleBasedCollator.java -- Concrete Collator Class
   Copyright (C) 1998, 1999, 2000, 2001, 2003  Free Software Foundation, Inc.

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


package java.text;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.Vector;

/* Written using "Java Class Libraries", 2nd edition, plus online
 * API docs for JDK 1.2 from http://www.javasoft.com.
 * Status: Believed complete and correct
 */

/**
 * This class is a concrete subclass of <code>Collator</code> suitable
 * for string collation in a wide variety of languages.  An instance of
 * this class is normally returned by the <code>getInstance</code> method
 * of <code>Collator</code> with rules predefined for the requested
 * locale.  However, an instance of this class can be created manually
 * with any desired rules.
 * <p>
 * Rules take the form of a <code>String</code> with the following syntax
 * <ul>
 * <li> Modifier: '@' 
 * <li> Relation: '&lt;' | ';' | ',' | '=' : <text>
 * <li> Reset: '&amp;' : <text>
 * </ul>
 * The modifier character indicates that accents sort backward as is the
 * case with French.  The relational operators specify how the text 
 * argument relates to the previous term.  The relation characters have
 * the following meanings:
 * <ul>
 * <li>'&lt;' - The text argument is greater than the prior term at the primary
 * difference level.
 * <li>';' - The text argument is greater than the prior term at the secondary
 * difference level.
 * <li>',' - The text argument is greater than the prior term at the tertiary
 * difference level.
 * <li>'=' - The text argument is equal to the prior term
 * </ul>
 * <p>
 * As for the text argument itself, this is any sequence of Unicode
 * characters not in the following ranges: 0x0009-0x000D, 0x0020-0x002F,
 * 0x003A-0x0040, 0x005B-0x0060, and 0x007B-0x007E. If these characters are 
 * desired, they must be enclosed in single quotes.  If any whitespace is 
 * encountered, it is ignored.  (For example, "a b" is equal to "ab").  
 * <p>
 * The reset operation inserts the following rule at the point where the
 * text argument to it exists in the previously declared rule string.  This
 * makes it easy to add new rules to an existing string by simply including
 * them in a reset sequence at the end.  Note that the text argument, or
 * at least the first character of it, must be present somewhere in the
 * previously declared rules in order to be inserted properly.  If this
 * is not satisfied, a <code>ParseException</code> will be thrown. 
 * <p>
 * This system of configuring <code>RuleBasedCollator</code> is needlessly
 * complex and the people at Taligent who developed it (along with the folks
 * at Sun who accepted it into the Java standard library) deserve a slow
 * and agonizing death.
 * <p>
 * Here are a couple of example of rule strings:
 * <p>
 * "&lt; a &lt; b &lt; c" - This string says that a is greater than b which is 
 * greater than c, with all differences being primary differences.
 * <p>
 * "&lt; a,A &lt; b,B &lt; c,C" - This string says that 'A' is greater than 'a' with
 * a tertiary strength comparison.  Both 'b' and 'B' are greater than 'a' and
 * 'A' during a primary strength comparison.  But 'B' is greater than 'b'
 * under a tertiary strength comparison.
 * <p>
 * "&lt; a &lt; c &amp; a &lt; b " - This sequence is identical in function to the 
 * "&lt; a &lt; b &lt; c" rule string above.  The '&amp;' reset symbol indicates that
 * the rule "&lt; b" is to be inserted after the text argument "a" in the
 * previous rule string segment.
 * <p>
 * "&lt; a &lt; b &amp; y &lt; z" - This is an error.  The character 'y' does not appear
 * anywhere in the previous rule string segment so the rule following the
 * reset rule cannot be inserted.
 * <p>
 * For a description of the various comparison strength types, see the
 * documentation for the <code>Collator</code> class.
 * <p>
 * As an additional complication to this already overly complex rule scheme,
 * if any characters precede the first rule, these characters are considered
 * ignorable.  They will be treated as if they did not exist during 
 * comparisons.  For example, "- &lt; a &lt; b ..." would make '-' an ignorable
 * character such that the strings "high-tech" and "hightech" would
 * be considered identical.
 * <p>
 * A <code>ParseException</code> will be thrown for any of the following
 * conditions:
 * <ul>
 * <li>Unquoted punctuation characters in a text argument.
 * <li>A relational or reset operator not followed by a text argument
 * <li>A reset operator where the text argument is not present in
 * the previous rule string section.
 * </ul>
 *
 * @author Aaron M. Renn <arenn@urbanophile.com>
 * @author Tom Tromey <tromey@cygnus.com>
 * @date March 25, 1999
 */

final class RBCElement
{
  String key;
  char relation;

  RBCElement (String key, char relation)
  {
    this.key = key;
    this.relation = relation;
  }
}

public class RuleBasedCollator extends Collator
{
  // True if we are using French-style accent ordering.
  private boolean frenchAccents;

  /**
   * This the the original rule string.
   */
  private String rules;

  // This maps strings onto collation values.
  private Hashtable map;
  
  // An entry in this hash means that more lookahead is required for
  // the prefix string.
  private Hashtable prefixes;
  
  public Object clone ()
  {
    RuleBasedCollator c = (RuleBasedCollator) super.clone ();
    c.map = (Hashtable) map.clone ();
    c.prefixes = (Hashtable) map.clone ();
    return c;
  }

  // A helper for CollationElementIterator.next().
  int ceiNext (CollationElementIterator cei)
  {
    if (cei.lookahead_set)
      {
	cei.lookahead_set = false;
	return cei.lookahead;
      }

    int save = cei.index;
    int max = cei.text.length();
    String s = null;

    // It is possible to have a case where `abc' has a mapping, but
    // neither `ab' nor `abd' do.  In this case we must treat `abd' as
    // nothing special.
    boolean found = false;

    int i;
    for (i = save + 1; i <= max; ++i)
      {
	s = cei.text.substring(save, i);
	if (prefixes.get(s) == null)
	  break;
	found = true;
      }
    // Assume s != null.

    Object obj = map.get(s);
    // The special case.
    while (found && obj == null && s.length() > 1)
      {
	--i;
	s = cei.text.substring(save, i);
	obj = map.get(s);
      }

    // Update state.
    cei.index = i;

    if (obj == null)
      {
	// This idea, and the values, come from JDK.
	// assert (s.length() == 1)
	cei.lookahead_set = true;
	cei.lookahead = s.charAt(0) << 8;
	return 0x7fff << 16;
      }

    return ((Integer) obj).intValue();
  }

  // A helper for compareTo() that returns the next character that has
  // a nonzero ordering at the indicated strength.  This is also used
  // in CollationKey.
  static final int next (CollationElementIterator iter, int strength)
  {
    while (true)
      {
	int os = iter.next();
	if (os == CollationElementIterator.NULLORDER)
	  return os;
	int c = 0;
	switch (strength)
	  {
	  case PRIMARY:
	    c = os & ~0xffff;
	    break;
	  case SECONDARY:
	    c = os & ~0x00ff;
	    break;
	  case TERTIARY:
	  case IDENTICAL:
	    c = os;
	    break;
	  }
	if (c != 0)
	  return c;
      }
  }

  public int compare (String source, String target)
  {
    CollationElementIterator cs, ct;

    cs = new CollationElementIterator (source, this);
    ct = new CollationElementIterator (target, this);

    while (true)
      {
	int os = next (cs, strength);
	int ot = next (ct, strength);

	if (os == CollationElementIterator.NULLORDER
	    && ot == CollationElementIterator.NULLORDER)
	  break;
	else if (os == CollationElementIterator.NULLORDER)
	  {
	    // Source string is shorter, so return "less than".
	    return -1;
	  }
	else if (ot == CollationElementIterator.NULLORDER)
	  {
	    // Target string is shorter, so return "greater than".
	    return 1;
	  }

	if (os != ot)
	  return os - ot;
      }

    return 0;
  }

  public boolean equals (Object obj)
  {
    if (! (obj instanceof RuleBasedCollator) || ! super.equals(obj))
      return false;
    RuleBasedCollator rbc = (RuleBasedCollator) obj;
    // FIXME: this is probably wrong.  Instead we should compare maps
    // directly.
    return (frenchAccents == rbc.frenchAccents
	    && rules.equals(rbc.rules));
  }

  public CollationElementIterator getCollationElementIterator (String source)
  {
    StringBuffer expand = new StringBuffer (source.length());
    int max = source.length();
    for (int i = 0; i < max; ++i)
      decomposeCharacter (source.charAt(i), expand);
    return new CollationElementIterator (expand.toString(), this);
  }

  public CollationElementIterator getCollationElementIterator (CharacterIterator source)
  {
    StringBuffer expand = new StringBuffer ();
    for (char c = source.first ();
	 c != CharacterIterator.DONE;
	 c = source.next ())
      decomposeCharacter (c, expand);

    return new CollationElementIterator (expand.toString(), this);
  }

  public CollationKey getCollationKey (String source)
  {
    return new CollationKey (getCollationElementIterator (source), source,
			     strength);
  }

  public String getRules ()
  {
    return rules;
  }

  public int hashCode ()
  {
    return (frenchAccents ? 1231 : 1237
	    ^ rules.hashCode()
	    ^ map.hashCode()
	    ^ prefixes.hashCode());
  }

  private final boolean is_special (char c)
  {
    // Rules from JCL book.
    return ((c >= 0x0009 && c <= 0x000d)
	    || (c >= 0x0020 && c <= 0x002f)
	    || (c >= 0x003a && c <= 0x0040)
	    || (c >= 0x005b && c <= 0x0060)
	    || (c >= 0x007b && c <= 0x007e));
  }

  private final int text_argument (String rules, int index,
				   StringBuffer result)
  {
    result.setLength(0);
    int len = rules.length();
    while (index < len)
      {
	char c = rules.charAt(index);
	if (c == '\'' && index + 2 < len
	    && rules.charAt(index + 2) == '\''
	    && is_special (rules.charAt(index + 1)))
	  index += 2;
	else if (is_special (c) || Character.isWhitespace(c))
	  return index;
	result.append(c);
	++index;
      }
    return index;
  }

  public RuleBasedCollator (String rules) throws ParseException
  {
    this.rules = rules;
    this.frenchAccents = false;

    // We keep each rule in order in a vector.  At the end we traverse
    // the vector and compute collation values from it.
    int insertion_index = 0;
    Vector vec = new Vector ();

    StringBuffer argument = new StringBuffer ();

    int len = rules.length();
    for (int index = 0; index < len; ++index)
      {
	char c = rules.charAt(index);

	// Just skip whitespace.
	if (Character.isWhitespace(c))
	  continue;

	// Modifier.
	if (c == '@')
	  {
	    frenchAccents = true;
	    continue;
	  }

	// Check for relation or reset operator.
	if (! (c == '<' || c == ';' || c == ',' || c == '=' || c == '&'))
	  throw new ParseException ("invalid character", index);

	++index;
	while (index < len)
	  {
	    if (! Character.isWhitespace(rules.charAt(index)))
	      break;
	    ++index;
	  }
	if (index == len)
	  throw new ParseException ("missing argument", index);

	int save = index;
	index = text_argument (rules, index, argument);
	if (argument.length() == 0)
	  throw new ParseException ("invalid character", save);
	String arg = argument.toString();
	int item_index = vec.indexOf(arg);
	if (c != '&')
	  {
	    // If the argument already appears in the vector, then we
	    // must remove it in order to re-order.
	    if (item_index != -1)
	      {
		vec.removeElementAt(item_index);
		if (insertion_index >= item_index)
		  --insertion_index;
	      }
	    RBCElement r = new RBCElement (arg, c);
	    vec.insertElementAt(r, insertion_index);
	    ++insertion_index;
	  }
	else
	  {
	    // Reset.
	    if (item_index == -1)
	      throw
		new ParseException ("argument to reset not previously seen",
				    save);
	    insertion_index = item_index + 1;
	  }

	// Ugly: in this case the resulting INDEX comes from
	// text_argument, which returns the index of the next
	// character we should examine.
	--index;
      }

    // Now construct a hash table that maps strings onto their
    // collation values.
    int primary = 0;
    int secondary = 0;
    int tertiary = 0;
    this.map = new Hashtable ();
    this.prefixes = new Hashtable ();
    Enumeration e = vec.elements();
    while (e.hasMoreElements())
      {
	RBCElement r = (RBCElement) e.nextElement();
	switch (r.relation)
	  {
	  case '<':
	    ++primary;
	    secondary = 0;
	    tertiary = 0;
	    break;
	  case ';':
	    ++secondary;
	    tertiary = 0;
	    break;
	  case ',':
	    ++tertiary;
	    break;
	  case '=':
	    break;
	  }
	// This must match CollationElementIterator.
	map.put(r.key, new Integer (primary << 16
				    | secondary << 8 | tertiary));

	// Make a map of all lookaheads we might need.
	for (int i = r.key.length() - 1; i >= 1; --i)
	  prefixes.put(r.key.substring(0, i), Boolean.TRUE);
      }
  }
}
