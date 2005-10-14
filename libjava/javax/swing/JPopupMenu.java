/* JPopupMenu.java --
   Copyright (C) 2002, 2004, 2005  Free Software Foundation, Inc.

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

import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.GridBagConstraints;
import java.awt.Insets;
import java.awt.Panel;
import java.awt.Point;
import java.awt.event.KeyEvent;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.ArrayList;
import java.util.EventListener;

import javax.accessibility.Accessible;
import javax.accessibility.AccessibleContext;
import javax.accessibility.AccessibleRole;
import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;
import javax.swing.plaf.PopupMenuUI;

/**
 * JPopupMenu is a container that is used to display popup menu's menu
 * items. By default JPopupMenu is a lightweight container, however if it
 * is the case that JPopupMenu's bounds are outside of main window, then
 * heawyweight container will be used to display menu items. It is also
 * possible to change JPopupMenu's default  behavior and set JPopupMenu
 * to always use heavyweight container.
 *
 * JPopupMenu can be displayed anywhere; it is a floating free popup menu.
 * However before JPopupMenu is diplayed, its invoker property should be set.
 * JPopupMenu's invoker is a component relative to which popup menu is
 * displayed.
 *
 * JPopupMenu fires PopupMenuEvents to its registered listeners. Whenever
 * JPopupMenu becomes visible on the screen then PopupMenuEvent indicating
 * that popup menu became visible will be fired. In the case when
 * JPopupMenu becomes invisible or cancelled without selection, then
 * popupMenuBecomeInvisible() or popupMenuCancelled() methods of
 * PopupMenuListeners will be invoked.
 *
 * JPopupMenu also fires PropertyChangeEvents when its bound properties 
 * change.In addittion to inheritted bound properties, JPopupMenu has 
 * 'visible' bound property. When JPopupMenu becomes visible/invisible on
 * the screen it fires PropertyChangeEvents to its registered 
 * PropertyChangeListeners.
 */
public class JPopupMenu extends JComponent implements Accessible, MenuElement
{
  private static final long serialVersionUID = -8336996630009646009L;

  /* indicates if popup's menu border should be painted*/
  private boolean borderPainted = true;

  /** Flag indicating whether lightweight, mediumweight or heavyweight popup
     is used to display menu items.

     These are the possible cases:

     1. if DefaultLightWeightPopupEnabled true
         (i)  use lightweight container if popup feets inside top-level window
         (ii) only use heavyweight container (JWindow) if popup doesn't fit.

     2. if DefaultLightWeightPopupEnabled false
         (i) if popup fits, use awt.Panel (mediumWeight)
         (ii) if popup doesn't fit, use JWindow (heavyWeight)
  */
  private static boolean DefaultLightWeightPopupEnabled = true;

  /* Component that invokes popup menu. */
  transient Component invoker;

  /* Label for this popup menu. It is not used in most of the look and feel themes. */
  private String label;

  /*Amount of space between menuItem's in JPopupMenu and JPopupMenu's border */
  private Insets margin;

  /** Indicates whether ligthWeight container can be used to display popup
     menu. This flag is the same as DefaultLightWeightPopupEnabled, but setting
     this flag can change popup menu after creation of the object */
  private boolean lightWeightPopupEnabled;

  /** SelectionModel that keeps track of menu selection. */
  private SingleSelectionModel selectionModel;

  /* Popup that is used to display JPopupMenu */
  private transient Popup popup;

  /* Location of the popup */
  private Point popupLocation;

  /* Field indicating if popup menu is visible or not */
  private boolean visible = false;
  
  /**
   * Creates a new JPopupMenu object.
   */
  public JPopupMenu()
  {
    this(null);
  }

  /**
   * Creates a new JPopupMenu with specified label
   *
   * @param label Label for popup menu.
   */
  public JPopupMenu(String label)
  {
    lightWeightPopupEnabled = getDefaultLightWeightPopupEnabled();
    setLabel(label);
    setSelectionModel(new DefaultSingleSelectionModel());
    super.setVisible(false);
    updateUI();
  }

