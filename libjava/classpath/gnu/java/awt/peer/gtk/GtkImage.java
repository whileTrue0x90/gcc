/* GtkImage.java
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


package gnu.java.awt.peer.gtk;

import java.awt.Graphics;
import java.awt.Color;
import java.awt.Image;
import java.awt.image.ColorModel;
import java.awt.image.DirectColorModel;
import java.awt.image.MemoryImageSource;
import java.awt.image.ImageConsumer;
import java.awt.image.ImageObserver;
import java.awt.image.ImageProducer;
import java.io.File;
import java.io.IOException;
import java.util.Hashtable;
import java.util.Vector;
import java.io.ByteArrayOutputStream;
import java.io.BufferedInputStream;
import java.net.URL;
import gnu.classpath.Pointer;

/**
 * GtkImage - wraps a GdkPixbuf or GdkPixmap.
 *
 * The constructor GtkImage(int, int) creates an 'off-screen' GdkPixmap,
 * this can be drawn to (it's a GdkDrawable), and correspondingly, you can
 * create a GdkGraphics object for it. 
 *
 * This corresponds to the Image implementation returned by 
 * Component.createImage(int, int). 
 *
 * A GdkPixbuf is 'on-screen' and the gdk cannot draw to it,
 * this is used for the other constructors (and other createImage methods), and
 * corresponds to the Image implementations returned by the Toolkit.createImage
 * methods, and is basically immutable. 
 *
 * @author Sven de Marothy
 */
public class GtkImage extends Image
{
  int width = -1, height = -1;

  /**
   * Properties.
   */
  Hashtable props;

  /**
   * Loaded or not flag, for asynchronous compatibility.
   */
  boolean isLoaded;

  /**
   * Pointer to the GdkPixbuf
   */
  Pointer pixmap;

  /**
   * Observer queue.
   */
  Vector observers;

  /**
   * If offScreen is set, a GdkBitmap is wrapped and not a Pixbuf.
   */
  boolean offScreen;

  /**
   * Error flag for loading.
   */
  boolean errorLoading;

  /**
   * Original source, if created from an ImageProducer.
   */
  ImageProducer source;

  /*
   * The 32-bit AABBGGRR format the GDK uses.
   */
  static ColorModel nativeModel = new DirectColorModel(32, 
						       0x000000FF,
						       0x0000FF00,
						       0x00FF0000,
						       0xFF000000);

  /**
   * Returns a copy of the pixel data as a java array.
   */
  private native int[] getPixels();

  /**
   * Sets the pixel data from a java array.
   */
  private native void setPixels(int[] pixels);

  /**
   * Loads an image using gdk-pixbuf from a file.
   */
  private native boolean loadPixbuf(String name);

  /**
   * Loads an image using gdk-pixbuf from data.
   */
  private native boolean loadImageFromData(byte[] data);

  /**
   * Allocates a Gtk Pixbuf or pixmap
   */
  private native void createPixmap();

  /**
   * Frees the above.
   */
  private native void freePixmap();

  /**
   * Sets the pixmap to scaled copy of src image. hints are rendering hints.
   */
  private native void createScaledPixmap(GtkImage src, int hints);

  /**
   * Draws the image, optionally scaled and composited.
   */
  private native void drawPixelsScaled (GdkGraphics gc, 
					int bg_red, int bg_green, int bg_blue, 
					int x, int y, int width, int height, 
					boolean composite);

  /**
   * Draws the image, optionally scaled flipped and composited.
   */
  private native void drawPixelsScaledFlipped (GdkGraphics gc, 
					       int bg_red, int bg_green, 
					       int bg_blue, 
					       boolean flipX, boolean flipY,
					       int srcX, int srcY,
					       int srcWidth, int srcHeight,
					       int dstX, int dstY,
					       int dstWidth, int dstHeight,
					       boolean composite);

  /**
   * Constructs a GtkImage from an ImageProducer. Asynchronity is handled in
   * the following manner: 
   * A GtkImageConsumer gets the image data, and calls setImage() when 
   * completely finished. The GtkImage is not considered loaded until the
   * GtkImageConsumer is completely finished. We go for all "all or nothing".
   */
  public GtkImage (ImageProducer producer)
  {
    isLoaded = false;
    observers = new Vector();
    source = producer;
    errorLoading = false;
    source.startProduction(new GtkImageConsumer(this, source));
    offScreen = false;
  }

  /**
   * Constructs a blank GtkImage.  This is called when
   * GtkToolkit.createImage (String) is called with an empty string
   * argument ("").  A blank image is loaded immediately upon
   * construction and has width -1 and height -1.
   */
  public GtkImage ()
  {
    isLoaded = true;
    observers = null;
    offScreen = false;
    props = new Hashtable();
    errorLoading = false;
  }

