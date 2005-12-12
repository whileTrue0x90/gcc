// Bridge method generation

// Copyright (C) 2005 Free Software Foundation, Inc.
//
// This file is part of GCC.
//
// gcjx is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// gcjx is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
//
// You should have received a copy of the GNU Library General Public
// License along with gcjx; see the file COPYING.LIB.  If
// not, write to the Free Software Foundation, Inc.,
// 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

#ifndef GCJX_BYTECODE_BRIDGE_HH
#define GCJX_BYTECODE_BRIDGE_HH

#include "bytecode/attribute.hh"

class bridge_method : public bytecode_attribute
{
  // The method which requires a bridge.
  model_method *method;

  // Facts about the generated bytecode.
  int max_stack;
  int max_locals;
  int bytecode_length;

  int emit_code (bytecode_stream *);

public:

  bridge_method (outgoing_constant_pool *cp, model_method *m)
    : bytecode_attribute (cp, "Code"),
      method (m),
      max_stack (0),
      max_locals (0)
  {
    // emit_code() doubles as a way to compute the size of the
    // generated bytecode.
    bytecode_length = emit_code (NULL);
  }

  void emit (bytecode_stream &);

  int size ();

  /// Return true if the given method requires a bridge.
  static bool required_p (model_method *);
};

#endif // GCJX_BYTECODE_BRIDGE_HH