  private void readObject(ObjectInputStream stream)
                   throws IOException, ClassNotFoundException
  {
  }

  private void writeObject(ObjectOutputStream stream) throws IOException
  {
  }

  /**
  * Adds given menu item to the popup menu
  *
  * @param item menu item to add to the popup menu
  *
  * @return menu item that was added to the popup menu
  */
  public JMenuItem add(JMenuItem item)
  {
    this.insert(item, -1);
    return item;
  }

  /**
   * Constructs menu item with a specified label and adds it to
   * popup menu
   *
   * @param text label for the menu item to be added
   *
   * @return constructed menu item that was added to the popup menu
   */
  public JMenuItem add(String text)
  {
    JMenuItem item = new JMenuItem(text);
    return add(item);
  }

  /**
   * Constructs menu item associated with the specified action
   * and adds it to the popup menu
   *
   * @param action Action for the new menu item
   *
   * @return menu item that was added to the menu
   */
  public JMenuItem add(Action action)
  {
    JMenuItem item = createActionComponent(action);

    if (action != null)
      action.addPropertyChangeListener(createActionChangeListener(item));

    return add(item);
  }

  /**
   * Revomes component at the given index from the menu.
   *
   * @param index index of the component that will be removed in the menu
   */
  public void remove(int index)
  {
    super.remove(index);

    GridBagConstraints constraints = new GridBagConstraints();
    constraints.fill = GridBagConstraints.BOTH;
    constraints.weightx = 100.0;
    constraints.weighty = 100.0;

    Component[] items = getComponents();
    for (int i = index; i < items.length; i++)
      {
	constraints.gridy = i;
	super.add(items[i], constraints, i);
      }
  }

  /**
   * Create menu item associated with the given action
   * and inserts it into the popup menu at the specified index
   *
   * @param action Action for the new menu item
   * @param index index in the popup menu at which to insert new menu item.
   */
  public void insert(Action action, int index)
  {
    JMenuItem item = new JMenuItem(action);
    this.insert(item, index);
  }

  /**
   * Insert given component to the popup menu at the
   * specified index
   *
   * @param component Component to insert
   * @param index Index at which to insert given component
   */
  public void insert(Component component, int index)
  {
    GridBagConstraints constraints = new GridBagConstraints();
    constraints.fill = GridBagConstraints.BOTH;
    constraints.weightx = 100.0;
    constraints.weighty = 100.0;

    if (index == -1)
      index = getComponents().length;

    constraints.gridy = index;
    super.add(component, constraints, index);

    // need to change constraints for the components that were moved by 1
    // due to the insertion
    if (index != -1)
      {
	Component[] items = getComponents();

	for (int i = index + 1; i < items.length; i++)
	  {
	    constraints.gridy = i;
	    super.add(items[i], constraints, i);
	  }
      }
  }

  /**
   * Returns flag indicating if newly created JPopupMenu will use
   * heavyweight or lightweight container to display its menu items
   *
   * @return true if JPopupMenu will use lightweight container to display
   * menu items by default, and false otherwise.
   */
  public static boolean getDefaultLightWeightPopupEnabled()
  {
    return DefaultLightWeightPopupEnabled;
  }

  /**
   * Sets whether JPopupMenu should use ligthWeight container to
   * display it menu items by default
   *
   * @param enabled true if JPopupMenu should use lightweight container
   * for displaying its menu items, and false otherwise.
   */
  public static void setDefaultLightWeightPopupEnabled(boolean enabled)
  {
    DefaultLightWeightPopupEnabled = enabled;
  }

  /**
   * This method returns the UI used to display the JPopupMenu.
   *
   * @return The UI used to display the JPopupMenu.
   */
  public PopupMenuUI getUI()
  {
    return (PopupMenuUI) ui;
  }

