/* Copyright (C) 2004, 2005  Free Software Foundation

   This file is part of libgcj.

This software is copyrighted work licensed under the terms of the
Libgcj License.  Please consult the file "LIBGCJ_LICENSE" for
details.  */

package gnu.gcj.tools.gcj_dbtool;


import gnu.gcj.runtime.PersistentByteMap;
import java.io.*;
import java.nio.channels.*;
import java.util.*;
import java.util.jar.*;
import java.security.MessageDigest;

public class Main
{
  static private boolean verbose = false;

  public static void main (String[] s)
  {
    insist (s.length >= 1);
    if (s[0].equals("-v") || s[0].equals("--version"))
      {
	insist (s.length == 1);
	System.out.println("gcj-dbtool ("
			   + System.getProperty("java.vm.name")
			   + ") "
			   + System.getProperty("java.vm.version"));
	System.out.println();
	System.out.println("Copyright 2005 Free Software Foundation, Inc.");
	System.out.println("This is free software; see the source for copying conditions.  There is NO");
	System.out.println("warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");
	return;
      }
    if (s[0].equals("--help"))
      {
	usage(System.out);
	return;
      }

    if (s[0].equals("-n"))
      {
	// Create a new database.
	insist (s.length >= 2 && s.length <= 3);

	int capacity = 32749;

	if (s.length == 3)
	  {	    
	    capacity = Integer.parseInt(s[2]);

	    if (capacity <= 2)
	      {
		usage(System.err);
		System.exit(1);
	      }
	  }
	    
	try
	  {
	    PersistentByteMap b 
	      = PersistentByteMap.emptyPersistentByteMap(new File(s[1]), 
							 capacity, capacity*32);
	  }
	catch (Exception e)
	  {
	    System.err.println ("error: could not create " 
				+ s[1] + ": " + e.toString());
	    System.exit(2);
	  }
	return;
      }

    if (s[0].equals("-a") || s[0].equals("-f"))
      {
	// Add a jar file to a database, creating it if necessary.
	// Copies the database, adds the jar file to the copy, and
	// then renames the new database over the old.
	try
	  {
	    insist (s.length == 4);
	    File database = new File(s[1]);
	    database = database.getAbsoluteFile();
	    File jar = new File(s[2]);	
	    PersistentByteMap map; 
	    if (database.isFile())
	      map = new PersistentByteMap(database, 
					  PersistentByteMap.AccessMode.READ_ONLY);
	    else
	      map = PersistentByteMap.emptyPersistentByteMap(database, 
							     100, 100*32);
	    File soFile = new File(s[3]);
	    if (! s[0].equals("-f") && ! soFile.isFile())
	      throw new IllegalArgumentException(s[3] + " is not a file");
 	    map = addJar(jar, map, soFile);
	  }
	catch (Exception e)
	  {
	    System.err.println ("error: could not update " + s[1] 
				+ ": " + e.toString());
	    System.exit(2);
	  }
	return;
      }

    if (s[0].equals("-t"))
      {
	// Test
	try
	  {
	    insist (s.length == 2);
	    PersistentByteMap b 
	      = new PersistentByteMap(new File(s[1]),
				      PersistentByteMap.AccessMode.READ_ONLY);
	    Iterator iterator = b.iterator(PersistentByteMap.ENTRIES);
	
	    while (iterator.hasNext())
	      {
		PersistentByteMap.MapEntry entry 
		  = (PersistentByteMap.MapEntry)iterator.next();
		byte[] key = (byte[])entry.getKey();
		byte[] value = (byte[])b.get(key);
		if (! Arrays.equals (value, (byte[])entry.getValue()))
		  {
		    String err 
		      = ("Key " + bytesToString(key) + " at bucket " 
			 + entry.getBucket());
		  
		    throw new RuntimeException(err);
		  }
	      }
	  }
	catch (Exception e)
	  {
	    e.printStackTrace();
	    System.exit(3);
	  }
	return;
      }
	 
    if (s[0].equals("-m"))
      {
	// Merge databases.
	insist (s.length >= 3);
	try
	  {
	    File database = new File(s[1]);
	    database = database.getAbsoluteFile();
	    File temp = File.createTempFile(database.getName(), "", 
					    database.getParentFile());
	    	
	    int newSize = 0;
	    int newStringTableSize = 0;
	    PersistentByteMap[] sourceMaps = new PersistentByteMap[s.length - 2];
	    // Scan all the input files, calculating worst case string
	    // table and hash table use.
	    for (int i = 2; i < s.length; i++)
	      {
		PersistentByteMap b 
		  = new PersistentByteMap(new File(s[i]),
					  PersistentByteMap.AccessMode.READ_ONLY);
		newSize += b.size();
		newStringTableSize += b.stringTableSize();
		sourceMaps[i - 2] = b;
	      }
	    
	    newSize *= 1.5; // Scaling the new size by 1.5 results in
			    // fewer collisions.
	    PersistentByteMap map 
	      = PersistentByteMap.emptyPersistentByteMap
	        (temp, newSize, newStringTableSize);

	    for (int i = 0; i < sourceMaps.length; i++)
	      {
		if (verbose)
		  System.err.println("adding " + sourceMaps[i].size() 
				     + " elements from "
				     + sourceMaps[i].getFile());
		map.putAll(sourceMaps[i]);
	      }
	    map.close();
	    temp.renameTo(database);
	  }
	catch (Exception e)
	  {
	    e.printStackTrace();
	    System.exit(3);
	  }
	return;
      }

    if (s[0].equals("-l"))
      {
	// List a database.
	insist (s.length == 2);
	try
	  {
	    PersistentByteMap b 
	      = new PersistentByteMap(new File(s[1]),
				      PersistentByteMap.AccessMode.READ_ONLY);

	    System.out.println ("Capacity: " + b.capacity());
	    System.out.println ("Size: " + b.size());
	    System.out.println ();

	    System.out.println ("Elements: ");
	    Iterator iterator = b.iterator(PersistentByteMap.ENTRIES);
    
	    while (iterator.hasNext())
	      {
		PersistentByteMap.MapEntry entry 
		  = (PersistentByteMap.MapEntry)iterator.next();
		byte[] digest = (byte[])entry.getKey();
		System.out.print ("[" + entry.getBucket() + "] " 
				  + bytesToString(digest)
				  + " -> ");
		System.out.println (new String((byte[])entry.getValue()));
	      }
	  }
	catch (Exception e)
	  {
	    System.err.println ("error: could not list " 
				+ s[1] + ": " + e.toString());
	    System.exit(2);
	  }
	return;
      }

    if (s[0].equals("-d"))
      {
	// For testing only: fill the byte map with random data.
	insist (s.length == 2);
	try
	  {    
	    MessageDigest md = MessageDigest.getInstance("MD5");
	    PersistentByteMap b 
	      = new PersistentByteMap(new File(s[1]), 
				      PersistentByteMap.AccessMode.READ_WRITE);
	    int N = b.capacity();
	    byte[] bytes = new byte[1];
	    byte digest[] = md.digest(bytes);
	    for (int i = 0; i < N; i++)
	      {
		digest = md.digest(digest);
		b.put(digest, digest);
	      }
	  }
	catch (Exception e)
	  {
	    e.printStackTrace();
	    System.exit(3);
	  }	    
	return;
      }

    if (s[0].equals("-p"))
      {
	insist (s.length == 1 || s.length == 2);
	String result;
	
	if (s.length == 1)
	  result = System.getProperty("gnu.gcj.precompiled.db.path", "");
	else 
	  result = (s[1] 
		    + (s[1].endsWith(File.separator) ? "" : File.separator)
		    + getDbPathTail ());

	System.out.println (result);
	return;
      }

    usage(System.err);
    System.exit(1);	    
  }

