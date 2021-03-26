/******************************************************************************

    @file    StateOS: ossemaphore.c
    @author  Rajmund Szymanski
    @date    26.12.2020
    @brief   This file provides set of functions for StateOS.

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

#include "inc/ossemaphore.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_sem_init( sem_t *sem, unsigned init, unsigned limit, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(sem, 0, sizeof(sem_t));

	core_obj_init(&sem->obj, res);

	sem->count = init < limit ? init : limit;
	sem->limit = limit;
}

/* -------------------------------------------------------------------------- */
void sem_init( sem_t *sem, unsigned init, unsigned limit )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(sem);
	assert(init<=limit);

	sys_lock();
	{
		priv_sem_init(sem, init, limit, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
sem_t *sem_create( unsigned init, unsigned limit )
/* -------------------------------------------------------------------------- */
{
	sem_t *sem;

	assert_tsk_context();

	sys_lock();
	{
		sem = malloc(sizeof(sem_t));
		if (sem)
			priv_sem_init(sem, init, limit, sem);
	}
	sys_unlock();

	return sem;
}

/* -------------------------------------------------------------------------- */
static
void priv_sem_reset( sem_t *sem, int event )
/* -------------------------------------------------------------------------- */
{
	sem->count = 0;

	core_all_wakeup(sem->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void sem_reset( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(sem);
	assert(sem->obj.res!=RELEASED);

	sys_lock();
	{
		priv_sem_reset(sem, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void sem_destroy( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(sem);
	assert(sem->obj.res!=RELEASED);

	sys_lock();
	{
		priv_sem_reset(sem, sem->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&sem->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
int priv_sem_take( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	if (sem->count == 0)
		return E_TIMEOUT;

	sem->count--;
	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int sem_take( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(sem);
	assert(sem->obj.res!=RELEASED);
	assert(sem->count<=sem->limit);

	sys_lock();
	{
		result = priv_sem_take(sem);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int sem_waitFor( sem_t *sem, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(sem);
	assert(sem->obj.res!=RELEASED);
	assert(sem->count<=sem->limit);

	sys_lock();
	{
		result = priv_sem_take(sem);
		if (result == E_TIMEOUT)
			result = core_tsk_waitFor(&sem->obj.queue, delay);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int sem_waitUntil( sem_t *sem, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(sem);
	assert(sem->obj.res!=RELEASED);
	assert(sem->count<=sem->limit);

	sys_lock();
	{
		result = priv_sem_take(sem);
		if (result == E_TIMEOUT)
			result = core_tsk_waitUntil(&sem->obj.queue, time);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_sem_give( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	if (core_one_wakeup(sem->obj.queue, E_SUCCESS) != NULL)
		return E_SUCCESS;

	if (sem->count >= sem->limit)
		return E_TIMEOUT;

	sem->count++;
	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
int sem_give( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(sem);
	assert(sem->obj.res!=RELEASED);
	assert(sem->count<=sem->limit);

	sys_lock();
	{
		result = priv_sem_give(sem);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
unsigned sem_getValue( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	unsigned val;

	assert(sem);
	assert(sem->obj.res!=RELEASED);

	sys_lock();
	{
		val = sem->count;
	}
	sys_unlock();

	return val;
}

/* -------------------------------------------------------------------------- */

#if OS_ATOMICS

/* -------------------------------------------------------------------------- */
static
int priv_sem_takeAsync( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	unsigned count = atomic_load((atomic_uint *)&sem->count);

	while (count > 0)
		if (atomic_compare_exchange_weak((atomic_uint *)&sem->count, &count, count - 1))
			return E_SUCCESS;

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int sem_takeAsync( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(sem);
	assert(sem->obj.res!=RELEASED);
	assert(sem->count<=sem->limit);

	sys_lock();
	{
		result = priv_sem_takeAsync(sem);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int sem_waitAsync( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();

	while (sem_takeAsync(sem) != E_SUCCESS)
		core_ctx_switch();

	return E_SUCCESS;
}

/* -------------------------------------------------------------------------- */
static
int priv_sem_giveAsync( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	unsigned count = atomic_load((atomic_uint *)&sem->count);

	while (count < sem->limit)
		if (atomic_compare_exchange_weak((atomic_uint *)&sem->count, &count, count + 1))
			return E_SUCCESS;

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int sem_giveAsync( sem_t *sem )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(sem);
	assert(sem->obj.res!=RELEASED);
	assert(sem->count<=sem->limit);

	sys_lock();
	{
		result = priv_sem_giveAsync(sem);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */

#endif//OS_ATOMICS