  /**
   * Set the "UI" property of the menu item, which is a look and feel class
   * responsible for handling popupMenu's input events and painting it.
   *
   * @param ui The new "UI" property
   */
  public void setUI(PopupMenuUI ui)
  {
    super.setUI(ui);
  }

  /**
   * This method sets this menuItem's UI to the UIManager's default for the
   * current look and feel.
   */
  public void updateUI()
  {
    setUI((PopupMenuUI) UIManager.getUI(this));
    invalidate();
  }

  /**
   * This method returns a name to identify which look and feel class will be
   * the UI delegate for the menuItem.
   *
   * @return The Look and Feel classID. "PopupMenuUI"
   */
  public String getUIClassID()
  {
    return "PopupMenuUI";
  }

  /**
   * Returns selectionModel used by this popup menu to keep
   * track of the selection.
   *
   * @return popup menu's selection model
   */
  public SingleSelectionModel getSelectionModel()
  {
    return selectionModel;
  }

  /**
   * Sets selection model for this popup menu
   *
   * @param model new selection model of this popup menu
   */
  public void setSelectionModel(SingleSelectionModel model)
  {
	selectionModel = model;
  }

  /**
   * Creates new menu item associated with a given action.
   *
   * @param action Action used to create new menu item
   *
   * @return new created menu item associated with a given action.
   */
  protected JMenuItem createActionComponent(Action action)
  {
    return new JMenuItem(action);
  }

  /**
   * Creates PropertyChangeListener that listens to PropertyChangeEvents
   * occuring in the Action associated with given menu item in this popup menu.
   *
   * @param item MenuItem
   *
   * @return The PropertyChangeListener
   */
  protected PropertyChangeListener createActionChangeListener(JMenuItem item)
  {
    return new ActionChangeListener();
  }

  /**
   * Returns true if this popup menu will display its menu item in
   * a lightweight container and false otherwise.
   *
   * @return true if this popup menu will display its menu items
   * in a lightweight container and false otherwise.
   */
  public boolean isLightWeightPopupEnabled()
  {
    return lightWeightPopupEnabled;
  }

  /**
   * DOCUMENT ME!
   *
   * @param enabled DOCUMENT ME!
   */
  public void setLightWeightPopupEnabled(boolean enabled)
  {
    lightWeightPopupEnabled = enabled;
  }

  /**
   * Returns label for this popup menu
   *
   * @return label for this popup menu
   */
  public String getLabel()
  {
    return label;
  }

  /**
   * Sets label for this popup menu. This method fires PropertyChangeEvent
   * when the label property is changed. Please note that most
   * of the Look &amp; Feel will ignore this property.
   *
   * @param label label for this popup menu
   */
  public void setLabel(String label)
  {
    if (label != this.label)
      {
	String oldLabel = this.label;
	this.label = label;
	firePropertyChange("label", oldLabel, label);
      }
  }

  /**
   * Adds separator to this popup menu
   */
  public void addSeparator()
  {
    // insert separator at the end of the list of menu items    
    this.insert(new Separator(), -1);
  }

  /**
   * Adds popupMenuListener to listen for PopupMenuEvents fired
   * by the JPopupMenu
   *
   * @param listener PopupMenuListener to add to JPopupMenu
   */
  public void addPopupMenuListener(PopupMenuListener listener)
  {
    listenerList.add(PopupMenuListener.class, listener);
  }

  /**
   * Removes PopupMenuListener from JPopupMenu's list of listeners
   *
   * @param listener PopupMenuListener which needs to be removed
   */
  public void removePopupMenuListener(PopupMenuListener listener)
  {
    listenerList.remove(PopupMenuListener.class, listener);
  }

  /**
   * Returns array of PopupMenuListeners that are listening to JPopupMenu
   *
   * @return Array of PopupMenuListeners that are listening to JPopupMenu
   */
  public PopupMenuListener[] getPopupMenuListeners()
  {
    return ((PopupMenuListener[]) listenerList.getListeners(PopupMenuListener.class));
  }

