/* TabSet.java --
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

package javax.swing.text;

import java.io.Serializable;

public class TabSet implements Serializable
{
  TabStop[] tabs;

  public TabSet(TabStop[] t) 
  {
    tabs = t;
  }
 
  public TabStop getTab(int i) 
  {
    return tabs[i];
  }

  public TabStop getTabAfter(float location) 
  {
    int idx = getTabIndexAfter(location);
    if (idx == -1)
      return null;
    else
      return tabs[idx];        
  }

  public int getTabCount() 
  {
    return tabs.length;
  }

  public int getTabIndex(TabStop tab) 
  {
    for (int i = 0; i < tabs.length; ++i)
      if (tabs[i] == tab)
        return i;
    return -1;
  }

  public int getTabIndexAfter(float location) 
  {
    int idx = -1;
    for (int i = 0; i < tabs.length; ++i)
      {
        if (location < tabs[i].getPosition())
          idx = i;
      }
    return idx;
  }

  public String toString()
  {
    StringBuffer sb = new StringBuffer();
    sb.append("[");
    for (int i = 0; i < tabs.length; ++i)
      {
        if (i != 0)
          sb.append(" - ");
        sb.append(tabs[i].toString());
      }
    sb.append("]");
    return sb.toString();
  }
}
