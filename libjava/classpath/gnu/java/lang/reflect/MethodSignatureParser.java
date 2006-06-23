/* MethodSignatureParser.java
   Copyright (C) 2005
   Free Software Foundation

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

package gnu.java.lang.reflect;

import java.lang.reflect.*;
import java.util.ArrayList;

public class MethodSignatureParser extends GenericSignatureParser
{
    private TypeVariable[] typeParameters;
    private Type[] argTypes;
    private Type retType;
    private Type[] throwsSigs;

    public MethodSignatureParser(Method method, String signature)
    {
        this(method, method.getDeclaringClass().getClassLoader(), signature);
    }

    public MethodSignatureParser(Constructor method, String signature)
    {
        this(method, method.getDeclaringClass().getClassLoader(), signature);
    }

    private MethodSignatureParser(GenericDeclaration wrapper,
        ClassLoader loader, String signature)
    {
        super(wrapper, loader, signature);

        if (peekChar() == '<')
        {
            typeParameters = readFormalTypeParameters();
        }
        else
        {
            typeParameters = new TypeVariable[0];
        }
        consume('(');
        ArrayList<Type> args = new ArrayList<Type>();
        while (peekChar() != ')')
        {
            args.add(readTypeSignature());
        }
        argTypes = new Type[args.size()];
        args.toArray(argTypes);
        consume(')');
        retType = readTypeSignature();
        ArrayList<Type> throwsSigs = new ArrayList<Type>();
        while (peekChar() == '^')
        {
            consume('^');
            if(peekChar() == 'T')
            {
                throwsSigs.add(readTypeVariableSignature());
            }
            else
            {
                throwsSigs.add(readClassTypeSignature());
            }
        }
        this.throwsSigs = new Type[throwsSigs.size()];
        throwsSigs.toArray(this.throwsSigs);
        end();
    }

    public TypeVariable[] getTypeParameters()
    {
        TypeImpl.resolve(typeParameters);
        return typeParameters;
    }

    public Type[] getGenericParameterTypes()
    {
        TypeImpl.resolve(argTypes);
        return argTypes;
    }

    public Type getGenericReturnType()
    {
        retType = TypeImpl.resolve(retType);
        return retType;
    }

    public Type[] getGenericExceptionTypes()
    {
        TypeImpl.resolve(throwsSigs);
        return throwsSigs;
    }

    private Type readTypeSignature()
    {
        switch (peekChar())
        {
            case 'T':
                return readTypeVariableSignature();
            case 'L':
                return readClassTypeSignature();
            case '[':
                return readArrayTypeSignature();
            case 'Z':
                consume('Z');
                return boolean.class;
            case 'B':
                consume('B');
                return byte.class;
            case 'S':
                consume('S');
                return short.class;
            case 'C':
                consume('C');
                return char.class;
            case 'I':
                consume('I');
                return int.class;
            case 'F':
                consume('F');
                return float.class;
            case 'J':
                consume('J');
                return long.class;
            case 'D':
                consume('D');
                return double.class;
            case 'V':
                consume('V');
                return void.class;
            default:
                throw new GenericSignatureFormatError();
        }
    }
}
