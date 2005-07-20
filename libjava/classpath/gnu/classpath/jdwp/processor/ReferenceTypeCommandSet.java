/* ReferenceTypeCommandSet.java -- lass to implement the ReferenceType
   Command Set
   Copyright (C) 2005 Free Software Foundation

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

package gnu.classpath.jdwp.processor;

import gnu.classpath.jdwp.IVirtualMachine;
import gnu.classpath.jdwp.Jdwp;
import gnu.classpath.jdwp.JdwpConstants;
import gnu.classpath.jdwp.exception.InvalidFieldException;
import gnu.classpath.jdwp.exception.JdwpException;
import gnu.classpath.jdwp.exception.JdwpInternalErrorException;
import gnu.classpath.jdwp.exception.NotImplementedException;
import gnu.classpath.jdwp.id.IdManager;
import gnu.classpath.jdwp.id.ObjectId;
import gnu.classpath.jdwp.id.ReferenceTypeId;
import gnu.classpath.jdwp.util.JdwpString;
import gnu.classpath.jdwp.util.Signature;
import gnu.classpath.jdwp.util.Value;

import java.io.DataOutputStream;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;

/**
 * A class representing the ReferenceType Command Set.
 * 
 * @author Aaron Luchko <aluchko@redhat.com>
 */
public class ReferenceTypeCommandSet implements CommandSet
{
  // Our hook into the jvm
  private final IVirtualMachine vm = Jdwp.getIVirtualMachine();

  // Manages all the different ids that are assigned by jdwp
  private final IdManager idMan = Jdwp.getIdManager();

  public boolean runCommand(ByteBuffer bb, DataOutputStream os, byte command)
    throws JdwpException
  {
    try
      {
        switch (command)
          {
          case JdwpConstants.CommandSet.ReferenceType.SIGNATURE:
            executeSignature(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.CLASS_LOADER:
            executeClassLoader(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.MODIFIERS:
            executeModifiers(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.FIELDS:
            executeFields(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.METHODS:
            executeMethods(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.GET_VALUES:
            executeGetValues(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.SOURCE_FILE:
            executeSourceFile(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.NESTED_TYPES:
            executeNestedTypes(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.STATUS:
            executeStatus(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.INTERFACES:
            executeInterfaces(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.CLASS_OBJECT:
            executeClassObject(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.SOURCE_DEBUG_EXTENSION:
            executeSourceDebugExtension(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.SIGNATURE_WITH_GENERIC:
            executeSignatureWithGeneric(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.FIELDS_WITH_GENERIC:
            executeFieldWithGeneric(bb, os);
            break;
          case JdwpConstants.CommandSet.ReferenceType.METHODS_WITH_GENERIC:
            executeMethodsWithGeneric(bb, os);
            break;
          default:
            throw new NotImplementedException("Command " + command +
              " not found in String Reference Command Set.");
          }
      }
    catch (IOException ex)
      {
        // The DataOutputStream we're using isn't talking to a socket at all
        // So if we throw an IOException we're in serious trouble
        throw new JdwpInternalErrorException(ex);
      }
    return true;
  }

  private void executeSignature(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    String sig = Signature.computeClassSignature(refId.getType());
    JdwpString.writeString(os, sig);
  }

  private void executeClassLoader(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);

    Class clazz = refId.getType();
    ClassLoader loader = clazz.getClassLoader();
    ObjectId oid = idMan.getId(loader);
    oid.write(os);
  }

  private void executeModifiers(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);

    Class clazz = refId.getType();
    os.writeInt(clazz.getModifiers());
  }

  private void executeFields(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();

    Field[] fields = clazz.getFields();
    os.writeInt(fields.length);
    for (int i = 0; i < fields.length; i++)
      {
        Field field = fields[i];
        idMan.getId(field).write(os);
        JdwpString.writeString(os, field.getName());
        JdwpString.writeString(os, Signature.computeFieldSignature(field));
        os.writeInt(field.getModifiers());
      }
  }

  private void executeMethods(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();

    Method[] methods = clazz.getMethods();
    os.writeInt(methods.length);
    for (int i = 0; i < methods.length; i++)
      {
        Method method = methods[i];
        idMan.getId(method).write(os);
        JdwpString.writeString(os, method.getName());
        JdwpString.writeString(os, Signature.computeMethodSignature(method));
        os.writeInt(method.getModifiers());
      }
  }

  private void executeGetValues(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();

    int numFields = bb.getInt();
    os.writeInt(numFields); // Looks pointless but this is the protocol
    for (int i = 0; i < numFields; i++)
      {
        ObjectId fieldId = idMan.readId(bb);
        Field field = (Field) (fieldId.getObject());
        Class fieldClazz = field.getDeclaringClass();

        // We don't actually need the clazz to get the field but we might as
        // well check that the debugger got it right
        if (fieldClazz.isAssignableFrom(clazz))
          Value.writeStaticValueFromField(os, field);
        else
          throw new InvalidFieldException(fieldId.getId());
      }
  }

  private void executeSourceFile(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();

    // We'll need to go into the jvm for this unless there's an easier way
    String sourceFileName = vm.getSourceFile(clazz);
    JdwpString.writeString(os, sourceFileName);
    // clazz.getProtectionDomain().getCodeSource().getLocation();
  }

  private void executeNestedTypes(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();
    Class[] declaredClazzes = clazz.getDeclaredClasses();
    os.writeInt(declaredClazzes.length);
    for (int i = 0; i < declaredClazzes.length; i++)
      {
        Class decClazz = declaredClazzes[i];
        ReferenceTypeId clazzId = idMan.getReferenceTypeId(decClazz);
        clazzId.writeTagged(os);
      }
  }

  private void executeStatus(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();

    // I don't think there's any other way to get this
    int status = vm.getStatus(clazz);
    os.writeInt(status);
  }

  private void executeInterfaces(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();
    Class[] interfaces = clazz.getInterfaces();
    os.writeInt(interfaces.length);
    for (int i = 0; i < interfaces.length; i++)
      {
        Class interfaceClass = interfaces[i];
        ReferenceTypeId intId = idMan.getReferenceTypeId(interfaceClass);
        intId.write(os);
      }
  }

  private void executeClassObject(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    ReferenceTypeId refId = idMan.readReferenceTypeId(bb);
    Class clazz = refId.getType();
    ObjectId clazzObjectId = idMan.getId(clazz);
    clazzObjectId.write(os);
  }

  private void executeSourceDebugExtension(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    // This command is optional, determined by VirtualMachines CapabilitiesNew
    // so we'll leave it till later to implement
    throw new NotImplementedException(
      "Command SourceDebugExtension not implemented.");
  }

  private void executeSignatureWithGeneric(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    // We don't have generics yet
    throw new NotImplementedException(
      "Command SourceDebugExtension not implemented.");
  }

  private void executeFieldWithGeneric(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    // We don't have generics yet
    throw new NotImplementedException(
      "Command SourceDebugExtension not implemented.");
  }

  private void executeMethodsWithGeneric(ByteBuffer bb, DataOutputStream os)
    throws JdwpException, IOException
  {
    // We don't have generics yet
    throw new NotImplementedException(
      "Command SourceDebugExtension not implemented.");
  }
}
