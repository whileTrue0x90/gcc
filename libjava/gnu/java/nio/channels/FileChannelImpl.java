/* FileChannelImpl.java -- 
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


package gnu.java.nio.channels;

import gnu.classpath.Configuration;
import gnu.gcj.RawData;
import gnu.java.nio.FileLockImpl;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.MappedByteBuffer;
import java.nio.channels.*;

/**
 * This file is not user visible !
 * But alas, Java does not have a concept of friendly packages
 * so this class is public. 
 * Instances of this class are created by invoking getChannel
 * Upon a Input/Output/RandomAccessFile object.
 */

public class FileChannelImpl extends FileChannel
{
  int mode;
  // These are WHENCE values for seek.
  static final int SET = 0;
  static final int CUR = 1;

  // These are mode values for open().
  static final int READ   = 1;
  static final int WRITE  = 2;
  static final int APPEND = 4;

  // EXCL is used only when making a temp file.
  static final int EXCL   = 8;
  static final int SYNC   = 16;
  static final int DSYNC  = 32;

  /**
   * This is the actual native file descriptor value
   */
  // System's notion of file descriptor.  It might seem redundant to
  // initialize this given that it is reassigned in the constructors.
  // However, this is necessary because if open() throws an exception
  // we want to make sure this has the value -1.  This is the most
  // efficient way to accomplish that.
  private int fd = -1;

  int length;
  private long pos;

  public FileChannelImpl ()
  {
  }

  /* Open a file.  MODE is a combination of the above mode flags. */
  public FileChannelImpl (String path, int mode) throws FileNotFoundException
  {
    fd = open (path, mode);
    this.mode = mode;
  }

  private static native void init();
  static { init (); }

  public static FileChannelImpl in;
  public static FileChannelImpl out;
  public static FileChannelImpl err;

  private native int open (String path, int mode) throws FileNotFoundException;

  /** Attach to an already-opened file.  */
  public FileChannelImpl (int desc, int mode)
  {
    fd = desc;
    this.mode = mode;
  }

  native int available () throws IOException;
  private native long implPosition ();
  private native void seek (long newPosition);
  private native void implTruncate (long size);
  
  public native void unlock (long pos, long len);

  public native long size () throws IOException;
    
  protected native void implCloseChannel() throws IOException;

  public int read (ByteBuffer dst) throws IOException
  {
    return implRead (dst);
  }

  public int read (ByteBuffer dst, long position)
    throws IOException
  {
    if (position < 0)
      throw new IllegalArgumentException ();
    long oldPosition = implPosition ();
    position (position);
    int result = implRead (dst);
    position (oldPosition);
    
    return result;
  }

  private int implRead (ByteBuffer dst) throws IOException
  {
    int result;
    byte[] buffer = new byte [dst.remaining ()];
    
    result = read (buffer, 0, buffer.length);

    if (result > 0)
      dst.put (buffer, 0, result);

    return result;
  }
  
  public native int read ()
    throws IOException;

  public native int read (byte[] buffer, int offset, int length)
    throws IOException;

  public long read (ByteBuffer[] dsts, int offset, int length)
    throws IOException
  {
    long result = 0;

    for (int i = offset; i < offset + length; i++)
      {
        result += read (dsts [i]);
      }

    return result;
  }

  public int write (ByteBuffer src) throws IOException
  {
    return implWrite (src);
  }
    
  public int write (ByteBuffer src, long position)
    throws IOException
  {
    if (position < 0)
      throw new IllegalArgumentException ();

    if (!isOpen ())
      throw new ClosedChannelException ();
    
    if ((mode & WRITE) == 0)
       throw new NonWritableChannelException ();

    int result;
    long oldPosition;

    oldPosition = implPosition ();
    seek (position);
    result = implWrite (src);
    seek (oldPosition);
    
    return result;
  }

  private int implWrite (ByteBuffer src) throws IOException
  {
    int len = src.remaining ();
    if (src.hasArray())
      {
	byte[] buffer = src.array();
	write(buffer, src.arrayOffset() + src.position(), len);
      }
    else
      {
	// Use a more efficient native method! FIXME!
	byte[] buffer = new byte [len];
    	src.get (buffer, 0, len);
	write (buffer, 0, len);
      }
    return len;
  }
  
  public native void write (byte[] buffer, int offset, int length)
    throws IOException;
  
  public native void write (int b) throws IOException;

