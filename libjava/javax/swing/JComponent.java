/* JComponent.java -- Every component in swing inherits from this class.
   Copyright (C) 2002, 2004 Free Software Foundation, Inc.

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


package javax.swing;

import java.awt.AWTEvent;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.FlowLayout;
import java.awt.Font;
import java.awt.Graphics;
import java.awt.Image;
import java.awt.Insets;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.event.ActionListener;
import java.awt.event.ContainerEvent;
import java.awt.event.ContainerListener;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.awt.image.ImageObserver;
import java.awt.peer.LightweightPeer;
import java.beans.PropertyChangeListener;
import java.beans.PropertyVetoException;
import java.beans.VetoableChangeListener;
import java.io.Serializable;
import java.util.EventListener;
import java.util.Hashtable;
import java.util.Locale;
import java.util.Vector;
import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleExtendedComponent;
import javax.accessibility.AccessibleRole;
import javax.accessibility.AccessibleStateSet;
import javax.swing.event.AncestorListener;
import javax.swing.event.EventListenerList;
import javax.swing.border.Border;
import javax.swing.plaf.ComponentUI;
import javax.swing.event.SwingPropertyChangeSupport;

/**
 * Every component in swing inherits from this class (JLabel, JButton, etc).
 * It contains generic methods to manage events, properties and sizes.
 * Actual drawing of the component is channeled to a look-and-feel class
 * that is implemented elsewhere.
 *
 * @author Ronald Veldema (rveldema@cs.vu.nl)
 */
public abstract class JComponent extends Container implements Serializable
{
  static final long serialVersionUID = -5242478962609715464L;

  protected EventListenerList listenerList = new EventListenerList();
  
  /**
   * accessibleContext
   */
  protected AccessibleContext accessibleContext;
  
  Dimension pref,min,max;
  Border border;
  JToolTip tooltip;
  String tool_tip_text;
  boolean use_double_buffer = false, opaque;
  Image doubleBuffer;
  int doubleBufferWidth = -1;
  int doubleBufferHeight = -1;
  ComponentUI ui;
  private SwingPropertyChangeSupport changeSupport;
  
  static Locale defaultLocale;
  
  Hashtable prop_hash;
  

	/**
	 * AccessibleJComponent
	 */
	public abstract class AccessibleJComponent 
		extends AccessibleAWTContainer {

		//-------------------------------------------------------------
		// Classes ----------------------------------------------------
		//-------------------------------------------------------------

		/**
		 * AccessibleFocusHandler
		 */
		protected class AccessibleFocusHandler implements FocusListener {
			/**
			 * Constructor AccessibleFocusHandler
			 * @param component TODO
			 */
			protected AccessibleFocusHandler(AccessibleJComponent component) {
				// TODO
			} // AccessibleFocusHandler()

			/**
			 * focusGained
			 * @param event TODO
			 */
			public void focusGained(FocusEvent event) {
				// TODO
			} // focusGained()

			/**
			 * focusLost
			 * @param event TODO
			 */
			public void focusLost(FocusEvent valevent) {
				// TODO
			} // focusLost()
		} // AccessibleFocusHandler

		/**
		 * AccessibleContainerHandler
		 */
		protected class AccessibleContainerHandler implements ContainerListener {
			/**
			 * Constructor AccessibleContainerHandler
			 * @param component TODO
			 */
			protected AccessibleContainerHandler(AccessibleJComponent component) {
				// TODO
			} // AccessibleContainerHandler()

			/**
			 * componentAdded
			 * @param event TODO
			 */
			public void componentAdded(ContainerEvent event) {
				// TODO
			} // componentAdded()

			/**
			 * componentRemoved
			 * @param event TODO
			 */
			public void componentRemoved(ContainerEvent valevent) {
				// TODO
			} // componentRemoved()
		} // AccessibleContainerHandler

		/**
		 * accessibleContainerHandler
		 */
		protected ContainerListener accessibleContainerHandler;

		/**
		 * accessibleFocusHandler
		 */
		protected FocusListener accessibleFocusHandler;

		/**
		 * Constructor AccessibleJComponent
		 * @param component TODO
		 */
		protected AccessibleJComponent(JComponent component) {
//			super((Container)component);
			// TODO
		} // AccessibleJComponent()

		/**
		 * addPropertyChangeListener
		 * @param listener TODO
		 */
		public void addPropertyChangeListener(PropertyChangeListener listener) { 
			// TODO
		} // addPropertyChangeListener()

		/**
		 * removePropertyChangeListener
		 * @param listener TODO
		 */
		public void removePropertyChangeListener(PropertyChangeListener listener) {
			// TODO
		} // removePropertyChangeListener()

		/**
		 * getAccessibleChildrenCount
		 * @returns int
		 */
		public int getAccessibleChildrenCount() {
			return 0; // TODO
		} // getAccessibleChildrenCount()

		/**
		 * getAccessibleChild
		 * @param value0 TODO
		 * @returns Accessible
		 */
		public Accessible getAccessibleChild(int value0) {
			return null; // TODO
		} // getAccessibleChild()

		/**
		 * getAccessibleStateSet
		 * @returns AccessibleStateSet
		 */
		public AccessibleStateSet getAccessibleStateSet() {
			return null; // TODO
		} // getAccessibleStateSet()

		/**
		 * getAccessibleName
		 * @returns String
		 */
		public String getAccessibleName() {
			return null; // TODO
		} // getAccessibleName()

		/**
		 * getAccessibleDescription
		 * @returns String
		 */
		public String getAccessibleDescription() {
			return null; // TODO
		} // getAccessibleDescription()

		/**
		 * getAccessibleRole
		 * @returns AccessibleRole
		 */
		public AccessibleRole getAccessibleRole() {
			return null; // TODO
		} // getAccessibleRole()

		/**
		 * getBorderTitle
		 * @param value0 TODO
		 * @returns String
		 */
		protected String getBorderTitle(Border value0) {
			return null; // TODO
		} // getBorderTitle()


	} // AccessibleJComponent


