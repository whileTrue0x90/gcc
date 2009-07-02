// -*- C++ -*-
//
// Copyright (C) 2008 Free Software Foundation, Inc.
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

/** @file profile/impl/profiler.h
 *  @brief Interface of the profiling runtime library.
 */

// Written by Lixia Liu

#ifndef PROFCXX_PROFILER_H__
#define PROFCXX_PROFILER_H__ 1

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#include <cstddef>
#else
#include <stddef.h>
#endif

// Forward declarations of instrumentation hooks.
namespace __cxxprof_impl
{

// Thread safe reentrance guard.
// Get in using __get_in.  Get out using the destructor.
template <int __Unused=0>
class __reentrance_guard {
 public:
  static __thread bool __inside_cxxprof_impl;
  static bool __get_in();
  __reentrance_guard() {}
  ~__reentrance_guard() { __inside_cxxprof_impl = false; }
};

template <int __Unused>
__thread bool __reentrance_guard<__Unused>::__inside_cxxprof_impl = false;

template <int __Unused>
bool __reentrance_guard<__Unused>::__get_in() {
  if (__inside_cxxprof_impl) {
    return false;
  } else {
    __inside_cxxprof_impl = true;
    return true;
  }
}

// XXX: All hook calls must be protected by a reentrance guard.
#define __GUARD(__x...)                                           \
  {                                                               \
    if (__cxxprof_impl::__reentrance_guard<0>::__get_in())        \
    {                                                             \
      __cxxprof_impl::__reentrance_guard<0> __auto_get_out;       \
      __x;                                                        \
    }                                                             \
  }

// State management.
void __turn_on();
void __turn_off();
bool __is_invalid();
bool __is_on();
bool __is_off();

// Instrumentation hooks.  Do not use them directly in instrumented headers.
// Instead use the corresponding __profcxx_ wrapper declared below.
void __trace_hashtable_size_resize(const void*, size_t, size_t);
void __trace_hashtable_size_destruct(const void*, size_t, size_t);
void __trace_hashtable_size_construct(const void*, size_t);
void __trace_vector_size_resize(const void*, size_t, size_t);
void __trace_vector_size_destruct(const void*, size_t, size_t);
void __trace_vector_size_construct(const void*, size_t);
void __trace_hash_func_destruct(const void*, size_t, size_t, size_t);
void __trace_hash_func_construct(const void*);
void __trace_vector_to_list_destruct(const void*);
void __trace_vector_to_list_construct(const void*);
void __trace_vector_to_list_insert(const void*, size_t, size_t);
void __trace_vector_to_list_iterate(const void*, size_t);
void __trace_vector_to_list_invalid_operator(const void*);
void __trace_vector_to_list_resize(const void*, size_t, size_t);
void __trace_map_to_unordered_map_construct(const void*);
void __trace_map_to_unordered_map_invalidate(const void*);
void __trace_map_to_unordered_map_insert(const void*, size_t, size_t);
void __trace_map_to_unordered_map_erase(const void*, size_t, size_t);
void __trace_map_to_unordered_map_iterate(const void*, size_t);
void __trace_map_to_unordered_map_find(const void*, size_t);
void __trace_map_to_unordered_map_destruct(const void*);
} // namespace __cxxprof_impl

// Master switch turns on all diagnostics.
#ifdef _GLIBCXX_PROFILE
#define _GLIBCXX_PROFILE_HASHTABLE_TOO_SMALL
#define _GLIBCXX_PROFILE_HASHTABLE_TOO_LARGE
#define _GLIBCXX_PROFILE_VECTOR_TOO_SMALL
#define _GLIBCXX_PROFILE_VECTOR_TOO_LARGE
#define _GLIBCXX_PROFILE_INEFFICIENT_HASH
#define _GLIBCXX_PROFILE_VECTOR_TO_LIST
#define _GLIBCXX_PROFILE_MAP_TO_UNORDERED_MAP
#endif

// Turn on/off instrumentation for HASHTABLE_TOO_SMALL and HASHTABLE_TOO_LARGE.
#if ((defined(_GLIBCXX_PROFILE_HASHTABLE_TOO_SMALL) \
      && !defined(_NO_GLIBCXX_PROFILE_HASHTABLE_TOO_SMALL)) \
     || (defined(_GLIBCXX_PROFILE_HASHTABLE_TOO_LARGE) \
         && !defined(_NO_GLIBCXX_PROFILE_HASHTABLE_TOO_LARGE)))
#define __profcxx_hashtable_resize(__x...) \
  __GUARD(__cxxprof_impl::__trace_hashtable_size_resize(__x))
#define __profcxx_hashtable_destruct(__x...) \
  __GUARD(__cxxprof_impl::__trace_hashtable_size_destruct(__x))
#define __profcxx_hashtable_construct(__x...) \
  __GUARD(__cxxprof_impl::__trace_hashtable_size_construct(__x))
#else
#define __profcxx_hashtable_resize(__x...)  
#define __profcxx_hashtable_destruct(__x...) 
#define __profcxx_hashtable_construct(__x...)  
#endif