  /**
   * Constructs a GtkImage by loading a given file.
   *
   * @throws IllegalArgumentException if the image could not be loaded.
   */
  public GtkImage (String filename)
  {
    File f = new File(filename);
    try
      {
	if (loadPixbuf(f.getCanonicalPath()) != true)
	  throw new IllegalArgumentException("Couldn't load image: "+filename);
      } 
    catch(IOException e)
      {
	  throw new IllegalArgumentException("Couldn't load image: "+filename);
      }

    isLoaded = true;
    observers = null;
    offScreen = false;
    props = new Hashtable();
  }

  /**
   * Constructs a GtkImage from a byte array of an image file.
   *
   * @throws IllegalArgumentException if the image could not be
   * loaded.
   */
  public GtkImage (byte[] data)
  {
    if (loadImageFromData (data) != true)
      throw new IllegalArgumentException ("Couldn't load image.");

    isLoaded = true;
    observers = null;
    offScreen = false;
    props = new Hashtable();
    errorLoading = false;
  }

  /**
   * Constructs a GtkImage from a URL. May result in an error image.
   */
  public GtkImage (URL url)
  {
    isLoaded = false;
    observers = new Vector();
    errorLoading = false;
    if( url == null)
      return;
    ByteArrayOutputStream baos = new ByteArrayOutputStream (5000);
    try
      {
        BufferedInputStream bis = new BufferedInputStream (url.openStream());

        byte[] buf = new byte[5000];
        int n = 0;

        while ((n = bis.read(buf)) != -1)
	  baos.write(buf, 0, n); 
        bis.close();
      }
    catch(IOException e)
      {
	throw new IllegalArgumentException ("Couldn't load image.");
      }
    if (loadImageFromData (baos.toByteArray()) != true)
      throw new IllegalArgumentException ("Couldn't load image.");

    isLoaded = true;
    observers = null;
    props = new Hashtable();
  }

  /**
   * Constructs an empty GtkImage.
   */
  public GtkImage (int width, int height)
  {
    this.width = width;
    this.height = height;
    props = new Hashtable();
    isLoaded = true;
    observers = null;
    offScreen = true;
    createPixmap();
  }

  /**
   * Constructs a scaled version of the src bitmap, using the GDK.
   */
  private GtkImage (GtkImage src, int width, int height, int hints)
  {
    this.width = width;
    this.height = height;
    props = new Hashtable();
    isLoaded = true;
    observers = null;
    offScreen = false;

    // Use the GDK scaling method.
    createScaledPixmap(src, hints);
  }

  /**
   * Package private constructor to create a GtkImage from a given
   * PixBuf pointer.
   */
  GtkImage (Pointer pixbuf)
  {
    pixmap = pixbuf;
    createFromPixbuf();
    isLoaded = true;
    observers = null;
    offScreen = false;
    props = new Hashtable();
  }

  // The singleton GtkImage that is returned on errors by GtkToolkit.
  private static GtkImage errorImage;

  /**
   * Returns an empty GtkImage with the errorLoading flag set.
   * Called from GtkToolKit when some error occured, but an image needs
   * to be returned anyway.
   */
  static synchronized GtkImage getErrorImage()
  {
    if (errorImage == null)
      {
	errorImage = new GtkImage();
	errorImage.errorLoading = true;
      }
    return errorImage;
  }

  /**
   * Native helper function for constructor that takes a pixbuf Pointer.
   */
  private native void createFromPixbuf();

  /**
   * Callback from the image consumer.
   */
  public void setImage(int width, int height, 
		       int[] pixels, Hashtable properties)
  {
    this.width = width;
    this.height = height;
    props = (properties != null) ? properties : new Hashtable();

    if (width <= 0 || height <= 0 || pixels == null)
      {
	errorLoading = true;
	return;
      }

    isLoaded = true;
    deliver();
    createPixmap();
    setPixels(pixels);
  }

  // java.awt.Image methods ////////////////////////////////////////////////

  public synchronized int getWidth (ImageObserver observer)
  {
    if (addObserver(observer))
      return -1;

    return width;
  }
  
  public synchronized int getHeight (ImageObserver observer)
  {
    if (addObserver(observer))
      return -1;
    
    return height;
  }

  public synchronized Object getProperty (String name, ImageObserver observer)
  {
    if (addObserver(observer))
      return UndefinedProperty;
    
    Object value = props.get (name);
    return (value == null) ? UndefinedProperty : value;
  }

  /**
   * Returns the source of this image.
   */
  public ImageProducer getSource ()
  {
    if (!isLoaded)
      return null;
    return new MemoryImageSource(width, height, nativeModel, getPixels(), 
				 0, width);
  }

  /**
   * Creates a GdkGraphics context for this pixmap.
   */
  public Graphics getGraphics ()
  {
    if (!isLoaded) 
      return null;
    if (offScreen)
      return new GdkGraphics(this);
    else
      throw new IllegalAccessError("This method only works for off-screen"
				   +" Images.");
  }
  
  /**
   * Returns a scaled instance of this pixmap.
   */
  public Image getScaledInstance(int width,
				 int height,
				 int hints)
  {
    if (width <= 0 || height <= 0)
      throw new IllegalArgumentException("Width and height of scaled bitmap"+
					 "must be >= 0");

    return new GtkImage(this, width, height, hints);
  }

