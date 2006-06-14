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
 * @file erase_fn_imps.hpp
 * Contains an implementation for rc_binomial_heap_.
 */

PB_DS_CLASS_T_DEC
inline void
PB_DS_CLASS_C_DEC::
pop()
{
  make_binomial_heap();

  PB_DS_DBG_ASSERT(!base_type::empty());

  base_type::pop();

  base_type::find_max();
}

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
clear()
{
  base_type::clear();

  m_rc.clear();
}

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
make_binomial_heap()
{
  node_pointer p_nd = base_type::m_p_root;

  while (p_nd != NULL)
    {
      node_pointer p_next = p_nd->m_p_next_sibling;

      if (p_next == NULL)
	p_nd = p_next;
      else if (p_nd->m_metadata == p_next->m_metadata)
	p_nd = link_with_next_sibling(p_nd);
      else if (p_nd->m_metadata < p_next->m_metadata)
	p_nd = p_next;
#ifdef PB_DS_RC_BINOMIAL_HEAP__DEBUG_
      else
	PB_DS_DBG_ASSERT(0);
#endif // #ifdef PB_DS_RC_BINOMIAL_HEAP__DEBUG_
    }

  m_rc.clear();
}

PB_DS_CLASS_T_DEC
template<typename Pred>
typename PB_DS_CLASS_C_DEC::size_type
PB_DS_CLASS_C_DEC::
erase_if(Pred pred)
{
  make_binomial_heap();

  const size_type ersd = base_type::erase_if(pred);

  base_type::find_max();

  PB_DS_DBG_ONLY(assert_valid();)

    return ersd;
}

PB_DS_CLASS_T_DEC
void
PB_DS_CLASS_C_DEC::
erase(point_iterator it)
{
  make_binomial_heap();

  base_type::erase(it);

  base_type::find_max();
}

