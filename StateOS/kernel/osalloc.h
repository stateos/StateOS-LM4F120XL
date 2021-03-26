/******************************************************************************

    @file    StateOS: osalloc.h
    @author  Rajmund Szymanski
    @date    12.06.2020
    @brief   This file contains definitions for StateOS.

 ******************************************************************************

   Copyright (c) 2018-2020 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

 ******************************************************************************/

#ifndef __STATEOSALLOC_H
#define __STATEOSALLOC_H

#include "oskernel.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *
 * Name              : memory segment header
 *
 ******************************************************************************/

typedef struct __seg seg_t;

struct __seg
{
	seg_t  * next;  // next memory block
	seg_t  * owner; // owner of memory block (used as free / occupied flag)
};

/******************************************************************************
 *
 * Alias             : sys_malloc
 *
 * Description       : system malloc procedure, deprecated
 *
 * Parameters
 *   size            : required size of the memory segment (in bytes)
 *
 * Return            : pointer to the beginning of allocated memory segment
 *   NULL            : memory segment not allocated (not enough free memory)
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
void *sys_malloc( size_t size ) { return malloc(size); }

/******************************************************************************
 *
 * Alias             : sys_free
 *
 * Description       : system free procedure, deprecated
 *
 * Parameters
 *   ptr             : pointer to a memory segment previously allocated with sys_malloc, xxx_create or xxx_new functions
 *
 * Return            : none
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

__STATIC_INLINE
void sys_free( void *ptr ) { free(ptr); }

/******************************************************************************
 *
 * Name              : sys_heapSize
 *
 * Description       : get total size of free blocks of the dedicated heap memory
 *
 * Parameters        : none
 *
 * Return            : size of free heap memory
 *   0               : there is no dedicated heap memory or the heap is full
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

size_t sys_heapSize( void );

/******************************************************************************
 *
 * Name              : sys_segSize
 *
 * Description       : get memory segment size
 *
 * Parameters
 *   ptr             : pointer to a memory segment previously allocated with sys_malloc, xxx_create or xxx_new functions
 *
 * Return            : size of allocated memory segment
 *   0               : there is no dedicated heap memory or the heap is full
 *
 * Note              : use only in thread mode
 *
 ******************************************************************************/

size_t sys_segSize( void *ptr );

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

/* -------------------------------------------------------------------------- */

#endif//__STATEOSALLOC_H