  /**
   * This method calls popupMenuWillBecomeVisible() of popup menu's
   * PopupMenuListeners. This method is invoked just before popup menu
   * will appear on the screen.
   */
  protected void firePopupMenuWillBecomeVisible()
  {
    EventListener[] ll = listenerList.getListeners(PopupMenuListener.class);

    for (int i = 0; i < ll.length; i++)
      ((PopupMenuListener) ll[i]).popupMenuWillBecomeVisible(new PopupMenuEvent(this));
  }

  /**
   * This method calls popupMenuWillBecomeInvisible() of popup
   * menu's PopupMenuListeners. This method is invoked just before popup
   * menu will disappear from the screen
   */
  protected void firePopupMenuWillBecomeInvisible()
  {
    EventListener[] ll = listenerList.getListeners(PopupMenuListener.class);

    for (int i = 0; i < ll.length; i++)
      ((PopupMenuListener) ll[i]).popupMenuWillBecomeInvisible(new PopupMenuEvent(this));
  }

  /**
   * This method calls popupMenuCanceled() of popup menu's PopupMenuListeners.
   * This method is invoked just before popup menu is cancelled. This happens
   * when popup menu is closed without selecting any of its menu items. This
   * usually happens when the top-level window is resized or moved.
   */
  protected void firePopupMenuCanceled()
  {
    EventListener[] ll = listenerList.getListeners(PopupMenuListener.class);

    for (int i = 0; i < ll.length; i++)
      ((PopupMenuListener) ll[i]).popupMenuCanceled(new PopupMenuEvent(this));
  }

  /**
   * This methods sets popup menu's size to its' preferred size. If the
   * popup menu's size is previously set it will be ignored.
   */
  public void pack()
  {
    super.setSize(null);
  }

  /**
   * Return visibility of the popup menu
   *
   * @return true if popup menu is visible on the screen and false otherwise.
   */
  public boolean isVisible()
  {
    return visible;
  }

  /**
   * Sets visibility property of this popup menu. If the property is
   * set to true then popup menu will be dispayed and popup menu will
   * hide itself if visible property is set to false.
   *
   * @param visible true if popup menu will become visible and false otherwise.
   */
  public void setVisible(boolean visible)
  {
    if (visible == isVisible())
      return;

    boolean old = isVisible();
    this.visible = visible;
    if (old != isVisible())
      {
	firePropertyChange("visible", old, isVisible());
	if (visible)
	  {
	    firePopupMenuWillBecomeVisible();
	    Container rootContainer = (Container) SwingUtilities.getRoot(invoker);

	    boolean fit = true;
	    Dimension size;

	    // Determine the size of the popup menu
	    if (this.getSize().width == 0 && this.getSize().width == 0)
	      size = this.getPreferredSize();
	    else
	      size = this.getSize();

	    if ((size.width > (rootContainer.getWidth() - popupLocation.x))
	        || (size.height > (rootContainer.getHeight() - popupLocation.y)))
	      fit = false;
	    if (lightWeightPopupEnabled && fit)
	      popup = new LightWeightPopup(this);
	    else
	      {
		if (fit)
		  popup = new MediumWeightPopup(this);
		else
		  popup = new HeavyWeightPopup(this);
	      }
	    if (popup instanceof LightWeightPopup
	        || popup instanceof MediumWeightPopup)
	      {
		JLayeredPane layeredPane;
		layeredPane = SwingUtilities.getRootPane(invoker)
		                            .getLayeredPane();
		Point p = new Point(popupLocation.x, popupLocation.y);
		SwingUtilities.convertPointFromScreen(p, layeredPane);
		popup.show(p.x, p.y, size.width, size.height);  
	      }
	    else
	      {
		// Subtract insets of the top-level container if popup menu's
		// top-left corner is inside it.
		Insets insets = rootContainer.getInsets();
		popup.show(popupLocation.x - insets.left,
		           popupLocation.y - insets.top, size.width,
		           size.height);
	      }
	  }
	else
	  {
	    firePopupMenuWillBecomeInvisible();
	    popup.hide();
	  }
      }
  }

