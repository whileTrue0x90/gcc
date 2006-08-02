/* RemoteServer.java --
   Copyright (c) 1996, 1997, 1998, 1999, 2004, 2006
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


package java.rmi.server;

import gnu.java.rmi.server.RMIIncomingThread;

import java.io.OutputStream;
import java.io.PrintStream;

/**
 * A common superclass for the server implementations.
 */
public abstract class RemoteServer
    extends RemoteObject
{
  private static final long serialVersionUID = - 4100238210092549637L;
  
  /**
   * Does nothing, delegates to super().
   */
  protected RemoteServer()
  {
    super();
  }
  
  /**
   * Does nothing, delegates to super(ref).
   */
  protected RemoteServer(RemoteRef ref)
  {
    super(ref);
  }
  
  /**
   * Get the host of the calling client. The current thread must be an instance
   * of the {@link RMIIncomingThread}.
   * 
   * @return the client host address
   * 
   * @throws ServerNotActiveException if the current thread is not an instance
   * of the RMIIncomingThread.
   */
  public static String getClientHost() throws ServerNotActiveException
  {
    Thread currThread = Thread.currentThread();
    if (currThread instanceof RMIIncomingThread)
      {
        RMIIncomingThread incomingThread = (RMIIncomingThread) currThread;
        return incomingThread.getClientHost();
      }
    else
      {
        throw new ServerNotActiveException(
          "Unknown client host - current thread not instance of 'RMIIncomingThread'");
      }
  }
  
  /**
   * Set the stream for logging RMI calls.
   * 
   * @param out the stream to set or null to turn the logging off.
   */
  public static void setLog(OutputStream out)
  {
    throw new Error("Not implemented");
  }
  
  /**
   * Get the stream for logging RMI calls.
   * 
   * @return the associated stream.
   */
  public static PrintStream getLog()
  {
    throw new Error("Not implemented");
  }

}
