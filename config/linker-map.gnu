## Linker script for GNU ld 2.11.94+ only.
##
## Copyright (C) 2002 Free Software Foundation, Inc.
##
## This file is part of the libstdc++ version 3 distribution.
##
## This file is part of the GNU ISO C++ Library.  This library is free
## software; you can redistribute it and/or modify it under the
## terms of the GNU General Public License as published by the
## Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License along
## with this library; see the file COPYING.  If not, write to the Free
## Software Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.


## The symbol version here is a SWAG based on libgcc-std.ver.
GCC_3.1 {

  global:

    ## Names inside the 'extern' block are human-readable (demangled) names;
    ## all but the last are terminated with a semicolon.
    extern "C++"
    {
      std::[A-Za-z]*;
      std::__throw_*;
      std::__basic_file*;
      #std::__convert_to_*;
      std::__num_base*;
      std::__timepunct*
    };


    ## Names not in an 'extern' block are mangled names.  In the non-C cases,
    ## their demangled names are written above them, in a comment.
    __cxa_*;
    __gxx_personality_v0;
    __dynamic_cast;

#    DW.ref.__gxx_personality_v0;   # I have no idea what this is.

######## these seem okay
    ## operator new(unsigned)
    _Znwj;

    ## operator delete(void*)
    _ZdlPv;

    ## operator new[](unsigned)
    _Znaj;

    ## operator delete[](void*)
    _ZdaPv;

######## hmmmmm
    ## std::_S_rb_tree_red
    _ZSt14_S_rb_tree_red;

    ## std::_S_rb_tree_black
    _ZSt16_S_rb_tree_black;

    ## std::__stl_threshold
    _ZSt15__stl_threshold;

    ## std::__stl_chunk_size
    _ZSt16__stl_chunk_size;

    ## VTT for std::basic_istringstream<char, std::char_traits<char>, std::allocator<char> >
    _ZTTSt19basic_istringstreamIcSt11char_traitsIcESaIcEE;

    ## VTT for std::basic_ostringstream<char, std::char_traits<char>, std::allocator<char> >
    _ZTTSt19basic_ostringstreamIcSt11char_traitsIcESaIcEE;

    # this is a function, "void std::__convert_to_v(stuff)", and as such
    # doesn't work well in the demangled section above
    _ZSt14__convert_to_v*;

    # likewise for "std::_Setfill<char> std::setfill<char>(char)"
    _ZSt7setfillIcESt8_SetfillIT_ES1_;

    # likewise for "bool std::has_facet<std::ctype<char> >(std::locale const&)"
    _ZSt9has_facetISt5ctypeIcEEbRKSt6locale;

    # likewise for "bool
    # std::has_facet<std::num_put<char, std::ostreambuf_iterator<char,
    #                       std::char_traits<char> > > >(std::locale const&)"
    _ZSt9has_facetISt7num_putIcSt19ostreambuf_iteratorIcSt11char_traitsIcEEEEbRKSt6locale;

    # ditto for istreambuf_iterator
    _ZSt9has_facetISt7num_getIcSt19istreambuf_iteratorIcSt11char_traitsIcEEEEbRKSt6locale;



#    ## vtable for std::logic_error
#    _ZTVSt11logic_error;
#
#    ## vtable for std::exception
#    _ZTVSt9exception;
#
#    ## vtable for __cxxabiv1::__class_type_info
#    _ZTVN10__cxxabiv117__class_type_infoE;
#
#    ## vtable for __cxxabiv1::__si_class_type_info
#    _ZTVN10__cxxabiv120__si_class_type_infoE;
#
#    ## vtable for __cxxabiv1::__vmi_class_type_info
#    _ZTVN10__cxxabiv121__vmi_class_type_infoE;
#
# gah, screw it, export /all/ the vtables in these namespaces, somebody
# smarter than me will have to figure this one out
_ZTVSt*;  _ZTVN10__cxxabiv1*;



  local:
    *;
};