  /**
   * If the image is loaded and comes from an ImageProducer, 
   * regenerate the image from there.
   *
   * I have no idea if this is ever actually used. Since GtkImage can't be
   * instantiated directly, how is the user to know if it was created from
   * an ImageProducer or not?
   */
  public synchronized void flush ()
  {
    if (isLoaded && source != null)
      {
	observers = new Vector();
	isLoaded = false;
	freePixmap();
	source.startProduction(new GtkImageConsumer(this, source));
      }
  }

  public void finalize()
  {
    if (isLoaded)
      freePixmap();
  }

  /**
   * Returns the image status, used by GtkToolkit
   */
  public int checkImage (ImageObserver observer)
  {
    if (addObserver(observer))
      {
	if (errorLoading == true)
	  return ImageObserver.ERROR;
	else
	  return 0;
      }

    return ImageObserver.ALLBITS | ImageObserver.WIDTH | ImageObserver.HEIGHT;
  }

  // Drawing methods ////////////////////////////////////////////////

  /**
   * Draws an image with eventual scaling/transforming.
   */
  public boolean drawImage (GdkGraphics g, int dx1, int dy1, int dx2, int dy2, 
			    int sx1, int sy1, int sx2, int sy2, 
			    Color bgcolor, ImageObserver observer)
  {
    if (addObserver(observer))
      return false;

    boolean flipX = (dx1 > dx2)^(sx1 > sx2);
    boolean flipY = (dy1 > dy2)^(sy1 > sy2);
    int dstWidth = Math.abs (dx2 - dx1);
    int dstHeight = Math.abs (dy2 - dy1);
    int srcWidth = Math.abs (sx2 - sx1);
    int srcHeight = Math.abs (sy2 - sy1);
    int srcX = (sx1 < sx2) ? sx1 : sx2;
    int srcY = (sy1 < sy2) ? sy1 : sy2;
    int dstX = (dx1 < dx2) ? dx1 : dx2;
    int dstY = (dy1 < dy2) ? dy1 : dy2;

    // Clipping. This requires the dst to be scaled as well, 
    if (srcWidth > width)
      {
	dstWidth = (int)((double)dstWidth*((double)width/(double)srcWidth));
	srcWidth = width - srcX;
      }

    if (srcHeight > height) 
      {
	dstHeight = (int)((double)dstHeight*((double)height/(double)srcHeight));
	srcHeight = height - srcY;
      }

    if (srcWidth + srcX > width)
      {
	dstWidth = (int)((double)dstWidth * (double)(width - srcX)/(double)srcWidth);
	srcWidth = width - srcX;
      }

    if (srcHeight + srcY > height)
      {
	dstHeight = (int)((double)dstHeight * (double)(width - srcY)/(double)srcHeight);
	srcHeight = height - srcY;
      }

    if ( srcWidth <= 0 || srcHeight <= 0 || dstWidth <= 0 || dstHeight <= 0)
      return true;

    if(bgcolor != null)
      drawPixelsScaledFlipped (g, bgcolor.getRed (), bgcolor.getGreen (), 
			       bgcolor.getBlue (), 
			       flipX, flipY,
			       srcX, srcY,
			       srcWidth, srcHeight,
			       dstX,  dstY,
			       dstWidth, dstHeight,
			       true);
    else
      drawPixelsScaledFlipped (g, 0, 0, 0, flipX, flipY,
			       srcX, srcY, srcWidth, srcHeight,
			       dstX,  dstY, dstWidth, dstHeight,
			       false);
    return true;
  }

  /**
   * Draws an image to the GdkGraphics context, at (x,y) scaled to 
   * width and height, with optional compositing with a background color.
   */
  public boolean drawImage (GdkGraphics g, int x, int y, int width, int height,
			    Color bgcolor, ImageObserver observer)
  {
    if (addObserver(observer))
      return false;

    if(bgcolor != null)
      drawPixelsScaled(g, bgcolor.getRed (), bgcolor.getGreen (), 
		       bgcolor.getBlue (), x, y, width, height, true);
    else
      drawPixelsScaled(g, 0, 0, 0, x, y, width, height, false);

    return true;
  }

  // Private methods ////////////////////////////////////////////////

  /**
   * Delivers notifications to all queued observers.
   */
  private void deliver()
  {
    int flags = ImageObserver.HEIGHT | 
      ImageObserver.WIDTH |
      ImageObserver.PROPERTIES |
      ImageObserver.ALLBITS;

    if (observers != null)
      for(int i=0; i < observers.size(); i++)
	((ImageObserver)observers.elementAt(i)).
	  imageUpdate(this, flags, 0, 0, width, height);

    observers = null;
  }
  
  /**
   * Adds an observer, if we need to.
   * @return true if an observer was added.
   */
  private boolean addObserver(ImageObserver observer)
  {
    if (!isLoaded)
      {
	if(observer != null)
	  if (!observers.contains (observer))
	    observers.addElement (observer);
	return true;
      }
    return false;
  }
}