  /**
   * Sets location of the popup menu.
   *
   * @param x X coordinate of the popup menu's location
   * @param y Y coordinate of the popup menu's location
   */
  public void setLocation(int x, int y)
  {
    if (popupLocation == null)
      popupLocation = new Point();

    popupLocation.x = x;
    popupLocation.y = y;
  }

  /**
   * Returns popup menu's invoker.
   *
   * @return popup menu's invoker
   */
  public Component getInvoker()
  {
    return invoker;
  }

  /**
   * Sets popup menu's invoker.
   *
   * @param component The new invoker of this popup menu
   */
  public void setInvoker(Component component)
  {
    invoker = component;
  }

  /**
   * This method displays JPopupMenu on the screen at the specified
   * location. Note that x and y coordinates given to this method
   * should be expressed in terms of the popup menus' invoker.
   *
   * @param component Invoker for this popup menu
   * @param x x-coordinate of the popup menu relative to the specified invoker
   * @param y y-coordiate of the popup menu relative to the specified invoker
   */
  public void show(Component component, int x, int y)
  {
    setInvoker(component);
    Point p = new Point(x, y);
    SwingUtilities.convertPointToScreen(p, component);
    setLocation(p.x, p.y);
    setVisible(true);
  }

  /**
   * Returns component located at the specified index in the popup menu
   *
   * @param index index of the component to return
   *
   * @return component located at the specified index in the popup menu
   *
   * @deprecated Replaced by getComponent(int)
   */
  public Component getComponentAtIndex(int index)
  {
    return getComponent(index);
  }

  /**
   * Returns index of the specified component in the popup menu
   *
   * @param component Component to look for
   *
   * @return index of the specified component in the popup menu
   */
  public int getComponentIndex(Component component)
  {
    Component[] items = getComponents();

    for (int i = 0; i < items.length; i++)
      {
	if (items[i].equals(component))
	  return i;
      }

    return -1;
  }

  /**
   * Sets size of the popup
   *
   * @param size Dimensions representing new size of the popup menu
   */
  public void setPopupSize(Dimension size)
  {
    super.setSize(size);
  }

  /**
   * Sets size of the popup menu
   *
   * @param width width for the new size
   * @param height height for the new size
   */
  public void setPopupSize(int width, int height)
  {
    super.setSize(width, height);
  }

  /**
   * Selects specified component in this popup menu.
   *
   * @param selected component to select
   */
  public void setSelected(Component selected)
  {
    int index = getComponentIndex(selected);
    selectionModel.setSelectedIndex(index);
  }

  /**
   * Checks if this popup menu paints its border.
   *
   * @return true if this popup menu paints its border and false otherwise.
   */
  public boolean isBorderPainted()
  {
    return borderPainted;
  }

  /**
   * Sets if the border of the popup menu should be
   * painter or not.
   *
   * @param painted true if the border should be painted and false otherwise
   */
  public void setBorderPainted(boolean painted)
  {
    borderPainted = painted;
  }

  /**
   * Returns margin for this popup menu.
   *
   * @return margin for this popup menu.
   */
  public Insets getMargin()
  {
    return margin;
  }

  /**
   * A string that describes this JPopupMenu. Normally only used
   * for debugging.
   *
   * @return A string describing this JMenuItem
   */
  protected String paramString()
  {
    StringBuffer sb = new StringBuffer();
    sb.append(super.paramString());
    sb.append(",label=");
    if (getLabel() != null)
      sb.append(getLabel());
    sb.append(",lightWeightPopupEnabled=").append(isLightWeightPopupEnabled());
    sb.append(",margin=");
    if (getMargin() != null)
      sb.append(margin);
    sb.append(",paintBorder=").append(isBorderPainted());
    return sb.toString();
  }