        public JComponent()
	{
		super();
		super.setLayout(new FlowLayout());
		
		// By default, all JComponents have a locale of 
		// the VM default.
		defaultLocale = Locale.getDefault();
		
		//eventMask |= AWTEvent.COMP_KEY_EVENT_MASK;
		// enableEvents( AWTEvent.KEY_EVENT_MASK );

		//updateUI(); // get a proper ui
	}

	public boolean contains(int x, int y)
	{
		//return dims.contains(x,y);
		return super.contains(x,y);
	}

	public  void addNotify()
	{
		//Notification to this component that it now has a parent component.
		super.addNotify();
	}

	Hashtable get_prop_hash()
	{
		if (prop_hash == null)
			prop_hash = new Hashtable();
		return prop_hash;
	}

	public Object getClientProperty(Object key)
        {	return get_prop_hash().get(key);    }

	public void putClientProperty(Object key, Object value)
	{    get_prop_hash().put(key, value);   }

  /**
   * Unregister an <code>AncestorListener</code>.
   */
  public void removeAncestorListener(AncestorListener listener)
  {
    listenerList.remove(AncestorListener.class, listener);
  }

  /**
   * Unregister a <code>PropertyChangeListener</code>.
   */
  public void removePropertyChangeListener(PropertyChangeListener listener)
  {
    if (changeSupport != null)
      changeSupport.removePropertyChangeListener(listener);
  }

  /**
   * Unregister a <code>PropertyChangeListener</code>.
   */
  public void removePropertyChangeListener(String propertyName, PropertyChangeListener listener)
  {
    listenerList.remove(PropertyChangeListener.class, listener);
  }

  /**
   * Unregister a <code>VetoableChangeChangeListener</code>.
   */
  public void removeVetoableChangeListener(VetoableChangeListener listener)
  {
    listenerList.remove(VetoableChangeListener.class, listener);
  }

  /**
   * Register an <code>AncestorListener</code>.
   */
  public void addAncestorListener(AncestorListener listener)
  {
    listenerList.add(AncestorListener.class, listener);
  }

  /**
   * Register a <code>PropertyChangeListener</code>.
   */
  public void addPropertyChangeListener(PropertyChangeListener listener)
  {
    if (changeSupport == null)
      changeSupport = new SwingPropertyChangeSupport(this);
    changeSupport.addPropertyChangeListener(listener);
  }

  /**
   * Register a <code>PropertyChangeListener</code>.
   */
  public void addPropertyChangeListener(String propertyName, PropertyChangeListener listener)
  {
    listenerList.add(PropertyChangeListener.class, listener);
  }

