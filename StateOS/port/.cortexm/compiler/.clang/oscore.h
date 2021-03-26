/******************************************************************************

    @file    StateOS: oscore.h
    @author  Rajmund Szymanski
    @date    15.12.2020
    @brief   StateOS port file for ARM Cotrex-M uC.

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

#ifndef __STATEOSCORE_H
#define __STATEOSCORE_H

#include "osbase.h"

#ifdef __cplusplus
extern "C" {
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_HEAP_SIZE
#define OS_HEAP_SIZE          0 /* default system heap: all free memory       */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_STACK_SIZE
#define OS_STACK_SIZE       256 /* default task stack size in bytes           */
#endif

#ifndef OS_IDLE_STACK
#define OS_IDLE_STACK       128 /* idle task stack size in bytes              */
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_LOCK_LEVEL
#define OS_LOCK_LEVEL         0 /* critical section blocks all interrupts     */
#endif

#if     OS_LOCK_LEVEL >= (1<<__NVIC_PRIO_BITS)
#error  osconfig.h: Incorrect OS_LOCK_LEVEL value! Must be less then (1<<__NVIC_PRIO_BITS).
#endif

/* -------------------------------------------------------------------------- */

#ifndef OS_MAIN_PRIO
#define OS_MAIN_PRIO          0 /* priority of main process                   */
#endif

/* -------------------------------------------------------------------------- */

typedef uint32_t              lck_t;
typedef uint64_t              stk_t;

/* -------------------------------------------------------------------------- */

extern  stk_t               __initial_sp[];
#define MAIN_TOP            __initial_sp

/* -------------------------------------------------------------------------- */

#ifndef __MICROLIB
#define  _PORT_DATA()       char libspace[96]
#define  _PORT_DATA_INIT()  { 0 }
#endif

/* -------------------------------------------------------------------------- */

// hardware context
typedef struct __hwx hwx_t;

struct __hwx
{
	unsigned r0, r1, r2, r3;
	unsigned ip; // r12
	unsigned lr; // r14
	fun_t  * pc; // r15
	unsigned cc; // psr
};

#define _HWX_INIT( pc ) { 0, 0, 0, 0, 0, 0, pc, 0x01000000 }

// software context
typedef struct __swx swx_t;

struct __swx
{
	unsigned r4, r5, r6, r7, r8, r9, r10, r11;
	unsigned lr;  // EXC_RETURN
};

#define _SWX_INIT() { 0, 0, 0, 0, 0, 0, 0, 0, 0xFFFFFFFD }

// task context
typedef struct __ctx ctx_t;

struct __ctx
{
	swx_t    swx;
	hwx_t    hwx;
};

#define _CTX_INIT( pc ) { _SWX_INIT(), _HWX_INIT(pc) }

/* -------------------------------------------------------------------------- */
// init task context

__STATIC_INLINE
void port_ctx_init( ctx_t *ctx, fun_t *pc )
{
	ctx->swx.lr = 0xFFFFFFFD; // EXC_RETURN: return from psp
	ctx->hwx.pc = pc;
	ctx->hwx.cc = 0x01000000;
}

/* -------------------------------------------------------------------------- */
// is procedure inside ISR?

__STATIC_INLINE
bool port_isr_context( void )
{
	return (__get_IPSR() != 0U);
}

/* -------------------------------------------------------------------------- */
// are interrupts masked?

__STATIC_INLINE
bool port_isr_masked( void )
{
#if __CORTEX_M >= 3
	return (__get_PRIMASK() != 0U) || (__get_BASEPRI() != 0U);
#else
	return (__get_PRIMASK() != 0U);
#endif
}

/* -------------------------------------------------------------------------- */
// get current stack pointer

__STATIC_INLINE
void * port_get_sp( void )
{
	return (void *) __get_PSP();
}

/* -------------------------------------------------------------------------- */

#if OS_LOCK_LEVEL && (__CORTEX_M >= 3)

__STATIC_INLINE
lck_t port_get_lock( void )
{
	return __get_BASEPRI();
}

__STATIC_INLINE
void port_put_lock( lck_t lck )
{
	__set_BASEPRI(lck);
}

__STATIC_INLINE
void port_set_lock( void )
{
	__set_BASEPRI((OS_LOCK_LEVEL) << (8 - (__NVIC_PRIO_BITS)));
}

__STATIC_INLINE
void port_clr_lock( void )
{
	__set_BASEPRI(0);
}

#else

__STATIC_INLINE
lck_t port_get_lock( void )
{
	return __get_PRIMASK();
}

__STATIC_INLINE
void port_put_lock( lck_t lck )
{
	__set_PRIMASK(lck);
}

__STATIC_INLINE
void port_set_lock( void )
{
	__disable_irq();
}

__STATIC_INLINE
void port_clr_lock( void )
{
	__enable_irq();
}

#endif

/* -------------------------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif//__STATEOSCORE_H
