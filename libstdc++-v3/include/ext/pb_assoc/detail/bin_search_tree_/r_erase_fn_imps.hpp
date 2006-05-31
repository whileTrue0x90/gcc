// -*- C++ -*-

// Copyright (C) 2005 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 2, or (at your option)
// any later version.

// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING.  If not, write to the Free
// Software Foundation, 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

// Copyright (C) 2004 Ami Tavory and Vladimir Dreizin, IBM-HRL.

// Permission to use, copy, modify, sell, and distribute this software
// is hereby granted without fee, provided that the above copyright
// notice appears in all copies, and that both that copyright notice and
// this permission notice appear in supporting documentation. None of
// the above authors, nor IBM Haifa Research Laboratories, make any
// representation about the suitability of this software for any
// purpose. It is provided "as is" without express or implied warranty.

/**
 * @file r_erase_fn_imps.hpp
 * Contains an implementation class for bin_search_tree_.
 */

PB_ASSOC_CLASS_T_DEC
inline void
PB_ASSOC_CLASS_C_DEC::
actual_erase_node(node_pointer p_z)
{
  PB_ASSOC_DBG_ASSERT(m_size > 0);
  --m_size;

  PB_ASSOC_DBG_ONLY(erase_existing(PB_ASSOC_V2F(p_z->m_value)));

  p_z->~node();

  s_node_allocator.deallocate(p_z, 1);
}

PB_ASSOC_CLASS_T_DEC
inline void
PB_ASSOC_CLASS_C_DEC::
update_min_max_for_erased_node(node_pointer p_z)
{
  if (m_size == 1)
    {
      m_p_head->m_p_left = m_p_head->m_p_right = m_p_head;

      return;
    }

  if (m_p_head->m_p_left == p_z)
    {
      iterator it(p_z);

      ++it;

      m_p_head->m_p_left = it.m_p_nd;
    }
  else if (m_p_head->m_p_right == p_z)
    {
      iterator it(p_z);

      --it;

      m_p_head->m_p_right = it.m_p_nd;
    }
}

PB_ASSOC_CLASS_T_DEC
void
PB_ASSOC_CLASS_C_DEC::
clear()
{
  PB_ASSOC_DBG_ONLY(assert_valid(true, true);)

    clear_imp(m_p_head->m_p_parent);

  m_size = 0;

  initialize();

  PB_ASSOC_DBG_ONLY(my_map_debug_base::clear();)

    PB_ASSOC_DBG_ONLY(assert_valid(true, true);)
    }

PB_ASSOC_CLASS_T_DEC
void
PB_ASSOC_CLASS_C_DEC::
clear_imp(node_pointer p_nd)
{
  if (p_nd == NULL)
    return;

  clear_imp(p_nd->m_p_left);

  clear_imp(p_nd->m_p_right);

  p_nd->~Node();

  s_node_allocator.deallocate(p_nd, 1);
}