  /**
  * Process mouse events forwarded from MenuSelectionManager. This method 
  * doesn't do anything. It is here to conform to the MenuElement interface.
  *
  * @param event event forwarded from MenuSelectionManager
  * @param path path to the menu element from which event was generated
  * @param manager MenuSelectionManager for the current menu hierarchy
  */
  public void processMouseEvent(MouseEvent event, MenuElement[] path,
                                MenuSelectionManager manager)
  {
    // Empty Implementation. This method is needed for the implementation
    // of MenuElement interface
  }

  /**
   * Process key events forwarded from MenuSelectionManager. This method
   * doesn't do anything. It is here to conform to the MenuElement interface.
   *
   * @param event event forwarded from MenuSelectionManager
   * @param path path to the menu element from which event was generated
   * @param manager MenuSelectionManager for the current menu hierarchy
   *
   */
  public void processKeyEvent(KeyEvent event, MenuElement[] path,
                              MenuSelectionManager manager)
  {
    // Empty Implementation. This method is needed for the implementation
    // of MenuElement interface
  }

  /**
   * Method of MenuElement Interface. It is invoked when
   * popupMenu's selection has changed
   *
   * @param changed true if this popupMenu is part of current menu
   * hierarchy and false otherwise.
   */
  public void menuSelectionChanged(boolean changed)
  {
    if (! changed)
      setVisible(false);
  }

  /**
   * Return subcomonents of this popup menu. This method returns only
   * components that implement the <code>MenuElement</code> interface.
   *
   * @return array of menu items belonging to this popup menu
   */
  public MenuElement[] getSubElements()
  {
    Component[] items = getComponents();
    ArrayList subElements = new ArrayList();

    for (int i = 0; i < items.length; i++)
      if (items[i] instanceof MenuElement)
	subElements.add(items[i]);

    return (MenuElement[])
      subElements.toArray(new MenuElement[subElements.size()]);
  }

  /**
   * Method of the MenuElement interface. Returns reference to itself.
   *
   * @return Returns reference to itself
   */
  public Component getComponent()
  {
    return this;
  }

  /**
   * Checks if observing mouse event should trigger popup
   * menu to show on the screen.
   *
   * @param event MouseEvent to check
   *
   * @return true if the observing mouse event is popup trigger and false otherwise
   */
  public boolean isPopupTrigger(MouseEvent event)
  {
    return ((PopupMenuUI) getUI()).isPopupTrigger(event);
  }

  /**
   * DOCUMENT ME!
   *
   * @return DOCUMENT ME!
   */
  public AccessibleContext getAccessibleContext()
  {
    if (accessibleContext == null)
      accessibleContext = new AccessibleJPopupMenu();

    return accessibleContext;
  }

  /**
   * This interface is used to display menu items of the JPopupMenu
   */
  private interface Popup
  {
    /**
     * Displays container on the screen
     *
     * @param x x-coordinate of popup menu's location on the screen
     * @param y y-coordinate of popup menu's location on the screen
     * @param width width of the container that is used to display menu
     * item's for popup menu
     * @param height height of the container that is used to display menu
     * item's for popup menu
     */
    void show(int x, int y, int width, int height);

    /**
     * Hides container used to display popup menu item's from the screen
     */
    void hide();
  }

  /**
   * This class represents Popup menu that uses light weight container
   * to display its contents.
   */
  private class LightWeightPopup extends Container implements Popup
  {
    /**
     * Creates a new LightWeightPopup menu
     *
     * @param c Container containing menu items
     */
    private Component c;

    public LightWeightPopup(Container c)
    {
      this.c = c;
    }

