/*
    Copyright (c) 2014-2015 Intel Corporation.  All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of Intel Corporation nor the names of its
        contributors may be used to endorse or promote products derived
        from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef MYO_SERVICE_H_INCLUDED
#define MYO_SERVICE_H_INCLUDED

#include <myo.h>
#include <myoimpl.h>
#include <myotypes.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define SYMBOL_VERSION(SYMBOL,VERSION) SYMBOL ## VERSION

#define MYOERROR(...)			    \
{					    \
  fprintf (stderr, "MYO ERROR - TARGET: "); \
  fprintf (stderr, __VA_ARGS__);	    \
  fprintf (stderr, "\n");		    \
  perror (NULL);			    \
  return MYO_ERROR;			    \
}

#ifdef DEBUG
  #define MYOTRACE(...)			      \
  {					      \
    fprintf (stderr, "MYO TRACE - TARGET: "); \
    fprintf (stderr, __VA_ARGS__);	      \
    fprintf (stderr, "\n");		      \
  }
#else
  #define MYOTRACE(...) {}
#endif

#endif // MYO_SERVICE_H_INCLUDED
