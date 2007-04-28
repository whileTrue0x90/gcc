// filebuf with encoding state type -*- C++ -*-

// Copyright (C) 2002, 2003, 2004, 2007 Free Software Foundation, Inc.
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
// Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
// USA.

// As a special exception, you may use this file as part of a free software
// library without restriction.  Specifically, if other files instantiate
// templates or use macros or inline functions from this file, or you compile
// this file and link it with other files to produce an executable, this
// file does not by itself cause the resulting executable to be covered by
// the GNU General Public License.  This exception does not however
// invalidate any other reasons why the executable file might be covered by
// the GNU General Public License.

/** @file ext/enc_filebuf.h
 *  This file is a GNU extension to the Standard C++ Library.
 */

#ifndef _EXT_ENC_FILEBUF_H
#define _EXT_ENC_FILEBUF_H 1

#include <fstream>
#include <locale>
#include <ext/codecvt_specializations.h>

_GLIBCXX_BEGIN_NAMESPACE(__gnu_cxx)

  /// @brief  class enc_filebuf.
  template<typename _CharT>
    class enc_filebuf
    : public std::basic_filebuf<_CharT, encoding_char_traits<_CharT> >
    {
    public:
      typedef encoding_char_traits<_CharT>     	traits_type;
      typedef typename traits_type::state_type	state_type;
      typedef typename traits_type::pos_type	pos_type;

      enc_filebuf(state_type& __state)
      : std::basic_filebuf<_CharT, encoding_char_traits<_CharT> >()
      { this->_M_state_beg = __state; }

    private:
      // concept requirements:
      // Set state type to something useful.
      // Something more than copyconstructible is needed here, so
      // require default and copy constructible + assignment operator.
      __glibcxx_class_requires(state_type, _SGIAssignableConcept)
    };

_GLIBCXX_END_NAMESPACE

#endif
