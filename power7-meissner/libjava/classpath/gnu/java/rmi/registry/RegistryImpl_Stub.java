/* RegistryImpl_Stub.java
   Copyright (C) 2002 Free Software Foundation, Inc.

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


// Stub class generated by rmic - DO NOT EDIT!

package gnu.java.rmi.registry;

public final class RegistryImpl_Stub
    extends java.rmi.server.RemoteStub
    implements java.rmi.registry.Registry
{
    private static final long serialVersionUID = 2L;
    
    private static final long interfaceHash = 4905912898345647071L;
    
    private static boolean useNewInvoke;
    
    private static final java.rmi.server.Operation[] operations = {
        new java.rmi.server.Operation("void bind(java.lang.String, java.rmi.Remote)"),
        new java.rmi.server.Operation("java.lang.String[] list()"),
        new java.rmi.server.Operation("java.rmi.Remote lookup(java.lang.String)"),
        new java.rmi.server.Operation("void rebind(java.lang.String, java.rmi.Remote)"),
        new java.rmi.server.Operation("void unbind(java.lang.String)")
    };
    
    private static java.lang.reflect.Method $method_bind_0;
    private static java.lang.reflect.Method $method_list_1;
    private static java.lang.reflect.Method $method_lookup_2;
    private static java.lang.reflect.Method $method_rebind_3;
    private static java.lang.reflect.Method $method_unbind_4;
    
    static {
        try {
            java.rmi.server.RemoteRef.class.getMethod("invoke", new java.lang.Class[] { java.rmi.Remote.class, java.lang.reflect.Method.class, java.lang.Object[].class, long.class });
            useNewInvoke = false;
            $method_bind_0 = gnu.java.rmi.registry.RegistryImpl.class.getMethod("bind", new java.lang.Class[] {java.lang.String.class, java.rmi.Remote.class});
            $method_list_1 = gnu.java.rmi.registry.RegistryImpl.class.getMethod("list", new java.lang.Class[] {});
            $method_lookup_2 = gnu.java.rmi.registry.RegistryImpl.class.getMethod("lookup", new java.lang.Class[] {java.lang.String.class});
            $method_rebind_3 = gnu.java.rmi.registry.RegistryImpl.class.getMethod("rebind", new java.lang.Class[] {java.lang.String.class, java.rmi.Remote.class});
            $method_unbind_4 = gnu.java.rmi.registry.RegistryImpl.class.getMethod("unbind", new java.lang.Class[] {java.lang.String.class});
            
        }
        catch (java.lang.NoSuchMethodException e) {
            useNewInvoke = false;
        }
    }
    
    public RegistryImpl_Stub() {
        super();
    }
    public RegistryImpl_Stub(java.rmi.server.RemoteRef ref) {
        super(ref);
    }
    
    public void bind(java.lang.String $param_0, java.rmi.Remote $param_1) throws java.rmi.AccessException, java.rmi.AlreadyBoundException, java.rmi.RemoteException {
        try {
            if (useNewInvoke) {
                ref.invoke(this, $method_bind_0, new java.lang.Object[] {$param_0, $param_1}, 7583982177005850366L);
            }
            else {
                java.rmi.server.RemoteCall call = ref.newCall((java.rmi.server.RemoteObject)this, operations, 0, interfaceHash);
                try {
                    java.io.ObjectOutput out = call.getOutputStream();
                    out.writeObject($param_0);
                    out.writeObject($param_1);
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.MarshalException("error marshalling arguments", e);
                }
                ref.invoke(call);
                try {
                    java.io.ObjectInput in = call.getInputStream();
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.UnmarshalException("error unmarshalling return", e);
                }
                finally {
                    ref.done(call);
                }
            }
        }
        catch (java.rmi.AccessException e) {
            throw e;
        }
        catch (java.rmi.AlreadyBoundException e) {
            throw e;
        }
        catch (java.rmi.RemoteException e) {
            throw e;
        }
        catch (java.lang.Exception e) {
            throw new java.rmi.UnexpectedException("undeclared checked exception", e);
        }
    }
    
    public java.lang.String[] list() throws java.rmi.AccessException, java.rmi.RemoteException {
        try {
            if (useNewInvoke) {
                java.lang.Object $result = ref.invoke(this, $method_list_1, null, 2571371476350237748L);
                return ((java.lang.String[])$result);
            }
            else {
                java.rmi.server.RemoteCall call = ref.newCall((java.rmi.server.RemoteObject)this, operations, 1, interfaceHash);
                try {
                    java.io.ObjectOutput out = call.getOutputStream();
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.MarshalException("error marshalling arguments", e);
                }
                ref.invoke(call);
                java.lang.String[] $result;
                try {
                    java.io.ObjectInput in = call.getInputStream();
                    $result = (java.lang.String[])in.readObject();
                    return ($result);
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.UnmarshalException("error unmarshalling return", e);
                }
                finally {
                    ref.done(call);
                }
            }
        }
        catch (java.rmi.AccessException e) {
            throw e;
        }
        catch (java.rmi.RemoteException e) {
            throw e;
        }
        catch (java.lang.Exception e) {
            throw new java.rmi.UnexpectedException("undeclared checked exception", e);
        }
    }
    
    public java.rmi.Remote lookup(java.lang.String $param_0) throws java.rmi.AccessException, java.rmi.NotBoundException, java.rmi.RemoteException {
        try {
            if (useNewInvoke) {
                java.lang.Object $result = ref.invoke(this, $method_lookup_2, new java.lang.Object[] {$param_0}, -7538657168040752697L);
                return ((java.rmi.Remote)$result);
            }
            else {
                java.rmi.server.RemoteCall call = ref.newCall((java.rmi.server.RemoteObject)this, operations, 2, interfaceHash);
                try {
                    java.io.ObjectOutput out = call.getOutputStream();
                    out.writeObject($param_0);
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.MarshalException("error marshalling arguments", e);
                }
                ref.invoke(call);
                java.rmi.Remote $result;
                try {
                    java.io.ObjectInput in = call.getInputStream();
                    $result = (java.rmi.Remote)in.readObject();
                    return ($result);
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.UnmarshalException("error unmarshalling return", e);
                }
                finally {
                    ref.done(call);
                }
            }
        }
        catch (java.rmi.AccessException e) {
            throw e;
        }
        catch (java.rmi.NotBoundException e) {
            throw e;
        }
        catch (java.rmi.RemoteException e) {
            throw e;
        }
        catch (java.lang.Exception e) {
            throw new java.rmi.UnexpectedException("undeclared checked exception", e);
        }
    }
    
    public void rebind(java.lang.String $param_0, java.rmi.Remote $param_1) throws java.rmi.AccessException, java.rmi.RemoteException {
        try {
            if (useNewInvoke) {
                ref.invoke(this, $method_rebind_3, new java.lang.Object[] {$param_0, $param_1}, -8381844669958460146L);
            }
            else {
                java.rmi.server.RemoteCall call = ref.newCall((java.rmi.server.RemoteObject)this, operations, 3, interfaceHash);
                try {
                    java.io.ObjectOutput out = call.getOutputStream();
                    out.writeObject($param_0);
                    out.writeObject($param_1);
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.MarshalException("error marshalling arguments", e);
                }
                ref.invoke(call);
                try {
                    java.io.ObjectInput in = call.getInputStream();
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.UnmarshalException("error unmarshalling return", e);
                }
                finally {
                    ref.done(call);
                }
            }
        }
        catch (java.rmi.AccessException e) {
            throw e;
        }
        catch (java.rmi.RemoteException e) {
            throw e;
        }
        catch (java.lang.Exception e) {
            throw new java.rmi.UnexpectedException("undeclared checked exception", e);
        }
    }
    
    public void unbind(java.lang.String $param_0) throws java.rmi.AccessException, java.rmi.NotBoundException, java.rmi.RemoteException {
        try {
            if (useNewInvoke) {
                ref.invoke(this, $method_unbind_4, new java.lang.Object[] {$param_0}, 7305022919901907578L);
            }
            else {
                java.rmi.server.RemoteCall call = ref.newCall((java.rmi.server.RemoteObject)this, operations, 4, interfaceHash);
                try {
                    java.io.ObjectOutput out = call.getOutputStream();
                    out.writeObject($param_0);
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.MarshalException("error marshalling arguments", e);
                }
                ref.invoke(call);
                try {
                    java.io.ObjectInput in = call.getInputStream();
                }
                catch (java.io.IOException e) {
                    throw new java.rmi.UnmarshalException("error unmarshalling return", e);
                }
                finally {
                    ref.done(call);
                }
            }
        }
        catch (java.rmi.AccessException e) {
            throw e;
        }
        catch (java.rmi.NotBoundException e) {
            throw e;
        }
        catch (java.rmi.RemoteException e) {
            throw e;
        }
        catch (java.lang.Exception e) {
            throw new java.rmi.UnexpectedException("undeclared checked exception", e);
        }
    }
    
}