  private static native String getDbPathTail ();
    
  private static void insist(boolean ok)
  {
    if (! ok)
      {
	usage(System.err);
	System.exit(1);
      }	    
  }

  private static void usage(PrintStream out)
  {
    out.println
      ("gcj-dbtool: Manipulate gcj map database files\n"
       + "\n"
       + "  Usage: \n"
       + "    gcj-dbtool -n file.gcjdb [size]     - Create a new gcj map database\n"
       + "    gcj-dbtool -a file.gcjdb file.jar file.so\n"
       + "            - Add the contents of file.jar to a new gcj map database\n"
       + "    gcj-dbtool -f file.gcjdb file.jar file.so\n"
       + "            - Add the contents of file.jar to a new gcj map database\n"
       + "    gcj-dbtool -t file.gcjdb            - Test a gcj map database\n"
       + "    gcj-dbtool -l file.gcjdb            - List a gcj map database\n"
       + "    gcj-dbtool -m dest.gcjdb [source.gcjdb]...\n"
       + "             - Merge gcj map databases into dest\n"
       + "               Replaces dest\n"
       + "               To add to dest, include dest in the list of sources\n"
       + "    gcj-dbtool -p [LIBDIR]              - Print default database name");
  }

  // Add a jar to a map.  This copies the map first and returns a
  // different map that contains the data.  The original map is
  // closed.

