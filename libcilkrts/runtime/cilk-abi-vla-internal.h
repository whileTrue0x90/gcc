/* cilk-abi-vla-internal.h        -*-C++-*-
 *
 *************************************************************************
 *
 *  @copyright
 *  Copyright (C) 2013, Intel Corporation
 *  All rights reserved.
 *  
 *  @copyright
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in
 *      the documentation and/or other materials provided with the
 *      distribution.
 *    * Neither the name of Intel Corporation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *  
 *  @copyright
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 *  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 **************************************************************************/

/**
 * @file cilk-abi-vla-internal.h
 *
 * @brief Allocation/deallocation function for use with Variable Length
 * Arrays in spawning functions.
 *
 * These should be the only functions in the Cilk runtime allocating memory
 * from the standard C runtime heap.  This memory will be provided to user
 * code for use in VLAs, when the memory cannot be allocated from the stack.
 *
 * While these functions are simply passthroughs to malloc and free at the
 * moment, once we've got the basics of VLA allocations working we'll make
 * them do fancier tricks.
 */

/**
 * @brief Allocate memory from the heap for use by a Variable Length Array in
 * a spawning function.
 *
 * @param sf The __cilkrts_stack_frame for the spawning function containing
 * the VLA.
 * @param full_size The number of bytes to be allocated, including any tags
 * needed to identify this as allocated from the heap.
 * @param align Any alignment necessary for the allocation.
 */

void *vla_internal_heap_alloc(__cilkrts_stack_frame *sf,
                              size_t full_size,
                              uint32_t align);

/**
 * @brief Deallocate memory from the heap used by a Variable Length Array in
 * a spawning function.
 *
 * @param t The address of the memory block to be freed.
 * @param size The size of the memory block to be freed.
 */

void vla_internal_heap_free(void *t,
                            size_t size);

/**
 * @brief Deallocate memory from the original stack.  We'll do this by adding
 * full_size to ff->sync_sp.  So after the sync, the Variable Length Array
 * will no longer be allocated on the stack.
 *
 * @param sf The __cilkrts_stack_frame for the spawning function that is
 * deallocating a VLA.
 * @param full_size The size of the VLA, including any alignment and tags.
 */
void vla_free_from_original_stack(__cilkrts_stack_frame *sf,
                                  size_t full_size);