    /**
     * Displayes lightweight container with menu items to the screen
     *
     * @param x x-coordinate of lightweight container on the screen
     * @param y y-coordinate of lightweight container on the screen
     * @param width width of the lightweight container
     * @param height height of the lightweight container
     */
    public void show(int x, int y, int width, int height)
    {
      JLayeredPane layeredPane;
      layeredPane = SwingUtilities.getRootPane(invoker).getLayeredPane();
      c.setBounds(x, y, width, height);
      layeredPane.add(c, JLayeredPane.POPUP_LAYER, 0);
    }

    /**
     * Hides lightweight container from the screen
     */
    public void hide()
    {
      // FIXME: Right now the lightweight container is removed from JLayered
      // pane. It is probably would be better in order to improve performance
      // to make the container invisible instead of removing it everytime.
      JLayeredPane layeredPane;
      layeredPane = SwingUtilities.getRootPane(invoker).getLayeredPane();
      int index = layeredPane.getIndexOf(c);
      layeredPane.remove(index);
    }
  }

  /**
   * MediumWeightPopup is an AWT Panel with JPopupMenu's menu items.
   * It is used to display JPopupMenu's menu items on the screen
   */
  private class MediumWeightPopup extends Panel implements Popup
  {
    /**
     * Creates a new MediumWeightPopup object.
     *
     * @param c Container with JPopupMenu's menu items
     */
    public MediumWeightPopup(Container c)
    {
      this.add(c);
    }

    /**
     * Displays AWT Panel with its components on the screen
     *
     * @param x x-coordinate of the upper-left corner of the panel's
     * @param y y-coordinate of the upper-left corner of the panel's
     * @param width width of the panel
     * @param height height of the panel
     */
    public void show(int x, int y, int width, int height)
    {
      JLayeredPane layeredPane;
      layeredPane = SwingUtilities.getRootPane(invoker).getLayeredPane();
      layeredPane.add(this, JLayeredPane.POPUP_LAYER, 0);
      this.setBounds(x, y, width, height);
    }

    /**
     * Hides This panel from the screen
     */
    public void hide()
    {
      // FIXME: Right now the lightweight container is removed from JLayered
      // pane. It is probably would be better in order to improve performance
      // to make the container invisible instead of removing it everytime.
      JLayeredPane layeredPane;
      layeredPane = SwingUtilities.getRootPane(invoker).getLayeredPane();
      int index = layeredPane.getIndexOf(this);
      layeredPane.remove(index);
    }
  }

  /**
   * HeavyWeightPopup is JWindow that is used to display JPopupMenu menu item's
   * on the screen
   */
  private class HeavyWeightPopup extends JWindow implements Popup
  {
    /**
     * Creates a new HeavyWeightPopup object.
     *
     * @param c Container containing menu items
     */
    public HeavyWeightPopup(Container c)
    {
      this.setContentPane(c);
    }

    /**
     * Displays JWindow container JPopupMenu's menu items to the screen
     *
     * @param x x-coordinate of JWindow containing menu items
     * @param y y-coordinate of JWindow containing menu items
     * @param width width of the JWindow
     * @param height height of the JWindow
     */
    public void show(int x, int y, int width, int height)
    {
      this.setBounds(x, y, width, height);
      this.show();
    }
  }

  /**
   * This is the separator that can be used in popup menu.
   */
  public static class Separator extends JSeparator
  {
    public Separator()
    {
    }

    public String getUIClassID()
    {
      return "PopupMenuSeparatorUI";
    }
  }

  protected class AccessibleJPopupMenu extends AccessibleJComponent
  {
    private static final long serialVersionUID = 7423261328879849768L;

    protected AccessibleJPopupMenu()
    {
    }

    public AccessibleRole getAccessibleRole()
    {
      return AccessibleRole.POPUP_MENU;
    }
  }

  /* This class resizes popup menu and repaints popup menu appropriately if one
   of item's action has changed */
  protected class ActionChangeListener implements PropertyChangeListener
  {
    public void propertyChange(PropertyChangeEvent evt)
    {
      JPopupMenu.this.revalidate();
      JPopupMenu.this.repaint();
    }
  }
}
