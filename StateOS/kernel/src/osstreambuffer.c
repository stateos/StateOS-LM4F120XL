/******************************************************************************

    @file    StateOS: osstreambuffer.c
    @author  Rajmund Szymanski
    @date    02.07.2020
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

#include "inc/osstreambuffer.h"
#include "inc/ostask.h"
#include "inc/oscriticalsection.h"

/* -------------------------------------------------------------------------- */
static
void priv_stm_init( stm_t *stm, void *data, size_t bufsize, void *res )
/* -------------------------------------------------------------------------- */
{
	memset(stm, 0, sizeof(stm_t));

	core_obj_init(&stm->obj, res);

	stm->limit = bufsize;
	stm->data  = data;
}

/* -------------------------------------------------------------------------- */
void stm_init( stm_t *stm, void *data, size_t bufsize )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(stm);
	assert(data);
	assert(bufsize);

	sys_lock();
	{
		priv_stm_init(stm, data, bufsize, NULL);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
stm_t *stm_create( size_t limit )
/* -------------------------------------------------------------------------- */
{
	struct stm_T { stm_t stm; char buf[]; } *tmp;
	stm_t *stm = NULL;
	size_t bufsize;

	assert_tsk_context();
	assert(limit);

	sys_lock();
	{
		bufsize = limit;
		tmp = malloc(sizeof(struct stm_T) + bufsize);
		if (tmp)
			priv_stm_init(stm = &tmp->stm, tmp->buf, bufsize, tmp);
	}
	sys_unlock();

	return stm;
}

/* -------------------------------------------------------------------------- */
static
void priv_stm_reset( stm_t *stm, int event )
/* -------------------------------------------------------------------------- */
{
	stm->count = 0;
	stm->head  = 0;
	stm->tail  = 0;

	core_all_wakeup(stm->obj.queue, event);
}

/* -------------------------------------------------------------------------- */
void stm_reset( stm_t *stm )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(stm);
	assert(stm->obj.res!=RELEASED);

	sys_lock();
	{
		priv_stm_reset(stm, E_STOPPED);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
void stm_destroy( stm_t *stm )
/* -------------------------------------------------------------------------- */
{
	assert_tsk_context();
	assert(stm);
	assert(stm->obj.res!=RELEASED);

	sys_lock();
	{
		priv_stm_reset(stm, stm->obj.res ? E_DELETED : E_STOPPED);
		core_res_free(&stm->obj);
	}
	sys_unlock();
}

/* -------------------------------------------------------------------------- */
static
void priv_stm_get( stm_t *stm, char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t i = stm->head;

	stm->count -= size;
	while (size--)
	{
		*data++ = stm->data[i++];
		if (i == stm->limit) i = 0;
	}
	stm->head = i;
}

/* -------------------------------------------------------------------------- */
static
void priv_stm_put( stm_t *stm, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	size_t i = stm->tail;

	stm->count += size;
	while (size--)
	{
		stm->data[i++] = *data++;
		if (i == stm->limit) i = 0;
	}
	stm->tail = i;
}

/* -------------------------------------------------------------------------- */
static
void priv_stm_skip( stm_t *stm, size_t size )
/* -------------------------------------------------------------------------- */
{
	stm->count -= size;
	stm->head  += size;
	if (stm->head >= stm->limit) stm->head -= stm->limit;
}

/* -------------------------------------------------------------------------- */
static
size_t priv_stm_getUpdate( stm_t *stm, char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (size > stm->count) size = stm->count;
	priv_stm_get(stm, data, size);

	while (stm->obj.queue != 0 && stm->count + stm->obj.queue->tmp.stm.size <= stm->limit)
	{
		priv_stm_put(stm, stm->obj.queue->tmp.stm.data.out, stm->obj.queue->tmp.stm.size);
		core_one_wakeup(stm->obj.queue, E_SUCCESS);
	}

	return size;
}

/* -------------------------------------------------------------------------- */
static
void priv_stm_putUpdate( stm_t *stm, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	priv_stm_put(stm, data, size);

	while (stm->obj.queue != 0 && stm->count > 0)
	{
		size = stm->obj.queue->tmp.stm.size;
		if (size > stm->count) size = stm->count;
		stm->obj.queue->tmp.stm.size = size;
		priv_stm_get(stm, stm->obj.queue->tmp.stm.data.in, size);
		core_one_wakeup(stm->obj.queue, E_SUCCESS);
	}
}

/* -------------------------------------------------------------------------- */
static
void priv_stm_skipUpdate( stm_t *stm, size_t size )
/* -------------------------------------------------------------------------- */
{
	while (stm->obj.queue != 0)
	{
		if (stm->count + stm->obj.queue->tmp.stm.size > stm->limit)
			priv_stm_skip(stm, stm->count + stm->obj.queue->tmp.stm.size - stm->limit);
		priv_stm_put(stm, stm->obj.queue->tmp.stm.data.out, stm->obj.queue->tmp.stm.size);
		core_one_wakeup(stm->obj.queue, E_SUCCESS);
	}

	if (stm->count + size > stm->limit)
		priv_stm_skip(stm, stm->count + size - stm->limit);
}

/* -------------------------------------------------------------------------- */
static
int priv_stm_take( stm_t *stm, char *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	if (stm->count > 0)
	{
		size = priv_stm_getUpdate(stm, data, size);
		if (read != NULL)
			*read = size;
		return E_SUCCESS;
	}

	return E_TIMEOUT;
}

/* -------------------------------------------------------------------------- */
int stm_take( stm_t *stm, void *data, size_t size, size_t *read )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(stm);
	assert(stm->obj.res!=RELEASED);
	assert(stm->data);
	assert(stm->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_stm_take(stm, data, size, read);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int stm_waitFor( stm_t *stm, void *data, size_t size, size_t *read, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(stm);
	assert(stm->obj.res!=RELEASED);
	assert(stm->data);
	assert(stm->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_stm_take(stm, data, size, read);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.stm.data.in = data;
			System.cur->tmp.stm.size = size;
			result = core_tsk_waitFor(&stm->obj.queue, delay);
			if (result == E_SUCCESS && read != NULL)
				*read = System.cur->tmp.stm.size;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int stm_waitUntil( stm_t *stm, void *data, size_t size, size_t *read, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(stm);
	assert(stm->obj.res!=RELEASED);
	assert(stm->data);
	assert(stm->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_stm_take(stm, data, size, read);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.stm.data.in = data;
			System.cur->tmp.stm.size = size;
			result = core_tsk_waitUntil(&stm->obj.queue, time);
			if (result == E_SUCCESS && read != NULL)
				*read = System.cur->tmp.stm.size;
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_stm_give( stm_t *stm, const char *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (stm->count + size <= stm->limit)
	{
		priv_stm_putUpdate(stm, data, size);
		return E_SUCCESS;
	}

	if (size <= stm->limit)
		return E_TIMEOUT;

	return E_FAILURE;
}

/* -------------------------------------------------------------------------- */
int stm_give( stm_t *stm, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(stm);
	assert(stm->obj.res!=RELEASED);
	assert(stm->data);
	assert(stm->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_stm_give(stm, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int stm_sendFor( stm_t *stm, const void *data, size_t size, cnt_t delay )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(stm);
	assert(stm->obj.res!=RELEASED);
	assert(stm->data);
	assert(stm->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_stm_give(stm, data, size);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.stm.data.out = data;
			System.cur->tmp.stm.size = size;
			result = core_tsk_waitFor(&stm->obj.queue, delay);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
int stm_sendUntil( stm_t *stm, const void *data, size_t size, cnt_t time )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert_tsk_context();
	assert(stm);
	assert(stm->obj.res!=RELEASED);
	assert(stm->data);
	assert(stm->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_stm_give(stm, data, size);
		if (result == E_TIMEOUT)
		{
			System.cur->tmp.stm.data.out = data;
			System.cur->tmp.stm.size = size;
			result = core_tsk_waitUntil(&stm->obj.queue, time);
		}
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
static
int priv_stm_push( stm_t *stm, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	if (size <= stm->limit)
	{
		priv_stm_skipUpdate(stm, size);
		priv_stm_putUpdate(stm, data, size);

		return E_SUCCESS;
	}

	return E_FAILURE;
}

/* -------------------------------------------------------------------------- */
int stm_push( stm_t *stm, const void *data, size_t size )
/* -------------------------------------------------------------------------- */
{
	int result;

	assert(stm);
	assert(stm->obj.res!=RELEASED);
	assert(stm->data);
	assert(stm->limit);
	assert(data);
	assert(size);

	sys_lock();
	{
		result = priv_stm_push(stm, data, size);
	}
	sys_unlock();

	return result;
}

/* -------------------------------------------------------------------------- */
size_t stm_count( stm_t *stm )
/* -------------------------------------------------------------------------- */
{
	size_t count;

	assert(stm);
	assert(stm->obj.res!=RELEASED);

	sys_lock();
	{
		count = stm->count;
	}
	sys_unlock();

	return count;
}

/* -------------------------------------------------------------------------- */
size_t stm_space( stm_t *stm )
/* -------------------------------------------------------------------------- */
{
	size_t space;

	assert(stm);
	assert(stm->obj.res!=RELEASED);

	sys_lock();
	{
		space = stm->limit - stm->count;
	}
	sys_unlock();

	return space;
}

/* -------------------------------------------------------------------------- */
size_t stm_limit( stm_t *stm )
/* -------------------------------------------------------------------------- */
{
	size_t limit;

	assert(stm);
	assert(stm->obj.res!=RELEASED);

	sys_lock();
	{
		limit = stm->limit;
	}
	sys_unlock();

	return limit;
}

/* -------------------------------------------------------------------------- */