  private static PersistentByteMap 
  addJar(File f, PersistentByteMap b, File soFile)
    throws Exception
  {
    MessageDigest md = MessageDigest.getInstance("MD5");

    JarFile jar = new JarFile (f);

    int count = 0;
    {
      Enumeration entries = jar.entries();      
      while (entries.hasMoreElements())
	{
	  JarEntry classfile = (JarEntry)entries.nextElement();
	  if (classfile.getName().endsWith(".class"))
	    count++;
	}
    }

    if (verbose)
      System.err.println("adding " + count + " elements from "
			 + f + " to " + b.getFile());
    
    // Maybe resize the destination map.  We're allowing plenty of
    // extra space by using a loadFactor of 2.  
    b = resizeMap(b, (b.size() + count) * 2, true);

    Enumeration entries = jar.entries();

    byte[] soFileName = soFile.getCanonicalPath().getBytes("UTF-8");
    while (entries.hasMoreElements())
      {
	JarEntry classfile = (JarEntry)entries.nextElement();
	if (classfile.getName().endsWith(".class"))
	  {
	    InputStream str = jar.getInputStream(classfile);
	    long length = classfile.getSize();
	    if (length == -1)
	      throw new EOFException();

	    byte[] data = new byte[length];
	    int pos = 0;
	    while (length - pos > 0)
	      {
		int len = str.read(data, pos, (int)(length - pos));
		if (len == -1)
		  throw new EOFException("Not enough data reading from: "
					 + classfile.getName());
		pos += len;
	      }
	    b.put(md.digest(data), soFileName);
	  }
      }
    return b;
  }    

  // Resize a map by creating a new one with the same data and
  // renaming it.  If close is true, close the original map.

  static PersistentByteMap resizeMap(PersistentByteMap m, int newCapacity, boolean close)
    throws IOException, IllegalAccessException
  {
    newCapacity = Math.max(m.capacity(), newCapacity);
    File name = m.getFile();
    File copy = File.createTempFile(name.getName(), "", name.getParentFile());
    try
      {
	PersistentByteMap dest 
	  = PersistentByteMap.emptyPersistentByteMap
	  (copy, newCapacity, newCapacity*32);
	dest.putAll(m);
	dest.force();
	if (close)
	  m.close();
	copy.renameTo(name);
	return dest;
      }
    catch (Exception e)
      {
	copy.delete();
      }
    return null;
  }
    
	 
  static String bytesToString(byte[] b)
  {
    StringBuffer hexBytes = new StringBuffer();
    int length = b.length;
    for (int i = 0; i < length; ++i)
      hexBytes.append(Integer.toHexString(b[i] & 0xff));
    return hexBytes.toString();
  }
}
    