// Turn on/off instrumentation for VECTOR_TOO_SMALL and VECTOR_TOO_LARGE.
#if ((defined(_GLIBCXX_PROFILE_VECTOR_TOO_SMALL) \
      && !defined(_NO_GLIBCXX_PROFILE_VECTOR_TOO_SMALL)) \
     || (defined(_GLIBCXX_PROFILE_VECTOR_TOO_LARGE) \
         && !defined(_NO_GLIBCXX_PROFILE_VECTOR_TOO_LARGE)))
#define __profcxx_vector_resize(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_size_resize(__x))
#define __profcxx_vector_destruct(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_size_destruct(__x))
#define __profcxx_vector_construct(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_size_construct(__x))
#else
#define __profcxx_vector_resize(__x...)  
#define __profcxx_vector_destruct(__x...) 
#define __profcxx_vector_construct(__x...)  
#endif 

// Turn on/off instrumentation for INEFFICIENT_HASH.
#if (defined(_GLIBCXX_PROFILE_INEFFICIENT_HASH) \
     && !defined(_NO_GLIBCXX_PROFILE_INEFFICIENT_HASH))
#define __profcxx_hashtable_construct2(__x...) \
  __GUARD(__cxxprof_impl::__trace_hash_func_construct(__x))
#define __profcxx_hashtable_destruct2(__x...) \
  __GUARD(__cxxprof_impl::__trace_hash_func_destruct(__x))
#else
#define __profcxx_hashtable_destruct2(__x...) 
#define __profcxx_hashtable_construct2(__x...)  
#endif

// Turn on/off instrumentation for VECTOR_TO_LIST.
#if (defined(_GLIBCXX_PROFILE_VECTOR_TO_LIST) \
     && !defined(_NO_GLIBCXX_PROFILE_VECTOR_TO_LIST))
#define __profcxx_vector_construct2(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_to_list_construct(__x))
#define __profcxx_vector_destruct2(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_to_list_destruct(__x))
#define __profcxx_vector_insert(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_to_list_insert(__x))
#define __profcxx_vector_iterate(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_to_list_iterate(__x))
#define __profcxx_vector_invalid_operator(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_to_list_invalid_operator(__x))
#define __profcxx_vector_resize2(__x...) \
  __GUARD(__cxxprof_impl::__trace_vector_to_list_resize(__x))
#else
#define __profcxx_vector_destruct2(__x...)
#define __profcxx_vector_construct2(__x...)
#define __profcxx_vector_insert(__x...)
#define __profcxx_vector_iterate(__x...)
#define __profcxx_vector_invalid_operator(__x...)
#define __profcxx_vector_resize2(__x...)
#endif

// Turn on/off instrumentation for MAP_TO_UNORDERED_MAP.
#if (defined(_GLIBCXX_PROFILE_MAP_TO_UNORDERED_MAP) \
     && !defined(_NO_GLIBCXX_PROFILE_MAP_TO_UNORDERED_MAP))
#define __profcxx_map_to_unordered_map_construct(__x...) \
  __GUARD(__cxxprof_impl::__trace_map_to_unordered_map_construct(__x))
#define __profcxx_map_to_unordered_map_destruct(__x...) \
  __GUARD(__cxxprof_impl::__trace_map_to_unordered_map_destruct(__x))
#define __profcxx_map_to_unordered_map_insert(__x...) \
  __GUARD(__cxxprof_impl::__trace_map_to_unordered_map_insert(__x))
#define __profcxx_map_to_unordered_map_erase(__x...) \
  __GUARD(__cxxprof_impl::__trace_map_to_unordered_map_erase(__x))
#define __profcxx_map_to_unordered_map_iterate(__x...) \
  __GUARD(__cxxprof_impl::__trace_map_to_unordered_map_iterate(__x))
#define __profcxx_map_to_unordered_map_invalidate(__x...) \
  __GUARD(__cxxprof_impl::__trace_map_to_unordered_map_invalidate(__x))
#define __profcxx_map_to_unordered_map_find(__x...) \
  __GUARD(__cxxprof_impl::__trace_map_to_unordered_map_find(__x))
#else
#define __profcxx_map_to_unordered_map_construct(__x...) \
  
#define __profcxx_map_to_unordered_map_destruct(__x...)
#define __profcxx_map_to_unordered_map_insert(__x...)
#define __profcxx_map_to_unordered_map_erase(__x...)
#define __profcxx_map_to_unordered_map_iterate(__x...)
#define __profcxx_map_to_unordered_map_invalidate(__x...)
#define __profcxx_map_to_unordered_map_find(__x...)
#endif

// Run multithreaded unless instructed not to do so.
#ifndef _GLIBCXX_PROFILE_NOTHREADS
#define _GLIBCXX_PROFILE_THREADS
#endif

// Instrumentation hook implementations.
// XXX: Don't move to top of file.
#include "profile/impl/profiler_hash_func.h"
#include "profile/impl/profiler_hashtable_size.h"
#include "profile/impl/profiler_map_to_unordered_map.h"
#include "profile/impl/profiler_vector_size.h"
#include "profile/impl/profiler_vector_to_list.h"

#endif // PROFCXX_PROFILER_H__