  public long write(ByteBuffer[] srcs, int offset, int length)
    throws IOException
  {
    long result = 0;

    for (int i = offset;i < offset + length;i++)
      {
        result += write (srcs[i]);
      }
    
    return result;
  }
				   
  public native MappedByteBuffer mapImpl (char mode, long position, int size)
    throws IOException;

  public MappedByteBuffer map (FileChannel.MapMode mode,
			       long position, long size)
    throws IOException
  {
    char nmode = 0;
    if (mode == MapMode.READ_ONLY)
      {
	nmode = 'r';
	if ((this.mode & READ) == 0)
	  throw new NonReadableChannelException();
      }
    else if (mode == MapMode.READ_WRITE || mode == MapMode.PRIVATE)
      {
	nmode = mode == MapMode.READ_WRITE ? '+' : 'c';
	if ((this.mode & (READ|WRITE)) != (READ|WRITE))
	  throw new NonWritableChannelException();
      }
    else
      throw new IllegalArgumentException ();
    
    if (position < 0 || size < 0 || size > Integer.MAX_VALUE)
      throw new IllegalArgumentException ();
    return mapImpl(nmode, position, (int) size);
  }

  /**
   * msync with the disk
   */
  public void force (boolean metaData) throws IOException
  {
    if (!isOpen ())
      throw new ClosedChannelException ();
  }

  public long transferTo (long position, long count, WritableByteChannel target)
    throws IOException
  {
    if (position < 0
        || count < 0)
      throw new IllegalArgumentException ();

    if (!isOpen ())
      throw new ClosedChannelException ();

    if ((mode & READ) == 0)
       throw new NonReadableChannelException ();
   
    // XXX: count needs to be casted from long to int. Dataloss ?
    ByteBuffer buffer = ByteBuffer.allocate ((int) count);
    read (buffer, position);
    buffer.flip();
    return target.write (buffer);
  }

  public long transferFrom (ReadableByteChannel src, long position, long count)
    throws IOException
  {
    if (position < 0
        || count < 0)
      throw new IllegalArgumentException ();

    if (!isOpen ())
      throw new ClosedChannelException ();

    if ((mode & WRITE) == 0)
       throw new NonWritableChannelException ();

    // XXX: count needs to be casted from long to int. Dataloss ?
    ByteBuffer buffer = ByteBuffer.allocate ((int) count);
    src.read (buffer);
    buffer.flip();
    return write (buffer, position);
  }

  public FileLock tryLock (long position, long size, boolean shared)
    throws IOException
  {
    if (position < 0
        || size < 0)
      throw new IllegalArgumentException ();

    if (!isOpen ())
      throw new ClosedChannelException ();

    if (shared && (mode & READ) == 0)
      throw new NonReadableChannelException ();
	
    if (!shared && (mode & WRITE) == 0)
      throw new NonWritableChannelException ();
	
    boolean completed = false;
    
    try
      {
	begin();
        lock(position, size, shared, true);
	completed = true;
	return new FileLockImpl(this, position, size, shared);
      }
    finally
      {
	end(completed);
      }
  }

  /** Try to acquire a lock at the given position and size.
   * On success return true.
   * If wait as specified, block until we can get it.
   * Otherwise return false.
   */
  private native boolean lock(long position, long size,
			      boolean shared, boolean wait);
  
  public FileLock lock (long position, long size, boolean shared)
    throws IOException
  {
    if (position < 0
        || size < 0)
      throw new IllegalArgumentException ();

    if (!isOpen ())
      throw new ClosedChannelException ();

    boolean completed = false;

    try
      {
	boolean lockable = lock(position, size, shared, false);
	completed = true;
	return (lockable
		? new FileLockImpl(this, position, size, shared)
		: null);
      }
    finally
      {
	end(completed);
      }
  }

  public long position ()
    throws IOException
  {
    if (!isOpen ())
      throw new ClosedChannelException ();

    return implPosition ();
  }
  
  public FileChannel position (long newPosition)
    throws IOException
  {
    if (newPosition < 0)
      throw new IllegalArgumentException ();

    if (!isOpen ())
      throw new ClosedChannelException ();

    // FIXME note semantics if seeking beyond eof.
    // We should seek lazily - only on a write.
    seek (newPosition);
    return this;
  }
  
  public FileChannel truncate (long size)
    throws IOException
  {
    if (size < 0)
      throw new IllegalArgumentException ();

    if (!isOpen ())
      throw new ClosedChannelException ();

    if ((mode & WRITE) == 0)
       throw new NonWritableChannelException ();

    implTruncate (size);
    return this;
  }
}