  /**
   * Register a <code>VetoableChangeListener</code>.
   */
  public void addVetoableChangeListener(VetoableChangeListener listener)
  {
    listenerList.add(VetoableChangeListener.class, listener);
  }

  /**
   * Return all registered listeners of a special type.
   * 
   * @since 1.3
   */
  public EventListener[] getListeners (Class listenerType)
  {
    return listenerList.getListeners (listenerType);
  }
  
  /**
   * Return all registered <code>Ancestor</code> objects.
   * 
   * @since 1.4
   */
  public AncestorListener[] getAncestorListeners()
  {
    return (AncestorListener[]) getListeners (AncestorListener.class);
  }

  /**
   * Return all registered <code>VetoableChangeListener</code> objects.
   * 
   * @since 1.4
   */
  public VetoableChangeListener[] getVetoableChangeListeners()
  {
    return (VetoableChangeListener[]) getListeners (VetoableChangeListener.class);
  }

	public void computeVisibleRect(Rectangle rect)
	{
		//Returns the Component's "visible rect rectangle" - the intersection of the visible rectangles for this component and all of its ancestors.
		//super.computeVisibleRect(rect);
	}
	
        public PropertyChangeListener[] getPropertyChangeListeners(String property)
        {
          return changeSupport == null ? new PropertyChangeListener[0]
                 : changeSupport.getPropertyChangeListeners(property);
        }	

