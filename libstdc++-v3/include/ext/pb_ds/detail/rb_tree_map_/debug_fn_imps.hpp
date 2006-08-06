// -*- C++ -*-

// Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the terms
// of the GNU General Public License as published by the Free Software
// Foundation; either version 2, or (at your option) any later
// version.

// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this library; see the file COPYING.  If not, write to
// the Free Software Foundation, 59 Temple Place - Suite 330, Boston,
// MA 02111-1307, USA.

// As a special exception, you may use this file as part of a free
// software library without restriction.  Specifically, if other files
// instantiate templates or use macros or inline functions from this
// file, or you compile this file and link it with other files to
// produce an executable, this file does not by itself cause the
// resulting executable to be covered by the GNU General Public
// License.  This exception does not however invalidate any other
// reasons why the executable file might be covered by the GNU General
// Public License.

// Copyright (C) 2004 Ami Tavory and Vladimir Dreizin, IBM-HRL.

// Permission to use, copy, modify, sell, and distribute this software
// is hereby granted without fee, provided that the above copyright
// notice appears in all copies, and that both that copyright notice
// and this permission notice appear in supporting documentation. None
// of the above authors, nor IBM Haifa Research Laboratories, make any
// representation about the suitability of this software for any
// purpose. It is provided "as is" without express or implied
// warranty.

/**
 * @file debug_fn_imps.hpp
 * Contains an implementation for rb_tree_.
 */

#ifdef PB_DS_RB_TREE_DEBUG_

PB_DS_CLASS_T_DEC
typename PB_DS_CLASS_C_DEC::size_type
PB_DS_CLASS_C_DEC::
assert_node_consistent(const node_pointer p_nd) const
{
  if (p_nd == NULL)
    return (1);

  const size_type l_height = assert_node_consistent(p_nd->m_p_left);
  const size_type r_height = assert_node_consistent(p_nd->m_p_right);

  if (p_nd->m_red)
    {
      PB_DS_DBG_ASSERT(is_effectively_black(p_nd->m_p_left));

      PB_DS_DBG_ASSERT(is_effectively_black(p_nd->m_p_right));
    }

  PB_DS_DBG_ASSERT(l_height == r_height);

  return ((p_nd->m_red? 0 : 1) + l_height);
}

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
assert_valid() const
{
  PB_DS_BASE_C_DEC::assert_valid();

  const node_pointer p_head = PB_DS_BASE_C_DEC::m_p_head;

  PB_DS_DBG_ASSERT(p_head->m_red);

  if (p_head->m_p_parent != NULL)
    {
      PB_DS_DBG_ASSERT(!p_head->m_p_parent->m_red);

      assert_node_consistent(p_head->m_p_parent);
    }
}

#endif // #ifdef PB_DS_RB_TREE_DEBUG_

