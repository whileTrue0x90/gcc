/***
 * ASM: a very small and fast Java bytecode manipulation framework
 * Copyright (c) 2000-2005 INRIA, France Telecom
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */
package org.objectweb.asm.commons;

import org.objectweb.asm.Label;
import org.objectweb.asm.MethodAdapter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;
import org.objectweb.asm.Type;

/**
 * A {@link MethodAdapter} that renumbers local variables in their order of
 * appearance. This adapter allows one to easily add new local variables to a
 * method.
 * 
 * @author Chris Nokleberg
 * @author Eric Bruneton
 */
public class LocalVariablesSorter extends MethodAdapter {

    /**
     * Mapping from old to new local variable indexes. A local variable at index
     * i of size 1 is remapped to 'mapping[2*i]', while a local variable at
     * index i of size 2 is remapped to 'mapping[2*i+1]'.
     */
    private int[] mapping = new int[40];

    protected final int firstLocal;

    private int nextLocal;

    public LocalVariablesSorter(
        final int access,
        final String desc,
        final MethodVisitor mv)
    {
        super(mv);
        Type[] args = Type.getArgumentTypes(desc);
        nextLocal = ((Opcodes.ACC_STATIC & access) != 0) ? 0 : 1;
        for (int i = 0; i < args.length; i++) {
            nextLocal += args[i].getSize();
        }
        firstLocal = nextLocal;
    }

    public void visitVarInsn(final int opcode, final int var) {
        int size;
        switch (opcode) {
            case Opcodes.LLOAD:
            case Opcodes.LSTORE:
            case Opcodes.DLOAD:
            case Opcodes.DSTORE:
                size = 2;
                break;
            default:
                size = 1;
        }
        mv.visitVarInsn(opcode, remap(var, size));
    }

    public void visitIincInsn(final int var, final int increment) {
        mv.visitIincInsn(remap(var, 1), increment);
    }

    public void visitMaxs(final int maxStack, final int maxLocals) {
        mv.visitMaxs(maxStack, nextLocal);
    }

    public void visitLocalVariable(
        final String name,
        final String desc,
        final String signature,
        final Label start,
        final Label end,
        final int index)
    {
        int size = "J".equals(desc) || "D".equals(desc) ? 2 : 1;
        mv.visitLocalVariable(name, desc, signature, start, end, remap(index, size));
    }

    // -------------

    protected int newLocal(final int size) {
        int var = nextLocal;
        nextLocal += size;
        return var;
    }

    private int remap(final int var, final int size) {
        if (var < firstLocal) {
            return var;
        }
        int key = 2 * var + size - 1;
        int length = mapping.length;
        if (key >= length) {
            int[] newMapping = new int[Math.max(2 * length, key + 1)];
            System.arraycopy(mapping, 0, newMapping, 0, length);
            mapping = newMapping;
        }
        int value = mapping[key];
        if (value == 0) {
            value = nextLocal + 1;
            mapping[key] = value;
            nextLocal += size;
        }
        return value - 1;
    }
    
}