	public void firePropertyChange(String propertyName, boolean oldValue, boolean newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Boolean(oldValue), 
	                                     new Boolean(newValue));
	}
	public void firePropertyChange(String propertyName, byte oldValue, byte newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Byte(oldValue), 
	                                     new Byte(newValue));
	}
	public void firePropertyChange(String propertyName, char oldValue, char newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Character(oldValue), 
	                                     new Character(newValue));
	}

	public void firePropertyChange(String propertyName, double oldValue, double newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Double(oldValue), 
	                                     new Double(newValue));
	}

	public void firePropertyChange(String propertyName, float oldValue, float newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Float(oldValue), 
	                                     new Float(newValue));
	}
	public void firePropertyChange(String propertyName, int oldValue, int newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Integer(oldValue), 
	                                     new Integer(newValue));
	}
	public void firePropertyChange(String propertyName, long oldValue, long newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Long(oldValue), 
	                                     new Long(newValue));
	}

        protected void firePropertyChange(String propertyName, Object oldValue, Object newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, oldValue, newValue);
	}
	public void firePropertyChange(String propertyName, short oldValue, short newValue)
	{
          if (changeSupport != null)
            changeSupport.firePropertyChange(propertyName, new Short(oldValue), 
	                                     new Short(newValue));
	}

	protected  void fireVetoableChange(String propertyName, Object oldValue, Object newValue)
          throws PropertyVetoException
	{
		//       Support for reporting constrained property changes.
	}

        public AccessibleContext getAccessibleContext()
	{
		//       Get the AccessibleContext associated with this JComponent
		return null;
	}
	
        public ActionListener getActionForKeyStroke(KeyStroke aKeyStroke)
	{
		//Return the object that will perform the action registered for a given keystroke.
		return null;
	}
	public float getAlignmentX()
	{
		//    Overrides Container.getAlignmentX to return the vertical alignment.
		return 0;
	}

	public float getAlignmentY()
	{
		//       Overrides Container.getAlignmentY to return the horizontal alignment.
		return 0;
	}
	public boolean getAutoscrolls()
	{
		//Returns true if this component automatically scrolls its contents when dragged, (when contained in a component that supports scrolling, like JViewport
		return false;
	}

	public void setBorder(Border border)
	{
		//System.out.println("set border called !, new border = " + border);
		this.border = border;
		revalidate();
		repaint();
	}

	public Border getBorder()
	{	return border;    }


        public Rectangle getBounds(Rectangle rv)
	{
		if (rv == null)
			return new Rectangle(getX(),getY(),getWidth(),getHeight());
		else
		{
			rv.setBounds(getX(),getY(),getWidth(),getHeight());
			return rv;
		}
	}

	protected  Graphics getComponentGraphics(Graphics g)
	{      return g;       }

	public int getConditionForKeyStroke(KeyStroke aKeyStroke)
	{
		//Return the condition that determines whether a registered action occurs in response to the specified keystroke.
		return 0;
	}
	public int getDebugGraphicsOptions()
	{
		return 0;
	}

	public Graphics getGraphics()
	{	return super.getGraphics();    }


	//    static MantaNative void DebugMe(Border b);

	public Insets getInsets()
	{
		//	System.out.println("watch this border");
		//	DebugMe(border);
		//	System.out.println("border = " + border);

		if (border == null)
		{
			//System.out.println("compares to null !");
			return super.getInsets();
		}
		//	System.out.println("compare failed !");
		return getBorder().getBorderInsets(this);
	}

	public Insets getInsets(Insets insets)
	{
	    Insets t = getInsets();

	    if (insets == null)
		return t;
	    
	    
	    return new Insets(t.top, t.left, t.bottom, t.right);
	}
	public Point getLocation(Point rv)
	{
		//Store the x,y origin of this component into "return value" rv and return rv.

		if (rv == null)
			return new Point(getX(),
					 getY());

		rv.setLocation(getX(),
		               getY());
		return rv;
	}

	public Dimension getMaximumSize()
	{
		if (max != null)
		{
			//System.out.println("HAVE_MAX_SIZE =  " + max);
			return max;
		}
		if (ui != null)
		{
		    Dimension s = ui.getMaximumSize(this);
		    if (s != null)
			{
				//System.out.println("        UI-MAX = " + s + ", UI = " + ui + ", IM="+this);
				return s;
			}
		}
		Dimension p = super.getMaximumSize();
		//System.out.println("               MAX = " + p + ", COMP="+this);
		return p;
	}

	public Dimension getMinimumSize()
	{
		if (min != null)
		{
			//System.out.println("HAVE_MIN_SIZE =  " + min);
			return min;
		}
		if (ui != null)
		{
			Dimension s = ui.getMinimumSize(this);
			if (s != null)
			{
				//	System.out.println("        UI-MIN = " + s + ", UI = " + ui + ", IM="+this);
				return s;
			}
		}
		Dimension p = super.getMinimumSize();
		//	System.out.println("              MIN = " + p + ", COMP="+this);
		return p;
	}

	public Dimension getPreferredSize()
	{
		if (pref != null)
		{
			//System.out.println("HAVE_PREF_SIZE =  " + pref);
			return pref;
		}

		if (ui != null)
		{
			Dimension s = ui.getPreferredSize(this);
			if (s != null)
			{
				//System.out.println("        UI-PREF = " + s + ", UI = " + ui + ", IM="+this);
				return s;
			}
		}
		Dimension p = super.getPreferredSize();
		//	System.out.println("              PREF = " + p + ", COMP="+this);
		return p;
	}

	public Component getNextFocusableComponent()
	{
		//          Return the next focusable component or null if the focus manager should choose the next focusable component automatically
		return null;
	}


	public KeyStroke[] getRegisteredKeyStrokes()
	{
		//          Return the KeyStrokes that will initiate registered actions.
		return null;
	}

	public JRootPane getRootPane()
	{
		JRootPane p = SwingUtilities.getRootPane(this);
		System.out.println("root = " + p);
		return p;
	}

	public Dimension getSize(Dimension rv)
	{
		//	System.out.println("JComponent, getsize()");
		if (rv == null)
			return new Dimension(getWidth(),
			                     getHeight());
		else
		{
			rv.setSize(getWidth(),
			           getHeight());
			return rv;
		}
	}

	public JToolTip createToolTip()
	{
		if (tooltip == null)
			tooltip = new JToolTip(tool_tip_text);
		return tooltip;
	}

	public Point getToolTipLocation(MouseEvent event)
        {	return null;    }

	public void setToolTipText(String text)
	{	tool_tip_text = text;    }

	public String getToolTipText()
	{	return tool_tip_text;    }

	public String getToolTipText(MouseEvent event)
	{	return tool_tip_text;    }

	public Container getTopLevelAncestor()
	{
		//      Returns the top-level ancestor of this component (either the containing Window or Applet), or null if this component has not been added to any container.
		System.out.println("JComponent, getTopLevelAncestor()");
		return null;
	}

	public Rectangle getVisibleRect()
	{
		///    Returns the Component's "visible rectangle" - the intersection of this components visible rectangle:
		System.out.println("JComponent, getVisibleRect()");
		return null;
	}

	public void grabFocus()
	{
		//      Set the focus on the receiving component.
	}

	public boolean hasFocus()
	{
		//      Returns true if this Component has the keyboard focus.
		return false;
	}

	public boolean isDoubleBuffered()
	{	return use_double_buffer;    }

	public boolean isFocusCycleRoot()
	{
		//      Override this method and return true if your component is the root of of a component tree with its own focus cycle.
		return false;
	}

	public boolean isFocusTraversable()
	{
		//      Identifies whether or not this component can receive the focus.
		return false;
	}

	public static boolean isLightweightComponent(Component c)
	{
		return c.getPeer() instanceof LightweightPeer;
	}

	public boolean isManagingFocus()
	{
		//      Override this method and return true if your JComponent manages focus.
		return false;
	}

        public boolean isOpaque()
	{	return opaque;    }

	public boolean isOptimizedDrawingEnabled()
	{
		//      Returns true if this component tiles its children,
		return true;
	}

	public boolean isPaintingTile()
	{
		//      Returns true if the receiving component is currently painting a tile.
		return false;
	}

	public boolean isRequestFocusEnabled()
	{
		//      Return whether the receiving component can obtain the focus by calling requestFocus
		return false;
	}

	public boolean isValidateRoot()
	{
		//      If this method returns true, revalidate() calls by descendants of this component will cause the entire tree beginning with this root to be validated.
		return false;
	}

	public void paint(Graphics g)
	{
		Graphics g2 = g;
		Rectangle r = getBounds ();
		
		if (use_double_buffer)
		{

                  if (doubleBuffer == null 
                      || doubleBufferWidth != r.width 
                      || doubleBufferHeight != r.height)
                    {
                      doubleBuffer = createImage(r.width, r.height);
                      doubleBufferWidth = r.width;
                      doubleBufferHeight = r.height;
                    }

                  g2 = doubleBuffer.getGraphics ();
                  if (this.getBackground() != null)
                    {
                      Color save = g2.getColor();
                      g2.setColor(this.getBackground());
                      g2.fillRect (0, 0, r.width, r.height);
                      g2.setColor(save);
                    }
                  else
                    g2.clearRect(0, 0, r.width, r.height);
		}
		
		paintBorder(g2);
		paintComponent(g2);
		paintChildren(g2);

		if (use_double_buffer)
		{
			// always draw at 0,0, because regardless of your current bounds,
			// the graphics object you were passed was positioned so the origin
			// was at the upper left corner of your bounds.
			g.drawImage (doubleBuffer, 0, 0, (ImageObserver)null);
		}
	}

	protected  void paintBorder(Graphics g)
	{
		//	System.out.println("PAINT_BORDER      x XXXXXXX x x x x x x x x x x x x:" + getBorder() + ", THIS="+this);

		//       Paint the component's border.
		if (getBorder() != null)
		{
			//System.out.println("PAINT_BORDER      x XXXXXXX x x x x x x x x x x x x:" + getBorder() + ", THIS="+this);

			getBorder().paintBorder(this,
			                        g,
			                        0,
			                        0,
			                        getWidth(),
			                        getHeight());
		}
	}

	protected  void paintChildren(Graphics g)
	{
	    //      Paint this component's children.
		super.paint(g);
	}

	protected  void paintComponent(Graphics g)
	{
		//      If the UI delegate is non-null, call its paint method.
		if (ui != null)
		{
			ui.paint(g, this);
		}
	}
    
    /**
     * Paint the specified region in this component and all of 
     * its descendants that overlap the region, immediately.
     */
	public void paintImmediately(int x, int y, int w, int h)
        {
	
	    //Ronald: this shoudld probably redirect to the PLAF ....
	}

	public void paintImmediately(Rectangle r)
	{
	    ///      Paint the specified region now.
	    paintImmediately((int)r.getX(),
			     (int)r.getY(),
			     (int)r.getWidth(),
			     (int)r.getHeight());
	}
	protected  String paramString()
	{
		//      Returns a string representation of this JComponent.
		return "JComponent";
	}

	public void registerKeyboardAction(ActionListener anAction,
	                            KeyStroke aKeyStroke,
	                            int aCondition)
	{
		registerKeyboardAction(anAction,
		                       null,
		                       aKeyStroke,
		                       aCondition);
	}

	public void registerKeyboardAction(ActionListener anAction,
	                            String aCommand,
	                            KeyStroke aKeyStroke,
	                            int aCondition)
	{
		//  Register a new keyboard action.
	}


	public void removeNotify()
	{
		//      Notification to this component that it no longer has a parent component.
	}

	public void repaint(long tm, int x, int y, int width, int height)
	{
		//   Adds the specified region to the dirty region list if the component is showing.
		//System.out.println("JC: repaint");
		super.repaint(tm, x,y,width,height);
	}

	public void repaint(Rectangle r)
	{
		//      Adds the specified region to the dirty region list if the component is showing.
		repaint((long)0,
		        (int)r.getX(),
		        (int)r.getY(),
		        (int)r.getWidth(),
		        (int)r.getHeight());
	}

	public boolean requestDefaultFocus()
	{
		//      Request the focus for the component that should have the focus by default.
		return false;
	}

	public void requestFocus()
	{
		//      Set focus on the receiving component if isRequestFocusEnabled returns true
		super.requestFocus();
	}

	public void resetKeyboardActions()
	{
		//      Unregister all keyboard actions
	}

	public void reshape(int x, int y, int w, int h)
	{
		///      Moves and resizes this component.
		super.reshape(x,y,w,h);
	}

	public void revalidate()
	{
		//     Support for deferred automatic layout.
		if (getParent() == null)
			invalidate();
	}

	public void scrollRectToVisible(Rectangle aRect)
	{
		//      Forwards the scrollRectToVisible() message to the JComponent's parent.
	}

	public void setAlignmentX(float alignmentX)
	{
		//      Set the the vertical alignment.
	}

	public void setAlignmentY(float alignmentY)
	{
		//      Set the the horizontal alignment.
	}

	public void setAutoscrolls(boolean autoscrolls)
	{
		//      If true this component will automatically scroll its contents when dragged, if contained in a component that supports scrolling, such as JViewport
	}

	public void setDebugGraphicsOptions(int debugOptions)
	{
		//      Enables or disables diagnostic information about every graphics operation performed within the component or one of its children.
	}

	public void setDoubleBuffered(boolean aFlag)
	{
		use_double_buffer = aFlag;
	}

	public void setEnabled(boolean enabled)
	{
		// Sets whether or not this component is enabled.
		super.setEnabled(enabled);
		repaint();
	}

	public void setFont(Font font)
	{
		super.setFont(font);
		revalidate();
		repaint();
	}

	public void setBackground(Color bg)
	{
		super.setBackground(bg);
		revalidate();
		repaint();
	}
	public void setForeground(Color fg)
	{
		super.setForeground(fg);
		revalidate();
		repaint();
	}

	public void setMaximumSize(Dimension maximumSize)
	{	max = maximumSize;    }

	public void setMinimumSize(Dimension minimumSize)
	{   min = minimumSize; }

	public void setPreferredSize(Dimension preferredSize)
	{   pref = preferredSize;   }

	public void setNextFocusableComponent(Component aComponent)
	{
		//       Specifies the next component to get the focus after this one, for example, when the tab key is pressed.
	}

	public void setOpaque(boolean isOpaque)
	{
		opaque = isOpaque;
		revalidate();
		repaint();
	}


	public void setRequestFocusEnabled(boolean aFlag)
	{
	}


	public void setVisible(boolean aFlag)
	{
		//    Makes the component visible or invisible.

		super.setVisible(aFlag);
		if (getParent() != null)
		{
			Rectangle dims = getBounds();
			getParent().repaint((int)dims.getX(),
			                    (int)dims.getY(),
			                    (int)dims.getWidth(),
			                    (int)dims.getHeight());
		}
	}

	public void unregisterKeyboardAction(KeyStroke aKeyStroke)
	{
		//          Unregister a keyboard action.
	}


	public void update(Graphics g)
	{
		paint(g);
	}

        public String getUIClassID()
	{
		///          Return the UIDefaults key used to look up the name of the swing.
		return "ComponentUI";
	}

	protected void setUI(ComponentUI newUI)
	{
		if (ui != null)
		{
			ui.uninstallUI(this);
		}

		//          Set the look and feel delegate for this component.
		ui = newUI;

		if (ui != null)
		{
			ui.installUI(this);
		}

		revalidate();
		repaint();
	}

	public void updateUI()
	{
		//        Resets the UI property to a value from the current look and feel.
		System.out.println("update UI not overwritten in class: " + this);
	}
	
	public static Locale getDefaultLocale()
	{
	  return defaultLocale;
	}
	
	public static void setDefaultLocale(Locale l)
	{
	  defaultLocale = l;
	}

}
